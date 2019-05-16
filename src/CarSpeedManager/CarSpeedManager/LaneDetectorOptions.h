#pragma once



struct LaneDetectorOptions {

public:

	bool enabled = true;

	// Percentage padding on bottom row to start the ray starting points
	double pathTracingBottomRowPadding = 0.25;

	// Maximum number of reflections the rays can undergo
	int pathTracingDepth = 5;
	// Number of ray spawn points at the bottom of the image
	int pathTracingNrOriginSpawnPoints = 10;

	// Number of rays per spawn point spawned over an arc of 180°
	int pathTracingNrOfRaysPerOrigin = 64;

	// Maximum distance between contour points to be considered neighbours (and thus can be
	// connected to each other in the road contour
	int maxDistanceBetweenContourPoints = 100;

	// Hough line threshold for filling the gaps between canny segments
	double houghLineTreshold = 50;
	// Maximum angle deviation from the vertical that's allowed for the Hough segments to be accepted
	double houghLinesMaxAngleDeviationDegrees = 20;
	// Maximum line gap between points of the Hough line segment
	double houghLineMaxLineGap = 200;

	// percentage of the full frame that's ignored at the top. This is used to remove pixels that are environment
	// and never road. They will only produce noise and slow everything down
	double ignoreTopHeight = 0.4;

	// Upper threshold for the Canny edge detection
	int cannyUpperThreshold = 100;
	// Absolute minimum that the upper threshold of the Canny edge detection can be
	double minimumCannyUpperThreshold = 10;
	// Number of steps the upper threshold is reduced starting from the normal threshold at the bottom and 
	// linearly reduced to the absolute minimum towards the top
	int adaptiveCannyUpperThresholdSteps = 8;

	// Minimum arc length of the contours that the Canny edge detection produced. This is used
	// to remove all invalid contours cheaply while calculating the penalty is relatively expensive
	int minimumContourLengthTreshold = 10;

	// The top percentage of contours with lowest penalty value
	double contourEdgePenaltyTreshold = 0.15;

};

