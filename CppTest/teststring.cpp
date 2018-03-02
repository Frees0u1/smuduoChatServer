#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <endian.h>
#include <algorithm>
#include <stdio.h>
#include <cstring>

using namespace std;

int main() {
    string str(7,'\0');
    str += "FUCK";
    str += string(100, 0);
    cout << str.size() << endl;
    cout << "[" << str <<  "]" << endl;

    int64_t be64;
    ::memcpy(&be64, str.data(), sizeof be64);
   

    cout << "be64 = " << ::be64toh(be64) << endl;


    return 0;
}