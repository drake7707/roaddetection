#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "SetViewer.h"
#include "AbstractSetViewer.h"

using namespace std;
using namespace cv;


SetViewer::SetViewer(string& path) {
	this->path = path;

	if (path != "") {
		inSpeed.open(this->path + PATH_SEPARATOR + "actualspeeds.txt");
		if (!inSpeed.is_open())
			throw exception("Could not read actual speeds from set");

		inGtDistance.open(this->path + PATH_SEPARATOR + "gtdistances.txt");
		if (!inGtDistance.is_open())
			throw exception("Could not read gt distances from set");
	}

}

void SetViewer::next() {
	if (this->path == "") {
		Mat m = Mat(Size(1280, 720), CV_8UC3, Scalar(192,192,192));
		Mat mask = Mat(Size(1280, 720), CV_8UC4, Scalar(0));
		this->currentFrame = SetFrame("", 0, 0, m, mask, Mat());
		return;
	}

	string curFrame;
	double speed;
	inSpeed >> curFrame;
	inSpeed >> speed;

	string curFrameDistance;
	double gtDistance;
	inGtDistance >> curFrameDistance;
	inGtDistance >> gtDistance;

	if (curFrame != "" && curFrameDistance != "") {

		Mat frame = imread(this->path + PATH_SEPARATOR + "frame" + curFrameDistance + ".png");
		Mat mask = imread(this->path + PATH_SEPARATOR + "mask" + curFrameDistance + ".png");

		Mat gt = imread(this->path + PATH_SEPARATOR + "gt" + curFrameDistance + ".png");
		if (gt.rows > 0)
			cvtColor(gt, gt, CV_BGR2GRAY);

		this->currentFrame = SetFrame(curFrameDistance, speed, gtDistance, frame, mask, gt);
	}
	else
		this->currentFrame = SetFrame();
}