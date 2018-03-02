#ifndef SMUDUO_BASE_COMMON_H
#define SMUDUO_BASE_COMMON_H

#include <string>

namespace smuduo
{
class noncopyable{
protected:
    noncopyable() {};
private:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
};

struct copyable{

};

//For passing C-style argument to a function
class StringArg : copyable{
public:
    StringArg(const char* str): str_(str)
    { }

    StringArg(const std::string& str): str_(str.c_str())
    { }

    const char* c_str() const {return str_;}

private:
    const char* str_;
};

//类型强转，implicit_cast只能执行up-cast，派生类->基类
//这里模拟了boost库里面的implicit_cast
template<typename To, typename From>
inline To implicit_cast(From const &f)
{
  return f;
}



}

#endif