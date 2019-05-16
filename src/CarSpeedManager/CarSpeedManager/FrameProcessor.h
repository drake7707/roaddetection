#pragma once

#include "FrameProcessResult.h"
#include "FrameProcessOptions.h"
#include "MergedFrameProcessResult.h"

MergedFrameProcessResult processFrame(Mat& img, Mat& mask, FrameProcessOptions& options, bool showLog, bool generateAnnotations);
