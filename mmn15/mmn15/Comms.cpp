#include "Comms.h"
#include "Protocol.h"

#include <boost/asio.hpp>
#include <vector>
#include <iostream>

using namespace boost::asio::ip;
using boost::asio::ip::tcp;

ClientComms::ClientComms(const string &ip, const string &port) : endp(address::from_string(ip), stoi(port)), context(), sock(context) {}

int ClientComms::Connect() {
	sock.connect(endp);
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
	catch (const std::exception& error) {
		std::cerr << error.what() << std::endl;
	}
	std::cout << "Got " << read_bytes << " Bytes! " << " And buffer is of size " << buffer.size() << std::endl;
	return buffer;
}

int ClientComms::Close() {
	sock.close();
	return 0;
}
