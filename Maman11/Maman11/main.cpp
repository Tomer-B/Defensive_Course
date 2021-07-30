#include <iostream>
using namespace std;
#include "User.h"
#include "USocial.h"
#include "Audio.h"
#include "Photo.h"
#include "Video.h"
#include "Message.h"
#include "BusinessUser.h"
#include "Exception.h"

int test1() {
    USocial us;
    User* u1 = us.registerUser("Liron");
    User* u2 = us.registerUser("Yahav");
    User* u3 = us.registerUser("Shachaf");
    User* u4 = us.registerUser("Tsur", true);
    User* u5 = us.registerUser("Elit");
    u1->post("Hello world!");
    u2->post("I'm having a great time here :)", new Audio());
    u3->post("This is awesome!", new Photo());
    u5->addFriend(u1);
    u5->addFriend(u2);
    u5->viewFriendsPosts(); // should see only u1, u2 s' posts
    u4->sendMessage(u5, new Message("Buy Falafel!"));
    u5->viewReceivedMessages();

    try {
        u3->sendMessage(u5, new Message("All your base are belong to us"));
    }
    catch (const std::exception& e) {
        std::cout << "error: " << e.what() << std::endl;
    }

    u5->viewReceivedMessages();
    u3->addFriend(u5);
    u3->sendMessage(u5, new Message("All your base are belong to us"));
    u5->viewReceivedMessages();
    return 0;
}

int test2() {
    USocial us;
    User* u1 = us.registerUser("Liron");
    User* u6 = us.registerUser("Moshe");
    try {
        u6->addFriend(u1);
        u6->addFriend(u1);
    }
    catch (alreadyFriends) {
        cout << "cannot add friend twice!" << endl;
    }
    u6->removeFriend(u1);
    try {
        u6->sendMessage(u1, new Message("I'm not your friend"));
    }
    catch (messageCouldntBeSent) {
        cout << "Can only send messages to friends!" << endl;
    }
    us.removeUser(u6);
    u6 = us.registerUser("a", true);
    u6->sendMessage(u1, new Message("I'm not your friend"));
    return 0;
}

int test3() {
    USocial us;
    User* u1 = us.registerUser("Liron"); 
    USocial us2;
    try {
        us2.removeUser(u1);
    }
    catch (userDoesNotBelong) {
        cout << "Users can only be deleted from their own USocial!" << endl;
    }
    return 0;
}

int test4() {
    USocial us;
    try {
        us.getUserById(500);
    }
    catch (userDoesntExist) {
        cout << "There is no such user with such id!" << endl;
    }
    return 0;
}

int main() {
    test1();
    test2();
    test3();
    test4();
    
    return 0;
}