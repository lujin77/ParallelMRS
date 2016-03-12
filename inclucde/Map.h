/* 
 * File:   Map.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午12:52
 */

#ifndef __MAP_H__
#define	__MAP_H__

#include "Global.h"
#include "PipePool.h"
#include "ThreadBase.h"

class Map : public ThreadBase {
public:
    Map(char * map_bin, int index);
    virtual ~Map();
protected:
    void Run();
private:
    char _map_bin[256];
    //pipe index
    int _index;
};

#endif	/* __M AP_H__ */

