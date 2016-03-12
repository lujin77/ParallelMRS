/* 
 * File:   PipePool.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午2:50
 */

#ifndef PIPEPOOL_H
#define	PIPEPOOL_H

#include "Global.h"
#include "SafePipe.h"
#include "DumpBuff.h"
#include "Mediator.h"

typedef std::vector<SafePipe*> safe_pipes_t;
typedef std::vector<DumpBuff*> dump_buffs_t;

class PipePool {
public:
    static PipePool * getInst();
    int init(int map_num, int reduce_num, char * save_path);
    virtual ~PipePool();
    //初始化
    int init();
    //get data from shuffled pipe
    int get_reduce_task_r(int index, char * str);
    
    //map使用的管道
    safe_pipes_t _pipes4map;
    //reduce使用的管道
    //safe_pipes_t _pipes4reduce;
    Mediator * _mediator;
    //最后一步dump使用的管道
    dump_buffs_t _pipes4dump;
    
protected:
    PipePool();
    static PipePool * _instance;
};

#endif	/* PIPEPOOL_H */

