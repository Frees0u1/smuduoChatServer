#ifndef SMUDUO_BASE_CURRENTTHREAD_H
#define SMUDUO_BASE_CURRENTTHREAD_H
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
//#include <Linux/unistd.h>
#include <stdint.h>
namespace CurrentThread {
    //internal
    extern __thread int t_cachedTid;
    void cacheTid();

    inline int tid() {
        if(__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }

}


#endif