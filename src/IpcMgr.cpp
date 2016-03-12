/* 
 * File:   IpcMgr.cpp
 * Author: syani
 * 
 * Created on 2012年12月31日, 下午6:42
 */

#include "IpcMgr.h"

IpcMgr * IpcMgr::_instance = NULL;

IpcMgr::IpcMgr() {
}

IpcMgr::~IpcMgr() {
    int i;
    for (i = 0; i < _ipc4map.size(); ++i) {
        if (_ipc4map[i] != NULL) {
            delete _ipc4map[i];
        }
    }

    for (i = 0; i < _ipc4reduce.size(); ++i) {
        if (_ipc4reduce[i] != NULL) {
            delete _ipc4reduce[i];
        }
    }

    pthread_mutex_destroy(&_mutex);
}

IpcMgr * IpcMgr::getInst() {
    if (_instance == NULL) {
        _instance = new IpcMgr();
    }
    return _instance;
}

int IpcMgr::init(int map_num, int reduce_num) {
    _map_num = map_num;
    _reduce_num = reduce_num;
    int i;
    for (i = 0; i < _map_num; ++i) {
        dual_pipe_t * dpipe = new (std::nothrow) dual_pipe_t();
        if (dpipe == NULL) {
            fprintf(stderr, "[WARNING] IpcMgr::init() | can't create map's ipc!\n");
            return -1;
        }
        pipe(dpipe->child2parent);
        pipe(dpipe->parent2child);
        dpipe->is_closed_c2p_r = false;
        dpipe->is_closed_c2p_w = false;
        dpipe->is_closed_p2c_r = false;
        dpipe->is_closed_p2c_w = false;
        _ipc4map.push_back(dpipe);

//        fprintf(stderr, "[DEBUG] IpcMgr::init() | map[%d]->child2parent, read:%d write:%d\n"
//                , i, dpipe->child2parent[0], dpipe->child2parent[1]);
//        fprintf(stderr, "[DEBUG] IpcMgr::init() | map[%d]->parent2child, read:%d write:%d\n"
//                , i, dpipe->parent2child[0], dpipe->parent2child[1]);
    }

    for (i = 0; i < _reduce_num; ++i) {
        dual_pipe_t * dpipe = new (std::nothrow) dual_pipe_t();
        if (dpipe == NULL) {
            fprintf(stderr, "[WARNING] IpcMgr::init() | can't create reduce's ipc!\n");
            return -1;
        }
        pipe(dpipe->child2parent);
        pipe(dpipe->parent2child);
        dpipe->is_closed_c2p_r = false;
        dpipe->is_closed_c2p_w = false;
        dpipe->is_closed_p2c_r = false;
        dpipe->is_closed_p2c_w = false;
        _ipc4reduce.push_back(dpipe);
    }

    int ret = pthread_mutex_init(&_mutex, NULL);
    if (0 != ret) {
        return -1;
    }

    return 0;
}

int IpcMgr::close_useless_in_parent_r(int index, proc_type_t type) {
    if (type == PROC_MAP || type == PROC_REDUCE) {
        pthread_mutex_lock(&_mutex);

        dual_pipe_t * dpipe;
        if (type == PROC_MAP) {
            dpipe = _ipc4map[index];
        } else if (type == PROC_REDUCE) {
            dpipe = _ipc4reduce[index];
        }

        if (dpipe->is_closed_c2p_w == false) {
            close(dpipe->child2parent[1]); // 关闭读管道的写端
            dpipe->is_closed_c2p_w = true;
//            fprintf(stderr, "[DEBUG] IpcMgr::close_useless_in_parent_r() | map[%d] "
//                    "close child2parent_write->%d\n", index, dpipe->child2parent[1]);
        }
        if (dpipe->is_closed_p2c_r == false) {
            close(dpipe->parent2child[0]); // 关闭写管道的读端
            dpipe->is_closed_p2c_r = true;
//            fprintf(stderr, "[DEBUG] IpcMgr::close_useless_in_parent_r() | map[%d] "
//                    "close parent2child_read->%d\n", index, dpipe->parent2child[0]);
        }

        pthread_mutex_unlock(&_mutex);

//        fprintf(stderr, "[TRACE] IpcMgr::close_useless_in_parent | "
//                "parent proc:%d closed its useless c2p_w: %d and p2c_r: %d pipe\n"
//                , getpid(), dpipe->child2parent[1], dpipe->parent2child[0]);
        return 0;
    } else {
        fprintf(stderr, "[TRACE] IpcMgr::close_useless_in_parent | invalid proc_type_t\n", getpid());
        return -1;
    }
}

