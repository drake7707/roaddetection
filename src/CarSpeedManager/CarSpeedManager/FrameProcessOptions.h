#pragma once

#include "LaneDetectorOptions.h"
#include "RoadTextureDetectorOptions.h"
#include "ClusteringDetectorOptions.h"

class FrameProcessOptions {

public:	
	int finalErosion = 6;
	LaneDetectorOptions laneDetector;
	RoadTextureDetectorOptions roadTextureDetector;
	ClusteringDetectorOptions clusterDetector;

};