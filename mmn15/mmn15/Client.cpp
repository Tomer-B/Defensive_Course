#include "Client.h"
#include "Comms.h"
#include "Protocol.h"
#include "Base64Wrapper.h"
#include "file_utils.h"

#include <io.h>
#include <fstream>

using namespace std;

Client::Client(const string& ip, const string& port) : comm(ip, port) {
    rsa_private = NULL;
    ReadInfoFile();
    if (!rsa_private) {
        rsa_private = new RSAPrivateWrapper();
    }
    rsa_public = new RSAPublicWrapper(rsa_private->getPublicKey());
}

Client::~Client() {
    if (rsa_public) {
        delete rsa_public;
    }
    if (rsa_private) {
        delete rsa_private;
    }
}


int Client::ReadInfoFile() {
    std::ifstream InfoFile(CLIENTINFO);
    std::vector<std::string> lines;

    if (_access(CLIENTINFO, 0) == -1) {
        cout << CLIENTINFO << " File does not exist, Must register to use UMessage!" << endl;
        return 0;
    }
    if (!InfoFile.is_open()) {
        throw InvalidMeInfoFileError();
    }
    for (std::string line; getline(InfoFile, line);) {
        lines.push_back(line);
    }
    InfoFile.close();

    if ((lines.size() < 3) || (lines[0].length() > MAX_NAME_SIZE)) {
        throw InvalidMeInfoFileError();
    }
    strncpy_s(ClientName, (char*)&lines[0], MAX_NAME_SIZE);
    
    lines[1] = ascii_to_hex(lines[1]);
    if (lines[1].length() != UUID_SIZE) {
        throw InvalidMeInfoFileError();
    }
    memcpy(ClientID, (char*)&lines[1][0], UUID_SIZE);

    string EncodedPrivateKey;
    for (auto line = lines.begin() + 2; line < lines.end(); line++) {
        EncodedPrivateKey += *line + "\n";
    }

    rsa_private = new RSAPrivateWrapper(Base64Wrapper::decode(EncodedPrivateKey));
}

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
    return vector<char>(ServerResponse.begin() + sizeof(ServerResponseHeader), ServerResponse.begin() + sizeof(ServerResponseHeader) + ServerResponseH->PayloadSize);
}

int Client::registerClient() {
    int result = 0;
    RegisterPayload* r = NULL;
    ProtocolMessage* p = NULL;
    vector<char> PayLoadResponse = { 0 };

    if (_access(CLIENTINFO, 0) != -1) {
        cout << "File " << CLIENTINFO << " already exists!" << endl;
        return -1;
    }
    comm.Connect();

    cout << "Enter Your name" << endl;
    cin >> ClientName;
    if (strlen(ClientName) > 255) {
        cout << "Client name too long." << endl;
        VERIFY(-1);
    }

    r = new RegisterPayload(ClientName, rsa_public->getPublicKey());
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
        goto cleanup;
    }
    
    cout << "Input Requested Username: " << endl;
    cin >> RequestedClientName;
    user = GetUserByName(RequestedClientName);
    VERIFY(user != NULL);

    comm.Connect();
    r = new RequestClientPublicKey(user->ClientID);
    p = new ProtocolMessage(ClientID, CLIENT_VERSION, CLIENT_PUBLIC_KEY_REQUEST, sizeof(RequestClientPublicKey), (Payload*)r);
    PayLoadResponse = SendMessageAndExpectCode(p, UUID_SIZE + PUBLIC_KEY_SIZE, PUBLIC_KEY_RESPONSE);
    user->SetPublicKey(vector<char>(PayLoadResponse.begin() + UUID_SIZE, PayLoadResponse.end()));
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
    ClientInfoFile << hex_to_ascii(ClientID, UUID_SIZE);
    ClientInfoFile << endl;
    ClientInfoFile << base64.encode(rsa_private->getPrivateKey()) << endl;
    ClientInfoFile.close();
    return 0;
}

User* Client::GetUserByName(char RequestedClientName[MAX_NAME_SIZE]) {
    for (User &user : ClientsList) {
        if (strncmp(user.ClientName, RequestedClientName, MAX_NAME_SIZE) == 0) {
            return &user;
        }
    }
    cout << "Could not find User by name. Try Requesting the client list (20) " << RequestedClientName << endl;
    return NULL;
}

User* Client::GetUserByID(char id[UUID_SIZE]) {
    for (User& user : ClientsList) {
        if (strncmp(user.ClientID, id, MAX_NAME_SIZE) == 0) {
            return &user;
        }
    }
    cout << "Could not find such User by id. Try Requesting the client list (20) " << id << endl;
    return NULL;
}

