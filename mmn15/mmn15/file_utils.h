#pragma once
#include <iostream>

using namespace std;

string readServerData();
int getMyInfo(string &name, string &ClientID, string &PrivateKey);
string ascii_to_hex(const string& Ascii);