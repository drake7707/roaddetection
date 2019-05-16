#pragma once

#include "SetViewer.h"
#include "SetFrame.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "SetForm.h"
#include "FrameProcessor.h"
#include "MergedFrameProcessResult.h"
#include "Helper.h"

using namespace std;
using namespace cv;

void SetForm::initializeControls() {
	/*createTrackbar("[Lane] Canny lower", "Set", &(this->options.laneDetector.cannyLowerTreshold), 100, [](int val, void* userdata) -> void {
		((SetForm*)userdata)->updateProcessResult();
		((SetForm*)userdata)->updateDisplay();
	}, this);

	createTrackbar("[Lane] Canny upper", "Set", &(this->options.laneDetector.cannyUpperTreshold), 500, [](int val, void* userdata) -> void {
		((SetForm*)userdata)->updateProcessResult();
		((SetForm*)userdata)->updateDisplay();
	}, this);*/
	/*
	createTrackbar("[Road] LBP minDiff", "Set", &(this->options.roadTextureDetector.lbpMinDiff), 100, [](int val, void* userdata) -> void {
		((SetForm*)userdata)->updateProcessResult();
		((SetForm*)userdata)->updateDisplay();
	}, this);

	createTrackbar("[Road] LBP nrOfHistogramBins", "Set", &(this->options.roadTextureDetector.nrOfHistogramBins), 100, [](int val, void* userdata) -> void {
		((SetForm*)userdata)->updateProcessResult();
		((SetForm*)userdata)->updateDisplay();
	}, this);


	*/
}

void SetForm::messagePump() {
	bool stop = false;
	while (!stop) {
		auto key = (waitKey(0) & 0xFF);
		if (key == 'q') {
			stop = true;
		}
		else if (key == 't') {
			currentProcessorIndex = (currentProcessorIndex + 1) % (this->processedResult.getAnnotatedImages().size() + 1);
			if (currentProcessorIndex == 0)
				showAnnotatedFrame = false;
			else {
				showAnnotatedFrame = true;
			}

			updateDisplay();
		}
		else if (key == 'n') {
			nextFrame();
			updateDisplay();
		}
		else if (key == 'g') {
			showGroundTruth = !showGroundTruth;
			updateDisplay();
		}
		else if (key == 'a') {
			showInfo = !showInfo;
			updateDisplay();
		}
		else if (key == 'c') {
			showClassifier = !showClassifier;
			updateDisplay();
		}
	}
}

void SetForm::nextFrame() {
	set->next();
	updateProcessResult();
}

void SetForm::updateProcessResult() {
	Mat frame = set->getCurrentFrame().getFrame();
	Mat mask = set->getCurrentFrame().getMask();
	if (frame.cols > 0) {
		this->processedResult = processFrame(frame, mask, this->options, true, true);

		// als enabled/disabled verandert verandert ook het aantal annotated images, update de huidige view
		if (currentProcessorIndex > processedResult.getAnnotatedImages().size()) {
			currentProcessorIndex = 0;
			showAnnotatedFrame = false;
		}

		updateROC();
		drawROC();
	}
}

void SetForm::onClick(int x, int y) {
	if (currentProcessorIndex < processedResult.getAnnotatedImages().size()) {
		if (this->currentProcessorIndex != 0 && processedResult.getAnnotatedImages()[currentProcessorIndex - 1].onClick != nullptr) {
			processedResult.getAnnotatedImages()[currentProcessorIndex - 1].onClick(x, y);
		}
	}
}


