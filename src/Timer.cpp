/* 
 * File:   Timer.cpp
 * Author: syani
 * 
 * Created on 2013年1月13日, 上午9:38
 */

#include "Timer.h"

Timer::Timer() {
}

Timer::~Timer() {
}

void Timer::start_timer(const char * timer_name) {
    time_alter[0] = '\0';
    strcpy(time_alter, timer_name);
    gettimeofday(&tvstart, NULL);
}

void Timer::end_timer(time_type_t type) {
    if (strlen(time_alter) == 0) {
        fprintf(stderr, "timer name not defined, or timer does not start!\n");
        return;
    }
    
    gettimeofday(&tvend, NULL);
    switch (type) {
        case SHOW_AS_SEC:
            fprintf(stderr, "[TIMER] [%s] run time: %fsec\n", time_alter, 
                    (float)(tvend.tv_sec - tvstart.tv_sec) + (tvend.tv_usec - tvstart.tv_usec)/1000000.0);
            break;
        case SHOW_AS_MS:
            fprintf(stderr, "[TIMER] [%s] time:%ldms\n", time_alter, 
                    (tvend.tv_sec - tvstart.tv_sec)*1000 + (tvend.tv_usec - tvstart.tv_usec)/1000);
            break;
        case SHOW_AS_US:
            fprintf(stderr, "[TIMER] [%s] time:%ldus\n", time_alter, 
                    (tvend.tv_sec - tvstart.tv_sec)*1000000 + (tvend.tv_usec - tvstart.tv_usec));
            break;
        default:
            fprintf(stderr, "[WARNING] error time type for timer util\n");
            break;
    }

}

