#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <vector>

#include "FrameProcessOptions.h"

namespace LaneDetector {
	FrameProcessResult classify(Mat& frame, Mat& roadMask, LaneDetectorOptions& options, bool generateAnnotations);
};