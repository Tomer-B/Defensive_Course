#pragma once
#include "except.h"
#include <iostream>
#include <vector>

#define MAX_PAYLOAD_LENGTH (1024)

using namespace std;

#pragma pack(push, 1)
class ServerResponseHeader {
	unsigned char Version;
	unsigned short Code;
	size_t PayloadSize;
	friend class Client;
};
#pragma pack(pop)

class Payload {
public:
	vector<char> pack(size_t size);
};

class ProtocolMessage {
	char ClientID[16];
	unsigned char Version;
	unsigned short Code;
	size_t PayloadSize;
	Payload *payload;
public:
	ProtocolMessage(char id[16], unsigned char Version, unsigned short Code, unsigned int PayloadSize, Payload *payload) :
		Version(Version), Code(Code), PayloadSize(PayloadSize), payload(payload) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, id, sizeof(id));
	};
	virtual vector<char> pack();
};

class RegisterPayload : Payload {
	char Name[256]; 
	char PublicKey[160];
public:
	RegisterPayload(char Name[256], string PublicKey);
};

class RequestClientPublicKey : Payload {
	char ClientID[16];
public:
	RequestClientPublicKey(char ClientID[16]) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, ClientID, sizeof(ClientID));
	}
};

class RequestSendMessageToClient : Payload {
	char ClientID[16];
	char MessageType;
	size_t Size;
	char* Content;
public:
	RequestSendMessageToClient(char ClientID[16], char MessageType, size_t Size, char* Content) : MessageType(MessageType), Size(Size), Content(Content) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, ClientID, sizeof(ClientID));
	};
};
