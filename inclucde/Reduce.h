/* 
 * File:   Reduce.h
 * Author: syani
 *
 * Created on 2012年12月21日, 下午2:00
 */

#ifndef REDUCE_H
#define	REDUCE_H

#include "Global.h"
#include "ThreadBase.h"

class Reduce : public ThreadBase {
public:
    Reduce(char * reduce_bin, int index);
    virtual ~Reduce();
protected:
    void Run();
private:
    char _reduce_bin[256];
    int _index;
};

#endif	/* REDUCE_H */

