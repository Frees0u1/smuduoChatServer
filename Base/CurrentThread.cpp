#include "CurrentThread.h"

namespace CurrentThread{
    __thread int t_cachedTid = 0;

    pid_t gettid() {
        return static_cast<pid_t>(syscall(SYS_gettid));
    }

    void  cacheTid(){
        if(t_cachedTid == 0){
            t_cachedTid = gettid();
        }
    }


}