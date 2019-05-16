#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include "SetViewer.h"
#include "SetFrame.h"
#include "VideoSetViewer.h"

#include "FrameProcessResult.h"
#include "FrameProcessOptions.h"
#include "MergedFrameProcessResult.h"

using namespace std;
using namespace cv;

class SetForm;

class SetForm {

private:

	unique_ptr<AbstractSetViewer> set;

	FrameProcessOptions options;
	MergedFrameProcessResult processedResult;

	Mat rocCurve;
	list<Point2f> rocPoints;

	long nrTP = 0;
	long nrFP = 0;
	long nrTN = 0;
	long nrFN = 0;
	long nrP = 0;
	long nrN = 0;

	int currentProcessorIndex = 0;
	bool showAnnotatedFrame = false;
	bool showGroundTruth = false;
	bool showClassifier = false;

	bool showInfo = true;
	
	void updateProcessResult();
	void updateROC();

	void updateDisplay() const;
	void nextFrame();
	
	void messagePump();
	void initializeControls();

	void onClick(int x, int y);
public:

	SetForm(string& path) {
	
		setSet(path, false);

		namedWindow("Set", CV_WINDOW_AUTOSIZE);
		namedWindow("ROC", CV_WINDOW_AUTOSIZE);
		drawROC();

		initializeControls();
		
		cv::setMouseCallback("Set", [](int ev, int x, int y, int flags, void* userdata) -> void {
			if (ev != EVENT_LBUTTONDOWN)
				return;
			((SetForm*)userdata)->onClick(x, y);
		}, this);
	}

	void resetROC() {
		nrFN = 0;
		nrFP = 0;
		nrTN = 0;
		nrTP = 0;
		nrP = 0;
		nrN = 0;
	}

	void updateROC(double TPR, double FPR);

	void drawROC();

	void run() {
		messagePump();
	}

	void setSet(string& path, bool isVideo) {
		if (isVideo) {
			set = unique_ptr<AbstractSetViewer>(new VideoSetViewer(path));
			
		}
		else {
			set = unique_ptr<AbstractSetViewer>(new SetViewer(path));
		}
		resetROC();
		// go to the first frame
		nextFrame();
		updateDisplay();
	}
	
	void setOptions(FrameProcessOptions& options) {
		this->options = options;
		updateProcessResult();
		updateDisplay();
	}

};
