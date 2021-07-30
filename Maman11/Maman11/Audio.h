#pragma once 
#include "Media.h"
#include <iostream>
using namespace std;


class Audio : public Media {
public:
	void display() {
		cout << "Audio!\n";
	}
};
