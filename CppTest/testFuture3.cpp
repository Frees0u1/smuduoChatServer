#include <iostream>
#include <future>
#include <thread>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <utility>
#include <exception>

using namespace std;

uint32_t add(uint32_t a, uint32_t b) {
    cout << "thread id = " << std::this_thread::get_id() << ", a = " << a 
         << ", b = " << b << endl;
    if(::numeric_limits<uint32_t>::max() - a < b) {
        throw std::overflow_error("overflow_error"); //抛出异常
    }
    return a + b;
}

//add函数的包装函数
void add_wrapper(std::promise<uint32_t> prom, uint32_t a, uint32_t b) {
    try {
        //设置值为供std::future对象获取
        prom.set_value(add(a, b));
    }
    catch(...) {
        //设置异常在std::future获取值时抛出
        prom.set_exception(std::current_exception());
    }
}

int main() {
    //创建std::promise对象
    std::promise<uint32_t> prom1;
    //获取关联的std::future对象
    std::future<uint32_t> f1 = prom1.get_future();
    //启动线程执行add函数的包装函数
    std::thread(add_wrapper, std::move(prom1), 1ul, 2ul).detach();
    //等待并获取异步操作的结果
    uint32_t sum1 = f1.get();
    cout << "thread id = " << std::this_thread::get_id()
         << ", sum1 = " << sum1 << endl;

    //测试异常
    std::promise<uint32_t> prom2;
    std::future<uint32_t> f2 = prom2.get_future();
    //4000000000ul + 4000000000ul 将导致异常
    std::thread(add_wrapper, std::move(prom2), 4000000000ul, 4000000000ul).detach();

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