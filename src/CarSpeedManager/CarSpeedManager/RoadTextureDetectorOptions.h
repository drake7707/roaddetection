#pragma once


struct RoadTextureDetectorOptions {

	bool enabled = true;
	// The minimum difference between the center pixel and its neighbours in the LBP to be considered different
	int lbpMinDiff = 2;
	// The patch size to divide the image in
	int lbpPatchSize = 16;
	// The number of classes in the histogram for each patch
	int nrOfHistogramBins = 20;

	// The percentage of the total width of the image that should be used as a reference
	double referenceHistogramsBottomRowsMiddlePercentage = 0.3;
	// The maximum difference when comparing histograms of patches with the histograms from patches in the reference area
	double maxDifferenceToReferenceHistograms = 0.15;

	// The number of erosion/dilation steps that need to be done to close holes.
	int dilationIterations = 5;

	// If true store the most occurring patches from previous frames to test against as well
	bool usePreviousBestHistogramMatches = true;

	// Ignore the top % of the image
	double ignoreTopHeight = 0.4;

};