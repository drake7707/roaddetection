#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>

#include "FrameProcessor.h"
#include "FrameProcessOptions.h"
#include "MergedFrameProcessResult.h"

#include "LaneDetector.h"
#include "RoadTextureDetector.h"
#include "ClusteringDetector.h"
#include "Helper.h"
#include "ShadowRemoval.h"

#include <thread>


MergedFrameProcessResult processFrame(Mat& img, Mat& mask, FrameProcessOptions& options, bool outputLog, bool generateAnnotations) {

	
	std::vector<thread> threads;

	MergedFrameProcessResult result;
	Mat finalClassifiedMask = Mat(Size(img.cols, img.rows), CV_8UC1, Scalar(255));

	Mat weakerClassifiedMask = Mat(Size(img.cols, img.rows), CV_8UC1, Scalar(0));

	FrameProcessResult laneDetectorClassifyResult;
	FrameProcessResult roadTextureDetectorClassifyResult;
	FrameProcessResult clusteringDetectorResult;
	if (options.laneDetector.enabled) {
		threads.push_back(std::thread([&]() -> void {
			auto ms = measure<chrono::milliseconds>::execution([&]() -> void {
				laneDetectorClassifyResult = LaneDetector::classify(img, mask, options.laneDetector, generateAnnotations);
				bitwise_and(finalClassifiedMask, laneDetectorClassifyResult.getClassifiedMask(), finalClassifiedMask);
			});

			if (outputLog) cout << "Lane detector took " << ms << " ms" << endl;
		}));
	}

	if (options.roadTextureDetector.enabled) {
		threads.push_back(std::thread([&]() -> void {
			auto ms = measure<chrono::milliseconds>::execution([&]() -> void {
				roadTextureDetectorClassifyResult = RoadTextureDetector::classify(img, mask, options.roadTextureDetector, generateAnnotations);
				bitwise_or(weakerClassifiedMask, roadTextureDetectorClassifyResult.getClassifiedMask(), weakerClassifiedMask);
			});
			if (outputLog) cout << "Road texture detector took " << ms << " ms" << endl;
		}));
	}

	if (options.clusterDetector.enabled) {
		threads.push_back(std::thread([&]() -> void {
			auto ms = measure<chrono::milliseconds>::execution([&]() -> void {
				clusteringDetectorResult = ClusteringDetector::classify(img, mask, options.clusterDetector, generateAnnotations);
				bitwise_or(weakerClassifiedMask, clusteringDetectorResult.getClassifiedMask(), weakerClassifiedMask);
			});

			if (outputLog) cout << "Clustering detector took " << ms << " ms" << endl;
		}));
	}

	for (auto& t : threads)
		t.join();

	if (options.clusterDetector.enabled || options.roadTextureDetector.enabled) // only apply the weaker classifier mask if at least one classifier has created a mask
		bitwise_and(finalClassifiedMask, weakerClassifiedMask, finalClassifiedMask);

	// erode the final mask a bit on all sides to prevent going over the speed limit
	for (int i = 0; i < finalClassifiedMask.cols; i++)
	{
		int curNrOfMask = 0;
		for (int j = 0; j < finalClassifiedMask.rows && curNrOfMask < options.finalErosion; j++)
		{
			while (j < finalClassifiedMask.rows && finalClassifiedMask.at<uchar>(Point(i, j)) > 0 && curNrOfMask < options.finalErosion) {
				finalClassifiedMask.at<uchar>(Point(i, j)) = 0; // remove top n pixels
				curNrOfMask++;
			}
		}
	}

	// remove 1st column and last column
	for (int j = 0; j < finalClassifiedMask.rows; j++)
	{
		finalClassifiedMask.at<uchar>(Point(0, j)) = 0;
		finalClassifiedMask.at<uchar>(Point(finalClassifiedMask.cols-1, j)) = 0;
	}

	if (outputLog) cout << "---------------------" << endl;

	if (options.laneDetector.enabled) {
		result.addAnnotatedImage(laneDetectorClassifyResult.getAnnotatedImage());
		result.addClassifiedMask(laneDetectorClassifyResult.getClassifiedMask());
	}

	if (options.roadTextureDetector.enabled) {
		result.addAnnotatedImage(roadTextureDetectorClassifyResult.getAnnotatedImage());
		result.addClassifiedMask(roadTextureDetectorClassifyResult.getClassifiedMask());
	}

	if (options.clusterDetector.enabled) {
		result.addAnnotatedImage(clusteringDetectorResult.getAnnotatedImage());
		result.addClassifiedMask(clusteringDetectorResult.getClassifiedMask());
	}


	result.setFinalClassifiedMask(finalClassifiedMask);
	return result;
}

