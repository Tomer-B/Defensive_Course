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
	memset(SymmetricKey, 0, sizeof(SymmetricKey));
	memcpy(SymmetricKey, &key[0], sizeof(SymmetricKey));
	SymmetricKeySet = true;
	return 0;
}

int User::DecryptAndDisplayMessage(Message* message) {
	vector<char> key;
	cout << "Message From: " << ClientName << endl; // Make sure the correct user is called
	switch (message->MessageType) {
		case GET_SYMMETRIC_KEY_MSG_TYPE:
			cout << "Request for symmetric key" << endl;
			return SEND_SYMMETRIC_KEY;
		case SEND_SYMMETRIC_KEY_MSG_TYPE:
			cout << "Symmetric key received" << endl;
			key.assign(message->Content, message->Content + message->MessageSize);
			SetSymmetricKey(key);
			break;
		case SEND_TEXT_MSG_TYPE:
			if (!SymmetricKeySet) {
				cout << "Can't decrypt message" << endl;
				break;
			}
			cout << aes.decrypt(message->Content, message->MessageSize) << endl;
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
