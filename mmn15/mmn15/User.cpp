#include "User.h"

User::User(char id[UUID_SIZE], char name[MAX_NAME_SIZE])  {
	memset(ClientID, 0, sizeof(ClientID));
	memcpy(ClientID, id, sizeof(ClientID));
	memset(ClientName, 0, sizeof(ClientName));
	memcpy(ClientName, name, sizeof(ClientName));
	PublicKeySet = false;
};

int User::SetPublicKey(vector<char> key) {
	memset(PublicKey, 0, sizeof(PublicKey));
	memcpy(PublicKey, &key[0], sizeof(PublicKey));
	PublicKeySet = true;
	return 0;
}
