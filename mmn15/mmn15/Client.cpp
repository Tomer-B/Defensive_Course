#include "Client.h"
#include "Comms.h"
#include "Protocol.h"
#include "Base64Wrapper.h"

#include <io.h>
#include <fstream>

using namespace std;

Client::Client(const string& ip, const string& port) : comm(ip, port), rsa_public(rsa_private.getPublicKey()) {}
    //getMyInfo(); }

int Client::printPrompt() {
    cout << "MessageU client at your service." << endl;
    cout << "10) Register" << endl;
    cout << "20) Request for clients list" << endl;
    cout << "30) Request for public key" << endl;
    cout << "40) Request for waiting messages" << endl;
    cout << "50) Send a text message" << endl;
    cout << "51) Send a request for symmetric key" << endl;
    cout << "52) Send your symmetric key" << endl;
    cout << "0) Exit client" << endl;
    return 0;
}

vector<char> Client::SendMessageAndExpectCode(ProtocolMessage *p, size_t ExpectedPayloadSize, unsigned short ExpectedCode) {
    vector<char> ServerResponse = { 0 };
    ServerResponseHeader* ServerResponseH;
    
    comm.SendMessage(p->pack());
    ServerResponse = comm.ReceiveMessage(sizeof(ServerResponseHeader) + ExpectedPayloadSize);
    ServerResponseH = (ServerResponseHeader*)&ServerResponse[0];

    if (ServerResponseH->Code != ExpectedCode) {
        cout << "Got Bad Response: " << ServerResponseH->Code << endl;
        throw BadResponseCodeError();
    }

    return vector<char>(ServerResponse.begin() + sizeof(ServerResponseHeader), ServerResponse.end());
}

int Client::registerClient() {
    int result = 0;
    RegisterPayload* r = NULL;
    ProtocolMessage* p = NULL;
    vector<char> PayLoadResponse = { 0 };

    if (_access(CLIENTINFO, 0) != -1) {
        cout << "File " << CLIENTINFO << " already exists!" << endl;
        VERIFY(-1);
    }
    comm.Connect();

    cout << "Enter Your name" << endl;
    cin >> ClientName;
    if (strlen(ClientName) > 255) {
        cout << "Client name too long." << endl;
        VERIFY(-1);
    }

    cout << "My Public Key: " << rsa_public.getPublicKey() << endl; // remove

    r = new RegisterPayload(ClientName, rsa_public.getPublicKey());
    p = new ProtocolMessage(ClientID, CLIENT_VERSION, REGISTER_REQUEST, sizeof(RegisterPayload), (Payload*)r);
    
    PayLoadResponse = SendMessageAndExpectCode(p, UUID_SIZE, REGISTRATION_SUCCESS_RESPONSE);
    memcpy(ClientID, &PayLoadResponse[0], UUID_SIZE);

    cout << "Writing info to file" << endl;
    WriteInfoToFile();

cleanup:
    if (r) {
        delete r;
    }
    if (p) {
        delete p;
    }
    if (comm.IsConnected()) {
        comm.Close();
    }
    return result;
}

int Client::getClientList() {
    cout << "Getting client list" << endl;
    comm.Connect();
    ProtocolMessage* p = new ProtocolMessage(ClientID, CLIENT_VERSION, CLIENT_LIST_REQUEST, 0, NULL);
    vector<char> PayLoadResponse = SendMessageAndExpectCode(p, MAX_PAYLOAD_SIZE, CLIENT_LIST_RESPONSE);
    RemoteClientResponse* c;
    
    cout << "Client Names:" << endl;
    for (int i = 0; i < (PayLoadResponse.size() / (UUID_SIZE + MAX_NAME_SIZE)) ; i++) {
        c = (RemoteClientResponse*)&PayLoadResponse[i * (UUID_SIZE + MAX_NAME_SIZE)];
        if (c->ClientName[0]) {
            cout << c->ClientName << endl;
            ClientsList.push_back(User(c->ClientID, c->ClientName));
        }
    }     

cleanup:
    delete p;
    comm.Close();
    return 0;
}

int Client::GetRemotePublicKey() {
    int result = 0;
    char RequestedClientName[MAX_NAME_SIZE];
    RequestClientPublicKey* r = NULL;
    ProtocolMessage* p = NULL;
    User* user = NULL;
    vector<char> PayLoadResponse;

    if (!ClientsList.size()) {
        cout << "Unknwon Client, request ClientList (20)" << endl;
        VERIFY(-1);
    }
    
    cout << "Input Requested Username: " << endl;
    cin >> RequestedClientName;
    user = GetUserByName(RequestedClientName);
    VERIFY(user == NULL);

    comm.Connect();
    r = new RequestClientPublicKey(user->ClientID);
    p = new ProtocolMessage(ClientID, CLIENT_VERSION, CLIENT_PUBLIC_KEY_REQUEST, sizeof(RequestClientPublicKey), (Payload*)r);
    PayLoadResponse = SendMessageAndExpectCode(p, UUID_SIZE + PUBLIC_KEY_SIZE, PUBLIC_KEY_RESPONSE);
    user->SetPublicKey(PayLoadResponse);
    cout << "Requested Public Key: " << user->PublicKey << endl;

cleanup:
    if (r) {
        delete r;
    }
    if (p) {
        delete p;
    }
    if (comm.IsConnected()) {
        comm.Close();
    }
    return result;
}

