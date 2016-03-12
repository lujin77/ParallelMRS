/* 
 * File:   CommUtil.h
 * Author: syani
 *
 * Created on 2013年1月13日, 上午9:38
 */

#ifndef TIMER_H
#define	TIMER_H

#include "Global.h"

typedef enum {
    SHOW_AS_SEC,
    SHOW_AS_MS,
    SHOW_AS_US
} time_type_t;

class Timer {
public:
    Timer();
    virtual ~Timer();
    /**time utility*/
    void start_timer(const char * timer_name);
    void end_timer(time_type_t type = SHOW_AS_SEC);
protected:
    struct timeval tvstart, tvend;
    char time_alter[256];       //计时器名称
};

#endif	/* TIMER_H */

