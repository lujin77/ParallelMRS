/* 
 * File:   ThreadBase.h
 * Author: yndx
 *
 * Created on 2012年12月20日, 下午12:54
 */

#ifndef __THREADBASE_H__
#define __THREADBASE_H__

#define THREAD_CALL
typedef void* (*ThreadCall)(void* aArg);

#include <pthread.h> //pthread API

class ThreadBase {
public:
    ThreadBase(bool aDetached = false);
    virtual ~ThreadBase();

    bool Start();

    pthread_t Id();
    int ErrorCode();

    void Join();
    bool IsJoinable();

protected:
    virtual void Run() {};

    pthread_t iId;
    int iErrorCode;
    bool iDetached; //用来标识线程是否分离

private:
    static void* THREAD_CALL ThreadFunc(void* aArg);
};

#endif /* __THREADBASE_H__ */

