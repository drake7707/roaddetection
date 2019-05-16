#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "SetFrame.h"

using namespace std;
using namespace cv;

#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 

class AbstractSetViewer {

protected:

	
	SetFrame currentFrame;
	string path;


public:

	AbstractSetViewer() : path("") {

	}

	AbstractSetViewer(string& path) : path(path) {
		
	}

	
	virtual void next() = 0;

	SetFrame getCurrentFrame() const {
		return this->currentFrame;
	}

	string getPath() const {
		return this->path;
	}

	AbstractSetViewer& operator=(AbstractSetViewer&& other) {
		path = std::move(other.path);
		return *this;
	}

	virtual ~AbstractSetViewer() {
		
	}
};
