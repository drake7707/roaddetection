#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>

using namespace cv;

class MergedFrameProcessResult {

private:

	std::vector<AnnotatedImage> annotatedImages;
	std::vector<Mat> masks;
	Mat finalClassifiedMask;

public:

	std::vector<AnnotatedImage> getAnnotatedImages() const {
		return annotatedImages;
	}

	void addAnnotatedImage(AnnotatedImage& img) {
		this->annotatedImages.push_back(img);
	}

	std::vector<Mat> getClassifiedMasks() const {
		return this->masks;
	}

	void addClassifiedMask(Mat& classifiedMask) {
		this->masks.push_back(classifiedMask);
	}

	Mat getFinalClassifiedMask() const {
		return this->finalClassifiedMask;
	}

	void setFinalClassifiedMask(Mat& classifiedMask) {
		this->finalClassifiedMask = classifiedMask;
	}

};