int IpcMgr::close_pipe_in_parent_r(int index, proc_type_t proc_type, pipe_type_t pipe_type) {
    if (proc_type == PROC_MAP || proc_type == PROC_REDUCE) {
        pthread_mutex_lock(&_mutex);

        dual_pipe_t * dpipe;
        if (proc_type == PROC_MAP) {
            dpipe = _ipc4map[index];
        } else if (proc_type == PROC_REDUCE) {
            dpipe = _ipc4reduce[index];
        }

        //close read pipe -> c2p[0]
        if (pipe_type == PIPE_READ && !dpipe->is_closed_c2p_r) {
            close(dpipe->child2parent[0]); // 关闭写管道的读端
            dpipe->is_closed_c2p_r = true;
//            fprintf(stderr, "[DEBUG] IpcMgr::close_pipe_in_parent_r() | map[%d] "
//                    "close child2parent_read->%d\n", index, dpipe->child2parent[0]);
        }

        //close write pipe -> p2c[1]
        if (pipe_type == PIPE_WRITE && !dpipe->is_closed_p2c_w) {
            close(dpipe->parent2child[1]); // 关闭写管道的读端
            dpipe->is_closed_p2c_w = true;
//            fprintf(stderr, "[DEBUG] IpcMgr::close_pipe_in_parent_r() | map[%d] "
//                    "close parent2child_write->%d\n", index, dpipe->parent2child[1]);
        }

        pthread_mutex_unlock(&_mutex);

        return 0;
    } else {
        fprintf(stderr, "[TRACE] IpcMgr::close_pipe_in_parent_r | invalid proc_type_t\n", getpid());
        return -1;
    }
}

int IpcMgr::redirect_child_io(int index, proc_type_t type) {
    if (type == PROC_MAP || type == PROC_REDUCE) {
        dual_pipe_t * dpipe;
        if (type == PROC_MAP) {
            dpipe = _ipc4map[index];
        } else if (type == PROC_REDUCE) {
            dpipe = _ipc4reduce[index];
        }
        //切换本子进程到管道，关闭不需要到管道
        close(dpipe->child2parent[0]); // 关闭父进程的读管道的子进程读端
        close(dpipe->parent2child[1]); // 关闭父进程的写管道的子进程写端
        dup2(dpipe->child2parent[1], STDOUT_FILENO); // 复制父进程的读管道到子进程的标准输出
        dup2(dpipe->parent2child[0], STDIN_FILENO); // 复制父进程的写管道到子进程的标准输入
        close(dpipe->child2parent[1]); // 关闭已复制的读管道
        close(dpipe->parent2child[0]); // 关闭已复制的写管道

//        fprintf(stderr, "[DEBUG] IpcMgr::redirect_child_io() | reduce[%d] "
//                "close c2p_r->%d, "
//                "close p2c_w->%d, "
//                "close c2p_w->%d, "
//                "close p2c_r->%d\n"
//                , index
//                , dpipe->child2parent[0]
//                , dpipe->parent2child[1]
//                , dpipe->child2parent[1]
//                , dpipe->parent2child[0]);

        //关闭其他进程冗余管道
        //如果不关闭或导致其他子进程block(stdin结束不掉)
        close_useless_in_child(index, type);

        return 0;
    } else {
        fprintf(stderr, "[TRACE] IpcMgr::redirect_child_io | invalid proc_type_t\n", getpid());
        return -1;
    }
}

int IpcMgr::close_useless_in_child(int index, proc_type_t type) {
    int i;
    for (i = 0; i < _map_num; ++i) {
        if (i == index and type == PROC_MAP) {
            //fprintf(stderr, "[DEBUG] IpcMgr::close_useless_in_child() | skip index = %d\n", index);
            continue;
        }
        
        dual_pipe_t * dpipe = _ipc4map[i];
        close(dpipe->child2parent[0]);
        close(dpipe->child2parent[1]);
        close(dpipe->parent2child[0]);
        close(dpipe->parent2child[1]);
//        fprintf(stderr, "[DEBUG] IpcMgr::close_useless_in_child() | map[%d] "
//                "close c2p_r->%d, "
//                "close c2p_w->%d, "
//                "close p2c_r->%d, "
//                "close p2c_w->%d\n"
//                , index
//                , dpipe->child2parent[0]
//                , dpipe->child2parent[1]
//                , dpipe->parent2child[0]
//                , dpipe->parent2child[1]);
    }

    for (i = 0; i < _reduce_num; ++i) {
        if (i == index and type == PROC_REDUCE) {
            continue;
        }

        dual_pipe_t * dpipe = _ipc4reduce[i];
        close(dpipe->child2parent[0]);
        close(dpipe->child2parent[1]);
        close(dpipe->parent2child[0]);
        close(dpipe->parent2child[1]);
//        fprintf(stderr, "[DEBUG] IpcMgr::close_useless_in_child() | reduce[%d] "
//                "close c2p_r->%d, "
//                "close c2p_w->%d, "
//                "close p2c_r->%d, "
//                "close p2c_w->%d\n"
//                , index
//                , dpipe->child2parent[0]
//                , dpipe->child2parent[1]
//                , dpipe->parent2child[0]
//                , dpipe->parent2child[1]);
    }
    return 0;
}

int IpcMgr::get_pipe(int index, proc_type_t proc_type, pipe_direct_t direct_type, pipe_type_t pipe_type) {
    if (proc_type == PROC_MAP || proc_type == PROC_REDUCE) {
        dual_pipe_t * dpipe;
        if (proc_type == PROC_MAP) {
            dpipe = _ipc4map[index];
        } else if (proc_type == PROC_REDUCE) {
            dpipe = _ipc4reduce[index];
        }

        if (direct_type == PARENT2CHILD) {
            return dpipe->parent2child[pipe_type];
        } else if (direct_type == CHILD2PARENT) {
            return dpipe->child2parent[pipe_type];
        } else {
            return -1;
        }
    } else {
        fprintf(stderr, "[TRACE] IpcMgr::get_pipe | invalid proc_type_t\n", getpid());
        return -1;
    }
}
