Computervisie Project
----------------------

Usage
-----

In the GUI you can 

- Open Set: Opens a set to check frame by frame. See the Help > Shortcuts to view which keys to use to toggle between different layers and classifiers.


- Open Video: Opens a video to check frame by frame with 1 second in between

- Process entire video: Opens a video and completely processes it frame by frame using the current settings. The different results will be saved into 4 different output video files.

- Calculate set performance: This calculates the performance using the ground truths and evaluates whether the output is not over the maximum allowed speed and still within the time limit. This will also create a results.txt (along with a result0XXX.png for each frame) at the end to evaluate. The final performance is shown in console.

Note: opening a set expects the actualspeeds.txt and gtdistances.txt to be present to correctly read the frame numbers.


Folder structure
----------------

- src: Contains source code for full project. The 'other' solution contains try-outs of the SVM attempt.

- bin: Contains the executable. Except for the C++ redistributables it should work out of the box.

- Sets: Contains the 4 input sets. We have added the gtXXXX.png files as ground truth every 5 frames.

- Paper: Contains a pdf version of our paper along with the tex source

- Presentation: Contains the presentation and other demonstrations such as sample videos and clustering demo.

- Requirements: Contains all the installers for the required runtimes to be able to run the project.

Requirements for executable
---------------------------

- Windows x64
- .NET Framework 4 (default in Windows8 or higher)
- Visual C++ redistributables 2015: https://www.microsoft.com/en-us/download/confirmation.aspx?id=48145&6B49FDFB-8E5B-4B07-BC31-15695C5A2143=1


Requirements for source code
-----------------------------

- Visual Studio 2015

- The project is configured to use the OpenCV folder at C:\opencv

- Use x64 release build, x86/win32 is not configured


The C++ should be compilable in a linux environment as well, only the GUI is using specific Windows API's to link the windows together. Running the C++ project directly would require some changes in the main method in Program.cpp.

