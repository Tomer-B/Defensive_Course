#pragma once
#pragma pack(push, 1)
#include "Protocol.h"
#include "AESWrapper.h"
#include "Message.h"

#define SEND_SYMMETRIC_KEY (5)

class User {
	char ClientID[UUID_SIZE];
	char ClientName[MAX_NAME_SIZE];
	char PublicKey[PUBLIC_KEY_SIZE];
	char SymmetricKey[SYMMETRIC_KEY_SIZE];
	bool PublicKeySet;
	bool SymmetricKeySet;
	AESWrapper aes;

public:
	User(char id[UUID_SIZE], char name[MAX_NAME_SIZE]);
	int SetPublicKey(vector<char> key);
	int SetSymmetricKey(vector<char> key);
	int DecryptAndDisplayMessage(Message* message);
	friend class Client;
};

#pragma pack(pop)