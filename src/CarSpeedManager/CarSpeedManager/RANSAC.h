#pragma once


#include "opencv2/opencv.hpp"
#include <vector>
#include <unordered_set>

using namespace std;
using namespace cv;

namespace Algorithms {


	struct LineSegment {

		LineSegment(Point2i p1, Point2i p2) : p1(p1), p2(p2) { }
		Point2i p1;
		Point2i p2;
	};

	class RANSAC {

	private:
		int nrOfIterations = 10;
		int distanceTresholdForFitting = 5;
		int minPointsPerSegmentRequired = 10;

		int currentIteration = 0;

		Point2i maxP1;
		Point2i maxP2;
		int maxInlierCount = 0;
		unordered_set<int> pointsofMaxSegment;

		vector<Point2i> points;

	private:

		void step();

		double distance(Point2i& p1, Point2i& p2) {
			return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
		}

	public:
		RANSAC(vector<Point2i>& points, int distanceTresholdForFitting, int minPointsPerSegmentRequired, int nrOfIterations) 
			: points(points), distanceTresholdForFitting(distanceTresholdForFitting), nrOfIterations(nrOfIterations), minPointsPerSegmentRequired(minPointsPerSegmentRequired){

		}

		vector<LineSegment> getBestLineSegments(int nrOflineSegments);
	};

}