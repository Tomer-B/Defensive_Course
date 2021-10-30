#pragma once
#include <iostream>
#include <rsa.h>
#include <cryptlib.h>
#include <osrng.h>

#define PublicKeySize (160)

using namespace CryptoPP;

void create_private_public_pair() {
	AutoSeededRandomPool rng;
	RSA::PrivateKey privateKey;
	privateKey.GenerateRandomWithKeySize(rng, PublicKeySize * 8);
	RSA::PublicKey publicKey(privateKey);
}

