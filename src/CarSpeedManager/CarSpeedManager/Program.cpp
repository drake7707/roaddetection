#pragma once

#include "SetForm.h"
#include "FrameProcessor.h"
#include "SetViewer.h"
#include "Helper.h"
#include <chrono>

SetForm* singletonFormInstance = NULL;


void processVideo(const string& inputFile, const string& outputPath, long fromMs, long durationMs) {

	cv::VideoCapture cap;
	if (!cap.open(inputFile))
		return;

	int videoType = CV_FOURCC('X', 'V', 'I', 'D');//(int)cap.get(CV_CAP_PROP_FORMAT);
	cout << videoType << endl;
	int fps = (int)cap.get(CV_CAP_PROP_FPS);

	int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	FrameProcessOptions options;

	string laneVideo = outputPath + "output_lane.avi";
	string textureVideo = outputPath + "output_texture.avi";
	string clusterVideo = outputPath + "output_cluster.avi";
	string finalMaskVideo = outputPath + "output_finalmask.avi";

	VideoWriter voutLane;
	VideoWriter voutTexture;
	VideoWriter voutCluster;
	VideoWriter voutFinalMask;
	voutLane.open(laneVideo, videoType, fps, Size(width, height), true);
	voutTexture.open(textureVideo, videoType, fps, Size(width, height), true);
	voutCluster.open(clusterVideo, videoType, fps, Size(width, height), true);
	voutFinalMask.open(finalMaskVideo, videoType, fps, Size(width, height), true);

	// skip frames to next second


	// skip to 15min
	cap.set(CV_CAP_PROP_POS_MSEC, fromMs);

	auto initialCurMsec = cap.get(CV_CAP_PROP_POS_MSEC);
	auto curMsec = initialCurMsec;


	// run for 10 min
	while (cap.isOpened() && curMsec - initialCurMsec < durationMs) {
		curMsec = cap.get(CV_CAP_PROP_POS_MSEC);

		Mat img;
		cap.read(img);


		cout << "Frame @ " << std::to_string(curMsec) << endl;
		auto result = processFrame(img, Mat(), options, true, true);

		voutLane.write(result.getAnnotatedImages().at(0).image);

		// draw mask on the texture as well
		Mat textureMask = result.getClassifiedMasks().at(1).clone();
		cvtColor(textureMask, textureMask, CV_GRAY2BGR);
		Mat textureChannels[3];
		cv::split(textureMask, textureChannels);
		vector<Mat> vTextureChannels;
		vTextureChannels.push_back(Mat::zeros(Size(textureMask.cols, textureMask.rows), CV_8UC1));
		vTextureChannels.push_back(textureChannels[1]);
		vTextureChannels.push_back(Mat::zeros(Size(textureMask.cols, textureMask.rows), CV_8UC1));
		cv::merge(vTextureChannels, textureMask);
		Mat outputTexture = result.getAnnotatedImages().at(1).image * 0.7 + textureMask *0.3;

		voutTexture.write(outputTexture);
		voutCluster.write(result.getAnnotatedImages().at(2).image);


		Mat classMask = result.getFinalClassifiedMask().clone();
		cvtColor(classMask, classMask, CV_GRAY2BGR);
		Mat channels[3];
		cv::split(classMask, channels);
		vector<Mat> vChannels;
		vChannels.push_back(Mat::zeros(Size(classMask.cols, classMask.rows), CV_8UC1));
		vChannels.push_back(channels[1]);
		vChannels.push_back(Mat::zeros(Size(classMask.cols, classMask.rows), CV_8UC1));
		cv::merge(vChannels, classMask);
		Mat output = img * 0.7 + classMask *0.3;

		voutFinalMask.write(output);

	}
	cap.release();
	voutLane.release();
	voutTexture.release();
	voutCluster.release();
	voutFinalMask.release();
}


void writeResultMasksForSet(string& path, FrameProcessOptions& options) {

	SetViewer viewer(path);
	viewer.next();
	
	auto frame = viewer.getCurrentFrame();

	while (frame.getFrame().cols > 0) {

		Mat curFrame = frame.getFrame();
		Mat curRoadMaskMask = frame.getMask();
		auto result = processFrame(curFrame, curRoadMaskMask, options, false, false);
		Mat finalMask = result.getFinalClassifiedMask();

		cout << frame.getNumber() << endl;
		string outputPath = path + "\\result" + frame.getNumber() + ".png";
		imwrite(outputPath, finalMask);

		viewer.next();
		frame = viewer.getCurrentFrame();
	}

}

