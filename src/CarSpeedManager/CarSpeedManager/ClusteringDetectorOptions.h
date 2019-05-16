#pragma once


struct ClusteringDetectorOptions {

	bool enabled = true;

	// Initial starting value K for k-means. Cluster count can be reduced but can never be increased
	int initialNrOfClusters = 10;

	// Window radius of mean shift step. As all values are normalized between [0-1]. A value of 0.1 is 10% radius of the total space.
	double windowRadius = 0.1;
	// The overlap threshold of clusters. If 2 clusters overlap more they will be merged in the mean shift step
	double overlapThreshold = 0.75;

	// Number of dilation/erosion steps to remove holes from the mask
	int dilationIterations = 5;

	// The factor to shrink the frame with to reduce the number of pixels and speed up performance
	int downScaleFactor = 4;

	// The percentage of the width to use as reference. Clusters represented in that area will be interpreted as road.
	double bottomRowPaddingPercentage = 0.2;
	// The height of the reference area
	int bottomRowHeight = 16;

	// The max. number of clusters in the reference area to take as road
	int topNClustersOnBottomRow = 3;

	// The minimum number of pixels a cluster must contain in the reference area. Clusters that are poorly represented in the
	// reference area will be discarded
	int minNrOfPixelsInClusterInReference = 100;

	// Ignore the top % of the image
	double ignoreTopHeight = 0.4;

};