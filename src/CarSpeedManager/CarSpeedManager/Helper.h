#pragma once
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

using namespace std;

string type2str(int type);
void printMatInfo(string name, cv::Mat& m);


void calculateMaxSpeed(cv::Mat& freeRoadMask, cv::Mat& road_mask, double& maxSpeed, int& maxRow);

void createFreeRoadMask(cv::Mat& roadMask, cv::Mat& finalMask, cv::Mat& result);

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
	template<typename F, typename ...Args>
	static typename TimeT::rep execution(F func, Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		// Now call the function with all the parameters you need.
		func(std::forward<Args>(args)...);

		auto duration = std::chrono::duration_cast<TimeT>(std::chrono::system_clock::now() - start);

		return duration.count();
	}
};

void showHistogram(int nrBins, cv::Mat& lbp);
cv::Mat getOpenCVHist(cv::Mat& lbp, int nrBins);

cv::Mat getLogLogColorSpace(cv::Mat& img, int size);
cv::Mat getInvariantImage(cv::Mat& img, double angle);