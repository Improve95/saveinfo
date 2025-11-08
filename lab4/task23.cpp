#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include <sstream>

using namespace std;

static const uint32_t W = 32; // половина длины блока в битах
static const uint32_t R = 12; // число раундов
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

        A = (A + S[0]) & (((size_t) 1 << W) - 1);
        B = (B + S[1]) & (((size_t) 1 << W) - 1);

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

        B = (B - S[1]) & (((size_t) 1 << W) - 1);
        A = (A - S[0]) & (((size_t) 1 << W) - 1);

        memcpy(block, &A, 4);
        memcpy(block + 4, &B, 4);
    }
};

void encryptFile(const string& inputFile, const string& outputFile, const string& keyStr) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    if (!in || !out) {
        cerr << "Ошибка открытия файлов\n";
        return;
    }

    RC5 rc5((const uint8_t*)keyStr.data(), keyStr.size());
    vector<uint8_t> buffer(BLOCK_SIZE);

    while (true) {
        in.read((char*)buffer.data(), BLOCK_SIZE);
        streamsize bytesRead = in.gcount();
        if (bytesRead == 0) break;

        if (bytesRead < BLOCK_SIZE)
            memset(buffer.data() + bytesRead, BLOCK_SIZE - bytesRead, BLOCK_SIZE - bytesRead);

        rc5.encryptBlock(buffer.data());
        out.write((char*)buffer.data(), BLOCK_SIZE);

        if (bytesRead < BLOCK_SIZE) break;
    }
}

void decryptFile(const string& inputFile, const string& outputFile, const string& keyStr) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    if (!in || !out) {
        cerr << "Ошибка открытия файлов\n";
        return;
    }

    RC5 rc5((const uint8_t*)keyStr.data(), keyStr.size());
    vector<uint8_t> buffer(BLOCK_SIZE);
    vector<uint8_t> result;

    while (in.read((char*)buffer.data(), BLOCK_SIZE)) {
        rc5.decryptBlock(buffer.data());
        result.insert(result.end(), buffer.begin(), buffer.end());
    }

    if (!result.empty()) {
        uint8_t pad = result.back();
        if (pad > 0 && pad <= BLOCK_SIZE)
            result.resize(result.size() - pad);
    }

    out.write((char*)result.data(), result.size());
}

vector<uint8_t> rc5Hash(const string& input, const string& keyStr) {
    RC5 rc5((const uint8_t*)keyStr.data(), keyStr.size());

    vector<uint8_t> buffer(BLOCK_SIZE, 0);
    vector<uint8_t> hash(8, 0);

    for (size_t i = 0; i < input.size(); i += BLOCK_SIZE) {
        size_t len = min((size_t) BLOCK_SIZE, input.size() - i);
        memcpy(buffer.data(), input.data() + i, len);
        if (len < BLOCK_SIZE) memset(buffer.data() + len, 0, BLOCK_SIZE - len);

        rc5.encryptBlock(buffer.data());

        for (size_t j = 0; j < 8; ++j)
            hash[j] ^= buffer[j];
    }

    rc5.encryptBlock(hash.data());
    return hash;
}

void testAvalancheEffect(const string& input, const string& keyStr) {
    auto hash1 = rc5Hash(input, keyStr);

    string modified = input;
    modified[0] ^= 0x01;

    auto hash2 = rc5Hash(modified, keyStr);

    int diffBits = 0;
    for (size_t i = 0; i < hash1.size(); ++i) {
        uint8_t diff = hash1[i] ^ hash2[i];
        for (int b = 0; b < 8; ++b)
            if (diff & (1 << b)) diffBits++;
    }

    cout << "\nAvalanche effect: " << fixed << setprecision(2)
         << (diffBits * 100.0 / ((double) hash1.size() * 8)) << "% бит изменилось\n";
}

int main() {
    string key = "qwertyuiopasdfgh";

    string inputFile = "input.txt";
    string encFile = "encrypted.bin";
    string decFile = "decrypted.bin";

    encryptFile(inputFile, encFile, key);
    decryptFile(encFile, decFile, key);

    string text;
    ifstream in(inputFile, ios::binary);
    ostringstream buffer;
    buffer << in.rdbuf();
    text = buffer.str();

    auto hash = rc5Hash(text, key);

    cout << "Input: " << text << "\nHash: ";
    for (uint8_t b : hash)
        cout << hex << setw(2) << setfill('0') << (int)b;
    cout << dec << endl;

    testAvalancheEffect(text, key);

    return 0;
}
