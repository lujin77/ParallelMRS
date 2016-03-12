/* 
 * File:   PipePool.cpp
 * Author: yndx
 * 
 * Created on 2012年12月20日, 下午2:50
 */

#include "PipePool.h"
#include "Mediator.h"
#include "Timer.h"

PipePool * PipePool::_instance = NULL;

PipePool::PipePool() {

}

PipePool::~PipePool() {
    int i;
    for (i = 0; i < _pipes4map.size(); ++i) {
        if (_pipes4map[i] != NULL) {
            delete _pipes4map[i];
        }
    }

    delete _mediator;
    /*
    for (i = 0; i < _pipes4reduce.size(); ++i) {
        if (_pipes4reduce[i] != NULL) {
            delete _pipes4reduce[i];
        }
    }
     */

    for (i = 0; i < _pipes4dump.size(); ++i) {
        if (_pipes4dump[i] != NULL) {
            delete _pipes4dump[i];
        }
    }

    fprintf(stderr, "[TRACE] ~PipePool() | delete PipePool instance complete!\n");
}

PipePool * PipePool::getInst() {
    if (_instance == NULL) {
        _instance = new PipePool();
    }
    return _instance;
}

//初始化

int PipePool::init(int map_num, int reduce_num, char * save_path) {
    int i, ret = 0;
    for (i = 0; i < map_num; ++i) {
        SafePipe * pipe = new (std::nothrow) SafePipe(PIPE_SIZE);
        if (pipe == NULL) {
            fprintf(stderr, "[WARNING] PipePool::init() | can't create map pipes!\n");
            return -1;
        }
        if (pipe->init() != 0) {
            fprintf(stderr, "[WARNING] PipePool::init() | map pipe[%d] init failed!\n", i);
            delete pipe;
            return -1;
        }
        _pipes4map.push_back(pipe);
    }

    _mediator = new (std::nothrow) Mediator();
    ret = _mediator->init(reduce_num);
    if (ret != 0) {
        fprintf(stderr, "[ERROR] PipePool::init() | init mediator failed!\n");
        return -1;
    }

    for (i = 0; i < reduce_num; ++i) {
        /*
        SafePipe * pipe = new (std::nothrow) SafePipe(PIPE_SIZE);
        if (pipe == NULL) {
            fprintf(stderr, "[WARNING] PipePool::init() | can't create reduce pipes!\n");
            return -1;
        }
        if (pipe->init() != 0) {
            fprintf(stderr, "[WARNING] PipePool::init() | reduce pipe[%d] init failed!\n", i);
            delete pipe;
            return -1;
        }
        _pipes4reduce.push_back(pipe);
         */

        DumpBuff * buff = new (std::nothrow) DumpBuff(PIPE_SIZE, i, save_path);
        if (buff == NULL) {
            fprintf(stderr, "[WARNING] PipePool::init() | can't create reduce dumpbuff!\n");
            return -1;
        }
        if (buff->init() != 0) {
            fprintf(stderr, "[WARNING] PipePool::init() | dumpbuff[%d] init failed!\n", i);
            delete buff;
            return -1;
        }
        _pipes4dump.push_back(buff);
    }
    return 0;
}

int PipePool::get_reduce_task_r(int index, char * str) {
    int ret = _mediator->get_reduce_task_r(index, str);
    if (ret == 1) { // all task complete, try to steel from others
        return 1;
        // retry for get task
    } else {
        return 0;
    }
}

