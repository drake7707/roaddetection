#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <vector>
#include "RANSAC.h"

#include "FrameProcessor.h"
#include "FrameProcessResult.h"

#include "FrameProcessOptions.h"
#include "RoadTextureDetector.h"
#include "LocalBinaryPattern.h"
#include "Helper.h"

using namespace std;



namespace RoadTextureDetector {

	deque<Mat> bestReferenceHistograms;


	struct ReferenceHistogram {
		ReferenceHistogram(Mat& histogram, int count) : histogram(histogram), count(count) {}
		Mat histogram;
		int count;
	};

	FrameProcessResult classify(Mat& frame, Mat& roadMask, RoadTextureDetectorOptions& options, bool generateAnnotations) {

		// convert to LUV color space and take the lightness channel
		Mat luv;
		cvtColor(frame, luv, CV_BGR2Luv);
		std::vector<cv::Mat> luvChannels(3);
		cv::split(luv, luvChannels);
		
		Mat lightness = luvChannels[0];
	
		// use an overlay to annotate things
		Mat overlay = Mat(Size(lightness.cols, lightness.rows), CV_8UC3, Scalar(0, 0, 0));


		// calculate a local binary pattern code for each pixel
		Mat globallbp = Mat(Size(lightness.cols, lightness.rows), CV_8UC1, Scalar(0));
		Mat globallbpROI = globallbp(Rect(1, 1, globallbp.cols - 2, globallbp.rows - 2));
		Mat lbp;
		Algorithms::OLBP(lightness, lbp, options.lbpMinDiff);
		lbp.copyTo(globallbpROI);

		int patch_size = options.lbpPatchSize;

		// padding wanneer patch_size * x niet exact img rows is;
		auto padding = frame.rows - (int)(frame.rows / patch_size) * patch_size;
		int nrOfColPatches = frame.cols / patch_size;

		// assume bottom row with padding on both sides, for example 25%  - 75% is road

		int bottomRow = padding + patch_size * (frame.rows / patch_size) - patch_size;

		vector<ReferenceHistogram> currentReferenceHistograms;

		// determine left & right x bounds
		int lbound = options.referenceHistogramsBottomRowsMiddlePercentage * nrOfColPatches;
		int ubound = nrOfColPatches - lbound;
		for (int i = lbound; i < ubound; i++) {
			// get a reference patch 
			Rect region = Rect(patch_size * i, bottomRow, patch_size, patch_size);
			Mat patch = globallbp(region);

			// calculate the histogram and store it
			currentReferenceHistograms.push_back(ReferenceHistogram(getOpenCVHist(patch, options.nrOfHistogramBins), 0));

			// draw the reference region on the overlay
			rectangle(overlay, region, Scalar(0, 255, 255), -1);
		}

		// create a classification mask
		Mat classificationMask = Mat(Size(frame.cols, frame.rows), CV_8UC1, Scalar(0));

		double ignoreTopHeight = floor((options.ignoreTopHeight * frame.rows) / patch_size);

		// iterate over all patches in the image
		for (int j = ignoreTopHeight; j < frame.rows / patch_size; j++)
		{
			for (int i = 0; i < frame.cols / patch_size; i++)
			{
				// determine the bounds of the patch
				Rect region = Rect(patch_size * i, padding + patch_size * j, patch_size, patch_size);
				Mat patch = globallbp(region);

				// calculate the histogram
				Mat hist = getOpenCVHist(patch, options.nrOfHistogramBins);

				// compare the histogram using hellinger distance with the reference patches
				// as soon as it's close enough to 1 of the reference patches it's accepted as part of the road
				
				// test against the previously collected best histograms first
				// that way we won't possibly add the same reference patches to the best reference histogram list multiple times
				if (options.usePreviousBestHistogramMatches) {
					for (auto& refHist : bestReferenceHistograms) {
						double diff = compareHist(refHist, hist, cv::HistCompMethods::HISTCMP_KL_DIV);
						if (diff < options.maxDifferenceToReferenceHistograms) {
							classificationMask(region) = Scalar(255);
							break;
						}
					}
				}

				// no match in the best histogram list, use the current road bottom row as reference
				for (int h = 0; h < currentReferenceHistograms.size(); h++) {
					auto& refHist = currentReferenceHistograms[h];
					double diff = compareHist(refHist.histogram, hist, cv::HistCompMethods::HISTCMP_HELLINGER);
					if (diff < options.maxDifferenceToReferenceHistograms) {
						classificationMask(region) = Scalar(255);
						currentReferenceHistograms[h].count++;
						break;
					}
				}
			}
		}

		if (options.usePreviousBestHistogramMatches) {
			sort(currentReferenceHistograms.begin(), currentReferenceHistograms.end(), [](const ReferenceHistogram& a, const ReferenceHistogram& b) -> bool { return a.count > b.count; });
			// add the 10 most matched 
			for (int i = 0; i < (currentReferenceHistograms.size() < 10 ? currentReferenceHistograms.size() : 10); i++) {
			//	cout << "Histogram of reference patch size is hit " << currentReferenceHistograms[i].count << " times" << endl;
				bestReferenceHistograms.push_back(currentReferenceHistograms[i].histogram);
			}

			// keep only 100 reference histograms at most, otherwise it'll become very slow very fast
			while (bestReferenceHistograms.size() > 100)
				bestReferenceHistograms.pop_front();
		}

		// if there needs to be dilation done
		if (options.dilationIterations > 0) {
			// diltate the result mask
			dilate(classificationMask, classificationMask, Mat(Size(3, 3), CV_32F, Scalar(1)), Point(-1, -1), options.dilationIterations);
			// erode it again so only holes of dilationIterations / 2 size are closed and other edges are not affected
			erode(classificationMask, classificationMask, Mat(Size(3, 3), CV_32F, Scalar(1)), Point(-1, -1), options.dilationIterations);
		}

		// convert the global lbp to color
		FrameProcessResult result;
		cvtColor(globallbp, globallbp, CV_GRAY2BGR);

		// create the annotated image of the LBP and add the overlay
		Mat annotatedImg = globallbp + overlay * 0.4;

		// set annotated image & register onClick with by value lambda
		result.setAnnotatedImage(AnnotatedImage(annotatedImg, [=](int x, int y) -> void {
			int i = (int)(x / options.lbpPatchSize);
			int j = (int)((y - padding) / options.lbpPatchSize);

			Rect region = Rect(options.lbpPatchSize * i, options.lbpPatchSize * j, options.lbpPatchSize, options.lbpPatchSize);
			Mat patch = lightness(region);
			Mat dst;
			Algorithms::OLBP(patch, dst, options.lbpMinDiff);

			showHistogram(options.nrOfHistogramBins, dst);
		}));

		result.setClassifiedMask(classificationMask);
		return result;
	}
};
