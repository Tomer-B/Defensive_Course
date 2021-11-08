#include "Comms.h"
#include "Protocol.h"

#include <boost/asio.hpp>
#include <vector>
#include <iostream>

using namespace boost::asio::ip;
using boost::asio::ip::tcp;

ClientComms::ClientComms(const string& ip, const string& port) : endp(address::from_string(ip), stoi(port)), context(), sock(context), is_connected(false) {}

int ClientComms::Connect() {
	sock.connect(endp);
	is_connected = true;
	return 0;
}

int ClientComms::SendMessage(vector<char> bytes) {
	return sock.send(boost::asio::buffer(bytes));
}

vector<char> ClientComms::ReceiveMessage(size_t MessageSize) {
	vector<char> buffer(MessageSize);
	size_t read_bytes = 0;
	try {
		read_bytes = read(sock, boost::asio::buffer(buffer));
	}
	// This is okay since the server Disconnects after Request is finished
	catch (const exception& error) { 
		cerr << error.what() << endl;
	}
	return buffer;
}

int ClientComms::Close() {
	sock.close();
	is_connected = false;
	return 0;
}

bool ClientComms::IsConnected() {
	return is_connected;
}