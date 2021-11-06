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