int Client::WriteInfoToFile() {
    ofstream ClientInfoFile(CLIENTINFO);
    Base64Wrapper base64;
    
    ClientInfoFile << ClientName << endl;
    for (unsigned char c : ClientID) {
        ClientInfoFile << int(c);
    }
    ClientInfoFile << endl;
    ClientInfoFile << base64.encode(rsa_private.getPrivateKey()) << endl;
    ClientInfoFile.close();
    return 0;
}

User* Client::GetUserByName(char RequestedClientName[MAX_NAME_SIZE]) {
    for (User &user : ClientsList) {
        if (strncmp(user.ClientName, RequestedClientName, MAX_NAME_SIZE) == 0) {
            return &user;
        }
    }
    cout << "Could not find such User. Try Requesting the client list (20)" << endl;
    return NULL;
}

User* Client::GetUserByID(char id[UUID_SIZE]) {
    for (User& user : ClientsList) {
        if (strncmp(user.ClientID, id, MAX_NAME_SIZE) == 0) {
            return &user;
        }
    }
    cout << "Could not find such User. Try Requesting the client list (20)" << endl;
    return NULL;
}

int Client::SendMessageToClient(size_t MessageType, User* user) {
    RequestSendMessageToClient* r = NULL;
    ProtocolMessage* p = NULL;
    vector<char> PayLoadResponse;
    string MessageContent;
    char RequestedClientName[MAX_NAME_SIZE];
    
    if (!user) {
        cout << "Input Requested Username: " << endl;
        cin >> RequestedClientName;
        user = GetUserByName(RequestedClientName);
    }

    comm.Connect();
    switch (MessageType) {
    case GET_SYMMETRIC_KEY_MSG_TYPE:
        r = new RequestSendMessageToClient(user->ClientID, MessageType, 0, "");
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient), (Payload*)r);
        break;
    case SEND_SYMMETRIC_KEY_MSG_TYPE:
        r = new RequestSendMessageToClient(user->ClientID, MessageType, SYMMETRIC_KEY_SIZE, string((char*)user->aes.getKey()));
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient), (Payload*)r);
        break;
    case SEND_TEXT_MSG_TYPE:
        if (!user->SymmetricKeySet) {
            cout << "No Symmetric key for that client!" << endl;
            throw NoSymmetricKeyError();
        }
        cout << "Input Message Content: " << endl;
        cin >> MessageContent;
        r = new RequestSendMessageToClient(user->ClientID, MessageType, MessageContent.size(), user->aes.encrypt(MessageContent.c_str(), MessageContent.size()));
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient), (Payload*)r);
        break;
    //case SEND_FILE_MSG_TYPE:
    //    break;
    default:
        cout << "No such message" << endl;
        return -1;
        break;
    }
    PayLoadResponse = SendMessageAndExpectCode(p, UUID_SIZE + sizeof(size_t), TEXT_MESSAGE_SENT_RESPONSE);
    // cout << "Sent message ID: " << 
cleanup:
    if (r) {
        delete r;
    }
    if (p) {
        delete p;
    }
    if (comm.IsConnected()) {
        comm.Close();
    }
    return 0;
}

int Client::ReceiveMessageFromClient() {
    RequestSendMessageToClient* r = NULL;
    ProtocolMessage* p = NULL;
    vector<char> PayLoadResponse;
    User* user;
    Message* CurrentMessage;
    bool finished_reading = false;
    int MessageIndex = 0;
    int result = 0;
    int DecryptionResult = 0;

    comm.Connect();
    p = new ProtocolMessage(ClientID, CLIENT_VERSION, GET_MESSAGES_REQUEST, 0, NULL);
    PayLoadResponse = SendMessageAndExpectCode(p, MAX_PAYLOAD_SIZE, 2004);
    while (!finished_reading) {
        if (PayLoadResponse[MessageIndex] == '\0') {
            cout << "No more messages" << endl;
            finished_reading = true;
        } else {
            CurrentMessage = (Message*)&PayLoadResponse[MessageIndex];
            user = GetUserByID(CurrentMessage->ClientID);
            VERIFY(user != NULL);
            DecryptionResult = user->DecryptAndDisplayMessage(CurrentMessage);
            if (DecryptionResult == SEND_SYMMETRIC_KEY) {
                SendMessageToClient(GET_SYMMETRIC_KEY_MSG_TYPE, user);
            }
            MessageIndex += CurrentMessage->MessageSize;
        }
    }

cleanup:
    if (p) {
        delete p;
    }
    if (comm.IsConnected()) {
        comm.Close();
    }
    return result;
}

int Client::start() {
    int option;
    while (1) {
        try {
            printPrompt();
            cin >> option;
            switch (option) {
            case 10:
                registerClient();
                break;
            case 20:
                getClientList();
                break;
            case 30:
                GetRemotePublicKey();
                break;
            case 40:
                ReceiveMessageFromClient();
                break;
            case 50:
                SendMessageToClient(SEND_TEXT_MSG_TYPE, NULL);
                break;
            case 51:
                SendMessageToClient(GET_SYMMETRIC_KEY_MSG_TYPE, NULL);
                break;
            case 52:
                SendMessageToClient(SEND_SYMMETRIC_KEY_MSG_TYPE, NULL);
                break;
            case 53:
                break;
            case 0:
                return 0;
            default:
                cout << "Bad command" << endl;
                break;
            }
        }
        catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
        }
    }
}
