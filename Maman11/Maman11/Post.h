#pragma once
#include <iostream>
#include "Media.h"
using namespace std;

class Post {
	string text;
	Media* media;

public:
	Post(string text) : text(text), media(nullptr) {};
	Post(string text, Media* media) : text(text), media(media) {};
	Post(const Post&) = delete;
	Post(const Post&&) = delete;
	Post& operator=(Post const&) = delete;

	virtual ~Post() {
		delete media;
	}

	const string getText() const{
		return text;
	}

	Media* getMedia() const {
		return media;
	}
};