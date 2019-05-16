#pragma once
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "Helper.h"

using namespace std;

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

void printMatInfo(string name, cv::Mat& m) {
	cout << name << ":" << m.cols << " x " << m.rows << " " << type2str(m.type()) << endl;
}

void calculateMaxSpeed(cv::Mat& freeRoadMask, cv::Mat& road_mask, double& maxSpeed, int& maxRow) {

	// initial speed
	maxSpeed = std::numeric_limits<double>().max();
	for (int j = road_mask.rows - 1; j >= 0 ; j--)
	{
		for (int i = 0; i < road_mask.cols; i++) {
			uchar speed = road_mask.at<uchar>(cv::Point(i, j));
			if (speed > 0 && maxSpeed > speed)
				maxSpeed = speed;
		}
	}

	maxRow = road_mask.rows;
	bool rowIsFree = true;

	for (int j = road_mask.rows - 1; j >= 0 && rowIsFree; j--)
	{
		int nrOfColsWithMask = 0;

		for (int i = 0; i < road_mask.cols && rowIsFree; i++)
		{
			// always take the speed of the next line below
			uchar speed = road_mask.at<uchar>(cv::Point(i, j));
			uchar free = freeRoadMask.at<uchar>(cv::Point(i, j));
			if (speed > 0) {
				nrOfColsWithMask++;
				if (free == 0) {
					rowIsFree = false;
				}
				else {
					maxSpeed = speed;
					maxRow = j;
				}
			}

		}
		if (nrOfColsWithMask == 0) {
			// no more road mask
			maxRow = j;
			rowIsFree = false;
		}
	}
}

void createFreeRoadMask(cv::Mat& roadMask, cv::Mat& finalMask, cv::Mat& result) {
	cv::Mat freeRoadMask;
	if (roadMask.rows > 0 && roadMask.cols > 0) {
		cvtColor(roadMask, roadMask, CV_BGR2GRAY);

		roadMask.convertTo(roadMask, CV_8UC1);
		result = roadMask & finalMask;
	}
	else
		result = cv::Mat(cv::Size(roadMask.cols, roadMask.rows), CV_8U, cv::Scalar(0));
}


cv::Mat getOpenCVHist(cv::Mat& lbp, int nrBins) {
	cv::Mat hist;

	int channels[] = { 0 };
	int histSize[] = { nrBins };
	float range[] = { 0, 256 };
	const float* ranges[] = { range };

	calcHist(&lbp, 1, channels, cv::Mat(), // do not use mask
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);

	cv::Mat histNorm = hist / (lbp.rows * lbp.cols);

	return histNorm;

}
void showHistogram(int nrBins, cv::Mat& lbp) {

	cv::Mat result = getOpenCVHist(lbp, nrBins);


	cv::Mat hist(cv::Size(300, 300), CV_8U, cv::Scalar(255));

	int histWidth = 256;

	int padding = (hist.cols - histWidth) / 2;
	rectangle(hist, cv::Rect(0, 0, padding, hist.rows), cv::Scalar(200), -1);
	rectangle(hist, cv::Rect(hist.cols - (hist.cols - histWidth) / 2, 0, padding, hist.rows), cv::Scalar(200), -1);

	for (int i = 0; i < result.rows; i++)
	{
		float val = result.at<float>(cv::Point(0, i));

		int rectWidth = histWidth / result.rows;

		int x = padding + rectWidth * i;
		int y = hist.rows - val * hist.rows;
		rectangle(hist, cv::Rect(x, y, rectWidth, hist.rows), cv::Scalar(0), -1);
	}

	cv::namedWindow("Histogram", CV_WINDOW_KEEPRATIO);
	cv::imshow("Histogram", hist);
}

cv::Mat getInvariantImage(cv::Mat& img, double angle) {
	cv::Mat invariant = cv::Mat(cv::Size(img.cols, img.rows), CV_32F, cv::Scalar(0));
	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			cv::Vec3b v = img.at<cv::Vec3b>(cv::Point(i, j));
			double logBG = v[1] == 0 ? 0 : log(v[0] / (double)v[1]);
			double logRG = v[1] == 0 ? 0 : log(v[2] / (double)v[1]);

			double invariantValue = cos(angle) * logBG + sin(angle) * logRG;
			//	if (invariantValue > 1) invariantValue = 1;
			//	if (invariantValue < 0) invariantValue = 0;

			float val = invariant.at<float>(cv::Point(i, j));
			val += abs(invariantValue);
			invariant.at<float>(cv::Point(i, j)) = val;
		}
	}
	return invariant;
}

cv::Mat getLogLogColorSpace(cv::Mat& img, int size) {
	cv::Mat result = cv::Mat(cv::Size(size, size), CV_32F, cv::Scalar(0));

	double maxLogBG = numeric_limits<double>().min();
	double maxLogRG = numeric_limits<double>().min();
	double minLogRG = numeric_limits<double>().max();
	double minLogBG = numeric_limits<double>().max();

	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			cv::Vec3b v = img.at<cv::Vec3b>(cv::Point(i, j));
			double logBG = v[1] == 0 ? 0 : log(v[0] / (double)v[1]);
			double logRG = v[1] == 0 ? 0 : log(v[2] / (double)v[1]);

			if (!isinf(logBG)) {
				if (logBG < minLogBG) minLogBG = logBG;
				if (logBG > maxLogBG) maxLogBG = logBG;
			}
			if (!isinf(logRG)) {
				if (logRG < minLogRG) minLogRG = logRG;
				if (logRG > maxLogRG) maxLogRG = logRG;
			}
		}
	}

	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			cv::Vec3b v = img.at<cv::Vec3b>(cv::Point(i, j));
			double logBG = v[1] == 0 ? 0 : log(v[0] / (double)v[1]);
			double logRG = v[1] == 0 ? 0 : log(v[2] / (double)v[1]);
			if (!isinf(logBG) && !isinf(logRG)) {
				double x = (logRG - minLogRG) / (maxLogRG - minLogRG);
				double y = (logBG - minLogBG) / (maxLogBG - minLogBG);

				auto val = result.at<float>(cv::Point((int)(x*size), (int)(y*size)));
				result.at<float>(cv::Point((int)(x*size), (int)(y*size))) =  1;
			}
		}
	}
	return result;
}