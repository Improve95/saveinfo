#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>

using namespace std;

static const uint32_t W = 32;
static const uint32_t R = 12;
static const uint32_t B = 16;
static const uint32_t P32 = 0xB7E15163;
static const uint32_t Q32 = 0x9E3779B9;
static const uint32_t BLOCK_SIZE = 8;

struct RC5 {
    vector<uint32_t> S;

    RC5(const uint8_t* key, size_t keyLen) {
        keyExpansion(key, keyLen);
    }

    static uint32_t rotl(uint32_t x, uint32_t y) {
        return (x << (y & (W - 1))) | (x >> (W - (y & (W - 1))));
    }

    static uint32_t rotr(uint32_t x, uint32_t y) {
        return (x >> (y & (W - 1))) | (x << (W - (y & (W - 1))));
    }

    void keyExpansion(const uint8_t* key, size_t keyLen) {
        size_t u = W / 8;                  
        size_t c = (keyLen + u - 1) / u;

        vector<uint32_t> L(c, 0);
        for (int i = keyLen - 1; i >= 0; i--) {
            L[i / u] = (L[i / u] << 8) + key[i];
        }

        S.resize(2 * (R + 1));
        S[0] = P32;
        for (size_t i = 1; i < S.size(); i++) {
            S[i] = S[i - 1] + Q32;
        }

        uint32_t A = 0, B = 0;
        size_t i = 0, j = 0;
        size_t n = 3 * max(S.size(), L.size());
        for (size_t k = 0; k < n; k++) {
            A = S[i] = rotl(S[i] + A + B, 3);
            B = L[j] = rotl(L[j] + A + B, (A + B));
            i = (i + 1) % S.size();
            j = (j + 1) % L.size();
        }
    }

    void encryptBlock(uint8_t* block) {
        uint32_t A = *(uint32_t*)(block);
        uint32_t B = *(uint32_t*)(block + 4);

        A += S[0];
        B += S[1];
        for (uint32_t i = 1; i <= R; i++) {
            A = rotl(A ^ B, B) + S[2 * i];
            B = rotl(B ^ A, A) + S[2 * i + 1];
        }

        memcpy(block, &A, 4);
        memcpy(block + 4, &B, 4);
    }

    void decryptBlock(uint8_t* block) {
        uint32_t A = *(uint32_t*)(block);
        uint32_t B = *(uint32_t*)(block + 4);

        for (int i = R; i >= 1; i--) {
            B = rotr(B - S[2 * i + 1], A) ^ A;
            A = rotr(A - S[2 * i], B) ^ B;
        }

        B -= S[1];
        A -= S[0];

        memcpy(block, &A, 4);
        memcpy(block + 4, &B, 4);
    }
};

void encryptFile(const string& inputFile, const string& outputFile, const string& keyStr) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    if (!in || !out) {
        cerr << "Ошибка открытия файлов" << endl;
        return;
    }

    RC5 rc5((const uint8_t*)keyStr.data(), keyStr.size());

    vector<uint8_t> buffer(BLOCK_SIZE);
    while (true) {
        in.read((char*)buffer.data(), BLOCK_SIZE);
        streamsize bytesRead = in.gcount();

        if (bytesRead == 0)
            break;

        if (bytesRead < BLOCK_SIZE) {
            uint8_t pad = BLOCK_SIZE - bytesRead;
            for (size_t i = bytesRead; i < BLOCK_SIZE; ++i)
                buffer[i] = pad;
        }

        rc5.encryptBlock(buffer.data());
        out.write((char*)buffer.data(), BLOCK_SIZE);

        if (bytesRead < BLOCK_SIZE)
            break;
    }
}

void decryptFile(const string& inputFile, const string& outputFile, const string& keyStr) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    if (!in || !out) {
        cerr << "Ошибка открытия файлов" << endl;
        return;
    }

    RC5 rc5((const uint8_t*)keyStr.data(), keyStr.size());

    vector<uint8_t> buffer(BLOCK_SIZE);
    vector<uint8_t> prev;
    while (in.read((char*)buffer.data(), BLOCK_SIZE)) {
        rc5.decryptBlock(buffer.data());
        prev.insert(prev.end(), buffer.begin(), buffer.end());
    }

    if (!prev.empty()) {
        uint8_t pad = prev.back();
        if (pad > 0 && pad <= BLOCK_SIZE) {
            prev.resize(prev.size() - pad);
        }
    }

    out.write((char*)prev.data(), prev.size());
}

int main() {
    string inputFile = "input.bin";
    string encFile = "encrypted.bin";
    string decFile = "decrypted.bin";
    string key = "qwertyuiopasdfgh";

    encryptFile(inputFile, encFile, key);
    decryptFile(encFile, decFile, key);

    return 0;
}

