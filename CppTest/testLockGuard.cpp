#include <set>
#include <random>
#include <mutex>
#include <iostream>
#include <thread>

using namespace std;

set<int> g_intSet;
std::mutex g_mtx;


//Every thread add 10 random int to intSet
void threadFunc() {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);
        for(int i = 0; i < 10; i++) {
            {
            std::lock_guard<std::mutex> locker(g_mtx);
            g_intSet.insert(dis(gen));
            }
        }
    } catch(...) {}
}


int main () {
    std::thread thr1(threadFunc), thr2(threadFunc);

    thr1.join();
    thr2.join();

    cout << "[" << g_intSet.size() << "]: ";
     for(auto x : g_intSet) {
        cout << x << " ";
    }
    cout << endl;



    return 0;
}