#pragma once
#include <iostream>
using std::string;

class Message {
	string text;
public:
	Message() = delete;
	Message(const string text) : text(text) {};
	string getText() {
		return text;
	}
};