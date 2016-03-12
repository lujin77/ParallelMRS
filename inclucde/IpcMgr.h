/* 
 * File:   IpcMgr.h
 * Author: syani
 *
 * Created on 2012年12月31日, 下午6:42
 */

#ifndef IPCMGR_H
#define	IPCMGR_H

#include "Global.h"

//0 for read, 1 for write

typedef struct {
    int parent2child[2];
    int child2parent[2];
    bool is_closed_p2c_r;
    bool is_closed_p2c_w;
    bool is_closed_c2p_r;
    bool is_closed_c2p_w;
} dual_pipe_t;

typedef enum {
    PROC_MAP,
    PROC_REDUCE
} proc_type_t;

typedef enum {
    PARENT2CHILD,
    CHILD2PARENT
} pipe_direct_t;

typedef enum {
    PIPE_READ,
    PIPE_WRITE
} pipe_type_t;

typedef std::vector<dual_pipe_t*> ipc4map_t;
typedef std::vector<dual_pipe_t*> ipc4reduce_t;

class IpcMgr {
public:
    static IpcMgr * getInst();
    virtual ~IpcMgr();
    //初始化
    int init(int map_num, int reduce_num);
    //线程安全，关闭父进程中无效的管道: p2c[0], c2p[1]
    int close_useless_in_parent_r(int index, proc_type_t type);
    //close parent pipe
    int close_pipe_in_parent_r(int index, proc_type_t proc_type, pipe_type_t pipe_type);
    //替换子进程stdio，关闭不需要的pipe
    int redirect_child_io(int index, proc_type_t type);
    //get pipe
    int get_pipe(int index, proc_type_t proc_type, pipe_direct_t direct_type, pipe_type_t pipe_type);
protected:
    IpcMgr();
    int close_useless_in_child(int index, proc_type_t type);
    static IpcMgr * _instance;
    pthread_mutex_t _mutex; ///<互斥信号量
    int _map_num;
    int _reduce_num;

    //map使用的管道
    ipc4map_t _ipc4map;
    //reduce使用的管道
    ipc4reduce_t _ipc4reduce;
};

#endif	/* IPCMGR_H */

