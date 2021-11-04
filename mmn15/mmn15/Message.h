#pragma once
#pragma pack(push, 1)

#include <iostream>
#include <vector>

using namespace std;

class Message {
	char ClientID[16];
	size_t MessageID;
	unsigned char MessageType;
	size_t MessageSize;
	char* Content;
	friend class User;
	friend class Client;
};

#pragma pack(pop)