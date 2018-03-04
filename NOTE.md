# 相关技术学习笔记

## 优于select的epoll
 
**基于select的I/O复用技术速度慢的原因**
* 调用select函数后常见的针对所有文件描述符的循环语句
* 每次调用select函数时都需要向该函数传递监视对象的信息

**实现epoll必要的函数和结构体**

三个epoll服务器端实现需要的函数
* epoll_create: 创建保存epoll文件描述符的空间
* epoll_ctl: 向空间注册并注销文件描述符
* epoll_wait: 与select函数类似,等待文件描述符发生变化

### epoll结构体

```cpp
struct epoll_event {
    __uint32_t event;
    epoll_data_t data;
}

typedef union epoll_data { //注意这里是联合union而不是结构struct
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t

```
声明足够大的epoll\_event结构体数组后,传递给epoll\_wait函数时,发生变化的文件描述符信息将被填入该数组.因此,无需像select函数那样针对所有文件描述符进行循环.

### epoll_create  空间生成
```cpp
#include <sys/epoll.h>

int epoll_create(int size); 
    // 成功返回epoll文件描述符,失败返回-1
```
调用epoll_create函数时创建的**文件描述符保存空间**称为"epoll例程". Linux 2.6.8以前的版本通过size传递的值决定epoll例程的大小,但该值仅仅是向操作系统提一个建议.Linux 2.6.8之后的版本操作系统将完全忽略size参数

epoll_create创建的资源与套接字相同,均为由操作系统所管理的资源.因此,该函数和创建套接字的情况相同,也会返回文件描述符,用以区分不同的epoll例程. 需要终止时, 与其他文件描述符相同,也要调用close函数.

### epoll_ctl 注册事件
生成epoll例程后,应在内部注册监视对象文件描述符. 使用epoll_ctl函数

```cpp
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event); 
    //成功时返回0,失败返回-1

/* 参数
*  epfd     epoll例程的文件描述符
*  op       用于指明注册操作,包括添加/删除/更改等
*  fd       需要注册事件的监视对象文件描述符
*  event    监视对象的事件类型
*/
```
调用例子
```cpp
epoll_ctl(A, EPOLL_CTL_ADD, B, C);
epoll_ctl(A,EPOLL_CTL_DEL,B,NULL);
```
1意为: **向epoll例程A中添加对象B是否会发生C事件的监视内容**\
2意为: **在epoll例程A中注销监视B**

可向epoll_ctl第二个参数传递的常量及其含义
* EPOLL\_CTL\_ADD: 将文件描述符注册到epoll例程
* EPOLL\_CTL\_DEL: 从epoll例程中删除文件描述符
* EPOLL\_CTL\_MOD: 更改注册的文件描述符的关注事件发生情况

**epoll_ctl的第4个参数: struct epoll\_event***\

调用例子
```cpp
struct epoll_event event; 
// ...
event.events = EPOLLIN; //发生需要读取数据的情况(事件)时
event.data.fd = sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
```
上述代码将sockfd注册到epoll例程epfd中,并在需要读取数据的情况下产生相应事件.接下来给出epoll_event的成员events中可以保存的常量及所指类型.
* EPOLLIN: 需要读取数据的情况
* EPOLLOUT: 输出缓冲为空,可以立即发送数据的情况
* EPOLLPRI: 收到OOB数据的情况
* EPOLLRDHUP: 断开连接或半关闭的情况,这在边缘触发的情况下非常有用
* EPOLLERR: 发生错误的情况
* EPOLLET: 以边缘触发的方式得到事件通知
* EPOLLONESHOT: 发生一次事件后,相应文件描述符不再收到事件通知.因此需要向epoll_ctl函数的第二个参数传递EPOLL\_CTL\_MOD,再次设置事件.

