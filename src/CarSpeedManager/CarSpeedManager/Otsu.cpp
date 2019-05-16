#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

int getOtsuTreshold(Mat& m) {

	Mat hist;

	int channels[] = { 0 };
	int histSize[] = { 256 };
	float range[] = { 0, 256 };
	const float* ranges[] = { range };

	calcHist(&m, 1, channels, Mat(), // do not use mask
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);


	int maxTreshold = 0;
	int maxVarianceBetween = 0;

	long nrOfPixels = (m.cols * m.rows);
	// find optimal treshold 
	for (int tresh = 0; tresh < 256; tresh++) {
		// max between class variance
		// = maximize * w1 * w2 *(avg1-avg2)²
		// where w1 = weight of class 1 and avg1 = average of class1
		// and w2 = weight of class 2 and avg2 = average of class2

		double sum1 = 0;
		double sum2 = 0;

		double weightedSum1 = 0;
		double weightedSum2 = 0;
		for (double i = 0; i < tresh; i++) {
			sum1 += hist.at<float>(Point(0, i));
			weightedSum1 += i * hist.at<float>(Point(0, i));
		}

		for (int i = tresh; i < 256; i++) {
			sum2 += hist.at<float>(Point(0, i));
			weightedSum2 += i * hist.at<float>(Point(0, i));
		}

		double w1 = sum1 / nrOfPixels;
		double w2 = sum2 / nrOfPixels;

		double avg1 = weightedSum1 / sum1;
		double avg2 = weightedSum2 / sum2;

		double maxVariance = w1 * w2 * (avg1 - avg2) * (avg1 - avg2);

		if (maxVariance > maxVarianceBetween) {

			maxVarianceBetween = maxVariance;
			maxTreshold = tresh;
		}
	}


	return maxTreshold;

}