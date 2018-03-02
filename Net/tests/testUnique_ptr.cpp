#include <memory>
#include <iostream>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <set>
using namespace std;

typedef pair<int, unique_ptr<int>> Entry;

struct EntryComp {
    bool operator()(const Entry& lhs, const Entry& rhs) {
        if(lhs.first == rhs.first) {
            return lhs.second > rhs.second;
        } else {
            return lhs.first < rhs.first;
        }
    }
};

int main() {
   srand(time(NULL));

   set<Entry, EntryComp> mySet;

   for(int i = 0; i < 10; i++) {
       mySet.insert(make_pair(rand() % 3, make_unique<int>(rand() % 100) ));
   }
    int idx = 0;
   for(auto it = mySet.begin(); it != mySet.end(); it++, idx++) {
       printf("idx %d: first = %d, second = %p\n", idx, it->first, it->second.get());
   }

    {
        unique_ptr<int> np(nullptr); 
    }

    

    //测试unique_ptr底层是否可以传送raw pointer
 /*   unique_ptr<int> up1 = make_unique<int>(5);
    int *p = up1.get();

    cout <<"another raw pointer -> " << *p << endl;
    cout << "unique_ptr -> " << *p << endl;

    //测试传送后的unique_ptr是否可以正常析构
    int *pp;
    {
        unique_ptr<int> up2 = make_unique<int>(99);
        pp = up2.get();
        cout << "++pp2 = " << ++(*pp) << endl;
    }
    cout << "Everything is okay!" << endl;
    cout << "out scope: ++pp2 = " << (*pp) << endl;*/


    

}
    