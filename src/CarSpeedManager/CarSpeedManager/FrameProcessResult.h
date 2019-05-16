#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "AnnotatedImage.h"

using namespace cv;

class FrameProcessResult {

private:

	AnnotatedImage annotatedImage;
	Mat classifiedMask;

public:

	FrameProcessResult() {

	}

	AnnotatedImage getAnnotatedImage() const {
		return this->annotatedImage;
	}

	void setAnnotatedImage(AnnotatedImage& img) {
		this->annotatedImage = img;
	}

	Mat getClassifiedMask() const {
		return this->classifiedMask;
	}

	void setClassifiedMask(Mat& classifiedMask) {
		this->classifiedMask = classifiedMask;
	}
};