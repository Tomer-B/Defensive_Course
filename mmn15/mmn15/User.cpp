#include "User.h"

User::User(char id[UUID_SIZE], char name[MAX_NAME_SIZE]) {
	memset(ClientID, 0, sizeof(ClientID));
	memcpy(ClientID, id, sizeof(ClientID));
	memset(ClientName, 0, sizeof(ClientName));
	memcpy(ClientName, name, sizeof(ClientName));
	PublicKeySet = false;
	SymmetricKeySet = false;
};

int User::SetPublicKey(vector<char> key) {
	memset(PublicKey, 0, sizeof(PublicKey));
	memcpy(PublicKey, &key[0], sizeof(PublicKey));
	PublicKeySet = true;
	return 0;
}

int User::SetSymmetricKey(vector<char> key) {
	aes.setKey((unsigned char *)&key[0], AESWrapper::DEFAULT_KEYLENGTH);
	SymmetricKeySet = true;
	return 0;
}
/*
int User::DecryptAndDisplayMessage(Message* message) {
	vector<char> key;
	cout << "Message From: " << this->ClientName << endl; // Make sure the correct user is called
	switch (message->MessageType) {
		case GET_SYMMETRIC_KEY_MSG_TYPE:
			cout << "Request for symmetric key" << endl;
			return SEND_SYMMETRIC_KEY;
		case SEND_SYMMETRIC_KEY_MSG_TYPE:
			cout << "Symmetric key received: " << message->MessageSize  << endl;
			for (int i = 0; i < message->MessageSize; i++) {
				key.push_back(message->Content[i]);
			}
			this->SetSymmetricKey(key);
			break;
		case SEND_TEXT_MSG_TYPE:
			if (!this->SymmetricKeySet) {
				cout << "Can't decrypt message" << endl;
				break;
			}
			cout << this->aes.decrypt(message->Content, message->MessageSize) << endl;
			break;
			//case SEND_FILE_MSG_TYPE: 
				//break;
		default:
			cout << "Message Type " << message->MessageType << " Unrecognized" << endl;
			break;
	}
	cout << "-----<EOM>-----" << endl;
	cout << endl;
	return 0;
}
*/