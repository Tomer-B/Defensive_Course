#pragma once 
#include "Media.h"
#include <iostream>
using namespace std;


class Video : public Media {
public:
	void dispaly() {
		cout << "Video!\n";
	}
};