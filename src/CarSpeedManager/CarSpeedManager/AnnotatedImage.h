#pragma once

#include "opencv2/opencv.hpp"
#include <functional>

class AnnotatedImage {
public:
	cv::Mat image;
	std::function<void(int, int)>  onClick;

	AnnotatedImage() {

	}

	AnnotatedImage(cv::Mat img, std::function<void(int, int)>  onClick)
		: image(img), onClick(onClick) {

	}


		
};