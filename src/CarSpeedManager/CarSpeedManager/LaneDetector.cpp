#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include "RANSAC.h"

#include "FrameProcessor.h"
#include "FrameProcessResult.h"

#include "FrameProcessOptions.h"
#include "LaneDetector.h"
#include "Otsu.h"
#include "LocalBinaryPattern.h"

using namespace std;

namespace LaneDetector {


	/*
		Finds the hitpoint going from the source to the destination point on the canny image
		using Bresenham's line algorithm to step pixel by pixel
	*/
	Point findHitPoint(Point& src, Point& dst, Mat& canny) {
		// bresenham's line algorithm
		int x = src.x;
		int y = src.y;
		int x2 = dst.x;
		int y2 = dst.y;

		int w = x2 - x;
		int h = y2 - y;
		int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
		if (w < 0) dx1 = -1; else if (w>0) dx1 = 1;
		if (h < 0) dy1 = -1; else if (h>0) dy1 = 1;
		if (w < 0) dx2 = -1; else if (w>0) dx2 = 1;
		int longest = abs(w);
		int shortest = abs(h);
		if (!(longest > shortest)) {
			longest = abs(h);
			shortest = abs(w);
			if (h < 0) dy2 = -1; else if (h>0) dy2 = 1;
			dx2 = 0;
		}
		int numerator = longest >> 1;
		for (int i = 0; i <= longest; i++) {

			// return the bottom of the image if the ray exits at the bottom
			if (y >= canny.rows)
				return Point(x, canny.rows);

			// if the ray goes outside the frame, return -1,-1 as in no hit point found
			if (x < 0 || y < 0 || x >= canny.cols || y >= canny.rows)
				return Point(-1, -1);

			if (canny.at<uchar>(Point(x, y)) > 0)
				return Point(x, y);

			numerator += shortest;
			if (!(numerator < longest)) {
				numerator -= longest;
				x += dx1;
				y += dy1;
			}
			else {
				x += dx2;
				y += dy2;
			}
		}
		return Point(-1, -1);
	}


	struct PrimNode {
		PrimNode(int parent, int pIndex, double cost, int depth) : pIndex(pIndex), cost(cost), parent(parent), depth(depth) { }
		int pIndex;
		double cost;
		int depth;
		int parent;

		bool operator<(const PrimNode& n) const {
			return this->cost > n.cost;
		}
	};

	/*
		Returns the longest path between the points, starting from the given root point
	*/
	vector<Point> getLongestPath(Mat& img, vector<Point>& points, int maxDistanceBetweenPoints, int initialRootIdx) {
		if (points.size() == 0)
			return vector<Point>();

		int rootIdx = initialRootIdx;

		// prim algorithm

		// we need to be able to update the cost of elements that are already in the priority queue, so we use a vector instead
		// of a prio_queue (which does not support this)
		vector<PrimNode> prioqueue;
		vector<bool> visited(points.size(), false);

		// push the root point with 0 cost
		prioqueue.push_back(PrimNode(-1, rootIdx, 0, 0));

		// push in all nodes with a maximum cost value
		// the cost will be updated later on
		for (int i = 0; i < points.size(); i++) {
			if (i != rootIdx)
				prioqueue.push_back(PrimNode(-1, i, std::numeric_limits<int>().max(), -1));
		}
		// heapify
		std::make_heap(prioqueue.begin(), prioqueue.end());

		// keep track of what the parent is of each node in the graph, then we can easily traverse back to the root later
		vector<int> parents(points.size(), -1);

		// keep track of the deepest node so far
		int maxDepthIndex = -1;
		int maxDepth = -1;

		// keep going as long as there are nodes to be processed
		while (prioqueue.size() > 0) {

			// process the next node
			PrimNode n = prioqueue.front();
			std::pop_heap(prioqueue.begin(), prioqueue.end());
			prioqueue.pop_back();

			// keep track of the parent and mark it as processed (visited)
			parents[n.pIndex] = n.parent;
			visited[n.pIndex] = true;

			// check if this node is deeper than the deepest so far
			if (maxDepth < n.depth) {
				maxDepth = n.depth;
				maxDepthIndex = n.pIndex;
			}

			// check all the remaining nodes and determine the neighbours of the current node
			// nodes are neighbours when there are close enough together
			for (int i = 0; i < prioqueue.size(); i++)
			{
				PrimNode& neighbour = prioqueue[i];

				if (!visited[neighbour.pIndex]) {
					// determine distance between each other
					double distance = sqrt((points[neighbour.pIndex].x - points[n.pIndex].x) * (points[neighbour.pIndex].x - points[n.pIndex].x) +
						(points[neighbour.pIndex].y - points[n.pIndex].y) * (points[neighbour.pIndex].y - points[n.pIndex].y));

					// if the distance is acceptable and the cost was higher than the distance update the neighbour's cost
					if (distance < maxDistanceBetweenPoints &&  neighbour.cost >= distance) {
						neighbour.cost = distance;
						neighbour.depth = n.depth + 1;
						neighbour.parent = n.pIndex;
					}
				}
			}
			// heapify the queue again after updating the costs
			std::make_heap(prioqueue.begin(), prioqueue.end());
		}

		// take the deepest point & traverse back to root
		vector<Point> result;
		result.reserve(points.size());

		int curPointIndex = maxDepthIndex;
		while (curPointIndex != -1) {
			result.push_back(points[curPointIndex]);
			curPointIndex = parents[curPointIndex];
		}

		return result;
	}

