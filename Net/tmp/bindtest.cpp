#include <random>
#include <iostream>
#include <memory>
#include <functional>

void f(int n1, int n2, int n3, const int &n4, int n5) {
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' '
              << n4 << ' ' << n5 << std::endl;
}

int g(int n1) {
    return n1;
}

struct Foo { 
    void print_sum(int n1, int n2) {
        std::cout << n1 + n2 << std::endl;
    }
    int data = 10;
};

int main() {
    using namespace std::placeholders;
    
    int n = 7;
    //_1 and _2 are from std::placeorders, and represent future args
    // that will be passed to f1
    auto f1  = std::bind(f, _2, _1, 42, std::cref(n), n);
    f1(1,2,1001);
    n = 10;
    f1(1,2,1001);

    //nested bind subexpression share the placeorders
    auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5); //4 not _4!
    f2(10, 11, 12);

    //common use case: binding a RNG with a distribution
    std::default_random_engine e;
    std::uniform_int_distribution<> d(0, 10);
    std::function<int()> rnd = std::bind(d, e);

    for(int n = 0; n < 10; ++n) {
        std::cout << rnd() << ' ';
    } std::cout << std::endl;

    for(int n = 0; n < 10; ++n) {
        std::cout << d(e) << ' ';
    } std::cout << std::endl;

    // bind to a pointer to data function
    // the first arg of struct/classs is usually THIS pointer
    Foo foo;
    auto f3 = std::bind(&Foo::print_sum, &foo, 95, _1);
    f3(5);

    // bind to a pointer to data member
    auto f4 = std::bind(&Foo::data, _1); 
    std::cout << f4(foo) << '\n';

    // smart pointer can be used to call members of the reference objects, too
    std::cout << f4(std::make_shared<Foo >(foo)) << '\n';
    std::cout << f4(std::make_unique<Foo >(foo)) << '\n';

}