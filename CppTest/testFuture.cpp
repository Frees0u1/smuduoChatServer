#include <iostream>
#include <future>
#include <thread>
#include <cstdint>
#include <stdexcept>
#include <limits>

using namespace std;

uint32_t add(uint32_t a, uint32_t b) {
    cout << "thread id = " << std::this_thread::get_id() << ", a = " << a 
         << ", b = " << b << endl;
    if(::numeric_limits<uint32_t>::max() - a < b) {
        throw std::overflow_error("overflow_error"); //抛出异常
    }
    return a + b;
}

int main() {
    // 使用std::async启动异步操作并返回std::future对象
    std::future<uint32_t>  f1 = std::async(std::launch::async, add, 1ul, 2ul);

    //通过std::future::get()等待异步操作结果完成，并取得返回值
    std::uint32_t sum1 = f1.get();
    cout << "thread id = " << std::this_thread::get_id() 
         << ", sum1 = " << sum1 << endl;

    // 4000000000ul + 4000000000ul会抛出异常，异常会延迟到std::future::get或std::future::wait时抛出
    std::future<uint32_t>  f2 = std::async(std::launch::async, add, 4000000000ul, 4000000000ul);

    try {
        uint32_t sum2 = f2.get();
        cout << "thread id = " << std::this_thread::get_id() 
         << ", sum1 = " << sum1 << endl;
    }
    catch(const std::overflow_error& e) {
        cout << "thread id = " << std::this_thread::get_id()
             << ", e.what() = " << e.what() << endl;
    } 

    return 0;
}