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

class VideoSetViewer : public AbstractSetViewer {

private:
	cv::VideoCapture cap;
	

public:

	VideoSetViewer(string& path);

	virtual void next();
};
