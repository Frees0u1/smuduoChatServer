#include <atomic>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
const int niters = 100000;
const int nthreads = 10;

//测试：cnt的++操作中有三步，即从内存中读取/++更新/存回内存，如果没有按照正确的顺序操作，最后得到的结果则不准确
// 利用c++11中的atomic，可以将变量属性改为原子属性，对其的操作不可打断


 int64_t cnt;
// atomic<int64_t> cnt;

void threadFunc() {
    int i = 0;
    for(int i = 0; i < niters; i++){
        cnt++;
    }
}

int main() {
   
   vector<thread > thrVec;

   for(int i = 0; i < nthreads; ++i) {
       thrVec.push_back(thread(threadFunc));
   }

   for(int i = 0; i < thrVec.size(); ++i){
       thrVec[i].join();
   }
 
//    for(int i = 0; i < nthreads; i++) {
//        thread thr(threadFunc);
//    }
    cout << "cnt = " << cnt << ", and should be " << nthreads * niters << endl;

    return 0;
}