/* 
 * File:   JobTracker.cpp
 * Author: syani
 * 
 * Created on 2012年12月21日, 下午1:56
 */

#include "JobTracker.h"
#include "PipePool.h"
#include "IpcMgr.h"
#include "Timer.h"

JobTracker::JobTracker(const char * file, const char * output, const char * map_bin
        , const char * reduce_bin, int map_num, int reduce_num) :
_map_num(map_num), _reduce_num(reduce_num) {
    strcpy(_map_bin, map_bin);
    strcpy(_reduce_bin, reduce_bin);
    strcpy(_input_file, file);
    strcpy(_output_path, output);
}

JobTracker::~JobTracker() {
    for (int i = 0; i < _map_num; ++i) {
        if (_maps[i] != NULL) {
            delete _maps[i];
        }
    }

    for (int i = 0; i < _reduce_num; ++i) {
        if (_reduces[i] != NULL) {
            delete _reduces[i];
        }
    }

}

int JobTracker::init() {
    fprintf(stderr, "[DEBUG] JobTracker::init() | begin to init ParallelMRS...\n");
    int ret;
    ret = PipePool::getInst()->init(_map_num, _reduce_num, _output_path);
    if (ret != 0) {
        fprintf(stderr, "[ERROR] JobTracker::init() | init PipePool failed!\n");
        return -1;
    }

    ret = IpcMgr::getInst()->init(_map_num, _reduce_num);
    if (ret != 0) {
        fprintf(stderr, "[ERROR] JobTracker::init() | init IpcMgr failed!\n");
        return -1;
    }

    for (int i = 0; i < _map_num; ++i) {
        Map * map = new (std::nothrow) Map(_map_bin, i);
        if (map == NULL) {
            fprintf(stderr, "[ERROR] JobTracker::init() | create map failed!\n");
            return -1;
        }
        _maps.push_back(map);
    }

    for (int i = 0; i < _reduce_num; ++i) {
        Reduce * reduce = new (std::nothrow) Reduce(_reduce_bin, i);
        if (reduce == NULL) {
            fprintf(stderr, "[ERROR] JobTracker::init() | create reduce failed!\n");
            return -1;
        }
        _reduces.push_back(reduce);
    }

    fprintf(stderr, "[DEBUG] JobTracker::init() | init complete!\n");
    return 0;
}

int JobTracker::run() {
    fprintf(stderr, "[DEBUG] JobTracker::run() | starting MapReduce task...\n");
    //main thread read file and dispatch
    std::fstream fin;
    fin.open(_input_file, std::ios::in);
    if (fin == NULL) {
        fprintf(stderr, "[ERROR] JobTracker::run() | open file failed, path=%s\n", _input_file);
        return -1;
    }

    Timer timer;
    timer.start_timer("map");

    //start mapper
    for (int i = 0; i < _map_num; ++i) {
        if (!_maps[i]->Start()) {
            fprintf(stderr, "[ERROR] JobTracker::run() | mapper start failed!\n");
            return -1;
        }
    }


    //get file size to demostrate processing state
    fin.seekg(0, std::ios::end);
    long file_size = fin.tellg();
    fin.seekg(0);
    int size_of_char = sizeof (char);
    long progress_threshold = file_size / PROGRESS_PER;
    long progress_per_round = 0;
    long cur_size = 0;
    //some tmp variables
    char str[MAX_STR_LEN];
    int len = 0;
    long round = 0;
    bool tag = false;
    do {
        str[0] = '\0';
        fin.getline(str, MAX_STR_LEN);
        len = strlen(str);
        if (len == 0)
            continue; //delete last blank line
        add_newline(str, len);
        //printf("%s", str);
//        if (round >= ERROR_LINE) {
//            std::cout << "1" << std::endl;
//        }
        dispatch_mapper(str, round);
//        if (round >= ERROR_LINE) {
//            std::cout << "2" << std::endl;
//        }
        ++round;
        //show processing state
        cur_size += len * size_of_char;
        progress_per_round += len * size_of_char;
        if (progress_per_round >= progress_threshold) {
            fprintf(stderr, "[DEBUG] JobTracker::run() | input progress: %ld %\n", cur_size * 100 / file_size);
            progress_per_round = 0;
        }
        
        

//        if (round > ERROR_LINE) {
//            std::cout << round << std::endl;
//            lock_tag = true;
//        }
    } while (!fin.eof());

    //fprintf(stderr, "start to send EOF\n");
    //send EOF for mapper pipe
    strcpy(str, END_PIPE);
    for (int i = 0; i < _map_num; ++i) {
        dispatch_mapper(str, i);
    }
    //fprintf(stderr, "dispatch_mapper complete\n");

    //wait all mapper
    for (int i = 0; i < _map_num; ++i) {
        _maps[i]->Join();
    }

    timer.end_timer();

    timer.start_timer("reduce");
    //start reducer
    for (int i = 0; i < _reduce_num; ++i) {
        if (!_reduces[i]->Start()) {
            fprintf(stderr, "[ERROR] JobTracker::run() | reducer start failed!\n");
            return -1;
        }
    }

    //wait all mapper
    for (int i = 0; i < _reduce_num; ++i) {
        _reduces[i]->Join();
    }
    timer.end_timer();

    //send EOF for reducer pipe
    //    strcpy(str, END_PIPE);
    //    for (int i = 0; i < _reduce_num; ++i) {
    //        dispatch_reducer(str, i);
    //    }
    //
    //    //wait all mapper
    //    for (int i = 0; i < _reduce_num; ++i) {
    //        _reduces[i]->Join();
    //    }

    fprintf(stderr, "[NOTICE] JobTracker::run() | all task complete!\n");
    return 0;
}

int JobTracker::dispatch_mapper(const char * str, long round) {
    char * item;
    int index;
    int slot = round % _map_num;
    //dispatch to mapper
//    if (lock_tag == true) {
//        std::cout << "slot: " << slot << std::endl;
//        PipePool::getInst()->_pipes4map[slot]->show_state();
//        std::cout << str << "\n" << std::endl;
//    }
    while (PipePool::getInst()->_pipes4map[slot]->get_free_item(item, index) != 0) {
//        fprintf(stderr, "[NOTICE] JobTracker::run() | mapper[%d] pipe is full, "
//                "wait 1 sec to dispatch!\n", slot);
        usleep(PIPE_SLEEP_TIME);
    }
    strcpy(item, str);
    PipePool::getInst()->_pipes4map[slot]->push_item(index);
}

int JobTracker::dispatch_reducer(const char * str, int slot) {
    /*
    char * item;
    int index;
    //dispatch to mapper
    while (PipePool::getInst()->_pipes4reduce[slot]->get_free_item(item, index) != 0) {
        fprintf(stderr, "[NOTICE] JobTracker::run() | reduce[%d] pipe is full, "
                "wait 1 sec to dispatch!\n", slot);
        sleep(PIPE_SLEEP_TIME);
    }
    strcpy(item, str);
    PipePool::getInst()->_pipes4reduce[slot]->push_item(index);
     * */
}