void test(Mat& img, Mat& mask, FrameProcessOptions& options, bool outputLog) {


	options.roadTextureDetector.enabled = false;
	options.clusterDetector.enabled = false;

	double angle =  3.5 * CV_PI / 8;
	img(Rect(0, 0, img.cols, img.rows / 2)) = 0;
	normalize(img, img, 0, 255, cv::NormTypes::NORM_MINMAX);

	Mat loglog = getLogLogColorSpace(img, 500);

	normalize(loglog, loglog, 0, 1024, cv::NormTypes::NORM_MINMAX);
	namedWindow("LogLog");
	imshow("LogLog", loglog);


	double maxLogBG = numeric_limits<double>().min();
	double maxLogRG = numeric_limits<double>().min();
	double minLogRG = numeric_limits<double>().max();
	double minLogBG = numeric_limits<double>().max();

	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			cv::Vec3b v = img.at<cv::Vec3b>(cv::Point(i, j));
			double logBG = v[1] == 0 ? numeric_limits<double>().infinity() : log(v[0] / (double)v[1]);
			double logRG = v[1] == 0 ? numeric_limits<double>().infinity() : log(v[2] / (double)v[1]);

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

	Mat testImg = img.clone();
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

				float val = loglog.at<float>(Point((int)(x*loglog.cols), (int)(y*loglog.rows)));
				if (val < 10) {
					testImg(Rect(i, j, 1, 1)) = 0;
				}
			}
		}
	}
	namedWindow("Test image");
	imshow("Test image", testImg);


	Mat invariant = Mat(Size(img.cols, img.rows), CV_32F, Scalar(0));

	//	for (int a = 1; a <= 8; a++)
	//	{
	//	double angle = (180/(float)a) / (float)360 * (2 * CV_PI);// 3.5 * CV_PI / 8;

	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			Vec3b v = img.at<Vec3b>(Point(i, j));
			double logBG = v[1] == 0 ? 0 : log(v[0] / (double)v[1]);
			double logRG = v[1] == 0 ? 0 : log(v[2] / (double)v[1]);

			double invariantValue = cos(angle) * logBG + sin(angle) * logRG;
			//	if (invariantValue > 1) invariantValue = 1;
			//	if (invariantValue < 0) invariantValue = 0;

			float val = invariant.at<float>(Point(i, j));
			val += abs(invariantValue);
			invariant.at<float>(Point(i, j)) = val;
		}
	}
	//	}
	//	invariant /= 8;
	namedWindow("Invariant");
	imshow("Invariant", invariant);

	Mat inv = invariant * 255;
	inv.convertTo(inv, CV_8UC1);
	showHistogram(256, inv);

	/*	Mat hls;
	cvtColor(img, hls, CV_BGR2HLS);

	Mat channels[3];
	cv::split(hls, channels);

	Mat imgLightness = channels[1];

	auto clahe = cv::createCLAHE(5, Size(16, 16));

	Mat claheLightness;
	clahe->apply(imgLightness, imgLightness);


	namedWindow("Clahe");
	imshow("Clahe", imgLightness);

	threshold(imgLightness, imgLightness, 180, 255, THRESH_BINARY);



	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(imgLightness, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// clear the canny image
	imgLightness(Rect(0, 0, imgLightness.cols, imgLightness.rows)) = 0;
	for (int i = 0; i < contours.size(); i++)
	{
	vector<Point> hull;
	convexHull(contours[i], hull);

	auto epsilon = 0.1*arcLength(hull, true);
	vector<Point> approx;
	approxPolyDP(hull, approx, epsilon, true);

	auto area = contourArea(hull, false);
	//if (area > 100 && area < 1000) {
	const Point* lst = &(approx[0]);
	int num = (int)approx.size();
	cv::fillPoly(img, &lst, &num, 1, Scalar(0, 255, 0));
	//}
	//auto rect = boundingRect(contours[i]);
	//double ratio = rect.width / (float)rect.height;
	//if( rect.size.width > 10 && rect.size.height > 10) {
	//	cout << rect.size.width << "x" << rect.size.height << "," << ratio << endl;
	drawContours(imgLightness, contours, i, Scalar(255), -1, LineTypes::LINE_8, hierarchy, 2, Point());

	//	rectangle(img, rect, Scalar(0, 255, 0));
	Point2f vertices[4];
	rect.points(vertices);
	for (int i = 0; i < 4; i++)
	line(img, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
	//}


	}



	namedWindow("Treshold");
	imshow("Treshold", imgLightness);
	*/

}