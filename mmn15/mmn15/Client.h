#pragma once
#include <iostream>
#include "Comms.h"
#include "RSAWrapper.h"

#define SERVER_RESPONSE_HEADER_SIZE (7)

using namespace std;

class Client {
private:
	char ClientID[16];
	ClientComms comm;
	RSAPrivateWrapper rsa_private;
	RSAPublicWrapper rsa_public;

	string readServerData(string path);
	int initiateCommunication();
	int registerClient();
	int getClientList();
	int printPrompt();
public:
	Client(const string& ip, const string& port);
	int start();
};

#define SERVERINFO ("server.info")
#define CLIENTINFO ("me.info")