	/**
		Returns the best path from the intersectionpoints
	*/
	vector<Point> getPath(Mat& img, Mat& overlay, vector<Point>& points, int maxDistanceBetweenPoints) {

		std::unordered_set<long> pointsFromPath;

		vector<Point> pts;
		pts.reserve(points.size());

		// filter out all duplicate points
		std::unordered_set<long> hasPoint;
		for (auto& p : points) {
			if (pointsFromPath.find(p.y * img.cols + p.x) == pointsFromPath.end() && hasPoint.find(p.y * img.cols + p.x) == hasPoint.end()) {
				hasPoint.emplace(p.y * img.cols + p.x);
				pts.push_back(p);
			}
		}

		// find root, closest to the bottom
		int rootIdx = -1;
		double minDistToCenterBottom = std::numeric_limits<double>().max();
		Point rightBottom(img.cols, img.rows);
		for (int i = 0; i < pts.size(); i++)
		{
			double distance = sqrt((points[i].x - rightBottom.x) * (points[i].x - rightBottom.x) + (points[i].y - rightBottom.y) * (points[i].y - rightBottom.y));
			if (distance < minDistToCenterBottom) {
				rootIdx = i;
				minDistToCenterBottom = distance;
			}
		}

		auto path1 = getLongestPath(img, pts, maxDistanceBetweenPoints, rootIdx);
		// now path1 will be a path from the deepest point to the root idx point.
		// if the root point was in the middle of the path then only 1 segment of the full path will be detected
		// so choose the new root point as the deepest point of path1 (which is the outer point of the entire path) and build a longest path again.
		for (int i = 0; i < pts.size(); i++)
		{
			if (pts[i].x == path1.at(0).x && pts[i].y == path1.at(0).y) {
				rootIdx = i;
				break;
			}
		}
		path1 = getLongestPath(img, pts, maxDistanceBetweenPoints, rootIdx);

		return path1;
	}

