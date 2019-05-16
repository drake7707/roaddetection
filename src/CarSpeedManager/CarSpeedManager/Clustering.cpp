#pragma once

#include <vector>
#include <mutex>
#include "Async.h"

namespace Algorithms {

	class FeatureVector;

	class FeatureVector {

	public:
		std::vector<double> values;
		std::vector<double> tags;

		FeatureVector(std::vector<double>& values, std::vector<double>& tags) : values(values), tags(tags) {

		}

		double distanceTo(const FeatureVector& p) {
			double sumSquares = 0;
			for (int i = 0; i < this->values.size(); i++) {
				sumSquares += (p.values[i] - this->values[i]) * (p.values[i] - this->values[i]);
			}

			return sqrt(sumSquares);
		}

		double distanceToSquared(const FeatureVector& p) {
			double sumSquares = 0;
			for (int i = 0; i < this->values.size(); i++)
				sumSquares += (p.values[i] - this->values[i]) * (p.values[i] - this->values[i]);
			return sumSquares;
		}

		static FeatureVector average(const std::vector<FeatureVector>& pts) {
			int dims = pts[0].values.size();
			std::vector<double> values(dims, 0);


			for (auto& p : pts) {
				for (int i = 0; i < dims; i++)
					values[i] += p.values[i];
			}

			for (int i = 0; i < values.size(); i++) {
				values[i] /= pts.size();
			}

			std::vector<double> tagValues;
			return FeatureVector(values, tagValues);
		}

		static FeatureVector average(const std::vector<FeatureVector*>& pts) {
			int dims = pts[0]->values.size();
			std::vector<double> values(dims, 0);


			for (auto& p : pts) {
				for (int i = 0; i < dims; i++)
					values[i] += p->values[i];
			}

			for (int i = 0; i < values.size(); i++) {
				values[i] /= pts.size();
			}

			std::vector<double> tagValues;
			return FeatureVector(values, tagValues);
		}
	};



	class KMeans {

	public:
		int currentIteration = 0;
		std::vector<std::vector<FeatureVector*> > vectorsPerCluster;

		std::vector<FeatureVector> centroids;
		double currentDeltaDistanceDifference = 0;
		int k = 0;

		std::vector<FeatureVector>* points;

		KMeans() {

		}

		KMeans(std::vector<FeatureVector>* points, int k, std::vector<FeatureVector>& centroids) : points(points), k(k) {

			if (centroids.size() > 0)
				this->centroids = centroids;
			else
				this->initCentroids();

			this->vectorsPerCluster = std::vector<std::vector<FeatureVector*> >(k);

			// reserve enough room
			for (int i = 0; i < k; i++)
				this->vectorsPerCluster[i].reserve(points->size() / k);
		}

		virtual void initCentroids() {
			for (int i = 0; i < this->k; i++) {
				this->centroids.push_back(this->points->at(rand() % points->size()));
			}
		}


		void buildVectorsPerCluster() {
			// clear category
			for (int i = 0; i < this->k; i++) {
				this->vectorsPerCluster[i].clear();
			}

			// calculate points per centroid

			for (auto& p : *(this->points)) {

				double minDist = std::numeric_limits<double>().max();
				int centroidIndex = -1;
				for (int k = 0; k < this->k; k++) {

					// selecting the index of the minimum distance, so it doesn't matter if it's squared or not, 
					// this saves a sqrt(), which is otherwise run n * k times
					double dist = this->centroids[k].distanceToSquared(p);

					if (dist < minDist) {
						centroidIndex = k;
						minDist = dist;
					}
				}
				this->vectorsPerCluster[centroidIndex].push_back(&p);
			}
		}

		void step() {
			this->buildVectorsPerCluster();

			double totalDistanceDiff = 0;
			// adjust centroids
			for (int k = 0; k < this->vectorsPerCluster.size(); k++) {
				std::vector<FeatureVector*>& cat = this->vectorsPerCluster[k];
				if (cat.size() > 0) {
					FeatureVector& avg = FeatureVector::average(cat);

					auto dist = this->centroids[k].distanceTo(avg);
					totalDistanceDiff += dist;
					this->centroids[k] = avg;
				}
			}
			this->currentDeltaDistanceDifference = totalDistanceDiff;
			this->currentIteration++;

		}
	};

