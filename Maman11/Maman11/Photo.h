#pragma once
#include <iostream>
#include "Media.h"
using namespace std;


class Photo : public Media {
public:
	void display() {
		cout << "Photo!\n";
	}
};