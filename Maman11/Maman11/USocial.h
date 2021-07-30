#pragma once
#include <map>
#include "User.h"
#include "BusinessUser.h"
#include "Exception.h"
using namespace std;
#define MAX_USERS (10)

class User;

class USocial {
	unsigned long maxId;
	map<unsigned long, User*> users;
public:
	User* registerUser(const string name, bool business = false);
	void removeUser(User* user);
	User* getUserById(const unsigned long userId) const;
	USocial() : maxId(1) {};
	USocial(const USocial&) = delete;
	USocial(const USocial&&) = delete;
	USocial& operator=(USocial const&) = delete;
	virtual ~USocial();
};