#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include "keccak.h"
#include "utils.h"

using namespace std;

int TestOne(const string &input, const string &expected, const Instance &inst_) {
    string input_bytes = Hex2bytes(input);
    Instance inst = inst_;
    if (inst.o == -1) inst.o = expected.size() * 4;
    string output_bytes = Keccak(input_bytes, inst);
    string output = Bytes2hex(output_bytes);
    if (output != expected) {
        cout << "WA" << endl;
        cout << "input = " << input << endl << endl;
        cout << "output = " << output << endl << endl;
        cout << "expected = " << expected << endl << endl;
        return -1;
    }
    return 0;
}

void Test(const string &filename, const Instance &inst) {
    cout << "************* Testing " << filename << " **************" << endl;
    int total = 0, correct = 0;
    ifstream fin(filename.c_str());
    vector<string> filelines = Readlines(fin);
    for (int i = 2; i < filelines.size() - 2; i += 4) {
        string field, eq;
        string input, expected, len_; int len;
        istringstream sin1(filelines[i]);
        sin1 >> field >> eq >> len_; len = stoi(len_);
        istringstream sin2(filelines[i + 1]);
        sin2 >> field >> eq >> input;
        istringstream sin3(filelines[i + 2]);
        sin3 >> field >> eq >> expected;

        if (len % 8 == 0 && len != 0) {
            total++;
            if (TestOne(input, expected, inst) != 0) {
                cout << "#" << i+1 << endl;
            } else { 
                correct++; 
            }
        }
    }   
    cout << "Passed: " << correct << "/" << total << endl;
}

int main() {
    Test("cases/SHAKE128.txt", SHAKE128);
    Test("cases/SHAKE256.txt", SHAKE256);
    Test("cases/SHA3_224.txt", SHA3_224);
    Test("cases/SHA3_256.txt", SHA3_256);
    Test("cases/SHA3_384.txt", SHA3_384);
    Test("cases/SHA3_512.txt", SHA3_512);
    return 0;
}
