#pragma once
#include "except.h"
#include <iostream>
#include <vector>
#pragma pack(push, 1)

#define CLIENT_VERSION (1)

#define UUID_SIZE (16)
#define PUBLIC_KEY_SIZE (160)
#define MAX_NAME_SIZE (255)
#define MAX_PAYLOAD_SIZE (16384)

#define REGISTER_REQUEST (1000)
#define CLIENT_LIST_REQUEST (1001)
#define CLIENT_PUBLIC_KEY_REQUEST (1002)
#define SEND_MESSAGE_REQUEST (1003)
#define GET_MESSAGES_REQUEST (1004)

#define REGISTRATION_SUCCESS_RESPONSE (2000)
#define CLIENT_LIST_RESPONSE (2001)
#define PUBLIC_KEY_RESPONSE (2002)
#define TEXT_MESSAGE_SENT_RESPONSE (2003)
#define TEXT_MESSAGE_RECEIVED_RESPONSE (2004)
#define GENERAL_ERROR_RESPONSE (9000)

#define GET_SYMMETRIC_KEY_MSG_TYPE (1)
#define SEND_SYMMETRIC_KEY_MSG_TYPE (2)
#define SEND_TEXT_MSG_TYPE (3)
#define SEND_FILE_MSG_TYPE (4)

using namespace std;

class ServerResponseHeader {
	unsigned char Version;
	unsigned short Code;
	size_t PayloadSize;
	friend class Client;
};

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

class RemoteClient {
	char ClientID[UUID_SIZE];
	char ClientName[MAX_NAME_SIZE];
	friend class Client;
};

#pragma pack(pop)
