#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;


// Finds the intersection of two lines, or returns false.
// The lines are defined by (o1, p1) and (o2, p2).
bool intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2,
	Point2f &r)
{
	Point2f x = o2 - o1;
	Point2f d1 = p1 - o1;
	Point2f d2 = p2 - o2;

	float cross = d1.x*d2.y - d1.y*d2.x;
	if (abs(cross) < /*EPS*/1e-8)
		return false;

	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
	r = o1 + d1 * t1;
	return true;
}

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

int main_old(char argc, char** argv) {

	string path = "D:\\WP_20160217_12_40_57_Pro_noaudio.mp4";

	namedWindow("Input", CV_WINDOW_AUTOSIZE);
	namedWindow("Output", CV_WINDOW_AUTOSIZE);

	cv::VideoCapture cap(path, CAP_FFMPEG);

	cap.set(CV_CAP_PROP_POS_MSEC, 300000);

	auto mog = createBackgroundSubtractorMOG2();



	Mat img;
	Mat imgOutput;
	Mat edges;


	int gaussSize = 5;
	int gaussSigma1 = 500; // 0-100
	int gaussSigma2 = 50; // 0-100
	int angleDegrees = 45; //

	Mat gauss = cv::getGaussianKernel(gaussSize, gaussSigma1 / (float)100);
	Mat sqMatrix = Mat(gaussSize, gaussSize, CV_32FC1, Scalar(0));
	gauss.col(gauss.cols / 2).copyTo(sqMatrix.col(sqMatrix.cols / 2));
	Mat gauss2 = getGaussianKernel(gaussSize, gaussSigma2 / (float)100);
	Mat gauss2D;
	filter2D(sqMatrix, gauss2D, -1, gauss2.t());
	Mat dog;
	Sobel(gauss2D, dog, -1, 0, 1, 3);
	cv::abs(dog);

	Mat rot = getRotationMatrix2D(Point(dog.cols / 2, dog.rows / 2), angleDegrees, 1);
	Mat dogRot;
	warpAffine(dog, dogRot, rot, Size(dog.cols, dog.rows));

	Mat dogRot2;
	Mat rot2 = getRotationMatrix2D(Point(dog.cols / 2, dog.rows / 2), angleDegrees + 90, 1);
	warpAffine(dog, dogRot2, rot2, Size(dog.cols, dog.rows));

	Mat dilationKernel = Mat(5, 5, CV_8U, Scalar(0));
	int mid = dilationKernel.cols / 2;
	int width = 2;

	for (int i = mid - (width - 1); i <= mid + (width - 1); i++) {
		if (i >= 0 && i < dilationKernel.cols) {
			for (int j = 0; j < dilationKernel.rows; j++)
				dilationKernel.at<char>(j, i) = 1;
		}
	}

	while (cap.isOpened()) {


		cap.read(img);
		normalize(img, img, 0, 255, cv::NormTypes::NORM_MINMAX);


		Mat imgGrayscale;
		cvtColor(img, imgGrayscale, CV_RGB2GRAY);


		//Point2f srcPoints[4];
		//srcPoints[0] = Point2f(0.15 * img.cols, 0.25 * img.rows);
		//srcPoints[1] = Point2f(0.85 * img.cols, 0.25 * img.rows);
		//srcPoints[2] = Point2f(0, img.rows - 1);
		//srcPoints[3] = Point2f(img.cols - 1, img.rows - 1);

		//vector<Point2f> pts;
		//for (int i = 0; i < 4; i++)
		//	pts.push_back(srcPoints[i]);

		//Rect bbox = boundingRect(pts);
		//// top left, top right, bottom left, bottom right
		//vector<Point> bboxPoints = { Point(bbox.tl().x, bbox.tl().y), Point(bbox.br().x, bbox.tl().y), Point(bbox.tl().x, bbox.br().y), Point(bbox.br().x, bbox.br().y) };

		//Point2f dstPoints[4];
		//for (int i = 0; i < 4; i++) {
		//	dstPoints[i] = bboxPoints.at(i);
		//}

		//Mat perspectiveMatrix = getPerspectiveTransform(srcPoints, dstPoints);

		//// pas perspectief matrix toe
		//warpPerspective(img, img, perspectiveMatrix, Size(img.cols, img.rows));






		//mog->apply(imgOutput, imgOutput);



		//Mat dogApplied;
		//filter2D(imgGrayscale, dogApplied, -1, dogRot);

		//Mat dogApplied2;
		//filter2D(imgGrayscale, dogApplied2, -1, dogRot2);


	//	threshold(dogApplied, dogApplied, 64, 255, cv::THRESH_BINARY);
	//	threshold(dogApplied2, dogApplied2, 64, 255, cv::THRESH_BINARY);

		//Mat result = dogApplied+ dogApplied2;
		Mat result = imgGrayscale;

		normalize(result, result, 0, 255, cv::NormTypes::NORM_MINMAX);
		//threshold(result, result, 64, 255, cv::THRESH_BINARY);

		//dilate(result, result, dilationKernel);

		Mat gauss = cv::getGaussianKernel(gaussSize, gaussSigma1 / (float)100);
		cv::filter2D(result, result, -1, gauss);

		// remove top part
	//	result(Rect(0, 0, result.cols, result.rows / 2)) = 0;

		imshow("Dog applied", result);


		float downScaleFactor = 2;
		if (downScaleFactor > 1)
			resize(result, result, Size(img.cols / downScaleFactor, img.rows / downScaleFactor));


		Canny(result, edges, 50, 100, 3, false);


		imgOutput = edges;


		/*vector<Vec4i> lines;
		HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 20 / downScaleFactor, 10 / downScaleFactor);




		for (size_t i = 0; i < lines.size(); i++)
		{
			Vec4i l = lines[i];

			Vec2f v1(l[2] - l[0], l[3] - l[1]);
			v1 = v1 / norm(v1);

			Vec2f v2(1, 0);

			double angle = atan2(v1[1], v1[0]);
			angle = (angle > 0 ? angle : (2 * CV_PI + angle));

			if (angle > CV_PI)
				angle -= CV_PI;

			auto angle2 = angle > CV_PI / 2 ? angle - CV_PI / 2 : angle  + CV_PI / 2;
			if (abs(angle - CV_PI / 4) < (CV_PI * 2) / 10 || abs(angle2 - CV_PI / 4) < (CV_PI * 2) / 10) {
				line(img, Point(l[0] * downScaleFactor, l[1] * downScaleFactor), Point(l[2] * downScaleFactor, l[3] * downScaleFactor), Scalar(0, 255, 0), 2, CV_AA);


				for (int j = 0; j < lines.size(); j++)
				{
					Vec4i l2 = lines[j];

					if (i != j) {
						Point2f ip;
						if (intersection(Point(l[0], l[1]), Point(l[2], l[3]), Point(l2[0], l2[1]), Point(l2[2], l2[3]), ip)) {

							ip.x = ip.x * downScaleFactor;
							ip.y = ip.y * downScaleFactor;
							circle(img, ip, 3, Scalar(0, 0, 255), 2);
						}
					}
				}
			}
		}*/

		/*for (int i = 0; i < 100; i++)
		{
			double angle = i / (float)100 * CV_PI * 2;

			if ((angle > CV_PI / 4 && angle < 3 * CV_PI / 4) || (angle > 5 * CV_PI / 4 && angle < 7 * CV_PI / 4))
				line(img, Point(200,200), Point(200 + 100 * cos(angle), 200 + 100 * sin(angle)), Scalar(255, 0, 0), 3, CV_AA);
			else
				line(img, Point(200, 200), Point(200 + 100 * cos(angle), 200 + 100 * sin(angle)), Scalar(255, 255, 0), 3, CV_AA);
		}*/



		imshow("Input", img);
		imshow("Output", imgOutput);


		auto key = (waitKey(1) & 0xFF);
		if (key == 'q')
			break;
		else if (key == 'f') {
			auto curMsec = cap.get(CV_CAP_PROP_POS_MSEC);
			cap.set(CV_CAP_PROP_POS_MSEC, curMsec + 300000);
		}
	}

	cap.release();
	destroyAllWindows();

	return 0;
}


