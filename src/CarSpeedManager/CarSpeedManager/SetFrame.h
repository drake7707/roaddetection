#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

class SetFrame {

private:
	string number;
	Mat frame;
	Mat mask;
	Mat groundTruth;

	double actualSpeed;
	double gtDistance;

public:

	SetFrame() {

	}

	SetFrame(string number, double speed, double gtDistance, Mat& frame, Mat& mask, Mat& groundTruth) : number(number), frame(frame), mask(mask), actualSpeed(speed), gtDistance(gtDistance), groundTruth(groundTruth) {
	}

	const string& getNumber() const {
		return this->number;
	}

	const Mat& getFrame() const {
		return this->frame;
	}

	const Mat& getMask() const {
		return this->mask;
	}

	const Mat& getGroundTruth() const {
		return this->groundTruth;
	}

	double getActualSpeed() const {
		return this->actualSpeed;
	}

	double getGTDistance() const {
		return this->gtDistance;
	}
};