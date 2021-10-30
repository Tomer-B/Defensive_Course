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
        getline(clientInfo, ClientID); // base-64 encoded
        getline(clientInfo, PrivateKey); // base-64 encoded
    }
    return 0;
}