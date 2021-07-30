#pragma once
#include <stdexcept>
#include <iostream>
using namespace std;

class alreadyFriends : public logic_error {
public:
	alreadyFriends(string error) : logic_error(error) {};
};

class messageCouldntBeSent : public logic_error {
public:
	messageCouldntBeSent(string error) : logic_error(error) {};
};

class userDoesNotBelong : public logic_error {
public:
	userDoesNotBelong(string error) : logic_error(error) {};
};

class tooManyUsers : public logic_error {
public:
	tooManyUsers(string error) : logic_error(error) {};
};

class userDoesntExist : public logic_error {
public:
	userDoesntExist(string error) : logic_error(error) {};
};
