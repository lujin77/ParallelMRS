/* 
 * File:   Mediator.cpp
 * Author: syani
 * 
 * Created on 2013年1月13日, 上午10:19
 */

#include "Mediator.h"
#include "Timer.h"

Mediator::Mediator() : _reduce_num(0) {

}

Mediator::~Mediator() {
    //free reducer status
    for (int i = 0; i < _reduce_num; ++i) {
        pthread_mutex_destroy(&(_reducer_infos[i].mutex));
    }

    if (NULL != _reducer_infos) {
        delete [] _reducer_infos;
    }

    //free containers
    for (int i = 0; i < _reduce_num; ++i) {
        reduce_hash_t::iterator it;
        for (it = _hash[i].begin(); it != _hash[i].end(); ++it) {
            std::vector<char*> * arr = it->second;
            for (int j = 0; j < arr->size(); ++j) {
                if (arr->at(j) != NULL)
                    delete [] arr->at(j);
            }
            if (arr != NULL) 
                delete arr;
        }
        _hash[i].clear();
    }
    delete [] _hash;

    for (int i = 0; i < _reduce_num; ++i) {
        for (int j = 0; j < _key_list[i].size(); ++j) {
            if (_key_list[i][j].str != NULL)
                delete [] _key_list[i][j].str;
        }
    }

    if (NULL != _key_list) {
        delete [] _key_list;
    }

}

int Mediator::init(int reduce_num) {
    _reduce_num = reduce_num;
    int ret = 0;
    //初始化字符指针数组
    _reducer_infos = new (std::nothrow) reducer_status_t[_reduce_num];
    if (NULL == _reducer_infos) {
        fprintf(stderr, "[ERROR] Mediator::init | create reducer_infos failed, size=%d\n", _reduce_num);
        return -1;
    }

    for (int i = 0; i < _reduce_num; ++i) {
        _reducer_infos[i].task_num = 0;
        _reducer_infos[i].key_pos = 0;
        _reducer_infos[i].value_pos = 0;
        ret = pthread_mutex_init(&(_reducer_infos[i].mutex), NULL);
        if (0 != ret) {
            return -1;
        }
    }

    _hash = new (std::nothrow) reduce_hash_t[_reduce_num];
    if (NULL == _hash) {
        fprintf(stderr, "[ERROR] Mediator::init | create reducer_hashs failed, size=%d\n", _reduce_num);
        return -1;
    }
    
    //init dense_hash_map
//    for (int i = 0; i < _reduce_num; ++ i) {
//        _hash[i].set_empty_key(-1);
//    }

    _key_list = new (std::nothrow) key_list_t[_reduce_num];
    if (NULL == _key_list) {
        fprintf(stderr, "[ERROR] Mediator::init | create key_list failed, size=%d\n", _reduce_num);
        return -1;
    }

    return 0;
}

int Mediator::push_back_r(const char * str) {
    int slot;
    char key[MAX_STR_LEN];
    char value[MAX_STR_LEN];
    //hash to shuffle
    split_key_value(str, key, value);
    slot = BKDRHash(key) % _reduce_num;
    //lock pipe's mutex for thread safe
    pthread_mutex_lock(&(_reducer_infos[slot].mutex));
    //check whether the key is already exist
    unsigned long long sign = MurmurHash64B(key, strlen(key));
    //std::cout << "key: " << key << ", sign: " << sign << std::endl;
    reduce_hash_t::iterator it = _hash[slot].find(sign);
    if (it == _hash[slot].end()) {
        //cout << "not find" << endl;
        std::vector<char*> * value_list = new std::vector<char*>();
        char * tmp = new char[strlen(value)];
        strcpy(tmp, value);
        value_list->push_back(tmp);
        _hash[slot].insert(std::pair<unsigned long long, std::vector<char*>* >(sign, value_list));
        //inser key list
        key_node_t node;
        node.p = value_list;
        tmp = new char[strlen(key)];
        strcpy(tmp, key);
        node.str = tmp;
        _key_list[slot].push_back(node);
        node = _key_list[slot].at(_key_list[slot].size() - 1);
        //std::cout << "[first] " << node.str << ", " << node.p->size() << std::endl;
    } else {
        //cout << "find" << endl;
        std::vector<char*> * value_list = it->second;
        char * tmp = new char[strlen(value)];
        strcpy(tmp, value);
        //std::cout << "here";
        value_list->push_back(tmp);
        //std::cout << "[exist] " << key << ", " << value_list.size() << std::endl;
        //        for (int i = 0; i < value_list.size(); ++i) {
        //            std::cout << value_list[i] << std::endl;
        //        }

    }
    ++_reducer_infos[slot].task_num;
    pthread_mutex_unlock(&(_reducer_infos[slot].mutex));
    return 0;
}

/**sort function for shuffle*/
bool Mediator::cmp(key_node_t node1, key_node_t node2) {
    return strcmp(node1.str, node2.str) > 0;
}

int Mediator::sort_r(int index) {
    if (index < 0 || index > (_key_list->size() - 1)) {
        fprintf(stderr, "[WARNING] Mediator::sort_r | index out of boundary!\n");
        return -1;
    }
//    for (int i = 0; i < _key_list[index].size(); ++i) {
//        std::cout << _key_list[index][i].str << ", " << _key_list[index][i].p->size() << std::endl;
//    }
    //pthread_mutex_lock(&(_reducer_infos[index].mutex));
    std::sort(_key_list[index].begin(), _key_list[index].end(), Mediator::cmp);
    //pthread_mutex_unlock(&(_reducer_infos[index].mutex));
//    for (int i = 0; i < _key_list[index].size(); ++i) {
//        std::cout << "["<<index<<"] " <<  _key_list[index][i].str << ", " << _key_list[index][i].p->size() << std::endl;
//    }
    return 0;
}

int Mediator::get_reduce_task_r(int index, char * str) {
    //return 1;
    //compose the result string to reduce
    int key_pos = _reducer_infos[index].key_pos;
    int value_pos = _reducer_infos[index].value_pos;
    while (1) {
        //std::cout << key_pos << " -> " << value_pos << std::endl;
        if (key_pos > (_key_list[index].size() - 1)) {
            return 1;
        }
        key_node_t key_node = _key_list[index][key_pos];
        //std::cout << key_node.p-> << std::endl;
        //std::cout << "[" << index << "] " << key_node.str << " | " << key_node.p->size() << std::endl;
        if (value_pos > (key_node.p->size() - 1)) {
            value_pos = 0;
            ++ key_pos;
            //std::cout << "add key pos" << std::endl;
        } else {
            //std::cout << "here" << std::endl;
            snprintf(str, MAX_STR_LEN, "%s%c%s", key_node.str, SPLITTER, key_node.p->at(value_pos));
            ++ value_pos;
            break;
        }
    }
    
    //update process progress tag 
    _reducer_infos[index].key_pos = key_pos;
    _reducer_infos[index].value_pos = value_pos;
    return 0;
}

