#include <iostream>
#include <vector>
#include <string>

using namespace std;

void KSA(vector<unsigned char>& S, const string& key) {
    int key_length = key.size();
    for (int i = 0; i < 256; i++) {
        S[i] = i;
    }

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + S[i] + static_cast<unsigned char>(key[i % key_length])) % 256;
        swap(S[i], S[j]);
    }
}

string RC4(const string& text, const string& key) {
    vector<unsigned char> S(256);
    KSA(S, key);

    int i = 0, j = 0;
    string result;
    result.reserve(text.size());

    for (unsigned char c : text) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(S[i], S[j]);

        unsigned char K = S[(S[i] + S[j]) % 256];
        result.push_back(c ^ K);
    }

    return result;
}

int main() {
    string key, text;

    cout << "key: ";
    getline(cin, key);

    cout << "text: ";
    getline(cin, text);

    string encrypted = RC4(text, key);
    cout << "encoded: ";
    for (unsigned char c : encrypted) {
        cout << hex << (int)c << " ";
    }

    cout << endl;

    string decrypted = RC4(encrypted, key);
    cout << "devoded: " << decrypted << endl;

    return 0;
}