	class KMeansPlusPlus : public KMeans {


	public:
		KMeansPlusPlus() {

		}

		KMeansPlusPlus(std::vector<FeatureVector>* points, int k) : KMeans(points, k, std::vector<FeatureVector>()) {

		}

		virtual void initCentroids() {
			// add initial center
			this->centroids.push_back(this->points->at(rand() % points->size()));


			while (this->centroids.size() < this->k) {
				// determine distances of all points to its nearest centroid
				std::vector<double> distances(points->size(), 0);// = new Array(this.points.length);
				double maxDistance = 0;
				for (int i = 0; i < points->size(); i++) {
					double  minDist = std::numeric_limits<double>().max();

					// determine distance to nearest centroid
					for (auto& c : this->centroids) {
						double dist = this->points->operator[](i).distanceToSquared(c);
						if (minDist < dist)
							minDist = dist;
					}
					distances[i] = minDist;
					if (minDist > maxDistance)
						maxDistance = minDist;
				}
				// normalize distances
				for (int i = 0; i < distances.size(); i++)
					distances[i] /= maxDistance;

				// select random center according to weighted distribution of dist*dist
				int selectedIndex = this->random(this->points, [&](int idx) -> double { return distances[idx] * distances[idx]; });

				this->centroids.push_back(this->points->operator[](selectedIndex));

				// repeat until there are k centroids
			}
		}

	private:
		double random(std::vector<FeatureVector>* elements, std::function<double(int)> weightFunc) {
			double totalWeight = 0; // this stores sum of weights of all elements before current
			int selected = -1;
			for (int i = 0; i < elements->size(); i++) {
				double weight = weightFunc(i); // weight of current element
				double r = ((double)rand() / (RAND_MAX)) * (totalWeight + weight); // random value
				if (r >= totalWeight) // probability of this is weight/(totalWeight+weight)
					selected = i; // it is the probability of discarding last selected element and selecting current one instead
				totalWeight += weight; // increase weight sum
			}

			return selected; // when iterations end, selected is some element of sequence. 
		}
	};

	class MeanShift {

	public:
		double windowRadius = 50;
		double mergeWhenDistanceIsSmallerThanPercentageOfWindowRadius = 0.5;
		std::vector<FeatureVector> centroids;
		std::vector<std::vector<FeatureVector*> > vectorsPerCluster;

		double currentDeltaDistanceDifference = 0;
		int currentIteration = 0;

		std::vector<FeatureVector>* points;

		MeanShift() {

		}

		MeanShift(std::vector<FeatureVector>* points, std::vector<FeatureVector>& centroids, double windowRadius, double mergeWhenDistanceIsSmallerThanPercentageOfWindowRadius)
			: points(points), centroids(centroids), windowRadius(windowRadius), mergeWhenDistanceIsSmallerThanPercentageOfWindowRadius(mergeWhenDistanceIsSmallerThanPercentageOfWindowRadius) {

			this->vectorsPerCluster = std::vector<std::vector<FeatureVector*> >(centroids.size());
			// reserve enough room
			for (int i = 0; i < centroids.size(); i++)
				this->vectorsPerCluster[i].reserve(points->size() / centroids.size());
		}

		void calculateVectorsPerCluster() {
			// clear category
			for (int i = 0; i < this->centroids.size(); i++)
				this->vectorsPerCluster[i].clear();

			// determine all points of all centroids within the window radius
			for (auto& p : *(this->points)) {

				for (int c = 0; c < this->centroids.size(); c++) {
					double dist = this->centroids[c].distanceTo(p);
					if (dist < this->windowRadius) {
						this->vectorsPerCluster[c].push_back(&p);
					}
				}
			}
		}