string  calculateResultsForSet(string& path, FrameProcessOptions& options) {

	SetViewer viewer(path);
	viewer.next();
	auto frame = viewer.getCurrentFrame();

	int idx = 0;

	long nrFP = 0;
	long nrTP = 0;
	long nrFN = 0;
	long nrTN = 0;
	long nrP = 0;
	long nrN = 0;

	double time = 0;
	int frameWhereTimeRanOut = -1;
	vector<string> overSpeedLimitFrames;

	ofstream resultStream(path + "\\results.txt");
	
	double totalMs = 0;
	int nrOfFrames = 0;
	while (frame.getFrame().cols > 0) {

		Mat curFrame = frame.getFrame();
		Mat curRoadMaskMask = frame.getMask();
		MergedFrameProcessResult result; 
		auto ms = measure<chrono::milliseconds>::execution([&]() -> void {
			result = processFrame(curFrame, curRoadMaskMask, options, false, false);
		});
		totalMs += ms;
		nrOfFrames++;
		
		Mat finalMask = result.getFinalClassifiedMask();

		Mat freeRoadMask;
		createFreeRoadMask(curRoadMaskMask, finalMask, freeRoadMask);

		double maxSpeed;
		int maxRow;
		calculateMaxSpeed(freeRoadMask, curRoadMaskMask, maxSpeed, maxRow);

		auto& groundTruth = frame.getGroundTruth();

		if (groundTruth.rows > 0) {
			for (int j = 0; j < groundTruth.rows; j++)
			{
				for (int i = 0; i < groundTruth.cols; i++)
				{
					uchar gtval = groundTruth.at <uchar>(j, i);
					uchar classval = finalMask.at<uchar>(j, i);

					if (gtval > 0 && classval == 0) // missed one
						nrFN++;
					else if (gtval == 0 && classval > 0) // incorrectly detected it
						nrFP++;
					else if (gtval > 0 && classval > 0)
						nrTP++;
					else if (gtval == 0 && classval == 0)
						nrTN++;

					if (gtval > 0) nrP++; else nrN++;
				}
			}
		}

		resultStream << frame.getNumber() << " " << maxSpeed << endl;

		cout << idx << " - " << "GT: " << frame.getGTDistance() << " | Calculated: " << maxSpeed << (maxSpeed > frame.getGTDistance() ? "[!!!]" : "") << " ( in " << ms << "ms)" << endl;
		if (maxSpeed > frame.getGTDistance())
			overSpeedLimitFrames.push_back(std::to_string(idx) + " - " + "GT: " + std::to_string(frame.getGTDistance()) + " | Calculated: " + std::to_string(maxSpeed));

		auto oldTime = time;
		time += frame.getActualSpeed() / maxSpeed;
		if (oldTime < 60 && time >= 60) {
			frameWhereTimeRanOut = idx;
			cout << idx << " TIME RAN OUT" << endl;
		}
		idx++;
		viewer.next();
		frame = viewer.getCurrentFrame();
	}

	double recall = nrTP / (float)(nrP);
	double precision = nrTP / (float)(nrTP + nrFP);

	double f1Score = 2 * (precision * recall) / (float)(precision + recall);

	double TPR = nrTP / (float)nrP;
	double FPR = nrFP / (float)(nrN);


	ostringstream os;
	if (frameWhereTimeRanOut != -1) {
		os << "Frame where time ran out: " << frameWhereTimeRanOut << endl;
	}

	os << "Frames that were over speed limit: " << endl;
	if (overSpeedLimitFrames.size() > 0) {
		for (auto& f : overSpeedLimitFrames)
			os << f << endl;
	}
	else
		os << "None!" << endl;

	os << "---------" << endl;

	os << "#TP " << nrTP << " | #FP " << nrFP << " | #TN " << nrTN << " | #FN " << nrFN << endl;
	os << "------" << endl;
	os << "TPR " << TPR << endl;
	os << "FPR " << FPR << endl;
	os << "------" << endl;
	os << "F1 Score: " << f1Score << endl;

	os << "Total ms: " << (totalMs) << endl;
	os << "Average ms: " << (totalMs / (float)nrOfFrames) << endl;


	resultStream.close();

	return os.str();
}


