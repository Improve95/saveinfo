#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <iomanip>

using namespace std;

const uint32_t W = 32;
const uint32_t R = 12;
const uint32_t P32 = 0xB7E15163;
const uint32_t Q32 = 0x9E3779B9;
const uint32_t BLOCK_SIZE = 8;

struct RC5 {
    uint32_t S[2 * (R + 1)];

    RC5(const uint8_t* key, size_t keyLen) {
        keyExpansion(key, keyLen);
    }

    static uint32_t rotl(uint32_t x, uint32_t y) {
        return (x << (y & 31)) | (x >> (32 - (y & 31)));
    }

    static uint32_t rotr(uint32_t x, uint32_t y) {
        return (x >> (y & 31)) | (x << (32 - (y & 31)));
    }

    void keyExpansion(const uint8_t* key, size_t keyLen) {
        uint32_t L[4] = {0};
        for (int i = keyLen - 1; i >= 0; i--) {
            L[i / 4] = (L[i / 4] << 8) + key[i];
        }

        S[0] = P32;
        for (int i = 1; i < 2 * (R + 1); i++) S[i] = S[i - 1] + Q32;

        uint32_t A = 0, B = 0;
        int i = 0, j = 0;
        for (int k = 0; k < 3 * max((uint32_t) 2 * (R + 1), (uint32_t) 4); k++) {
            A = S[i] = rotl(S[i] + A + B, 3);
            B = L[j] = rotl(L[j] + A + B, A + B);
            i = (i + 1) % (2 * (R + 1));
            j = (j + 1) % 4;
        }
    }

    void encrypt(uint8_t* block) {
        uint32_t A, B;
        memcpy(&A, block, 4);
        memcpy(&B, block + 4, 4);

        A += S[0];
        B += S[1];
        for (uint32_t i = 1; i <= R; i++) {
            A = rotl(A ^ B, B) + S[2 * i];
            B = rotl(B ^ A, A) + S[2 * i + 1];
        }

        memcpy(block, &A, 4);
        memcpy(block + 4, &B, 4);
    }
};

vector<uint8_t> rc5_hash(const vector<uint8_t>& data, ) {

    RC5 rc5(key, 16);

    uint8_t H[8] = {};

    // Обработка по 8 байт
    for (size_t i = 0; i < data.size(); i += 8) {
        uint8_t block[8] = {0};
        size_t len = min((size_t)8, data.size() - i);
        memcpy(block, &data[i], len);

        // Davies–Meyer: E_key(H) ⊕ H
        rc5.encrypt(H);
        for (int j = 0; j < 8; j++)
            H[j] ^= block[j];
    }

    return vector<uint8_t>(H, H + 8);
}

void avalanche_test(const string& input) {
    vector<uint8_t> data(input.begin(), input.end());
    auto h1 = rc5_hash(data);

    // Инвертируем один бит во входе (например, первый)
    data[0] ^= 0x01;
    auto h2 = rc5_hash(data);

    // Считаем различающиеся биты
    int diff = 0;
    for (int i = 0; i < 8; i++) {
        diff += __builtin_popcount(h1[i] ^ h2[i]);
    }

    cout << "Исходный хеш: ";
    for (auto b : h1) cout << hex << setw(2) << setfill('0') << (int)b;
    cout << "\nИзменённый хеш: ";
    for (auto b : h2) cout << hex << setw(2) << setfill('0') << (int)b;

    cout << dec << "\nИзменилось бит: " << diff << " из 64 (" << (diff * 100.0 / 64) << "%)\n";
}

int main() {
    string text;
    cout << "Введите строку: ";
    getline(cin, text);

    vector<uint8_t> data(text.begin(), text.end());
    auto hash = rc5_hash(data);

    cout << "Хеш: ";
    for (auto b : hash)
        cout << hex << setw(2) << setfill('0') << (int)b;
    cout << endl;

    cout << "\nПроверка лавинного эффекта:\n";
    avalanche_test(text);

    return 0;
}
