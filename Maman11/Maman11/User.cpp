#include "User.h"
#include "USocial.h"
using namespace std;

User::~User() {
	for (auto post : posts) {
		delete post;
	}
	for (auto message : receivedMsgs) {
		delete message;
	}
	receivedMsgs.clear();
	for (auto friendId : friends) {
		cout << "User " << id << " removing friend " << friendId << endl;
		try {
			usocial->getUserById(friendId)->removeFriend(this);
		}
		catch (userDoesntExist) {};
	}
	friends.clear();
}

bool User::canSendMessage(const unsigned int sendId) const {
	return alreadyFriend(sendId);
}

bool User::alreadyFriend(const unsigned int friendId) const {
	cout << "User " << id << " Checking if " << friendId << " is a friend" << endl;
	for (auto existingId : friends) {
		cout << "Comparing " << friendId << " to " << existingId << endl;
		if (existingId == friendId) {
			cout << "They are!" << endl;
			return true;
		}
	}
	cout << "They aren't!" << endl;
	return false;
}

void User::addFriend(User* user) { 
	if (!alreadyFriend(user->id)) {
		cout << "User " << id << " adding as friend " << user->id << endl;
		friends.push_back(user->id);
	}
	else {
		throw alreadyFriends("Friend exists!\n");
	}
}

void User::removeFriend(User* user) { 
	cout << "User " << id << " Removing friend: " << user->id << endl;
	friends.remove(user->id);
}

void User::post(const string content) { 
	cout << "User " << id << " posting " << content << endl;
	posts.push_back(new Post(content));
}

void User::post(string content, Media* media) {
	cout << "User " << id << " posting (with media) " << content << endl;
	posts.push_back(new Post(content, media));
}

void User::viewPosts() const {
	Media* postMedia = nullptr;
	cout << "User " << id << " Posts: " << endl;
	for (auto post : posts) {
		cout << post->getText() << endl;
		postMedia = post->getMedia();
		if (postMedia) {
			postMedia->display();
		}
	}
}

void User::viewFriendsPosts() const {
	cout << "User " << id << " Friends Posts: " << endl;
	for (auto friendId : friends) {
		cout << "For friend " << friendId << endl;
		usocial->getUserById(friendId)->viewPosts();
	}
}

void User::receiveMessage(Message* message) { 
	cout << "User " << id << " received message" << endl;
	receivedMsgs.push_back(message);
}

void User::sendMessage(User* user, Message* message) {
	cout << "User " << id << " sending message to " << user->id << endl;
	if (canSendMessage(user->id)) {
		user->receiveMessage(message);
	} else {
		cout << "Not friends. deleting" << endl;
		throw messageCouldntBeSent("Can't send message to non-friend!\n");
		delete message;
		cout << "deleted" << endl;
		
	}
}

void User::viewReceivedMessages() { 
	cout << "User " << id << " Received messages: " << endl;
	for (auto msg : receivedMsgs) {
		cout << msg->getText() << endl;
	}
}