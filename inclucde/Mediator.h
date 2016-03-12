/* 
 * File:   Mediator.h
 * Author: syani
 * 
 * 供shuffle阶段使用pipe，带负载均衡
 * 数据管道和reducer状态描述采用分离到形式，便于steel实现
 * 
 * Created on 2013年1月13日, 上午10:19
 */

#ifndef MEDIATOR_H
#define	MEDIATOR_H

#include "Global.h"

typedef struct key_node {
    char * str;
    std::vector<char*> * p;
} key_node_t;

//typedef std::map<unsigned long long, std::vector<char*>* > reduce_hash_t;
typedef google::sparse_hash_map<unsigned long long, std::vector<char*>* > reduce_hash_t;
typedef std::vector<key_node_t> key_list_t;

/**reduce task的描述信息*/
typedef struct reducer_status {
    pthread_mutex_t mutex;
    uint task_num;
    int key_pos;
    int value_pos;
} reducer_status_t;

class Mediator {
public:
    Mediator();
    virtual ~Mediator();
    int init(int reduce_num);
    //shuffle的数据分配，根据hash值分配到对应reducer的pipe
    int push_back_r(const char * str);
    //sort reduce pipe by given index. thread safe
    int sort_r(int index);
    //get data from shuffled pipe
    int get_reduce_task_r(int index, char * str);
protected:
    int _reduce_num;
    reducer_status_t * _reducer_infos;
    reduce_hash_t * _hash;
    key_list_t * _key_list;
private:
    static bool cmp(key_node_t node1, key_node_t node2);
};

#endif	/* MEDIATOR_H */

