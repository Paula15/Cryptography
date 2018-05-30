#include <cstdio>
#include <iostream>
#include <tuple>

#include "aes.h"
#include "utils.h"

using namespace std;

int main() {
    string IV, C;
    const string P = "1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF";
    const string K = "1234567890ABCDEF1234567890ABCDEF";
    tie(IV, C) = aes::Encrypt(Hex2bytes(P), Hex2bytes(K));
    cout << "P:" << endl << P << endl;
    cout << "K:" << endl << K << endl;
    cout << "IV:" << endl << Bytes2hex(IV) << endl;
    cout << "C:" << endl << Bytes2hex(C) << endl;
    string D = aes::Decrypt(IV, C, Hex2bytes(K));
    cout << "D:" << endl << Bytes2hex(D) << endl;
    return 0;
}