	FrameProcessResult classify(Mat& frame, Mat& roadMask, LaneDetectorOptions& options, bool generateAnnotations) {

		// create an overlay for the annotations
		Mat overlay = Mat(Size(frame.cols, frame.rows), CV_8UC3, Scalar(0, 0, 0));

		// normalize the image so the canny tresholds are representable
		Mat normFrame;
		normalize(frame, normFrame, 0, 255, NormTypes::NORM_MINMAX);

		// Create a strong gaussian blur
		Mat blur;
		GaussianBlur(normFrame, blur, Size(7, 7), 100);

		// Apply the gaussian blur, varying in strength depending on the location of the pixel in the image
		// Blur more strongly in the towards the horizontal center and towards the bottom (or "front" of the road)
		Mat adaptiveBlur = frame.clone();
		for (int j = 0; j < blur.rows; j++)
		{
			for (int i = 0; i < blur.cols; i++)
			{
				// determine linear interpolation alpha's
				double alphaX = 1 - abs(blur.cols / (float)2 - i) / (float)(blur.cols / (float)2);
				double alphaY = (j / (float)blur.rows);

				// give slightly more weight to Y direction than X
				double alpha = alphaX * 0.3 + alphaY * 0.7;
				Vec3b blurRGB = blur.at<Vec3b>(Point(i, j));
				Vec3b frameRGB = frame.at<Vec3b>(Point(i, j));
				adaptiveBlur.at<Vec3b>(Point(i, j)) = alpha * blurRGB + frameRGB * (1 - alpha);
				//adaptiveBlur.at<Vec3b>(Point(i, j)) = Vec3b(alpha * 255, alpha * 255, alpha * 255);
			}
		}

		int cannyUpperTreshold = options.cannyUpperThreshold;// getOtsuTreshold(adaptiveBlur);
		double ignoreTopHeight = options.ignoreTopHeight;
		double minUpperCannyThreshold = options.minimumCannyUpperThreshold;
		// number of steps the canny upper threshold is reduced
		int thresholdLevels = options.adaptiveCannyUpperThresholdSteps;
		Mat canny = Mat(Size(frame.cols, frame.rows), CV_8UC1, Scalar(0));

		// determine the reduction in threshold per level
		int deltaCannyThreshold = thresholdLevels == 1 ? 0 : (options.cannyUpperThreshold - minUpperCannyThreshold) / (thresholdLevels-1);

		int bandHeight =  (1-ignoreTopHeight)* frame.rows / thresholdLevels;
		for (int yLevel = 1; yLevel <= thresholdLevels; yLevel++)
		{
			// determine the region of interest for the current level
			Rect roi(0, frame.rows - yLevel * bandHeight, frame.cols, bandHeight);
			Mat band = adaptiveBlur(roi);

			// and apply Canny on it with the current interpolated canny threshold
			Mat subCanny;

			double subCannyThreshold = cannyUpperTreshold - (yLevel - 1) * deltaCannyThreshold;
			Canny(band, subCanny, subCannyThreshold / 2, subCannyThreshold);

			subCanny.copyTo(canny(roi));
		}

		// clear the top of the image, it contains mostly environment which is noisy and can introduce false hough lines
		canny(Rect(0, 0, canny.cols, ignoreTopHeight * canny.rows)) = 0;


		// find the countours on the canny image
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(canny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		// clear the canny image
		canny(Rect(0, 0, canny.cols, canny.rows)) = 0;

		double maxMeasureOfContour = 0;

		// keep track for all countours what the calculated penalty is
		vector<double> counterPenaltyMeasures(contours.size(), 0);

		for (int i = 0; i < contours.size(); i++)
		{
			// remove contours that are too small
			auto length = arcLength(contours[i], false);
			if (length > options.minimumContourLengthTreshold) {

				auto contour = contours[i];
				Rect bbox = boundingRect(contour);

				// draw the contour on a temporary image
				Mat tmp = Mat(Size(bbox.width, bbox.height), CV_8UC1, Scalar(0));
				for (int i = 0; i < contour.size() - 1; i++)
					line(tmp, Point(contour[i].x - bbox.tl().x, contour[i].y - bbox.tl().y), Point(contour[i + 1].x - bbox.tl().x, contour[i + 1].y - bbox.tl().y), Scalar(255), 1);

				// calculate the density by checking which pixels are set
				int nrFilled = 0;
				for (int y = 0; y < tmp.rows; y++)
				{
					for (int x = 0; x < tmp.cols; x++)
					{
						if (tmp.at<uchar>(Point(x, y)) > 0)
							nrFilled++;
					}
				}
				// determine the squareness by calculating the ratio
				double ratio = bbox.height > bbox.width ? bbox.width / (float)bbox.height : bbox.height / (float)bbox.width;
				double centerBboxY = bbox.tl().y + bbox.height / (float)2;
				double centerBboxX = bbox.tl().x + bbox.width / (float)2;

				// calculate the penalty
				double measureOfContour = (nrFilled / (float)(bbox.width * bbox.height)) * ratio;
				counterPenaltyMeasures[i] = measureOfContour;

				// keep track of the maximum penalty
				if (measureOfContour > maxMeasureOfContour)
					maxMeasureOfContour = measureOfContour;
			}
		}

		// now iterate over all contours to calculate the percentage of
		for (int i = 0; i < contours.size(); i++)
		{
			// remove contours that are too small
			if (contours[i].size() > options.minimumContourLengthTreshold) {

				auto contour = contours[i];

				double measureOfContour = counterPenaltyMeasures[i];
				Rect bbox = boundingRect(contour);

				// draw the contours on the annotated image
				if (generateAnnotations) {
					rectangle(overlay, bbox, Scalar(255, 0, 0), 1);
					drawContours(overlay, contours, i, Scalar(0, (1 - measureOfContour / maxMeasureOfContour) * 255, (measureOfContour / maxMeasureOfContour) * 255), 2, LineTypes::LINE_8, hierarchy, 2, Point());
				}

				if (measureOfContour / maxMeasureOfContour < options.contourEdgePenaltyTreshold)
					drawContours(canny, contours, i, Scalar(255), 1, LineTypes::LINE_8, hierarchy, 2, Point());
			}
		}
		
		// Assist canny with hough lines, by specifying a big gap so it automatically fills holes
		vector<Vec4i> lines;
		HoughLinesP(canny, lines, 1, CV_PI / 180, options.houghLineTreshold, 0.0, options.houghLineMaxLineGap);
		for (auto& l : lines)
		{
			// create a normalized vector of the line segment
			Vec2f v1(l[2] - l[0], l[3] - l[1]);
			v1 = v1 / norm(v1);

			// calculate the angle of the line segment and the X-axis
			double angle = atan2(v1[1], v1[0]);
			angle = (angle > 0 ? angle : (2 * CV_PI + angle));

			// correct the angle so it's between [0, PI]
			if (angle > CV_PI)
				angle -= CV_PI;

			// determine secondary angle 45° further along
			auto angle2 = angle > CV_PI / 2 ? angle - CV_PI / 2 : angle + CV_PI / 2;


			// if the deviation of athe angle is smaller than the max deviation allowed, draw the line segment on the canny image
			double maxAngleDeviation = options.houghLinesMaxAngleDeviationDegrees / 180 * CV_PI;
			if (abs(angle - CV_PI / 4) < maxAngleDeviation || abs(angle2 - CV_PI / 4) < maxAngleDeviation) {
				line(canny, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1);
			}
		}

		// Calculate the derivative in X and Y with the sobel operator
		normFrame.convertTo(normFrame, CV_32F);
		Mat sobelX;
		Sobel(normFrame, sobelX, -1, 1, 0);
		Mat sobelY;
		Sobel(normFrame, sobelY, -1, 0, 1);

		// calculate the slope G_y / G_x
		Mat slope = sobelY / sobelX;

		// dilate the canny image to prevent diagonals leaking shot rays (as if they were passing  through the line)
		dilate(canny, canny, Mat(Size(3, 3), CV_8U, Scalar(255)));

		vector<Point2i> line_points;

		// determine the radius of enclosing circle of the entire image
		auto length = sqrt(frame.cols * frame.cols + frame.rows * frame.rows);

		int max_depth = options.pathTracingDepth;
		int nrOfOriginPoints = options.pathTracingNrOriginSpawnPoints;
		for (int i = 0; i < nrOfOriginPoints; i++)
		{
			// determine origin point for the ray
			Point initialOrigin = Point(options.pathTracingBottomRowPadding * frame.cols + i / (float)nrOfOriginPoints * (1 - 2 * options.pathTracingBottomRowPadding) * frame.cols, frame.rows - 1);

			// draw the origin point in red on the overlay
			if (generateAnnotations)
				circle(overlay, initialOrigin, 5, Scalar(0, 0, 255), -1);

			// shoot x nr of rays in an arc between [0, PI]
			for (double theta = 0; theta <= CV_PI; theta += CV_PI / options.pathTracingNrOfRaysPerOrigin)
			{

				Point origin(initialOrigin);

				// keep track of the direction vector
				Vec2f v(cos(theta), -sin(theta));

				// calculate the destination point on the enclosing frame circle 
				auto dst = Point(origin.x + length * v[0], origin.y + length * v[1]);

				// bounce around a few times
				int depth = max_depth;
				while (depth-- > 0) {

					// find the hitpoint wit the canny image
					Point hitPoint = findHitPoint(origin, Point(dst.x, dst.y), canny);
					if (hitPoint.x != -1 && hitPoint.y != -1) {
						// canny was hit or the bottom row was hit

						// check the bottom row, if the bottom row was hit bounce around the vertical to keep the rays going
						bool bottomRow = hitPoint.y == canny.rows;
						if (!bottomRow) {
							line_points.push_back(hitPoint);
							if (generateAnnotations) circle(overlay, hitPoint, 5, Scalar(0, 255, 0), -1);
						}

						// draw the ray trajectory to the hitpoint
						if (generateAnnotations) line(overlay, origin, hitPoint, Scalar(255, 255 - (1 - (depth / (float)max_depth)) * 255, 0), 1);


						// calculate reflected ray

						// y = m * x
						// determine the normal vector at the hitpoint
						Vec2f normal;
						if (bottomRow)
							normal = Vec2f(0, -1);
						else {
							normal = Vec2f(1, slope.at<float>(Point(hitPoint.x, hitPoint.y)));
							normal = normalize(normal);
						}
						// normalize the direction vector
						v = normalize(v);

						// draw the normal on the overlay
						if (generateAnnotations) line(overlay, hitPoint, Point2f(hitPoint.x + normal[0] * 30, hitPoint.y + normal[1] * 30), Scalar(255, 0, 0), 1);

						// calculate the cos(angle) with the dot product
						auto cosAngle = normal[0] * v[0] + normal[1] * v[1];

						// calculate the reflected vector
						Vec2f reflectedVector = Vec2f(v[0] - (2 * cosAngle) * normal[0], v[1] - (2 * cosAngle) * normal[1]);
						v = reflectedVector;
						// take the reflected vector as new direction vector
						v = normalize(v);

						// make sure to offset the new origin point with epsilon to avoid self intersection
						int epsilon = 1;
						origin = Point(hitPoint.x + epsilon * v[0], hitPoint.y + epsilon * v[1]);

						// get the new destination point on the circle around the frame
						dst = Point(origin.x + length * v[0], origin.y + length * v[1]);
					}
					else
						break;
				}
			}
		}

		cvtColor(canny, canny, CV_GRAY2BGR);

		Mat classificationMask = Mat(Size(frame.cols, frame.rows), CV_8UC1, Scalar(0));

		// determine the best contour of the road by finding the longest path in the intersection points
		auto contour = getPath(canny, overlay, line_points, options.maxDistanceBetweenContourPoints);
		if (contour.size() >= 2) {

			// draw the path in a big red line on the overlay
			if (generateAnnotations) {
				for (int i = 0; i < contour.size() - 1; i++)
					line(overlay, contour[i], contour[i + 1], Scalar(0, 0, 255), 8);
			}

			// add 2 points at the start and end of the path, which are the projected start and end points onto the X-axis at the bottom
			contour.push_back(Point(contour[contour.size() - 1].x, canny.rows));
			contour.push_back(Point(contour[0].x, canny.rows));

			const Point* lst = &(contour[0]);
			int num = (int)contour.size();
			// fill everything in the closed path
			cv::fillPoly(classificationMask, &lst, &num, 1, Scalar(255));
		}
		else {
			// unable to determine a mask, allow everything
			classificationMask(Rect(0, 0, classificationMask.cols, classificationMask.rows)) = 255;
		}

		// create the final annotated image
		if (generateAnnotations)
			canny = canny + overlay * 0.8 + frame * 0.3;

		// create & return the result
		FrameProcessResult result;
		result.setAnnotatedImage(AnnotatedImage(canny, [=](int x, int y) -> void {

		}));
		result.setClassifiedMask(classificationMask);

		return result;
	}
};