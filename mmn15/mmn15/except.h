#pragma once

#include <exception>
#include <iostream>

using namespace std;

#define FAIL_AND_CLEAN(ErrorCode)                  \
	do {                                           \
		result = ErrorCode;                        \
		goto cleanup;                              \
	} while (0)

class NotAnIntegerError : public exception {};
class ArgumentTooLongError : public exception {};
class ServerInfoReadError: public exception {};
class BadResponseCodeError : public exception {};
class InvalidMeInfoFileError : public exception {};
class UserNotFoundError : public exception {};

#define NotAnInteger (1)
#define ArgumentTooLong (2)
#define ServerInfoRead (3)
#define BadResponseCode (4)
#define NoSymmetricKey (5)
#define InvalidMeInfoFile (6)
#define NoPublicKey (7)
#define ClientNameTooLong (8)
#define ClientInfoDoesNotExist (9)
#define UserNotFound (10)
