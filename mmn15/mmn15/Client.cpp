#include "Client.h"
#include "Comms.h"
#include "Protocol.h"
#include "Crypto.h"
#include "Base64Wrapper.h"

#include  <io.h>
#include <fstream>

using namespace std;

Client::Client(const string &ip, const string &port) : comm(ip, port), rsa_public(rsa_private.getPublicKey()) {}

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
    comm.Connect();
    ProtocolMessage* p = new ProtocolMessage(ClientID, CLIENT_VERSION, CLIENT_LIST_REQUEST, 0, NULL);
    vector<char> PayLoadResponse = SendMessageAndExpectCode(p, MAX_PAYLOAD_SIZE, CLIENT_LIST_RESPONSE);
    RemoteClient* c;
    cout << "Client Names:" << endl;
    for (int i = 0; i < (PayLoadResponse.size() / (UUID_SIZE + MAX_NAME_SIZE)) ; i++) {
        c = (RemoteClient*)&PayLoadResponse[i * (UUID_SIZE + MAX_NAME_SIZE)];
        if (c->ClientName[0]) {
            cout << c->ClientName << endl;
        }
    }     

cleanup:
    delete p;
    comm.Close();
    return 0;
}

string Client::GetRemotePublicKey(char RemoteClientUUID[16]) {
    RequestClientPublicKey* r = new RequestClientPublicKey(RemoteClientUUID);
    ProtocolMessage* p = new ProtocolMessage(ClientID, CLIENT_VERSION, CLIENT_LIST_REQUEST, sizeof(RegisterPayload), (Payload*)r);
    vector<char> PayLoadResponse = SendMessageAndExpectCode(p, UUID_SIZE + PUBLIC_KEY_SIZE, PUBLIC_KEY_RESPONSE);
    return string(PayLoadResponse.begin() + UUID_SIZE, PayLoadResponse.end());

cleanup:
    delete r;
    delete p;
    return 0;
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

int Client::start() {
    int option;
    while (1) {
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
            //GetRemotePublicKey();
            break;
        case 40:
            break;
        case 50:
            break;
        case 51:
            break;
        case 52:
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
}
