#include "Client.h"
#include "file_utils.h"

using namespace std;

int main() {
    size_t pos = 0;
    string ip, port;

    try {
        string s = readServerData();
        pos = s.find(":");
        ip = s.substr(0, pos);
        port = s.substr(pos + 1, s.length());
        cout << "Got server info: " << ip << ":" << port << endl;
    }
    catch (ServerInfoReadError) {
        cout << "Failed to read Server Info File" << endl;
        return ServerInfoRead;
    }

	Client client(ip, port);
	return client.start();
}
