/* 
 * File:   DumpBuff.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午3:09
 */

#ifndef __DUMPBUFF_H__
#define	__DUMPBUFF_H__

#include "Global.h"

class DumpBuff {
public:
    //No必须是唯一值
    DumpBuff(int maxnum, int No, const char * save_path, int line_size = MAX_STR_LEN);
    //初始化
    int init();
    //添加string到buff，如果buff满，则会自动dump到文件
    int append(const char * str);
    //导出当前buff的所有数据到文件，清空buff
    int dump();
    virtual ~DumpBuff();
protected:

    char ** _buff; ///<数据队列
    int _length;
    int _maxnum; ///<最大个数

    //每一个数据行的长度，默认1024
    int _line_size;
    char _save_path[MAX_STR_LEN];
};

#endif	/* __DUMPBUFF_H__ */

