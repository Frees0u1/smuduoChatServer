#include <string.h>
#include <fstream>
#include <assert.h>
using namespace std;

int main(int argc, char** argv) {
    if(argc != 3){
        printf("Usage: %s <char> <len>\n", argv[0]);
    }
    char c = argv[1][0];
    int len = atoi(argv[2]);
    assert(len > 0);

    string output(len, c);
    ofstream fout("newfile.txt", ios::out);
    fout << output;
    fout.close(); 

    return 0;
}