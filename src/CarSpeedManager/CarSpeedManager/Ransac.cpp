#pragma once
#include "RANSAC.h"
#include <unordered_set>

namespace Algorithms {


	void RANSAC::step() {

		vector<int> currentInliers;
		currentInliers.reserve(this->points.size());
		vector<int> currentOutliers;
		currentOutliers.reserve(this->points.size());

		Point2i curP1;
		Point2i curP2;;


		// take 2 samples randomly
		auto p1 = this->points[rand() % points.size()];
		auto p2 = this->points[rand() % points.size()];
		curP1 = p1;
		curP2 = p2;

		double lineLength = distance(p1, p2);
		double dy = p2.y - p1.y;
		double dx = p2.x - p1.x;

		for (int i = 0; i < points.size(); i++)
		{
			Point2i p = points[i];

			double distance = std::abs(dy * p.x - dx * p.y + p2.x * p1.y - p2.y * p1.x) / lineLength;
			if (distance < this->distanceTresholdForFitting) {
				// inlier
				currentInliers.push_back(i);
			}
			else {
				// outlier
				currentOutliers.push_back(i);
			}
		}

		if (this->maxInlierCount <= currentInliers.size()) {
			this->maxP1 = p1;
			this->maxP2 = p2;
			this->maxInlierCount = currentInliers.size();

			this->pointsofMaxSegment.clear();
			this->pointsofMaxSegment.reserve(currentInliers.size());
			for (int i = 0; i < currentInliers.size(); i++)
				this->pointsofMaxSegment.emplace(currentInliers[i]);

		}
		this->currentIteration++;
	}


	vector<LineSegment> RANSAC::getBestLineSegments(int nrOfSegments) {

		vector<LineSegment> segments;

		while (segments.size() < nrOfSegments && this->points.size() > 2) {

			this->currentIteration = 0;
			this->maxInlierCount = 0;
			this->pointsofMaxSegment.clear();
			while (this->currentIteration < this->nrOfIterations) {
				step();
			}

			if (pointsofMaxSegment.size() > minPointsPerSegmentRequired) {
				segments.push_back(LineSegment(maxP1, maxP2));

				vector<Point2i> newPoints;
				newPoints.reserve(points.size() - pointsofMaxSegment.size());

				for (int i = 0; i < points.size(); i++)
				{
					if (this->pointsofMaxSegment.find(i) == this->pointsofMaxSegment.end()) {
						// als point niet in de points van de gevonden line segment zit
						newPoints.push_back(points[i]);
					}
				}
				points = newPoints;
			}
			else {
				// done , te weinig punten gevonden voor nog een segment te maken
				break;
			}
		}

		return segments;
	}
}
