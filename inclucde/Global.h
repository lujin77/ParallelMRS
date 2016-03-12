/* 
 * File:   Global.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午2:49
 */

#ifndef __GLOBAL_H__
#define	__GLOBAL_H__

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <google/sparse_hash_map>
#include <google/dense_hash_map>


//#define MAP_NUM 1
//#define REDUCE_NUM 1
//#define SAVE_PATH "./output"

#define MAX_STR_LEN 1024
#define PIPE_SIZE 1000
#define STEEL_THRESHOLD 1000
#define STEEL_TRY_SKIP_BASE (STEEL_THRESHOLD/20)
#define STEEL_TRY_TIME 6
#define END_PIPE "%[EOF]%"
#define PROGRESS_PER 10
#define PIPE_SLEEP_TIME 1000
#define SPLITTER '\t'
#define KEY_NUM 1

#define ERROR_LINE 689760
static bool lock_tag = false;


#ifdef	__cplusplus
extern "C" {
#endif

    inline void add_newline(char * str, const int str_len) {
        str[str_len + 2] = '\0';
        str[str_len] = '\r';
        str[str_len + 1] = '\n';
    }

    inline size_t BKDRHash(const char * str) {
        register size_t hash = 0;
        while (size_t ch = (size_t) * str++) {
            hash = hash * 131 + ch; // 也可以乘以31、131、1313、13131、131313..       
        }
        return hash;
    }

    /* the famous DJB Hash Function for strings */
    inline unsigned int DJBHash(char *str) {
        unsigned int hash = 5381;
        while (*str) {
            hash = ((hash << 5) + hash) + (*str++); /* times 33 */
        }
        hash &= ~(1 << 31); /* strip the highest bit */
        return hash;
    }

    inline unsigned long long MurmurHash64B(const void * key, int len, unsigned int seed = 0xEE6B27EB) {
        const unsigned int m = 0x5bd1e995;
        const int r = 24;

        unsigned int h1 = seed ^ len;
        unsigned int h2 = 0;

        const unsigned int * data = (const unsigned int *) key;

        while (len >= 8) {
            unsigned int k1 = *data++;
            k1 *= m;
            k1 ^= k1 >> r;
            k1 *= m;
            h1 *= m;
            h1 ^= k1;
            len -= 4;

            unsigned int k2 = *data++;
            k2 *= m;
            k2 ^= k2 >> r;
            k2 *= m;
            h2 *= m;
            h2 ^= k2;
            len -= 4;
        }

        if (len >= 4) {
            unsigned int k1 = *data++;
            k1 *= m;
            k1 ^= k1 >> r;
            k1 *= m;
            h1 *= m;
            h1 ^= k1;
            len -= 4;
        }

        switch (len) {
            case 3: h2 ^= ((unsigned char*) data)[2] << 16;
            case 2: h2 ^= ((unsigned char*) data)[1] << 8;
            case 1: h2 ^= ((unsigned char*) data)[0];
                h2 *= m;
        };

        h1 ^= h2 >> 18;
        h1 *= m;
        h2 ^= h1 >> 22;
        h2 *= m;
        h1 ^= h2 >> 17;
        h1 *= m;
        h2 ^= h1 >> 19;
        h2 *= m;

        unsigned long long h = h1;

        h = (h << 32) | h2;

        return h;
    }

    inline void get_key(const char * str, char * key,
            char splitter = SPLITTER, int key_num = KEY_NUM) {
        char * start = (char *) str;
        char * end = (char *) str;
        while (key_num > 0) {
            end = strchr(end, splitter);
            if (end == NULL) {
                fprintf(stderr, "[ERROR] get_key() | invalid splitter! code = %d\n", splitter);
                return;
            }
            --key_num;
            ++end;
        }

        int i = 0;
        for (int k = ((end - 1) - start); k > 0; --k) {
            key[i] = *start;
            ++start;
            ++i;
        }

        key[i] = '\0';
    }

    inline void split_key_value(const char * str, char * key, char * value,
            char splitter = SPLITTER, int key_num = KEY_NUM) {
        char * start = (char *) str;
        char * end = (char *) str;
        while (key_num > 0) {
            end = strchr(end, splitter);
            if (end == NULL) {
                fprintf(stderr, "[ERROR] get_key() | invalid splitter! code = %d\n", splitter);
                return;
            }
            --key_num;
            ++end;
        }
        
        //copy key
        int i = 0;
        for (int k = ((end - 1) - start); k > 0; --k) {
            key[i] = *start;
            ++start;
            ++i;
        }
        key[i] = '\0';
        
        //copy end
        strncpy(value, end, MAX_STR_LEN);
    }

#ifdef	__cplusplus
}
#endif

#endif	/* __GLOBAL_H__ */

