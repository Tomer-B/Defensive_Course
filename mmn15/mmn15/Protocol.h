#pragma once
#include "except.h"
#include <iostream>
#include <vector>

#define MAX_PAYLOAD_LENGTH (1024)
#define UUID_SIZE (16)
#define PUBLIC_KEY_SIZE (160)
#define MAX_NAME_SIZE (256)

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
	char ClientID[UUID_SIZE];
	unsigned char Version;
	unsigned short Code;
	size_t PayloadSize;
	Payload *payload;
public:
	ProtocolMessage(char id[UUID_SIZE], unsigned char Version, unsigned short Code, unsigned int PayloadSize, Payload *payload) :
		Version(Version), Code(Code), PayloadSize(PayloadSize), payload(payload) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, id, sizeof(id));
	};
	virtual vector<char> pack();
};

class RegisterPayload : Payload {
	char Name[MAX_NAME_SIZE];
	char PublicKey[PUBLIC_KEY_SIZE];
public:
	RegisterPayload(char Name[MAX_NAME_SIZE], string PublicKey);
};

class RequestClientPublicKey : Payload {
	char ClientID[UUID_SIZE];
public:
	RequestClientPublicKey(char ClientID[UUID_SIZE]) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, ClientID, sizeof(ClientID));
	}
};

class RequestSendMessageToClient : Payload {
	char ClientID[UUID_SIZE];
	char MessageType;
	size_t Size;
	char* Content;
public:
	RequestSendMessageToClient(char ClientID[UUID_SIZE], char MessageType, size_t Size, char* Content) : MessageType(MessageType), Size(Size), Content(Content) {
		memset(ClientID, 0, sizeof(ClientID));
		memcpy(ClientID, ClientID, sizeof(ClientID));
	};
};
