#include "file_utils.h"
#include "except.h"
#include "Client.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string readServerData() {
    ifstream serverInfo(SERVERINFO);
    serverInfo.open(SERVERINFO);

    if (serverInfo.is_open()) {
        return string((std::istreambuf_iterator<char>(serverInfo)), std::istreambuf_iterator<char>());
    }
    throw ServerInfoReadError();
}

int getMyInfo(string& name, string& ClientID, string& PrivateKey) {
    ifstream clientInfo(CLIENTINFO);
    clientInfo.open(CLIENTINFO);
    string line;

    if (clientInfo.is_open()) {
        getline(clientInfo, name);
        getline(clientInfo, ClientID);
        getline(clientInfo, PrivateKey); // base-64 encoded
    }
    return 0;
}

string ascii_to_hex(const string& Ascii) {
    std::string result;
    for (auto it = Ascii.begin(); it != Ascii.end(); advance(it, 2)) {
        string Current;
        Current += *it;
        Current += *next(it);
        result += static_cast<char>(stoi(Current, nullptr, UUID_SIZE));
    }
    return result;
}