#include "Protocol.h"
#include <iostream>
using namespace std;


vector<char> Payload::pack(size_t size) {
	vector<char> v;
	for (int i = 4; i < size+4; i++) {
		v.push_back(((char*)this)[i]);
	}
	return v;
}

vector<char> RequestClientPublicKey::pack(size_t total_size) {
	vector<char> v;
	for (int i = 0; i < UUID_SIZE; i++) {
		v.push_back(ClientID[i]);
	}
	return v;
}

vector<char> RequestSendMessageToClient::pack(size_t total_size) {
	vector<char> v;
	for (int i = 4; i < total_size + 4 - Size; i++) {
		v.push_back(((char*)this)[i]);
	}
	for (int i = 0; i < Size; i++) {
		v.push_back(Content[i]);
	}
	return v;
}

vector<char> ProtocolMessage::pack() {
	vector<char> v;	
	char* data = (char*)&ClientID;
	for (int i = 0; i < UUID_SIZE; i++) {
		v.push_back(data[i]);
	}
	v.push_back(Version);
	data = (char*)&Code;
	for (int i = 0; i < sizeof(Code); i++) {
		v.push_back(data[i]);
	}
	data = (char*)&PayloadSize;
	for (int i = 0; i < sizeof(PayloadSize); i++) {
		v.push_back(data[i]);
	}

	if (payload) {
		vector<char> packed_payload = payload->pack(PayloadSize);
		v.insert(v.end(), packed_payload.begin(), packed_payload.end());
	}
	return v;
}

RegisterPayload::RegisterPayload(char name[256], string publickey) {
	if (publickey.length() > 160) {
		throw ArgumentTooLongError();
	}
	memcpy(Name, name, sizeof(Name));
	memcpy(PublicKey, publickey.c_str(), sizeof(PublicKey));
}
