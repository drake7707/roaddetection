#pragma once

#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <vector>

#include "RoadTextureDetectorOptions.h"

namespace RoadTextureDetector {
	FrameProcessResult classify(Mat& frame, Mat& roadMask, RoadTextureDetectorOptions& options, bool generateAnnotations);
};