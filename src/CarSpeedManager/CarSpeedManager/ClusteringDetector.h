#pragma once
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <vector>
#include "ClusteringDetectorOptions.h"


namespace ClusteringDetector {
	
	FrameProcessResult classify(Mat& frame, Mat& roadMask, ClusteringDetectorOptions& options, bool generateAnnotations);

}