		void step() {
			this->calculateVectorsPerCluster();

			double totalDistanceDiff = 0;
			// adjust centroids based on their new points
			for (int c = 0; c < this->centroids.size(); c++) {
				auto& cat = this->vectorsPerCluster[c];
				if (cat.size() > 0) {
					FeatureVector avg = FeatureVector::average(cat);
					double dist = this->centroids[c].distanceTo(avg);
					totalDistanceDiff += dist;
					this->centroids[c] = avg;
				}

			}
			this->currentDeltaDistanceDifference = totalDistanceDiff;

			// merge centroids that are close
			double maxDistance = this->mergeWhenDistanceIsSmallerThanPercentageOfWindowRadius * this->windowRadius;

			std::vector<bool> merged(this->centroids.size(), false);

			std::vector<FeatureVector> newCentroids;

			for (int c1 = 0; c1 < this->centroids.size(); c1++) {
				std::vector<FeatureVector> centroidsToMerge;
				// if not merged already
				if (!merged[c1]) {

					// check if there are other centroids close and add them to the list to merge
					for (int c2 = c1 + 1; c2 < this->centroids.size(); c2++) {
						if (!merged[c2] && this->centroids[c1].distanceTo(this->centroids[c2]) < maxDistance) {
							centroidsToMerge.push_back(this->centroids[c2]);
							merged[c2] = true;
						}
					}
					// if there are centroids to merge with the current one
					if (centroidsToMerge.size() > 0) {
						// add the current one and take the average
						centroidsToMerge.push_back(this->centroids[c1]);
						merged[c1] = true;
						FeatureVector newCentroid = FeatureVector::average(centroidsToMerge);
						newCentroids.push_back(newCentroid);
					}
					else {
						// no merging to be done, just take the centroid as is
						newCentroids.push_back(this->centroids[c1]);
					}
				}
				else {
					// the centroid is already merged
				}
			}
			// take the merged centroids as new basis
			this->centroids = newCentroids;

			// update clusters
			this->calculateVectorsPerCluster();

			this->currentIteration++;
		}

	};


	class Clustering {

	private:
		KMeans kmeans;
		MeanShift meanShift;
		std::vector<FeatureVector> vectors;
		bool isDoingKmeans;
		bool done = false;
		double windowRadius;
		double overlapThreshold;
		int initialK;
		int iteration = 0;

		void initialize() {
			this->isDoingKmeans = true;
			std::vector<FeatureVector> centroids = {};
			this->kmeans = KMeansPlusPlus(&(this->vectors), initialK);
		}

		void doKMeansStep() {
			this->kmeans.step();


			if (this->kmeans.currentDeltaDistanceDifference < 0.01) {
				this->isDoingKmeans = false;
				this->meanShift = MeanShift(&(this->vectors), this->kmeans.centroids, this->windowRadius, overlapThreshold);
				this->doMeanShiftStep();
			}
		}

		void doMeanShiftStep() {
			this->meanShift.step();


			if (this->meanShift.currentDeltaDistanceDifference < 0.01) {
				if (this->kmeans.k != this->meanShift.centroids.size()) {
					this->isDoingKmeans = true;
					this->kmeans = KMeans(&(this->vectors), this->meanShift.centroids.size(), this->meanShift.centroids);
					this->doKMeansStep();
				}
				else {
					this->done = true;
				}
			}
		}

	public:

		Clustering(std::vector<FeatureVector> vectors, int initialK, double windowRadius, double overlapThreshold) : vectors(vectors), initialK(initialK), windowRadius(windowRadius), overlapThreshold(overlapThreshold) {
			this->initialize();
		}

		bool isDone() const {
			return this->done;
		}

		void step() {
			if (this->isDoingKmeans)
				this->doKMeansStep();
			else
				this->doMeanShiftStep();
			this->iteration++;
		}

		std::vector<FeatureVector> getCentroids() const {
			return this->isDoingKmeans ? this->kmeans.centroids : this->meanShift.centroids;
		}

		std::vector<std::vector<FeatureVector*> > getVectorsPerCluster() const {
			return this->isDoingKmeans ? this->kmeans.vectorsPerCluster : this->meanShift.vectorsPerCluster;
		}

		double getCurrentDeltaDistanceDifference() const {
			return this->isDoingKmeans ? this->kmeans.currentDeltaDistanceDifference : this->meanShift.currentDeltaDistanceDifference;
		}
	};

};