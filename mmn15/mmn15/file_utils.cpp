#include "file_utils.h"
#include "except.h"
#include "Client.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

string readServerData() {
    ifstream serverInfo(SERVERINFO);
    serverInfo.open(SERVERINFO);

    if (serverInfo.is_open()) {
        return string((std::istreambuf_iterator<char>(serverInfo)), std::istreambuf_iterator<char>());
    }
    throw ServerInfoReadError();
}

string ascii_to_hex(const string& Ascii) {
    string result;
    for (auto it = Ascii.begin(); it != Ascii.end(); advance(it, 2)) {
        string Current;
        Current += *it;
        Current += *next(it);
        result += static_cast<char>(stoi(Current, nullptr, UUID_SIZE));
    }
    return result;
}

string hex_to_ascii(const string& Hex, size_t len) {
    stringstream stream;
    for (int i = 0; i < len; i++) {
        stream << setw(2) << setfill('0') << hex << ((int)Hex[i] & 0xff);
    }
    return stream.str();
}