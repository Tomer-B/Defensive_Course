#pragma once
#include "USocial.h"

USocial::~USocial() {
	map<unsigned long, User*>::iterator itr = users.begin();
	while (itr != users.end()) {
		delete (*itr).second;
		itr = users.erase(itr);
	}
}

User* USocial::registerUser(const string name, const bool business) {
	unsigned long id = maxId++;
	cout << "Registering user " << name << " with Id " << id << endl;
	if (users.size() > MAX_USERS) {
		throw tooManyUsers("T oo many users!\n");
	}
	return users[id] = business ? new BusinessUser(this, id, name) : new User(this, id, name);
}

void USocial::removeUser(User* user) {
	if (user->usocial != this) {
		throw userDoesNotBelong("User is not in this USocial!\n");
	}
	users.erase(user->getId());
	delete user;
}

User* USocial::getUserById(const unsigned long userId) const {
	cout << "Trying to find user with id " << userId << endl;
	cout << "Currently has: " << endl;
	for (auto item : users) {
		cout << item.second->id << endl;
	}
	if (users.find(userId) != users.end()) {
		cout << "Found user with id " << users.at(userId)->id << endl;
		return users.at(userId);
	}
	throw userDoesntExist("User doesn't exist!\n");
}	