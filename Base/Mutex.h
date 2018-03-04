#ifndef SMUDUO_BASE_MUTEX_H
#define SMUDUO_BASE_MUTEX_H

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/syscall.h>
#include "Common.h"
#define gettid() syscall(SYS_gettid)

class MutexLock : smuduo::noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock() {
        assert(holder_ == 0); 
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread(){
        return (holder_ == gettid());
    }
    
    void asserLocked(){
        assert(isLockedByThisThread());
    }

    void lock(){ //仅供MutexLockGuard调用,严禁用户代码调用
        pthread_mutex_lock(&mutex_); //这两行顺序不能反
        holder_ = gettid();
    }

    void unlock(){ //仅供MutexLockGuard调用,严禁用户代码调用
        holder_ = 0; //这两行顺序不能反
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};


class MutexLockGuard : smuduo::noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
        mutex_.lock(); //创建Guard加锁
    }
    ~MutexLockGuard(){
        mutex_.unlock(); //析构时解锁
    }
private:
    MutexLock& mutex_;
};
#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")


#endif