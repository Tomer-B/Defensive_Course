#include "Client.h"
#include "Comms.h"
#include "Protocol.h"
#include "Crypto.h"

#include  <io.h>

using namespace std;

Client::Client(const string &ip, const string &port) : comm(ip, port), rsa_public(rsa_private.getPublicKey()) {}

int Client::printPrompt() {
    cout << "MessageU client at your service." << endl;
    cout << "10) Register" << endl;
    cout << "20) Request for clients list" << endl;
    cout << "30) Request for public key" << endl;
    cout << "40) Request for waiting messag0es" << endl;
    cout << "50) Send a text message" << endl;
    cout << "51) Send a request for symmetric key" << endl;
    cout << "52) Send your symmetric key" << endl;
    cout << "0) Exit client" << endl;
    return 0;
}

int Client::registerClient() {
    char ClientName[256] = "";
    vector<char> ServerResponse = { 0 };
    ServerResponseHeader *ServerResponseH;
    comm.Connect();

    if (_access(CLIENTINFO, 0) != -1) {
        cout << "File info.me already exists!" << endl;
        return -1;
    }

    cout << "Enter Your name" << endl;
    cin >> ClientName;
    if (strlen(ClientName) > 255) {
        cout << "Client name too long." << endl;
        return -1;
    }

    RegisterPayload *r = new RegisterPayload(ClientName, rsa_public.getPublicKey());
    ProtocolMessage *p = new ProtocolMessage(ClientID, 1, 1000, sizeof(RegisterPayload), (Payload*)r);
    comm.SendMessage(p->pack());

    ServerResponse = comm.ReceiveMessage(SERVER_RESPONSE_HEADER_SIZE + 16);
    std::cout << (char*)&ServerResponse[0] << std::endl;
    ServerResponseH = (ServerResponseHeader*)&ServerResponse[0];
    
    if (ServerResponseH->Code != 2000) {
        cout << "Registration Failed!" << endl;
        return -1;
    }

cleanup:
    delete r;
    delete p;
    comm.Close();
    return 0;
}

int Client::getClientList() {
    ProtocolMessage* p = new ProtocolMessage(ClientID, 1, 1001, 0, NULL);
    comm.SendMessage(p->pack());

cleanup:
    delete p;
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
        case 0:
            return 0;
        default:
            cout << "Bad command" << endl;
            break;
        }
    }
}
