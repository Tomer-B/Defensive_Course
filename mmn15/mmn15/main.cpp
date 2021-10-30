#include "Client.h"
#include "file_utils.h"

using namespace std;

int main() {
    size_t pos = 0;
    string ip, port;

    string s = readServerData();
    pos = s.find(":");
    ip = s.substr(0, pos);
    port = s.substr(pos+1, s.length());

    cout << "Got server info: " << ip << ":" << port << endl;

	Client client(ip, port);
	return client.start();
}
