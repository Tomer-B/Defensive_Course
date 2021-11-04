#include "Protocol.h"
#include <iostream>
using namespace std;


vector<char> Payload::pack(size_t size) {
	vector<char> v;
	for (char* c = (char*)this; c < (char*)this + size;  c++) {
		v.push_back(*c);
	}
	return v;
}

vector<char> ProtocolMessage::pack() {
	vector<char> v;	
	/*
	for (char* c = (char*)this + 4; c < (char*)this + sizeof(ProtocolMessage); c++) {
		v.push_back(*c);
	}
	*/
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
