/* 
 * File:   JobTracker.h
 * Author: syani
 *
 * Created on 2012年12月21日, 下午1:56
 */

#ifndef __JOBTRACKER_H__
#define	__JOBTRACKER_H__

#include "Global.h"
#include "Map.h"
#include "Reduce.h"

typedef std::vector<Map*> maps_t;
typedef std::vector<Reduce*> reduces_t;

class JobTracker {
public:
//    JobTracker(char * file, char * map_bin, char * reduce_bin,
//            int map_num = MAP_NUM, int reduce_num = REDUCE_NUM);
    JobTracker(const char * file, const char * output, const char * map_bin
                , const char * reduce_bin, int map_num, int reduce_num);
    virtual ~JobTracker();
    int init();
    int run();
    int dispatch_mapper(const char * str, long round);
    int dispatch_reducer(const char * str, int slot);
private:
    maps_t _maps;
    reduces_t _reduces;

    int _map_num;
    int _reduce_num;
    char _map_bin[MAX_STR_LEN];
    char _reduce_bin[MAX_STR_LEN];
    char _input_file[MAX_STR_LEN];
    char _output_path[MAX_STR_LEN];
};

#endif	/* __JOBTRACKER_H__ */

