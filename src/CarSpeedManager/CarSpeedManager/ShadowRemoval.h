#pragma once

#include "opencv2/opencv.hpp"
#include "ShadowRemoval.h"

using namespace cv;

namespace Algorithms {

	void removeShadows(Mat& input, Mat& output);
}