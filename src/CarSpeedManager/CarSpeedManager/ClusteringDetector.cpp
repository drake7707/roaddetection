#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "FrameProcessOptions.h"
#include "FrameProcessResult.h"
#include "Clustering.cpp"
#include "ClusteringDetectorOptions.h"
#include "ClusteringDetector.h"


using namespace std;
using namespace Algorithms;

namespace ClusteringDetector {


	struct Cluster {
		Cluster(int count, int index) : count(count), index(index) {
		}

		int count;
		int index;
	};

	// This function extracts the hue, saturation, and luminance from "color" 
	// and places these values in h, s, and l respectively.
	void RGBtoHSL(int r, int g, int b, double& h, double& s, double& l)
	{
		double r_percent = ((double)r) / 255;
		double g_percent = ((double)g) / 255;
		double b_percent = ((double)b) / 255;

		double max_color = 0;
		if ((r_percent >= g_percent) && (r_percent >= b_percent))
			max_color = r_percent;
		if ((g_percent >= r_percent) && (g_percent >= b_percent))
			max_color = g_percent;
		if ((b_percent >= r_percent) && (b_percent >= g_percent))
			max_color = b_percent;

		double min_color = 0;
		if ((r_percent <= g_percent) && (r_percent <= b_percent))
			min_color = r_percent;
		if ((g_percent <= r_percent) && (g_percent <= b_percent))
			min_color = g_percent;
		if ((b_percent <= r_percent) && (b_percent <= g_percent))
			min_color = b_percent;

		double L = 0;
		double S = 0;
		double H = 0;

		L = (max_color + min_color) / 2;

		if (max_color == min_color)
		{
			S = 0;
			H = 0;
		}
		else
		{
			if (L < .50)
				S = (max_color - min_color) / (max_color + min_color);
			else
				S = (max_color - min_color) / (2 - max_color - min_color);
			if (max_color == r_percent)
				H = (g_percent - b_percent) / (max_color - min_color);
			if (max_color == g_percent)
				H = 2 + (b_percent - r_percent) / (max_color - min_color);
			if (max_color == b_percent)
				H = 4 + (r_percent - g_percent) / (max_color - min_color);
		}
		s = S;
		l = L;
		H = H * 60;
		if (H < 0)
			H += 360;
		h = H / 360;
	}


	/*
		Converts the image to a list of normalized feature vectors that can be clustered.
	*/
	void convertImageDataToFeatureVectors(Mat& img, double ignoreTopHeight,  std::vector<FeatureVector>& resultVectors) {
		resultVectors.reserve(img.rows * img.cols);

		// convert to grayscale
		Mat grayscale;
		cvtColor(img, grayscale, CV_BGR2GRAY);

		
		for (int j = ignoreTopHeight; j < img.rows; j++) {
			for (int i = 0; i < img.cols; i++) {

				Vec3b vec = img.at<Vec3b>(Point(i, j));

				uchar b = vec[0];
				uchar g = vec[1];
				uchar r = vec[2];

				// convert rgb to hsl
				double h, s, l;
				RGBtoHSL(r, g, b, h, s, l);

				// add saturation and lightness as values, both are already within the 0-1 range
				std::vector<double> values = { s,l };
				std::vector<double> tags = { i / (float)img.cols, j / (float)img.rows };
				resultVectors.push_back(FeatureVector(values, tags));
			}
		};

	}

