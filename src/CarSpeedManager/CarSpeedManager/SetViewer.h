#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "SetFrame.h"
#include "AbstractSetViewer.h"

using namespace std;
using namespace cv;

#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 

class SetViewer : public AbstractSetViewer {

private:

	ifstream inSpeed;
	ifstream inGtDistance;

public:

	SetViewer(string& path);

	virtual void next();

	SetViewer& operator=(SetViewer&& other) {
		path = std::move(other.path);
		inSpeed = std::move(other.inSpeed);
		inGtDistance = std::move(other.inGtDistance);
		currentFrame = std::move(other.currentFrame);

		return *this;
	}

	virtual ~SetViewer() {
		inSpeed.close();
		inGtDistance.close();
	}
};
