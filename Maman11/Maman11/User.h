#pragma once
#include <iostream>
#include <list>
#include "Post.h"
#include "Message.h"
#include "Exception.h"
#include "Media.h"
using namespace std;

class USocial;

class User {
protected:
	USocial *usocial;
	unsigned long id;
	string name;
	list<unsigned long> friends;
	list<Post*> posts;
	list<Message*> receivedMsgs;
	User() = delete;
	User(const User&) = delete;
	User(const User&&) = delete;
	User& operator=(User const&) = delete;
	virtual ~User();
	User(USocial* usocial, unsigned long id, string name) : usocial(usocial), id(id), name(name) {};
	friend class USocial;
private:
	bool alreadyFriend(const unsigned int friendId) const;
	void viewPosts() const;
	virtual bool canSendMessage(const unsigned int sendId) const;
public:
	unsigned long getId() {
		return id;
	}

	string getName() {
		return name;
	}

	list<Post*> getPosts() {
		return posts;
	}

	void addFriend(User* user);
	void removeFriend(User* user);
	void post(const string content);
	void post(const string content, Media* media);
	void viewFriendsPosts() const;
	void receiveMessage(Message* message);
	void sendMessage(User* user, Message* message);
	void viewReceivedMessages();
};