void SetForm::updateROC() {


	//Mat freeRoadMask;
	//Mat roadMask = set->getCurrentFrame().getMask();
	//createFreeRoadMask(roadMask, processedResult.getFinalClassifiedMask(), freeRoadMask);

	//int maxOnMask = 0;
	//for (int j = 0; j < roadMask.rows; j++)
	//{
	//	for (int i = 0; i < roadMask.cols; i++)
	//	{
	//		auto val = roadMask.at<uchar>(Point(i, j));
	//		if (maxOnMask < val) maxOnMask = val;
	//	}
	//}

	//double maxSpeed;
	//int maxRow;
	//calculateMaxSpeed(freeRoadMask, roadMask, maxSpeed, maxRow);


	//if (maxSpeed > set->getCurrentFrame().getGTDistance()) {
	//	//                     gt       max Sp               max mask
	//	//   rrrrrrrrrrrrrrrrrr | nnnnnnnn | nnnnnnnnnnnnnnnn |
	//	//         TP               FN          TN

	//	// everything up to the ground truth is a true positive
	//	nrTP += set->getCurrentFrame().getGTDistance();
	//	nrFP += 0;
	//	nrTN += 0;// maxOnMask - maxSpeed;
	//	// everything higher than the ground truth is a false negative
	//	nrFN += maxSpeed - set->getCurrentFrame().getGTDistance();

	//	nrP += set->getCurrentFrame().getGTDistance();
	//	nrN += maxOnMask - set->getCurrentFrame().getGTDistance();
	//}
	//else {
	//	//                     maxSp      gt               max mask
	//	//   rrrrrrrrrrrrrrrrrr | rrrrrrr | nnnnnnnnnnnnnnnn |
	//	//         TP               FP          TN
	//	nrTP = maxSpeed;
	//	// everything after the calculated max speed up to the ground truth is a false positive
	//	nrFP += set->getCurrentFrame().getGTDistance() - maxSpeed;
	//	nrTN += 0;// maxOnMask - set->getCurrentFrame().getGTDistance();
	//	nrFN += 0;

	//	nrP += set->getCurrentFrame().getGTDistance();
	//	nrN += maxOnMask - set->getCurrentFrame().getGTDistance();
	//}

	auto classifier = processedResult.getFinalClassifiedMask();
	auto groundTruth = set->getCurrentFrame().getGroundTruth();

	if (groundTruth.rows > 0) {
		resetROC();
		for (int j = 0; j < groundTruth.rows; j++)
		{
			for (int i = 0; i < groundTruth.cols; i++)
			{
				uchar gtval = groundTruth.at < uchar>(j, i);
				uchar classval = classifier.at<uchar>(j, i);

				if (gtval > 0 && classval == 0) // missed one
					nrFN++;
				else if (gtval == 0 && classval > 0) // incorrectly detected it
					nrFP++;
				else if (gtval > 0 && classval > 0)
					nrTP++;
				else if (gtval == 0 && classval == 0)
					nrTN++;

				if (gtval > 0) nrP++; else nrN++;
			}
		}
	}

	double recall = nrTP / (float)(nrP);
	double precision = nrTP / (float)(nrTP + nrFP);

	double f1Score = 2 * (precision * recall) / (float)(precision + recall);

	double TPR = nrTP / (float)nrP;
	double FPR = nrFP / (float)(nrN);


	updateROC(TPR, FPR);
}


