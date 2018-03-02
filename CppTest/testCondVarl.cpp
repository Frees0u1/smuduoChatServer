#include <condition_variable>
#include <iostream>
#include <thread>
#include <queue>
#include <random>
#include <chrono> //时间戳

#include <fstream>
#include <vector>
#include <atomic>

#include <assert.h>
#include <unistd.h>
using namespace std;
//using namespace std::chrono_literals;
/*
考虑下面的场景需求编写程序：现有几个工作线程(worker thread)和一个记录线程(logger thread)。
worker会在工作的过程中随机产生错误码(errcode)，实时显示在屏幕上, 要求logger能够同时异步地将这些errcode记录下来

*/

std::mutex g_mutexForOStream; //为IO stream创建的互斥锁，防止显示混乱
std::mutex g_mutexForErrQue;
std::queue<int> errQue; //错误代码，worker线程产生的日志都将放在此
std::condition_variable cv_forErrQue;

bool everyThingIsDone = false;

void workerFunc(int id, std::mt19937 &generator) {
    { //print a start msg
        std::unique_lock<std::mutex> locker(g_mutexForOStream);
        cout << "[Worker " << id << "]\t"<< "start working!" << endl;
    }
    
    //模拟工作，每个线程休眠一个随机的时间 1~5秒不等
    std::this_thread::sleep_for(std::chrono::seconds(1 + generator() % 5));

    //模拟产生错误
    int errCode = id*100 + 1;
    {
        std::unique_lock<std::mutex> locker(g_mutexForOStream);
        cout << "[worker " << id << "]\tan error occurred: " << errCode << endl;
    }

    //notify logger
    {
        std::unique_lock<std::mutex> locker(g_mutexForErrQue);
        errQue.push(errCode);
        cv_forErrQue.notify_one();
    }

    {
        std::unique_lock<std::mutex> locker(g_mutexForOStream);
        cout << "[worker " << id << "]\tQuit!" << endl;
    }
}

void loggerFunc(ostream &LOG) {
    { //print a start msg
        std::unique_lock<std::mutex> locker(g_mutexForOStream);
        cout << "[logger]\tstart working!" << endl;
    }
    
    auto pred  = [&](){
    //    std::unique_lock<std::mutex> lck(g_mutexForErrQue);
    //    cout << "pred got mutex of errQue" << endl;
        return everyThingIsDone || !errQue.empty();
    };

   
    //直到所有的worker线程都工作完毕才退出循环
    while( !everyThingIsDone ) {
        std::unique_lock<std::mutex> locker(g_mutexForErrQue);
        //cv_forErrQue(locker, []{return !errQue.empty() || everyThingIsDone})
        cv_forErrQue.wait(locker, [](){return !errQue.empty() || everyThingIsDone; });
        assert(locker.owns_lock());
        while(!errQue.empty()) {
            std::unique_lock<std::mutex> locker(g_mutexForOStream);
            cout << "[logger]\tprocessing error " << errQue.front() << endl;
            LOG << errQue.front() << endl;
            errQue.pop();
        }
    }

    { //print a end msg
        std::unique_lock<std::mutex> locker(g_mutexForOStream);
        cout << "Everything is done, Logger quit!" << endl;
    }
}

int main (){
    everyThingIsDone = false;
    vector<std::thread> workers;
    ofstream LOG_ERR("error.log", ios::out);

    // initialize a random generator
    std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::thread logger(loggerFunc, std::ref(LOG_ERR));

    sleep(1); //保证logger先开始工作

    for(int i = 0; i < 5; i++){
        workers.push_back( std::thread(workerFunc, i+1, std::ref(generator) ));
    }

    //等到所有worker都完成工作，logger才可以结束工作

    for(int i = 0; i < workers.size(); i++) {
        workers[i].join();
    }
    everyThingIsDone = true;
    cv_forErrQue.notify_one();  //通知logger所有的workers已经退出
    
    logger.join();

    return 0;
}