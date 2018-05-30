#ifndef KECCAK_H
#define KECCAK_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "utils.h"

using namespace std;

struct Instance {
    int r;  // bit rate。表示b中有多少来自原有信息
    int c;  // capacity，填充r至b大小
    char d; // 根据具体instance确定
    int o;  // 输出bit数，为-1时表示输出变长
    Instance(int r, int c, char d, int o = -1): 
        r(r), c(c), d(d), o(o) {}
} 
SHAKE128(1344, 256, 0x1F),
SHAKE256(1088, 512, 0x1F),
SHA3_224(1152, 448, 0x06, 224),
SHA3_256(1088, 512, 0x06, 256),
SHA3_384(832, 768, 0x06, 384),
SHA3_512(576, 1024, 0x06, 512);

const int rxy[5][5] = {
    {0,    36,     3,    41,    18},
    {1,    44,    10,    45,     2},
    {62,    6,    43,    15,    61},
    {28,   55,    25,    21,    56},
    {27,   20,    39,     8,    14}
};

const uint64_t RCs[] = {
    0x0000000000000001,
    0x0000000000008082,
    0x800000000000808A,
    0x8000000080008000,
    0x000000000000808B,
    0x0000000080000001,
    0x8000000080008081,
    0x8000000000008009,
    0x000000000000008A,
    0x0000000000000088,
    0x0000000080008009,
    0x000000008000000A,
    0x000000008000808B,
    0x800000000000008B,
    0x8000000000008089,
    0x8000000000008003,
    0x8000000000008002,
    0x8000000000000080,
    0x000000000000800A,
    0x800000008000000A,
    0x8000000080008081,
    0x8000000000008080,
    0x0000000080000001,
    0x8000000080008008
};

uint64_t Rot(uint64_t num, uint64_t offset) {
    // NOTE: oversized shift (e.g. >=64 for uint64_t) MUST be avoided!
    if (offset == 0) return num;
    return (num >> (64 - offset)) + (num << offset);
}

vector<vector<uint64_t>> Round(const vector<vector<uint64_t>> &A_, uint64_t RC) {
    vector<vector<uint64_t>> A = A_;
    vector<vector<uint64_t>> B(5, vector<uint64_t>(5, 0));
    vector<uint64_t> C(5, 0);
    vector<uint64_t> D(5, 0);
    // θ
    for (int x = 0; x < 5; x++) C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
    for (int x = 0; x < 5; x++) D[x] = C[(x + 4)%5] ^ Rot(C[(x + 1)%5], 1);
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) {
        A[x][y] ^= D[x];
    }
    // ρ and π
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) {
        B[y][(2*x + 3*y)%5] = Rot(A[x][y], rxy[x][y]);
    }
    // χ
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) {
        A[x][y] = B[x][y] ^ ((~B[(x + 1)%5][y]) & B[(x + 2)%5][y]);
    }
    // ι
    A[0][0] ^= RC;
    return A;
}

vector<uint8_t> KaccakF1600(const vector<uint8_t> &S_) {
    /**
     * (3.1) state -> lanes
     */
    vector<uint8_t> S = S_;
    vector<vector<uint64_t>> lanes(5, vector<uint64_t>(5, 0L));
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) {
        for (int bytepos = 0; bytepos < 8; bytepos++) {
            lanes[x][y] += (((uint64_t) S[8*(x + 5*y) + bytepos]) << (8*bytepos));
        }
    }
    /**
     * (3.2) hash on lanes
     */
    for (int round = 0; round < 24; round++) {
        lanes = Round(lanes, RCs[round]);
    }
    /**
     * (3.3) lanes -> state
     */
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) {
        for (int bytepos = 0; bytepos < 8; bytepos++) {
            S[8*(x + 5*y) + bytepos] = (lanes[x][y] >> (8*bytepos))%256;
        }
    }
    return S;
}

string Keccak(const string &input_bytes, const Instance &inst) {
    assert(inst.r + inst.c == 1600);
    /** 
     * 1. Padding  
     *    P = input_bytes || d || 0x00 || … || 0x00
     *    P = P xor (0x00 || … || 0x00 || 0x80)
     * */
    int r = int(inst.r / 8);
    string P = input_bytes + inst.d;
    while (P.length() % r != 0) P += char(0x0);
    P[P.length() - 1] ^= 0x80;

    /**
     * 2. Init
     *    S[x,y] = 0,   for (x,y) in (0…4,0…4)
     */
    vector<uint8_t> S(5*5*8, 0);

    /**
     * 3. Absorbing phase
     *    for each block Pi in P
     *      S[x,y] = S[x,y] xor Pi[x+5*y],  for (x,y) such that x+5*y < r/w
     *      S = Keccak-f[r+c](S)
     */
    for (int begin = 0; begin <= P.length() - r; begin += r) {
        for (int i = 0; i < r; i++) S[i] ^= (uint8_t)P[begin + i];
        S = KaccakF1600(S);
    } 

    /**
     * 4. Squeezing phase
     *    Z = empty string
     *    while output is requested
     *      Z = Z || S[x,y],    for (x,y) such that x+5*y < r/w    
     *      S = Keccak-f[r+c](S)
     */
    string output_bytes;
    int n_output_bytes = int(inst.o / 8);
    while (n_output_bytes > 0) {
        int block_size = min(n_output_bytes, r);
        for (int i = 0; i < block_size; i++) output_bytes += (char)S[i];
        n_output_bytes -= block_size;
        S = KaccakF1600(S);
    }
    return output_bytes;
}

#endif // KECCAK_H
