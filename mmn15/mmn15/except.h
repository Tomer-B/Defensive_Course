#pragma once

#include <exception>
using namespace std;

class ArgumentTooLongError : exception {};

class ServerInfoReadError: exception {};