	FrameProcessResult classify(Mat& frame, Mat& roadMask, ClusteringDetectorOptions& options, bool generateAnnotations) {

		int downScaleFactor = options.downScaleFactor;
		Mat img;

		Mat overlay = Mat(Size(frame.cols, frame.rows), CV_8UC3, Scalar(0));
		
		// downscale the image if necessary
		resize(frame, img, Size(frame.cols / downScaleFactor, frame.rows / downScaleFactor));

		double ignoreTopHeight = options.ignoreTopHeight * img.rows;

		// convert the image to feature vectors
		std::vector<FeatureVector> featureVectors;
		convertImageDataToFeatureVectors(img, ignoreTopHeight, featureVectors);

		// apply iterative k-means/mean shift clustering
		Clustering clustering(featureVectors, options.initialNrOfClusters, options.windowRadius, options.overlapThreshold);
		while (!clustering.isDone()) {
			clustering.step();
		}

		// keep track per pixel which cluster it belongs to
		const int NO_CLUSTER = 255;
		Mat clusterPerPixel = Mat(Size(img.cols, img.rows), CV_8UC1, Scalar(NO_CLUSTER));

		// fill in the cluster index for each pixel
		auto vectorsPerCluster = clustering.getVectorsPerCluster();
		for (int i = 0; i < vectorsPerCluster.size(); i++)
		{
			for (auto& v : vectorsPerCluster[i]) {

				int x = round(v->tags[0] * clusterPerPixel.cols);
				int y = round(v->tags[1] * clusterPerPixel.rows);

				clusterPerPixel.at<uchar>(Point(x, y)) = i;
			}
		}

		// build a list of clusters with their nr of pixels in the reference bottom row
		std::vector<Cluster> clusterHist;
		clusterHist.reserve(vectorsPerCluster.size());
		for (int i = 0; i < vectorsPerCluster.size(); i++)
			clusterHist.push_back(Cluster(0, i));

		auto bottomRowPaddingPercentage = options.bottomRowPaddingPercentage;
		auto bottomRowHeight = options.bottomRowHeight / options.downScaleFactor;

		// calculate for each cluster the number of pixels
		for (int y = img.rows - bottomRowHeight; y < img.rows; y++)
		{
			for (int x = img.cols * bottomRowPaddingPercentage; x < img.cols - img.cols * bottomRowPaddingPercentage; x++)
			{
				uchar clusterIdx = clusterPerPixel.at<uchar>(Point(x, y));
				if (clusterIdx != NO_CLUSTER)
					clusterHist[clusterIdx].count++;
			}
		}

		// draw the reference row on the overlay
		if (generateAnnotations) {
			Rect rect = Rect(img.cols * bottomRowPaddingPercentage * options.downScaleFactor, (img.rows - bottomRowHeight) * options.downScaleFactor,
				(img.cols - 2 * img.cols * bottomRowPaddingPercentage)  * options.downScaleFactor, bottomRowHeight*  options.downScaleFactor);
			cv::rectangle(overlay, rect, Scalar(0, 255, 255), -1);
		}

		// sort the clusters by their size, the ones with the most pixels first
		std::sort(clusterHist.begin(), clusterHist.end(), [](const Cluster& a, const Cluster& b) -> bool { return a.count > b.count; });

		// build a color index list so the biggest clusters are always assigned the same colors in order
		vector<int> colorIndicesByClusterSize(clusterHist.size(), 0);
		for (int i = 0; i < clusterHist.size(); i++)
			colorIndicesByClusterSize[clusterHist[i].index] = i;

		Mat mask = Mat(Size(img.cols, img.rows), CV_8UC1, Scalar(0));

		int topNClustersOnBottomRow = options.topNClustersOnBottomRow;
		// Take the top n clusters that correspond with the bottom row
		if (topNClustersOnBottomRow > clusterHist.size())
			topNClustersOnBottomRow = clusterHist.size();


		// take the top n clusters  and if there are enough pixels from the bottom row
		// fill the mask for all pixels of the cluster
		for (int c = 0; c < topNClustersOnBottomRow; c++)
		{
			int clusterIndex = clusterHist[c].index;
			if (clusterHist[c].count > options.minNrOfPixelsInClusterInReference) { // at least 100 pixels in the reference
				
				for (auto& v : vectorsPerCluster[clusterIndex]) {

					int x = round(v->tags[0] * clusterPerPixel.cols);
					int y = round(v->tags[1] * clusterPerPixel.rows);
					mask.at<uchar>(Point(x, y)) = 255;
				}
			}
		}

		// draw all the clusters with their assigned colors
		Mat clusterColors;
		if (generateAnnotations) {
			vector<Vec3b> colors = { Vec3b(255,0,0), Vec3b(0,255,0), Vec3b(0,0,255), Vec3b(255,255,0), Vec3b(0,255,255), Vec3b(255,0,255), Vec3b(128,0,255), Vec3b(255,0,128), Vec3b(128,255,0), Vec3b(255,0,128) };

			clusterColors = Mat(Size(clusterPerPixel.cols, clusterPerPixel.rows), CV_8UC3, Scalar(0));
			for (int j = 0; j < clusterPerPixel.rows; j++)
			{
				for (int i = 0; i < clusterPerPixel.cols; i++)
				{
					int clusterIndex = clusterPerPixel.at<uchar>(Point(i, j));
					if (clusterIndex != NO_CLUSTER) {
						int colorIdx = colorIndicesByClusterSize[clusterIndex];
						clusterColors.at<Vec3b>(Point(i, j)) = colors[colorIdx % colors.size()];
					}
				}
			}
			resize(clusterColors, clusterColors, Size(frame.cols, frame.rows), 0, 0, 0);
		}

		// resize the mask to its original size again using pixel resize
		resize(mask, mask, Size(frame.cols, frame.rows), 0, 0, 0);

		// use morphological operations to remove tiny holes in the mask
		if (options.dilationIterations > 0) {
			dilate(mask, mask, Mat(Size(3, 3), CV_32F, Scalar(1)), Point(-1, -1), options.dilationIterations);
			// erode it again so only holes of dilationIterations / 2 size are closed and other edges are not affected
			erode(mask, mask, Mat(Size(3, 3), CV_32F, Scalar(1)), Point(-1, -1), options.dilationIterations);
		}

		mask.convertTo(mask, CV_8UC1);

		// combine the annotated image and set the result
		Mat annotatedImage;
		if (generateAnnotations)
			annotatedImage = frame * 0.4 + 0.6 * clusterColors + 0.6 * overlay;
		else
			annotatedImage = frame;

		FrameProcessResult result;
		result.setClassifiedMask(mask);
		result.setAnnotatedImage(AnnotatedImage(annotatedImage, nullptr));
		return result;
	}
}
