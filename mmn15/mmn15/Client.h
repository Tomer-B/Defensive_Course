#pragma once
#include <iostream>
#include "Comms.h"
#include "RSAWrapper.h"
#include "User.h"

#define SERVERINFO ("server.info")
#define CLIENTINFO ("me.info")

using namespace std;

#pragma pack(push, 1)
class Client {
private:
	char ClientID[UUID_SIZE];
	char ClientName[MAX_NAME_SIZE];
	ClientComms comm;
	RSAPrivateWrapper *rsa_private;
	RSAPublicWrapper *rsa_public;
	vector<User> ClientsList;

	int ReadInfoFile();
	string readServerData(string path);
	int initiateCommunication();
	int registerClient();
	int getClientList();
	int printPrompt();
	int WriteInfoToFile();
	int GetRemotePublicKey();
	vector<char> SendMessageAndExpectCode(ProtocolMessage* p, size_t ExpectedPayloadSize, unsigned short ExpectedCode);
	User* GetUserByName(char RequestedClientName[MAX_NAME_SIZE]);
	User* GetUserByID(char id[UUID_SIZE]);
	int SendMessageToClient(size_t MessageType, User* user);
	int ReceiveMessageFromClient();
public:
	Client(const string& ip, const string& port);
	~Client();
	int start();
};
#pragma pack(pop)

