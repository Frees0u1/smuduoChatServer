#include "Buffer.h"
#include <string>
#include <iostream>
#include <endian.h>
#include <cstdlib>
#include <cstdio>

//for OPEN
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
//支持有 == 重载运算符的比较

int allCase = 0;
int passedCase = 0;

template <typename T>
inline void EQUAL_CHECK_SIMPLE(const T& lhs, const T& rhs, const string& msg = ""){
    allCase++;
    cout << msg << " Result: ";
    if(lhs == rhs) {
        cout << "Pass!" << endl;
        passedCase++;
    }
    else cout << "Failed!" << endl;
}

//支持字符串之间的比较
inline void CHECK_3B(const Buffer& buf, size_t rd, size_t wt, size_t pre){
    EQUAL_CHECK_SIMPLE<size_t>(buf.readableBytes(), rd, "RD check");
    EQUAL_CHECK_SIMPLE<size_t>(buf.writableBytes(), wt, "WT check");
    EQUAL_CHECK_SIMPLE<size_t>(buf.prependableBytes(), pre, "PE check");
    cout << endl;
}

void test_Append_Retrieve(){
    Buffer buf;
    cout << "=========== test Buffer Append Retrieve ==========" << endl;
    cout << "initally..." <<endl;
    CHECK_3B(buf, 0, Buffer::kInitialSize, Buffer::kCheapPrepend);

    const string str(200, 'x');
    buf.append(str);
    cout << "append 200 \'x\'..." << endl;
    CHECK_3B(buf, str.size(), Buffer::kInitialSize - str.size(), Buffer::kCheapPrepend);


    const string str2 = buf.retrieveAsString(50);
    cout << "retrieveAsString 50..." <<endl;
    EQUAL_CHECK_SIMPLE<size_t>(str2.size(), 50);
    CHECK_3B(buf, str.size() - str2.size(), Buffer::kInitialSize - str.size(), Buffer::kCheapPrepend + str2.size());

    buf.append(str);
    CHECK_3B(buf, 2*str.size() - str2.size(), Buffer::kInitialSize - 2*str.size(), Buffer::kCheapPrepend + str2.size());


    const string str3 = buf.retrieveAllAsString();
    cout << "retrieved all as string..." << endl;
    EQUAL_CHECK_SIMPLE<size_t>(str3.size(), 350);
    CHECK_3B(buf, 0, Buffer::kInitialSize, Buffer::kCheapPrepend);

    //cout << str3 << endl;
    cout << endl;
}

void testBufferGrow(){
    Buffer buf;

    cout << "================ test Buffer Grow =================" << endl;

    buf.append(string(400, 'y'));
    CHECK_3B(buf, 400, Buffer::kInitialSize - 400, Buffer::kCheapPrepend);

    buf.retrieve(50);
    CHECK_3B(buf, 350, Buffer::kInitialSize - 400, Buffer::kCheapPrepend + 50);

    buf.append(string(1000, 'z'));
    CHECK_3B(buf, 1350, 0, Buffer::kCheapPrepend + 50);

    buf.retrieveAll();
    CHECK_3B(buf, 0, 1400, Buffer::kCheapPrepend);
}

void testBufferInsideGrow(){
    Buffer buf;

    cout << "================ test Buffer  Inside Grow =================" << endl;
    buf.append(string(800, 'y'));
    CHECK_3B(buf, 800, Buffer::kInitialSize - 800, Buffer::kCheapPrepend);

    buf.retrieve(500);
    CHECK_3B(buf, 300, Buffer::kInitialSize - 800, Buffer::kCheapPrepend + 500);

    buf.append(string(300, 'z'));
    CHECK_3B(buf, 600, Buffer::kInitialSize-600, Buffer::kCheapPrepend);
}

void testBufferReadInt() {
    Buffer buf;
    buf.append("HTTP");
    cout << "================ test Buffer  ReadInt Family =================" << endl;
    EQUAL_CHECK_SIMPLE<size_t>(buf.readableBytes(), 4);
    EQUAL_CHECK_SIMPLE<size_t>(buf.peekInt8(), 'H');
    int top16 = buf.peekInt16();
    EQUAL_CHECK_SIMPLE<int>(top16, 'H'*256 + 'T');
    EQUAL_CHECK_SIMPLE<int>(buf.peekInt32(), top16*65536 + 'T'*256 + 'P');

    EQUAL_CHECK_SIMPLE<int>(buf.readInt8(), 'H');
    EQUAL_CHECK_SIMPLE<int>(buf.readInt16(), 'T'*256 + 'T');
    EQUAL_CHECK_SIMPLE<int>(buf.readInt8(), 'P');

    EQUAL_CHECK_SIMPLE<size_t>(buf.readableBytes(), 0);
    EQUAL_CHECK_SIMPLE<size_t>(buf.writableBytes(), Buffer::kInitialSize);

    buf.appendInt8(-1);
    buf.appendInt16(-1);
    buf.appendInt32(-1);

    EQUAL_CHECK_SIMPLE<int>(buf.readableBytes(), 7);
    EQUAL_CHECK_SIMPLE<int>(buf.readInt8(), -1);
    EQUAL_CHECK_SIMPLE<int>(buf.readInt32(), -1);
    EQUAL_CHECK_SIMPLE<int>(buf.readInt16(), -1);
}

void output(Buffer&& buf, const void* inner) {
    Buffer newbuf(std::move(buf));
    EQUAL_CHECK_SIMPLE<const void*>(inner, newbuf.peek());
}
void testMove(){
    Buffer buf;
    cout << "========== test Move ===========" << endl;

    buf.append("smuduo", 6);
    const void* inner = buf.peek();
    output(std::move(buf), inner);
}

void testBufferPrepend(){
    Buffer buf;
    cout << "=========== test prepend =============" << endl;  
    buf.append(string(200, 'y'));
    CHECK_3B(buf, 200, Buffer::kInitialSize-200, Buffer::kCheapPrepend);

    int x = 0;
    buf.prepend(&x, sizeof x);
    CHECK_3B(buf, 204, Buffer::kInitialSize-200, Buffer::kCheapPrepend - 4);
    
}

void testReadFd(){
    Buffer buf;
    int errno_copy;
    cout << "========= test ReadFd ==========" << endl;
    int fd = open("newfile.txt", O_RDONLY, 0);

    buf.readFd(fd, &errno_copy);
    //cout << buf.readableBytes() << endl;
    //cout << buf.writableBytes() << endl;
    //cout << buf.prependableBytes() << endl;
    CHECK_3B(buf, 2048, 0, 8);  
    
}


int main() {
    
    test_Append_Retrieve();
    testBufferGrow();
    testBufferInsideGrow();
    testBufferReadInt();
    testMove();
    testBufferPrepend();
    testReadFd();

    cout << passedCase << "/" << allCase << " PASSED!" << endl;
    return 0;
} 