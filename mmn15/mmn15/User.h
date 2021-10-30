#pragma once

#include "Protocol.h"

class User {
	char ClientID[UUID_SIZE];
	char ClientName[MAX_NAME_SIZE];
	char PublicKey[PUBLIC_KEY_SIZE];
	bool PublicKeySet;
public:
	User(char id[UUID_SIZE], char name[MAX_NAME_SIZE]);
	int SetPublicKey(vector<char> key);
	friend class Client;
};