### epoll_wait 等待事件发生
```cpp
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    //成功返回发生事件的文件描述符数,失败时返回-1
/*参数
* epfd      表示事件发生监视范围的epoll例程的文件描述符
* events    保存发生事件的文件描述符集合的结构体地址值
* maxevents 第二个参数中可以保存的最大事件数
* timeout   以ms为单位的等待时间,传递-1时,一直等待直到发生事件
*/
```
调用例子:
```cpp
int event_cnt;
struct epoll_event *ep_events;
\\...
ep_events = malloc(sizeof(struct epoll_event)* EPOLL_SIZE); //EPOLL_SIZE是宏常量
\\...
event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
\\...
```


## 非阻塞IO-直接使用网络库

非阻塞IO比阻塞IO在编程实现上难度更高,尽量不要自己实现一个非阻塞IO,这是网络库的责任

* Code is much more complicated, short read and short write


# C++11: 线程 / 锁 / 条件变量

# 线程
```std::thread```类，位于```<thread>```头文件，实现了线程操作。```std::thread  ```可以和普通函数和lambda表达式搭配使用。它还允许向线程执行函数传递任意多参数。

```cpp
#include <thread>

void func() {
    // do some work here
}

int main() {
    std::thread thr(func);
    t.join();
    return 0;
} 
```
上面就是一个最简单的使用```std::thread```的例子，函数```func()```在新起的线程中执行。调用```join()```函数是为了阻塞主线程，直到这个新起的线程执行完毕。线程函数的返回值都会被忽略，但线程函数可以接受任意数目的输入参数。
```cpp
void func(int i, double d, const std::string) {
    std:;cout << i << ' ' << d << ' ' << s << std::endl;
}
int main() {
    std::thread thr(func, 1, 12.50, "sample"); 
    //参数直接在起线程时输入
    //也可以使用 std::bind()

    thr.join();
    return 0;
}
```
虽然可以向线程函数传递任意多参数，但是都必须以**值传递**。如果需要传引用，必须使用```std::ref```或者```std::cref```进行封装，如下：
```cpp
void func(int &a) {
    a++;
}

int main() {
    int a = 42;
    std::thread thr(func, std::ref(a));
    thr.join();
    std::cout << a << std::endl;
    return 0;
}

```
程序输出43, 但如果不使用std::ref封装，则输出会是42.

如上文所述，除了简单的函数指针或者函数名，```std::thread```线程执行体支持任何可调用(Callable)的执行体，在C++11中主要有：
* lambda表达式
* 重载了operator()的类的对象
* 使用std::bind()表达式绑定对象和其非静态成员函数

