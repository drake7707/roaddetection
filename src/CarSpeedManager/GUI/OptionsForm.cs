using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GUI
{
    public partial class OptionsForm : Form
    {

        private FrameProcessOptions options;

        public OptionsForm()
        {
            InitializeComponent();
            SetDefaults();


            propLane.SelectedObject = options.laneDetector;
            propRoadTexture.SelectedObject = options.roadTextureDetector;
            propClustering.SelectedObject = options.clusteringDetector;
        }

        private void SetDefaults()
        {
            options.finalErosion = 10;

            options.laneDetector.Enabled = true;

            options.laneDetector.PathTracingBottomRowPadding = 0.25;
            options.laneDetector.PathTracingDepth = 5;
            options.laneDetector.PathTracingNrOriginSpawnPoints = 10;
            options.laneDetector.PathTracingNrOfRaysPerOrigin = 64;
            options.laneDetector.MaxDistanceBetweenContourPoints = 100;
            options.laneDetector.HoughLineTreshold = 50;
            options.laneDetector.HoughLinesMaxAngleDeviationDegrees = 20;
            options.laneDetector.HoughLineMaxLineGap = 200;
            options.laneDetector.IgnoreTopHeight = 0.4;
            options.laneDetector.CannyUpperThreshold = 100;
            options.laneDetector.MinimumCannyUpperThreshold = 10;
            options.laneDetector.AdaptiveCannyUpperThresholdSteps = 8;

            options.laneDetector.MinimumContourLengthThreshold = 10;
            options.laneDetector.ContourEdgePenaltyThreshold = 0.15;

            options.roadTextureDetector.Enabled = true;
            options.roadTextureDetector.LbpMinDiff = 2;
            options.roadTextureDetector.LbpPatchSize = 16;
            options.roadTextureDetector.NrOfHistogramBins = 20;
            options.roadTextureDetector.ReferenceHistogramsBottomRowsMiddlePercentage = 0.3;
            options.roadTextureDetector.MaxDifferenceToReferenceHistograms = 0.15;
            options.roadTextureDetector.DilationIterations = 5;
            options.roadTextureDetector.UsePreviousBestHistogramMatches = true;
            options.roadTextureDetector.IgnoreTopHeight = 0.4;

            options.clusteringDetector.Enabled = true;
            options.clusteringDetector.InitialNrOfClusters = 10;
            options.clusteringDetector.WindowRadius = 0.1;
            options.clusteringDetector.OverlapThreshold = 0.75;
            options.clusteringDetector.DilationIterations = 5;
            options.clusteringDetector.DownScaleFactor = 4;

            options.clusteringDetector.BottomRowPaddingPercentage = 0.2;
            options.clusteringDetector.BottomRowHeight = 16;
            options.clusteringDetector.TopNClustersOnBottomRow = 3;
            options.clusteringDetector.MinNrOfPixelsInClusterInReference = 100;
            options.clusteringDetector.IgnoreTopHeight = 0.4;
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            this.BeginInvoke(new Action(() =>
            {
                // post in message queue
                this.BeginInvoke(new Action(() =>
                {
                    applyOptions();
                }));
                string path = "";
                try
                {
                    path = System.IO.Path.Combine(new DirectoryInfo(Environment.CurrentDirectory).Parent.Parent.Parent.Parent.Parent.FullName, "Sets", "01");
                    if (!System.IO.Directory.Exists(path))
                        path = "";
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Unable to construct path to set: " + ex.GetType().FullName + " - " + ex.Message);
                }

                try
                {
                    initialize(path);
                }
                catch (Exception ex)
                {
                }
                // message pump van opencv zorgt ervoor dat initialize nooit terug keert
            }));
        }


#if DEBUG
        const string DLL_PATH = @"..\..\..\x64\Debug\CarSpeedManager.dll";
#else
        const string DLL_PATH = @"CarSpeedManager.dll";
#endif

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern void updateOptions(FrameProcessOptions options);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern void initialize(string path);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern void setSet(string path, bool isVideo);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern void processEntireVideo(string inputFile, string outputPath, long fromMs, long duration);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern void calculateSetPerformance(string path, FrameProcessOptions options);



        private void btnUpdate_Click(object sender, EventArgs e)
        {
            pbar.Visible = true;
            Task.Factory.StartNew(() =>
            {
                applyOptions();


                this.Invoke(new Action(() => pbar.Visible = false));
            });
        }

        private void applyOptions()
        {
            // copy over
            this.options.laneDetector = (LaneDetectorOptions)propLane.SelectedObject;
            this.options.roadTextureDetector = (RoadTextureDetectorOptions)propRoadTexture.SelectedObject;
            this.options.clusteringDetector = (ClusteringDetectorOptions)propClustering.SelectedObject;
            updateOptions(options);
        }

        internal void LoadSet(string selectedPath, bool isVideo)
        {
            setSet(selectedPath, isVideo);
        }

        internal void ProcessEntireVideo(string fileName, string selectedPath)
        {
            Thread t = new Thread(() => processEntireVideo(fileName, selectedPath, 0, long.MaxValue));
            t.IsBackground = true;
            t.Start();
        }

        internal void CalculateSetPerformance(string selectedPath)
        {
            this.options.laneDetector = (LaneDetectorOptions)propLane.SelectedObject;
            this.options.roadTextureDetector = (RoadTextureDetectorOptions)propRoadTexture.SelectedObject;
            this.options.clusteringDetector = (ClusteringDetectorOptions)propClustering.SelectedObject;
            calculateSetPerformance(selectedPath, options);
        }
    }
}
