#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "AbstractSetViewer.h"
#include "VideoSetViewer.h"

using namespace std;
using namespace cv;


VideoSetViewer::VideoSetViewer(string& path) : AbstractSetViewer(path), cap(path, CAP_FFMPEG) {
	
	cap.set(CV_CAP_PROP_POS_MSEC, 1200000);
}

void VideoSetViewer::next() {

	if (this->cap.isOpened()) {
		Mat img;
		cap.read(img);

		this->currentFrame = SetFrame("",0, 0, img, Mat(), Mat());

		// skip frames to next second
		auto curMsec = cap.get(CV_CAP_PROP_POS_MSEC);
		cap.set(CV_CAP_PROP_POS_MSEC, curMsec + 1000);
	}
}