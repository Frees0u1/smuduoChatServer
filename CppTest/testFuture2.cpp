#include <iostream>
#include <future>
#include <thread>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <utility>

using namespace std;

uint32_t add(uint32_t a, uint32_t b) {
    cout << "thread id = " << std::this_thread::get_id() << ", a = " << a 
         << ", b = " << b << endl;
    if(::numeric_limits<uint32_t>::max() - a < b) {
        throw std::overflow_error("overflow_error"); //抛出异常
    }
    return a + b;
}

int main () {
    //1. 使用packaged_task包装add函数
    std::packaged_task<uint32_t (uint32_t, uint32_t)> addTask(add);
    //2. 取得furture对象用以获取异步操作的结果
    std::future<uint32_t> f1 = addTask.get_future();
    //3. 将task作为线程执行体
    std::thread(std::move(addTask), 1ul, 2ul).detach();
    //4. 通过future对象获取异步操作的结果
    uint32_t sum1 = f1.get();
    cout << "thread id = " << std::this_thread::get_id()
         << ", sum1 = " << sum1 << endl;

    //测试异常
    std::packaged_task<uint32_t (uint32_t, uint32_t)> addTask2(add);
    std::future<uint32_t> f2 = addTask2.get_future();
    std::thread(std::move(addTask2), 4000000000ul, 4000000000ul).detach();

    try {
        uint32_t sum2 = f2.get();
        cout << "thread id = " << std::this_thread::get_id()
         << ", sum2 = " << sum2 << endl;    
    }
    catch(const std::overflow_error& e) {
        cout << "thread id = " << std::this_thread::get_id()
             << ", e.what() = " << e.what() << endl;
    }

    return 0; 
}