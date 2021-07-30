#pragma once
#include "USocial.h"
#include "User.h"
#include "Message.h"

class BusinessUser : public User {
protected:
	BusinessUser(USocial* usocial, unsigned long id, string name) : User(usocial, id, name) {};
	friend class USocial;
public:
	BusinessUser() = delete;
	virtual ~BusinessUser() {}
private: 
	virtual bool canSendMessage(const unsigned int sendId) const override {
		return true;
	}
};