extern "C" {

	__declspec(dllexport) void initialize(char* path) {
		string strPath = path;

		SetForm form(strPath);
		singletonFormInstance = &form;
		form.run();
	}

	__declspec(dllexport) void setSet(char* path, bool isVideo) {
		string strPath = path;
		singletonFormInstance->setSet(strPath, isVideo);
	}

	__declspec(dllexport) void calculateSetPerformance(char* path, FrameProcessOptions options) {
		string strPath = path;
		cout << calculateResultsForSet(strPath, options).c_str();
	}

	__declspec(dllexport) void updateOptions(FrameProcessOptions options) {
		singletonFormInstance->setOptions(options);
	}


	__declspec(dllexport) void processEntireVideo(char* inputFile, char* outputPath, long fromMs, long duration) {
		processVideo(inputFile, outputPath, fromMs, duration);
	}

}

void test(string& inputFile) {
	cv::VideoCapture cap;
	if (!cap.open(inputFile))
		return;

	cap.set(CV_CAP_PROP_POS_MSEC, 250000);

	namedWindow("test");
	namedWindow("testout");

	namedWindow("testoutadj");

	double beta1 = 2.557;

	Mat img;
	while (cap.read(img)) {

		Mat out = Mat(Size(img.cols, img.rows), CV_32F, Scalar(0));

		
		imshow("test", img);
		cvtColor(img, img, CV_BGR2Lab);

		Mat adj = img.clone();

		double sumA = 0;
		double sumB = 0;
		double sumL = 0;
		for (int j = 0; j < img.rows; j++)
		{
			for (int i = 0; i < img.cols; i++)
			{

				Vec3b v = img.at<Vec3b>(Point(i, j));
				sumA += v[1];
				sumB += v[2];
				sumL += v[0];
			}
		}
		double meanA = sumA / (img.rows * img.cols);
		double meanB = sumB / (img.rows * img.cols);
		double meanL = sumL / (img.rows * img.cols);

		
		/*if (meanA + meanB < 256) {
			double squaresL = 0;
			for (int j = 0; j < img.rows; j++)
			{
				for (int i = 0; i < img.cols; i++)
				{
					Vec3b v = img.at<Vec3b>(Point(i, j));
					squaresL += (v[0] - meanL) * (v[0] - meanL);
				}
			}
			double stddevL = sqrt(squaresL / (img.rows * img.cols));

			for (int j = 0; j < img.rows; j++)
			{
				for (int i = 0; i < img.cols; i++)
				{
					Vec3b v = img.at<Vec3b>(Point(i, j));
					if (v[0] <= meanL - stddevL / 3) {
						out.at<float>(Point(i, j)) = 1;
					}
					else {
						out.at<float>(Point(i, j)) = 0;
					}	
				}
			}
			putText(out, "Mean is <= 255", Point(10, 20), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 10, Scalar(1));

		}
		else {
		*/
	//		for (int j = 0; j < img.rows; j++)
	//		{
	//			for (int i = 0; i < img.cols; i++)
	//			{
	//				Vec3b v = img.at<Vec3b>(Point(i, j));
	//				if (v[0] <= meanL && v[2] <= meanB) {

	//					Vec3b av = adj.at<Vec3b>(Point(i, j));
	//					av[0] += meanL - v[0];
	//					av[2] += meanB - v[2];
	//					adj.at<Vec3b>(Point(i, j)) = av;

	//					out.at<float>(Point(i, j)) = 1;
	//				}
	//				else {
	//					out.at<float>(Point(i, j)) = 0;
	//				}
	//			}
	//		}
	////		putText(out, "Else", Point(10, 20), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 10, Scalar(1));
	////	}
	//	
	//	imshow("testout", out);

	//	cvtColor(adj, adj, CV_Lab2BGR);
	//	imshow("testadj", adj);

		waitKey(10);
	}
}

int main(char argc, char** argv) {

	if (argc > 2 && strcmp(argv[1], "calculate") == 0) {
		string path = argv[2];
		cout << calculateResultsForSet(path, FrameProcessOptions());
		exit(0);
		return 0;
	}

	string path = "..\\Sets\\04";
	//writeResultMasksForSet(path, FrameProcessOptions());
	/*string vpath = "D:\\out_shadowy.mp4";
	test(vpath);*/
	/*processVideo("D:\\out.mp4", "D:\\output\\", 1500000, 600000);
	exit(0);
	return 0;*/

	/*
	std::vector<int> test;
	for (int i = 0; i < 100; i++)
		test.push_back(i);

	std::function<void(int)> func = [&](int idx) -> void {
		cout << test[idx] << endl;
	};

	parallel_for(test, 4, func);
	*/
	
	try {

		SetForm form(path);
		singletonFormInstance = &form;

		//string videoPath = "D:\\out_rainy.mp4";
		//singletonFormInstance->setSet(videoPath, true);
		form.run();
	}
	catch (exception e) {
		cout << e.what() << endl;
	}
	cin.get();
	return 0;
}
