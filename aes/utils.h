#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

string Bytes2hex(const string &bytes) {
    string ret;
    for (int i = 0; i < bytes.length(); i++) {
        char hex[5];
        memset(hex, 0, sizeof(hex));
        sprintf(hex, "%02X", (uint8_t)bytes[i]);
        ret += hex;
    }
    return ret;
}

string Hex2bytes(const string &hex) {
    string ret;
    for (int i = 0; i < hex.length() - 1; i += 2) {
        char *endptr;
        char byte = (char)strtol(hex.substr(i, 2).c_str(), &endptr, 16);
        ret += byte;
    }
    return ret;
}

template <class T>
void PrintVec(const vector<T> &vec) {
    for (int i = 0; i < vec.size(); i++) cout << vec[i] << "\t";
    cout << endl;
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

vector<string> Readlines(ifstream &fin) {
    string line;
    vector<string> ret;
    while (getline(fin, line)) {
	    trim(line);	
		ret.push_back(line);
	}
    return ret;
}

void pause() {
    string dummy;
    cout << "Press any key to continue." << endl;
    getline(cin, dummy);
}


#endif // UTILS_H
