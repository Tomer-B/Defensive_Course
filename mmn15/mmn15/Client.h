#pragma once
#include <iostream>
#include "Comms.h"
#include "RSAWrapper.h"

using namespace std;

class Client {
private:
	char ClientID[16];
	char ClientName[256];
	ClientComms comm;
	RSAPrivateWrapper rsa_private;
	RSAPublicWrapper rsa_public;

	string readServerData(string path);
	int initiateCommunication();
	int registerClient();
	int getClientList();
	int printPrompt();
	int WriteInfoToFile();
	string GetRemotePublicKey(char RemoteClientUUID[16]);
	vector<char> SendMessageAndExpectCode(ProtocolMessage* p, size_t ExpectedPayloadSize, unsigned short ExpectedCode);
public:
	Client(const string& ip, const string& port);
	int start();
};

#define SERVERINFO ("server.info")
#define CLIENTINFO ("me.info")
