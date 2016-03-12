/* 
 * File:   ThreadBase.cpp
 * Author: yndx
 * 
 * Created on 2012年12月20日, 下午12:54
 */

#include "ThreadBase.h"

#include <errno.h>
#include <string.h>                //strerror

#include "ThreadBase.h"

ThreadBase::ThreadBase(bool aDetached)
                : iErrorCode(0), iDetached(aDetached) {

}

ThreadBase::~ThreadBase() {
    
}

void* THREAD_CALL ThreadBase::ThreadFunc(void* aArg) {
    ThreadBase* self = static_cast<ThreadBase*> (aArg);

    self->Run();

    //pthread_exit( (void*)0 );
    return NULL;
    /*        return (void*)0;*/
}

bool ThreadBase::Start() {
    if (iDetached) {
        pthread_attr_t attr;

        iErrorCode = pthread_attr_init(&attr);

        if (0 != iErrorCode)
            return false;

        iErrorCode = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (0 != iErrorCode) {
            pthread_attr_destroy(&attr);
            return false;
        }

        iErrorCode = pthread_create(&iId, &attr, ThreadFunc, this);

        pthread_attr_destroy(&attr);
    }
    else
        iErrorCode = pthread_create(&iId, NULL, ThreadFunc, this);

    return ( (iErrorCode == 0) ? true : false);
}

pthread_t ThreadBase::Id() {
    return iId;
}

int ThreadBase::ErrorCode() {
    return iErrorCode;
}

bool ThreadBase::IsJoinable() {
    return !iDetached;
}

void ThreadBase::Join() {
    if (!iDetached)
        pthread_join(iId, NULL);
}

