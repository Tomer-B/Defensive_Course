#pragma once

#include <exception>
#include <iostream>

using namespace std;

#define VERIFY(exp)                                    \
	do {                                               \
		if (exp) {                                     \
			cout << "Got Error: " << result << endl;   \
			result = exp;                              \
			goto cleanup;                              \
		}                                              \
	} while (0)

class ArgumentTooLongError : exception {};
class ServerInfoReadError: exception {};
class BadResponseCodeError : exception {};
class NoSymmetricKeyError : exception {};