int Client::SendMessageToClient(size_t MessageType, User* user) {
    RequestSendMessageToClient* r = NULL;
    ProtocolMessage* p = NULL;
    RSAPublicWrapper* rsa = NULL;
    vector<char> PayLoadResponse;
    string MessageContent;
    char RequestedClientName[MAX_NAME_SIZE];
    
    
    string encrypted_key;
    string encrypted_message;
    char* symkey;
    
    if (!user) {
        cout << "Input Requested Username: " << endl;
        cin >> RequestedClientName;
        user = GetUserByName(RequestedClientName);
    }

    comm.Connect();
    switch (MessageType) {
    case GET_SYMMETRIC_KEY_MSG_TYPE:
        r = new RequestSendMessageToClient(user->ClientID, MessageType, 0, "");
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient)-sizeof(string)+r->Size, (Payload*)r);
        break;
    case SEND_SYMMETRIC_KEY_MSG_TYPE:
        if (!user->PublicKeySet) {
            cout << "No Public key for that client!" << endl;
            throw NoPublicKeyError();
        }

        cout << "Before wrapper" << endl;
        if (!user->SymmetricKeySet) {
            user->SymmetricKeySet = true;
        }
        cout << "new sym key " << (char*)user->aes.getKey() << endl;
        rsa = new RSAPublicWrapper(string(user->PublicKey, RSAPublicWrapper::KEYSIZE));
        encrypted_key = rsa->encrypt((char*)user->aes.getKey(), SYMMETRIC_KEY_SIZE);

        r = new RequestSendMessageToClient(user->ClientID, MessageType, encrypted_key.size(), encrypted_key);
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient) - sizeof(string) + r->Size, (Payload*)r);
        cout << "after message" << endl;
        break;
    case SEND_TEXT_MSG_TYPE:
        if (!user->SymmetricKeySet) {
            cout << "No Symmetric key for that client!" << endl;
            goto cleanup;
        }
        cout << "Input Message Content: " << endl;
        cin >> MessageContent;
        encrypted_message = user->aes.encrypt(MessageContent.c_str(), MessageContent.size());
        r = new RequestSendMessageToClient(user->ClientID, MessageType, encrypted_message.size(), encrypted_message);
        p = new ProtocolMessage(ClientID, CLIENT_VERSION, SEND_MESSAGE_REQUEST, sizeof(RequestSendMessageToClient) - sizeof(string) + r->Size, (Payload*)r);
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
    if (rsa) {
        delete rsa;
    }
    if (comm.IsConnected()) {
        comm.Close();
    }
    return 0;
}

int Client::DecryptAndDisplayMessage(Message* message, User* src_user) {
    vector<char> key;
    string decrypted_message;
    string crypted_message;

    cout << "Message From: " << src_user->ClientName << endl; // Make sure the correct user is called
    switch (message->MessageType) {
    case GET_SYMMETRIC_KEY_MSG_TYPE:
        cout << "Request for symmetric key" << endl;
        return SEND_SYMMETRIC_KEY;
    case SEND_SYMMETRIC_KEY_MSG_TYPE:
        cout << "Symmetric key received: " << message->MessageSize << endl;
        for (int i = 0; i < message->MessageSize; i++) {
            key.push_back(message->Content[i]);
        }
        crypted_message = string(key.begin(), key.end());
        decrypted_message = rsa_private->decrypt(crypted_message);
        src_user->SetSymmetricKey(vector<char>(decrypted_message.begin(), decrypted_message.end()));
        break;
    case SEND_TEXT_MSG_TYPE:
        if (!src_user->SymmetricKeySet) {
            cout << "Can't decrypt message" << endl;
            break;
        }
        cout << src_user->aes.decrypt(message->Content, message->MessageSize) << endl;
        break;
        //case SEND_FILE_MSG_TYPE: 
            //break;
    default:
        cout << "Message Type " << message->MessageType << " Unrecognized" << endl;
        break;
    }
    cout << "-----<EOM>-----" << endl;
    cout << endl;
    return 0;
}

int Client::ReceiveMessageFromClient() {
    RequestSendMessageToClient* r = NULL;
    ProtocolMessage* p = NULL;
    vector<char> PayLoadResponse;
    User* user;
    Message* CurrentMessage;
    vector<char> MessageContent;
    bool finished_reading = false;
    int MessageIndex = 0;
    int result = 0;
    int DecryptionResult = 0;

    comm.Connect();
    p = new ProtocolMessage(ClientID, CLIENT_VERSION, GET_MESSAGES_REQUEST, 0, NULL);
    cout << "Total length: " << p->PayloadSize << endl;
    PayLoadResponse = SendMessageAndExpectCode(p, MAX_PAYLOAD_SIZE, TEXT_MESSAGE_RECEIVED_RESPONSE);
    while (!finished_reading) {
        MessageContent.clear();
        if (MessageIndex >= PayLoadResponse.size()) {
            cout << "No more messages" << endl;
            finished_reading = true;
        } else {
            CurrentMessage = (Message*)&PayLoadResponse[MessageIndex];
            cout << "Getting content" << endl;
            for (int i = 0; i < CurrentMessage->MessageSize; i++) {
                MessageContent.push_back(PayLoadResponse[MessageIndex + 25 + i]);
            }
            cout << "Done pushing" << endl;
            if (CurrentMessage->MessageSize != 0) {
                CurrentMessage->Content = &MessageContent[0];
            }
            cout << "Checking src user" << endl;
            user = GetUserByID(CurrentMessage->ClientID);
            VERIFY(user != NULL);
            DecryptionResult = DecryptAndDisplayMessage(CurrentMessage, user);
            if (DecryptionResult == SEND_SYMMETRIC_KEY) {
                if (comm.IsConnected()) {
                    comm.Close();
                }
                SendMessageToClient(SEND_SYMMETRIC_KEY_MSG_TYPE, user);
            }
            MessageIndex += CurrentMessage->MessageSize + 25;
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