void SetForm::updateDisplay() const {

	Mat output;
	if (showAnnotatedFrame) {
		// currentAnnotatedImage gaat van 0 - size (incl) met 0 = niet tonen, dus -1
		output = processedResult.getAnnotatedImages()[currentProcessorIndex - 1].image.clone();
	}
	else {
		Mat frame = set->getCurrentFrame().getFrame();
		output = frame.clone();

		if (frame.cols <= 0)
			return;
	}


	Mat road_mask = set->getCurrentFrame().getMask();
	Mat finalMask = processedResult.getFinalClassifiedMask();


	Mat freeRoadMask;
	createFreeRoadMask(road_mask, finalMask, freeRoadMask);

	double maxSpeed;
	int maxRow;
	calculateMaxSpeed(freeRoadMask, road_mask, maxSpeed, maxRow);

	cvtColor(freeRoadMask, freeRoadMask, CV_GRAY2BGR);
	if (showInfo)
		output = output + freeRoadMask;

	if (!showAnnotatedFrame && showInfo)
		line(output, Point(0, maxRow), Point(road_mask.cols - 1, maxRow), Scalar(0, 0, 255), 1);

	double gtDistance = set->getCurrentFrame().getGTDistance();
	double speed = set->getCurrentFrame().getActualSpeed();



	if (showGroundTruth && set->getCurrentFrame().getGroundTruth().rows > 0) {
		output = set->getCurrentFrame().getGroundTruth();
		cvtColor(output, output, CV_GRAY2BGR);
	}

	if (showClassifier) {
		Mat classMask;
		if (currentProcessorIndex == 0) {
			// toon totaal class mask
			classMask = processedResult.getFinalClassifiedMask().clone();
		}
		else {
			// toon enkel class mask van huidige frame processor
			classMask = processedResult.getClassifiedMasks()[currentProcessorIndex - 1].clone();
		}

		cvtColor(classMask, classMask, CV_GRAY2BGR);
		Mat channels[3];
		cv::split(classMask, channels);
		vector<Mat> vChannels;
		vChannels.push_back(Mat::zeros(Size(classMask.cols, classMask.rows), CV_8UC1));
		vChannels.push_back(channels[1]);
		vChannels.push_back(Mat::zeros(Size(classMask.cols, classMask.rows), CV_8UC1));
		cv::merge(vChannels, classMask);
		output = output * 0.7 + classMask *0.3;
	}

	if (showInfo) {
		string text;
		if (set->getPath() == "")
			text = "No set loaded";
		else
			text = set->getCurrentFrame().getNumber() + " - " + "Actual speed: " + std::to_string(speed) + " -- Gt Distance: " + std::to_string(gtDistance) + " -- Calculated speed: " + std::to_string(maxSpeed);

		if (maxSpeed > gtDistance)
			putText(output, text, Point(0, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 2);
		else
			putText(output, text, Point(0, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 0, 0), 1);
	}

	cv::imshow("Set", output);
}


void SetForm::updateROC(double TPR, double FPR) {
	this->rocPoints.push_back(Point2f((float)FPR, (float)TPR));
	if (this->rocPoints.size() > 100)
		this->rocPoints.pop_front();
}

void SetForm::drawROC() {
	rocCurve = Mat(300, 300, CV_8UC1, Scalar(255));
	for (int j = 0; j < rocCurve.rows; j++)
		rocCurve.at<uchar>(j, rocCurve.rows - 1 - j) = 128;

	// teken grid
	int cellSize = rocCurve.rows / 10;
	for (int j = 0; j < rocCurve.rows; j += cellSize) {
		line(rocCurve, Point(0, j), Point(rocCurve.cols - 1, j), Scalar(200));
		line(rocCurve, Point(j, 0), Point(j, rocCurve.rows - 1), Scalar(200));
	}

	// teken alle punten
	int cur = 0;
	for (auto p : rocPoints) {

		int x = (int)(p.x * rocCurve.cols);
		int y = rocCurve.rows - 1 - (int)(p.y * rocCurve.rows);

		if (cur == rocPoints.size() - 1) {
			float alpha = cur / (float)rocPoints.size();
			circle(rocCurve, Point(x, y), 3, Scalar(0, 0, 0), -1);
		}
		else {
			float alpha = cur / (float)rocPoints.size();
			circle(rocCurve, Point(x, y), 3, Scalar(50 + (1 - alpha) * 100), -1);
		}
		cur++;
	}


	Point2f lastPoint = rocPoints.back();
	string text = "FPR: " + to_string((int)(lastPoint.x * 100)) + "% - TPR: " + to_string((int)(lastPoint.y * 100)) + "%";

	putText(rocCurve, text, Point(0, 10), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 0, 0), 1);

	imshow("ROC", rocCurve);
}