void kMeans(Mat& img, Mat& mOut, int k) {
	
	int origRows = img.rows;
	
	Mat colVec = img.reshape(1, img.rows*img.cols); // change to a Nx3 column vector
	
	Mat colVecD, bestLabels, centers, clustered;
	int attempts = 1;
	int clusts = k;
	double eps = 0.001;
	colVec.convertTo(colVecD, CV_32FC3, 1.0 / 255.0); // convert to floating point
	
	double compactness = cv::kmeans(colVecD, clusts, bestLabels,
		TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, attempts, eps),
		attempts, KMEANS_PP_CENTERS, centers);
	Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
	
	labelsImg.convertTo(labelsImg, img.type());

	mOut = labelsImg;
}

int main(char argc, char** argv) {

	string path = "D:\\WP_20160217_12_40_57_Pro_noaudio.mp4";

	namedWindow("Input", CV_WINDOW_AUTOSIZE);
	namedWindow("Output", CV_WINDOW_AUTOSIZE);

	cv::VideoCapture cap(path, CAP_FFMPEG);

	cap.set(CV_CAP_PROP_POS_MSEC, 300000);

	Mat img;
	Mat imgOutput;
	


	int gaussSize = 5;
	int gaussSigma1 = 500; // 0-100

	int cannyTreshold1 = 170;
	int cannyTreshold2 = 300;

	int alpha = 100;

	int dilationSize = 5;

	int lightnessLowerTreshold = 190;

	int claheSize = 2;

	createTrackbar("gaussSize", "Output", &gaussSize, 100);
	createTrackbar("gaussSigma1", "Output", &gaussSigma1, 500);

	createTrackbar("cannyTreshold1", "Output", &cannyTreshold1, 500);
	createTrackbar("cannyTreshold2", "Output", &cannyTreshold2, 500);

	createTrackbar("alpha", "Output", &alpha, 500);

	createTrackbar("dilationSize", "Output", &dilationSize, 50);

	createTrackbar("claheSize", "Output", &claheSize, 50);

	createTrackbar("lightnessLowerTreshold", "Output", &lightnessLowerTreshold,255);

	while (cap.isOpened()) {

		cap.read(img);
		normalize(img, img, 0, 255, cv::NormTypes::NORM_MINMAX);

		Mat imgGrayscale;
		cvtColor(img, imgGrayscale, CV_BGR2GRAY);

		Mat hls;
		cvtColor(img, hls, CV_BGR2HLS);

		Mat channels[3];
		cv::split(hls, channels);

		Mat imgLightness = channels[1];

		auto clahe =cv::createCLAHE(claheSize, Size(32,32));

		Mat claheLightness;
		clahe->apply(imgLightness, imgLightness);
		clahe->apply(imgGrayscale, imgGrayscale);
		//equalizeHist(imgLightness, imgLightness);		
		//equalizeHist(imgGrayscale, imgGrayscale);

		if (gaussSize > 0) {
			Mat gauss = cv::getGaussianKernel(gaussSize, gaussSigma1 / (float)100);
			cv::filter2D(imgGrayscale, imgGrayscale, -1, gauss);
		}

		
		Mat result1;
		threshold(imgLightness, result1, lightnessLowerTreshold, 255, cv::THRESH_TOZERO);

		if (dilationSize > 0) {
			Mat dilationKernel = Mat(dilationSize, dilationSize, CV_8U, Scalar(1));
			dilate(result1, result1, dilationKernel);
		}

		imshow("Before canny1", result1);
		imshow("Before canny2", imgGrayscale);

		Mat edges, edges2;
		Canny(result1, edges, cannyTreshold1, cannyTreshold2, 3, false);
		Canny(imgGrayscale, edges2, cannyTreshold1, cannyTreshold2, 3, false);

		edges += edges2;

		//resize(result, result, Size(result.cols / 2, result.rows / 2));
	//kMeans(result, result, 3); 
	//resize(result, result, Size(result.cols * 2, result.rows * 2));

		
		//result *=(255 / 3);		

		imgOutput = edges;
		imshow("Output", imgOutput);
		if (false) {
			cap.read(img);
			normalize(img, img, 0, 255, cv::NormTypes::NORM_MINMAX);

			Mat imgGrayscale;
			cvtColor(img, imgGrayscale, CV_RGB2GRAY);

			Mat result = imgGrayscale;



			result.convertTo(result, CV_8U);

			//normalize(result, result, 0, 255, cv::NormTypes::NORM_MINMAX);
			equalizeHist(result, result);

			result = (alpha / (float)100) * result;


			if (gaussSize > 0) {
				Mat gauss = cv::getGaussianKernel(gaussSize, gaussSigma1 / (float)100);
				cv::filter2D(result, result, -1, gauss);
			}



			float downScaleFactor = 1;
			if (downScaleFactor > 1)
				resize(result, result, Size(img.cols / downScaleFactor, img.rows / downScaleFactor));


			if (dilationSize > 0) {
				Mat dilationKernel = Mat(dilationSize, dilationSize, CV_8U, Scalar(1));
				erode(result, result, dilationKernel);
			}

			imshow("Before canny", result);

			Canny(result, edges, cannyTreshold1, cannyTreshold2, 3, false);


			// remove top part
			result(Rect(0, 0, result.cols, result.rows * 0.4)) = 0;


			imgOutput = edges;

			vector<Vec4i> lines;
			HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 20 / downScaleFactor, 10 / downScaleFactor);




			for (size_t i = 0; i < lines.size(); i++)
			{
				Vec4i l = lines[i];

				Vec2f v1(l[2] - l[0], l[3] - l[1]);
				v1 = v1 / norm(v1);

				Vec2f v2(1, 0);

				line(img, Point(l[0] * downScaleFactor, l[1] * downScaleFactor), Point(l[2] * downScaleFactor, l[3] * downScaleFactor), Scalar(0, 255, 0), 2, CV_AA);
			}

			imshow("Input", img);
			imshow("Output", imgOutput);

		}



	

		auto key = (waitKey(1) & 0xFF);
		if (key == 'q')
			break;
		else if (key == 'f') {
			auto curMsec = cap.get(CV_CAP_PROP_POS_MSEC);
			cap.set(CV_CAP_PROP_POS_MSEC, curMsec + 300000);
		}
	}

	cap.release();
	destroyAllWindows();

	return 0;
}