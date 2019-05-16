using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace GUI
{

    [StructLayout(LayoutKind.Sequential)]
    public struct FrameProcessOptions
    {
        public int finalErosion;

        public LaneDetectorOptions laneDetector;
        public RoadTextureDetectorOptions roadTextureDetector;
        public ClusteringDetectorOptions clusteringDetector;

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RoadTextureDetectorOptions
    {
        private bool enabled;
        private int lbpMinDiff;
        private int lbpPatchSize;
        private int nrOfHistogramBins;
        private double referenceHistogramsBottomRowsMiddlePercentage;
        private double maxDifferenceToReferenceHistograms;

        private int dilationIterations;
        private bool usePreviousBestHistogramMatches;

        double ignoreTopHeight;


        public int LbpMinDiff
        {
            get { return lbpMinDiff; }
            set { lbpMinDiff = value; }
        }

        public int LbpPatchSize
        {
            get { return lbpPatchSize; }
            set { lbpPatchSize = value; }
        }

        public int NrOfHistogramBins
        {
            get { return nrOfHistogramBins; }
            set { nrOfHistogramBins = value; }
        }

        public double ReferenceHistogramsBottomRowsMiddlePercentage
        {
            get { return referenceHistogramsBottomRowsMiddlePercentage; }
            set { referenceHistogramsBottomRowsMiddlePercentage = value; }
        }

        public double MaxDifferenceToReferenceHistograms
        {
            get { return maxDifferenceToReferenceHistograms; }
            set { maxDifferenceToReferenceHistograms = value; }
        }

        public bool Enabled
        {
            get { return enabled; }
            set { enabled = value; }
        }

        public int DilationIterations
        {
            get { return dilationIterations; }
            set { dilationIterations = value; }
        }

        public bool UsePreviousBestHistogramMatches
        {
            get { return usePreviousBestHistogramMatches; }
            set { usePreviousBestHistogramMatches = value; }
        }

        public double IgnoreTopHeight
        {
            get { return ignoreTopHeight; }
            set { ignoreTopHeight = value; }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct LaneDetectorOptions
    {
        private bool enabled;

        private double pathTracingBottomRowPadding;
        private int pathTracingDepth;
        private int pathTracingNrOriginSpawnPoints;
        private int pathTracingNrOfRaysPerOrigin;

        private int maxDistanceBetweenContourPoints;

        private double houghLineTreshold;
        private double houghLinesMaxAngleDeviationDegrees;
        private double houghLineMaxLineGap;

        private double ignoreTopHeight;

        private int cannyUpperThreshold;
        private double minimumCannyUpperThreshold;
        private int adaptiveCannyUpperThresholdSteps;

        private int minimumContourLengthThreshold;

        private double contourEdgePenaltyThreshold;

        public bool Enabled
        {
            get { return enabled; }
            set { enabled = value; }
        }

        public double PathTracingBottomRowPadding
        {
            get { return pathTracingBottomRowPadding; }
            set { pathTracingBottomRowPadding = value; }
        }

        public int PathTracingDepth
        {
            get { return pathTracingDepth; }
            set { pathTracingDepth = value; }
        }

        public int PathTracingNrOriginSpawnPoints
        {
            get { return pathTracingNrOriginSpawnPoints; }
            set { pathTracingNrOriginSpawnPoints = value; }
        }

        public int PathTracingNrOfRaysPerOrigin
        {
            get { return pathTracingNrOfRaysPerOrigin; }
            set { pathTracingNrOfRaysPerOrigin = value; }
        }

        public int MaxDistanceBetweenContourPoints
        {
            get { return maxDistanceBetweenContourPoints; }
            set { maxDistanceBetweenContourPoints = value; }
        }

        public double HoughLineTreshold
        {
            get { return houghLineTreshold; }
            set { houghLineTreshold = value; }
        }

        public double HoughLinesMaxAngleDeviationDegrees
        {
            get { return houghLinesMaxAngleDeviationDegrees; }
            set { houghLinesMaxAngleDeviationDegrees = value; }
        }

        public int CannyUpperThreshold
        {
            get { return cannyUpperThreshold; }
            set { cannyUpperThreshold = value; }
        }

        public int MinimumContourLengthThreshold
        {
            get { return minimumContourLengthThreshold; }
            set { minimumContourLengthThreshold = value; }
        }

        public double ContourEdgePenaltyThreshold
        {
            get { return contourEdgePenaltyThreshold; }
            set { contourEdgePenaltyThreshold = value; }
        }

        public double HoughLineMaxLineGap
        {
            get { return houghLineMaxLineGap; }
            set { houghLineMaxLineGap = value; }
        }

        public double MinimumCannyUpperThreshold
        {
            get { return minimumCannyUpperThreshold; }
            set { minimumCannyUpperThreshold = value; }
        }

        public int AdaptiveCannyUpperThresholdSteps
        {
            get { return adaptiveCannyUpperThresholdSteps; }
            set { adaptiveCannyUpperThresholdSteps = value; }
        }

        public double IgnoreTopHeight
        {
            get { return ignoreTopHeight; }
            set { ignoreTopHeight = value; }
        }
    };


    [StructLayout(LayoutKind.Sequential)]
    public struct ClusteringDetectorOptions
    {
        private bool enabled;
        private int initialNrOfClusters;
        private double windowRadius;
        private double overlapThreshold;
        private int dilationIterations;
        private int downScaleFactor;
        private double bottomRowPaddingPercentage;
        private int bottomRowHeight;
        private int topNClustersOnBottomRow;
        private int minNrOfPixelsInClusterInReference;
        private double ignoreTopHeight;

        public int InitialNrOfClusters
        {
            get { return initialNrOfClusters; }
            set { initialNrOfClusters = value; }
        }

        public double WindowRadius
        {
            get { return windowRadius; }
            set { windowRadius = value; }
        }

        public int DilationIterations
        {
            get { return dilationIterations; }
            set { dilationIterations = value; }
        }

        public int DownScaleFactor
        {
            get { return downScaleFactor; }
            set { downScaleFactor = value; }
        }

        public double BottomRowPaddingPercentage
        {
            get { return bottomRowPaddingPercentage; }
            set { bottomRowPaddingPercentage = value; }
        }

        public int BottomRowHeight
        {
            get { return bottomRowHeight; }
            set { bottomRowHeight = value; }
        }

        public bool Enabled
        {
            get { return enabled; }
            set { enabled = value; }
        }

        public int TopNClustersOnBottomRow
        {
            get { return topNClustersOnBottomRow; }
            set { topNClustersOnBottomRow = value; }
        }

        public double IgnoreTopHeight
        {
            get { return ignoreTopHeight; }
            set { ignoreTopHeight = value; }
        }

        public double OverlapThreshold
        {
            get { return overlapThreshold; }
            set { overlapThreshold = value; }
        }

        public int MinNrOfPixelsInClusterInReference
        {
            get { return minNrOfPixelsInClusterInReference; }
            set { minNrOfPixelsInClusterInReference = value; }
        }
    }
}
