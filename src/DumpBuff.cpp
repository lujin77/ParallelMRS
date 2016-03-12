/* 
 * File:   DumpBuff.cpp
 * Author: yndx
 * 
 * Created on 2012年12月20日, 下午3:09
 */

#include "DumpBuff.h"

DumpBuff::DumpBuff(int maxnum, int No, const char * save_path, int line_size) :
_buff(NULL), _length(0), _maxnum(maxnum), _line_size(line_size) {
    snprintf(_save_path, MAX_STR_LEN, "%s/part-%d", save_path, No);
}

DumpBuff::~DumpBuff() {
    for (int i = 0; i < _maxnum; ++i) {
        if (NULL != _buff[i]) {
            delete [] _buff[i];
        }
    }

    if (NULL != _buff) {
        delete [] _buff;
    }
}

//初始化

int DumpBuff::init() {
    int ret = 0;
    //初始化字符指针数组
    _buff = new (std::nothrow) char*[_maxnum];
    if (NULL == _buff) {
        fprintf(stderr, "[ERROR] DumpBuff::init() | create buff failed, size=%d\n", _maxnum);
        return -1;
    }
    //初始化每一个字符串单元
    for (int i = 0; i < _maxnum; ++i) {
        _buff[i] = new (std::nothrow) char[_line_size];
        if (NULL == _buff[i]) {
            fprintf(stderr, "[ERROR] DumpBuff::init() | "
                    "create string buff failed, size=%d, pos=%d\n", _line_size, i);
            return -1;
        }
    }
    
    //clear files in output path
    
    
    return 0;
}

//添加string到buff，如果buff满，则会自动dump到文件

int DumpBuff::append(const char * str) {
    if (_length < _maxnum) {
        strcpy(_buff[_length], str);
        ++_length;
    } else {
        fprintf(stderr, "[WARNING] DumpBuff::append() | buff is full\n");
        return -1;
    }

    //buff已满，则导出到文件
    if (_length == _maxnum) {
        dump();
    }
    
    return 0;
}

//导出当前buff的所有数据到文件，清空buff

int DumpBuff::dump() {
    if (_length == 0) {
        fprintf(stderr, "[WARNING] DumpBuff::dump() | buff is null, do not need dump\n");
        return 1;
    } else {
        std::fstream fout;
        fout.open(_save_path, std::ios::out | std::ios::app);
        //fprintf(stderr, "[DEBUG] DumpBuff::dump() | dump to %s\n", _save_path);
        if (NULL == fout) {
            fprintf(stderr, "[ERROR] DumpBuff::dump() | open file failed!, path=%s\n", _save_path);
            return -1;
        }

        //dump buffed string to file 
        int tail = _length;
        for (int i = (tail - 1); i >= 0; --i) {
            fout << _buff[i];
            --_length;
            //fprintf(stderr, "[TRACE] DumpBuff::dump() | dump one line, str=%s", _buff[i]);
        }
        fout.close();
        return 0;
    }
}