```cpp
#include <iostream>
#include <thread>
#include <functional> // for std::bind

struct functor {
    void operator()(int a, int b) {
        std::cout << a << '+' << b << '=' << a+b << std::endl;
    }
};

class C {
    int data_;
public:
    C(int data) : data_(data);
    void member_func() {
        std::cout << "this->data_ = " << data_ << std::endl;
    }
}


int main() {
    std::thread thr1( [](int a, int b) {
        std::cout << a << '+' << b << '=' << a+b << std::endl;
    }, 1, 2); //使用lambda表达式创建线程对象并传递参数1和2

    std::thread thr2(functor(), 1, 2); //第二种，注意括号！

    C obj(10);
    std::thread thr3(std::bind(&C::member_func, &obj)); //第三种， 注意类的非静态成员函数的第一个参数是this指针，所以要传递&obj!

    //还可以使用lambda表达式调用对象的非静态成员函数
    std::thread thr4([&obj]() {
        obj.member_func();
    });


    thr1.join();
    thr2.join();
    thr3.join();
    thr4.join();

    return 0;
}
```
## ```std::thread```的其他成员函数
* **joinable()**: 判断线程对象是否可以join,当线程对象被析构的时候如果对象````joinable()==true```会导致```std::terminate```被调用。
* **join()**: 阻塞当前进程(通常是主线程)，等待创建的新线程执行完毕被操作系统回收。
* **detach()**: 将线程分离，从此线程对象受操作系统管辖。

### 线程管理函数
除了```std::thread```的成员函数外，在```std::this_thread```命名空间也定义了一系列函数用于管理当前线程。

函数名 | 作用
---- | ----
get_id | 返回当前线程的id
yield | 告知调度器运行其他线程，可用于当前处于繁忙的等待状态。相当于主动让出剩下的执行时间，具体的调度算法取决于实现
sleep_for | 指定的一段时间内停止当前线程的执行
sleep_until | 停止当前线程的执行直到指定的时间点


## ```std::future```类模板
```std::future```类模板是标准库提供的一种用于获取异步操作的结果的机制。前面的演示代码中线程的执行体都没有返回值，但是事实上```std::thread```的线程执行函数是可以有返回值的，但是其返回值会被忽略。此外使用```std::future```还可以延迟异步操作中异常的抛出。下面的代码演示了通过```std::async```启动一个异步操作, 并通过std::future::get()取得返回值和捕获异步操作中抛出的异常。

所以： **异步的底层实现就是起一个线程**。 (KIKA面试之痛。。。)

```cpp
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
```
输出
```
thread id = 140612419639040, a = 1, b = 2
thread id = 140612436997952, sum1 = 3
thread id = 140612419639040, a = 4000000000, b = 4000000000
thread id = 140612436997952, e.what() = overflow_error
```


### 使用std::future获取std::thread对象创建线程异步操作的结果

有以下两种方法：
#### 使用```std::packaged_task类模板(#include <future>)
步骤如下
1. 使用```std::packaged_task```包装线程执行函数获得一个```std::packaged_task```对象，该对象会处理被包装函数的返回值和异常。
2. 通过这个```std::packaged_task```对象获取其关联的std::future对象，用于获取异步操作的结果.使用```task.get_future()```函数
3. 将```std::packaged_task```对象作为```std::thread```的线程执行函数，启动线程
4. 通过```std::future```对象获取返回值和异常

演示代码如下：
```cpp
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
         << ", sum1 = " << sum1 << endl;    
    }
    catch(const std::overflow_error& e) {
        cout << "thread id = " << std::this_thread::get_id()
             << ", e.what() = " << e.what() << endl;
    } 
}

```
输出
```
thread id = 139969674540800, a = 1, b = 2
thread id = 139969691899712, sum1 = 3
thread id = 139969666148096, a = 4000000000, b = 4000000000
thread id = 139969691899712, e.what() = overflow_error
```

#### 使用```std::promise```类模板 #include <future>
步骤如下：

1. 创建一个```std::promise```对象
2. 获取```std::promise```对象关联的```std::furture```对象
3. 使用```std::thread```创建线程并将```std::promise```传入
4. 线程执行函数内部通过```std::promise```的```set_value```、```set_value_at_thread_exit```、```set_exception```或```set_exception_at_thread_exit```设置值或异常供```std::future```对象获取；
5. 还用std::future对象等待并获取异步操作的结果

演示代码
```cpp
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
```
输出同上。
**从实现的角度看**，```std::async```内部可以使用```std::packaged_task```来实现，而```std::packaged_task```内部则可以使用```std::promise```来实现




## 线程同步

互斥锁和条件变量是控制线程同步的常用手段，用来保护多线程同时访问的共享数据。C++11提供了这些操作，同时还提供了原子变量和一次调用的操作，用起来非常方便。下面介绍C++11中这些线程同步工具的使用。


### C++11 锁(mutex)
C++11中提供了如下四种语义的 锁  #include <mutex>
类名 | 描述
---- | ----
std::mutex | 独占的互斥锁，不能递归使用
std::timed_mutex | 带超时的独占互斥锁，不能递归使用
std::recursive_mutex | 递归互斥锁，不带超时功能
std::recursivve_timed_mutex | 带超时的递归互斥锁
std::shared_timed_mutex(C++14) | 允许多个现场共享所有钱的互斥对象，如读写锁

### 锁(Lock)
互斥锁的主要操作为加锁（lock）和解锁(unlock)。当一个线程以互斥锁对象进行lock操作并成功获得这个对象的所有权开始直到unlock为止，这段时间其他线程对这个互斥对象的lock操作都会被阻塞。
为了避免死锁，C++中往往会使用RAII管理互斥对象来自动管理资源。C++11提供如下几个类模板: #include <mutex>

类模板 | 描述
---- | ----
std::lock_guard | 严格基于作用域(Scope-based)的锁管理类模板，构造时是否加锁是可选的(不加锁时假定当前线程已经获得锁的所有权)，析构时自动释放锁，所有权不可转移，对象生存期内不允许手动加锁和释放锁
std::unique_lock | 更加灵活的锁管理类模板，构造时是否加锁是可选的，在对象析构时持有锁会自动释放锁，所有权可以转移对象生命期内允许手动加锁和释放锁。
std::shared_lock(C++14) | 用于管理可转移和共享所有权的互斥对象

使用```std::lock_guard```来实现多线程同时往一个set中完成inset的操作。在lock_guard构造时加锁，析构时自动解锁，所以也就不会发生互斥对象没有释放锁而导致死锁的问题。

最常用的lock_guard使用方式如下：
```cpp
{ //用一个socpe把临界区圈起来
    std::lock_guard<std::mutex> locker(mtx); //mtx为已定义好的std::mutex变量
    //...
    //...  do some work here
    //...
}

```

#### 互斥对象管理类模板的加锁策略
前面提到std::lock_guard, std::unique_lock和std::shared_lock类模板在构造时是否加锁是可选的，C++11提供了三种加锁策略，分别如下：

策略 | tag type | 描述
---- | ---- | ----
(默认) | 无 | 请求锁，阻塞当前进程直到获得锁
std::defer_lock | std::defer_lock_t | 不请求锁
std::try_to_lock | std::try_to_lock_t | 尝试请求锁，但不阻塞线程，锁不可用时也会立即返回
std::adopt_lock | std::adopt_lock_t | 假定当前线程已经获得互斥对象的所有权，所以不再请求锁。
下表给出三种管理类模板对上述策略的支持情况

策略         | lock_guard | unique_lock | shared_lock |
----------- | ---------- | ----------- | ----------- |
(默认)       | √          |       √     |   √(共享)    |
defer_lock  | ×          |       √     |   √         |             
try_to_lock | ×          |       √     |   √         |
adopt_lock  | √          |       √     |   √         |

总而言之，除了lock_guard不支持defer_lock和try_to_lock之外，其他的管理类模板支持所有四种构造策略。
下面的代码中std::unique_lock指定了std::defer_lock
```cpp
std::mutex mt;
std::unique_lock<std::mutex> locker(mt, std::defer_lock);
assert(locker.owns_lock() == false);
locker.lock();
assert(locker.owns_lock(0 == true));
```

#### 对多个互斥对象加锁
在某些情况下我们需要多个互斥对象加锁，考虑下面的代码（Buggy!）:
```cpp
// ATTENTION: this code is buggy! (Dead lock)

std::mutex mt1, mt2;
// thread 1
{
    std::lock_guard<std::mutex> locker1(mt1);
    std::lock_guard<std::mutex> locker2(mt2);
}
// thread 2
{
    std::lock_guard<std::mutex> locker2(mt2);
    std::lock_guard<std::mutex> locker1(mt1);
}
```
如果两个线程都刚好加完第一个锁，即thread1获得了mt1,thread2获得了mt2:
* 线程1持有mt1并等待mt2
* 线程2持有mt2并等待mt1
这样的环形等待形成了一个**死锁**

为了避免死锁，**对于任意的两个互斥锁，在多个线程中进行加锁时应保证其先后顺序一致**。即：
```cpp
// ATTENTION: this code is buggy! (Dead lock)

std::mutex mt1, mt2;
// thread 1
{
    std::lock_guard<std::mutex> locker1(mt1);
    std::lock_guard<std::mutex> locker2(mt2);
}
// thread 2
{
    std::lock_guard<std::mutex> locker1(mt1);
    std::lock_guard<std::mutex> locker2(mt2);
}
```
当然，在C++11中更好的做法是采用标准库中的std::lock和std::try_lock来对多个Lockable对象加锁。std::lock（或std::try_lock）会使用一种避免死锁的算法对多个待加锁对象进行lock操作或try_lock操作。当待加锁的对象中有不可用的对象时std::lock会阻塞当前线程直到所有的对象都可用. std::try_lock不会阻塞线程，当有对象不可用时会释放已经加锁的其他对象并立即返回。演示代码如下，注意这里可以让加锁顺序不同
```cpp
std::mutex mt1, mt2;
// thread 1
{
    std::unique_lock<std::mutex> locker1(mt1, std::defer_lock);
    std::unique_lock<std::mutex> locker2(mt2, std::defer_lock);
    std::lock(locker1, locker2);
    // do something
}
// thread 2
{
    std::unique_lock<std::mutex> locker1(mt1, std::defer_lock);
    std::unique_lock<std::mutex> locker2(mt2, std::defer_lock);
    std::lock(locker2, locker1);
    // do something
}

```
此外std::lock和std::try_lock还是异常安全的函数（要求待加锁的对象unlock操作不允许抛出异常），当多个对象加锁时，其中如果有米一个对象在lock或者try_lock中抛出异常，std::lock和std::try_lock会捕获这个异常并将之前已经加锁的对象逐个执行unlock操作，然后重新抛出这个异常(异常中立)。

## 条件变量 (Condition Variable)
条件变量是一种同步原语(Synchronization Primitive)，用于多线程之间的通信。它可以阻塞一个或者同时阻塞多个线程直到：
* 收到来自其他线程的通知
* 超时
* 发生虚假唤醒(Spurious Wakeup)
C++11为条件变量提供了两个类 ```#include <condition_variable>```
* std::condition_variable: 必须与std::unique_lock配合使用
* std::condition_variable_any： 更加通用的条件变量，可与任意类型的锁配合使用，相比前者使用时会有额外的开销，因此只有在它的灵活性成为必要的情况下才应优先使用
上述二者拥有相同的成员函数

条件变量的工作机制如下：

* 至少有一个线程在等待某个条件成立。等待的线程必须先持有一个unique_lock锁。这个锁被传递给```wait()```方法，这会释放mutex, 阻塞线程直到条件变量收到通知信号。当收到通知信号，线程唤醒，重新持有锁
* 至少有一个线程在发送条件成立的通知信号。信号的发送可以用```notify_one()```方法，只解锁任意一个正在等待通知信号的线程，也可以用```notify_all()```方法通知所有正在等待信号的线程
* 在多核处理器系统上，由于某些复杂机制的存在，可能发生伪唤醒，即一个线程在没有别的线程发送通知信号时也会被唤醒。因而，当线程被唤醒时，**检查条件是否成立是必要的**。而且，伪唤醒可能多次发生，所以条件检查要在一个循环中进行。
* 无论是notify_one还是notify_all都是类似于发出脉冲信号，如果对wait的调用发生在notify之后，则这样的线程不会被唤醒。所以接收者应该在使用wait等待之前要检查条件是否满足，通知者在notify之前要修改相关的标识以供接受者检查


C++11中std::condition_variable的成员函数如下：
成员函数 | 说明 |
---- | ---- |
notify_one | 通知一个等待线程
notify_all | 通知全部等待线程
wait | 阻塞当前线程直到被唤醒


###为什么条件变量需要和锁一起使用
观察std::condition_variable::wait函数,均必须将锁作为参数
```cpp
void wait(std::unique_lock<std::mutex>& lock );
template< class Predicate >
void wait(std::unique_lock<std::mutex>& lock, Predicate pred) 
```

首先尝试写不用锁的代码，考虑如下情况： flag初始化为false,线程A将flag置为true并使用notify_one发出通知，线程B使用whle循环在wait前后都会检查flag,直到flag被置为true才会继续执行。
```cpp
// Thread A
{
    std::unique_lock lck(mt);
    flag = true;
}
cv.notify_one();

// Thread B
auto pred = []() {
    std::unique_lock lck(mt);
    return flag;
};
while(!pred()) {
    cv.wait();
}
```

如果两个线程的执行顺序为：
1. 线程B检查flag发现其值为false, 挂起当前线程
2. 线程A将flag置为true
3. 线程A使用notify_one发出通知
4. 线程B使用wait进行等待

那么线程B不会被唤醒，因为notify在wait之前。发生这种情况的原因在于线程B对条件的检查和进入等待的中间是有空挡的，wait函数需要锁作为参数的正是为了解决这一问题。
```cpp
// Thread B
auto pred = []() {
    return flag;
};
std::unique_lock<std::mutex> locker(mt)
while(!pred()) {
    cv.wait(locker);
}
```
也就是说，线程B检查标识，然后获取锁，使得线程A不能修改其值；线程B调用wait时会释放传入的锁并同时进入等待，当被唤醒时会重新获得锁，因此只要线程A在修改flag的时候是正确的加锁的那么就不会发生前面的情况。
刚才的写法也可通过wait的重载写成
```cpp
auto pred = []() {
    return flag;
}; 
std::unique_lock<std::mutex> locker(mt);
cv.wait(locker, pred); //隐含了一个while循环
```
不仅仅是C++，C#,java等语言也是一样需要锁来配合。

**例子：n workers & 1 logger**
```cpp
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
```

输出
```
[logger]        start working!
[Worker 1]      start working!
[Worker 2]      start working!
[Worker 3]      start working!
[Worker 5]      start working!
[Worker 4]      start working!
[worker 1]      an error occurred: 101
[worker 4]      an error occurred: 401
[worker 4]      Quit!
[worker 1]      Quit!
[logger]        processing error 101
[logger]        processing error 401 
[worker 5]      an error occurred: 501
[worker 5]      Quit!
[logger]        processing error 501
[worker 2]      an error occurred: 201
[worker 3]      an error occurred: 301
[worker 3]      Quit!
[worker 2]      Quit!
[logger]        processing error 201
[logger]        processing error 301
Everything is done, Logger quit!
```



## C++11带来的随机数生成器

### 1. random_device
标准库提供了一个**非确定性随机数**生成设备,在Linux的实现中,是读取/dev/urandom设备;random_device提供()操作符,用来返回一个min-max之间的一个数字.如果在Linux下,都可以使用这个来产生高质量的随机数,可以理解为**真随机数**.
```cpp
#include <iostream>
#include <random>
int main() {
    std::random_device rd;
    for(int n = 0; n < 20000; ++n) {
        std::cout << rd() <<  std::endl;
    }
    return 0;
}
```

### 2. random number engine
标准库中把随机数抽象成随机数引擎和分布两个部分. 引擎用来产生随机数,分布产生特定的随机数(比如平均分布,正态分布等). 标准库提供三种常用的引擎: ```linear_congruential_engine```,```mersenne_twister_engine```和```subtract_with_carry_engine```, 第一种是线性同余算法,第二种是梅森旋转算法,第三种带进位的线性同余算法. 第一种是最为常用的,而且速度也非常快; 第二种号称是最好的伪随机数生成器

随机数引擎接收一个整型参数作为种子,不提供的话则使用默认值.推荐使用random_device来产生一个随机数当做种子.

```cpp
#include <iostream>
#include <random>

int main() {
    std::random_device rd;
    std::mt19937 mt(rd());
    for(int i = 0; n < 10; n++){
        std::cout << mt() << std::endl;
    }
    return 0;
}
```
### 3. random number distibutions

标准库提供各种各样的分布,比如平均分布,正态分布等,使用方式如下
```cpp
#include <random>
#include <iostream>
int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 6);
    for(int n = 0; n < 10; ++n){
        std::cout << dis(gen) << ' ';
    }
    std::cout << '\n';
}
```

