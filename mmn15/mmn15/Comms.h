#pragma once
#include "Protocol.h"
#include <boost/asio.hpp>

class ClientComms {
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket sock;
	boost::asio::ip::tcp::endpoint endp;
	bool is_connected;

public:
	ClientComms(const string& ip, const string& port);
	bool IsConnected();
	int Connect();
	int SendMessage(vector<char> bytes);
	vector<char> ReceiveMessage(size_t MessageSize);
	int Close();
};
