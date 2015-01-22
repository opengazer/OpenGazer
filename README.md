CVC Eye-Tracker
===========================
This eye-tracking software is based on the open source project [Opengazer] (http://www.inference.phy.cam.ac.uk/opengazer/)
and is improved with several features. The updated software supports video recording
for offline processing, offline batch processing, enhanced video outputting for visual
analysis of experiments and a head repositioning feature which allows users to correct
their head pose during eye-tracker usage in order to improve accuracy.

This software is the result of [on-going research] (http://mv.cvc.uab.es/projects/eye-tracker) in [Computer Vision Center (CVC)] (http://www.cvc.uab.es/index.asp?idioma=en) and 
[Universitat Autònoma de Barcelona] (http://uab.es/english/), Barcelona.

The installation steps for [Mac OS X] (#installation-using-macports-on-mac-os-x-systems) and [Ubuntu] (#installation-on-ubuntu-1204) systems are below.

Scripted Installation for Mac OS X or Ubuntu systems
-----

OS X only: Install Xcode, Xcode Command Line Tools, XQuartz and Homebrew

Open a terminal and run:
  - cd opengazer
  - sudo bash dependencies.sh
  - make


Manual Installation using MacPorts on Mac OS X systems
-----------------------------------------------
This has been tested on a MacBook with OS X 10.6, 10.8 and 10.9, both with the built-in 
iSight camera and a Logitech webcam.

1. Download and install [Xcode] (https://developer.apple.com/xcode/)
2. Download and install the latest version of [MacPorts] (http://www.macports.org/)
3. Update the port definitions and install the required packages from MacPorts. Note that this
might take a long time since all these packages have a lot of dependencies:
  - sudo port selfupdate
  - sudo port install gtkmm cairomm gsl cmake ffmpeg opencv fann boost
4. Download and install [XQuartz] (http://xquartz.macosforge.org/landing/)
5. Open a terminal and build the software:
  - cd opengazer
  - make

**NOTES:** For reference, during my development phase, I was compiling with Apple's GCC (gcc
version 4.2.1 (Apple Inc. build 5658)). And these are the versions of the software
that I was using:

  - gtkmm @2.24.4_0+x11
  - cairomm @1.10.0_2+x11
  - gsl @1.16_0
  - cmake @2.8.11.2_0
  - ffmpeg @1.2.2_0+gpl2
  - opencv @2.4.6_0
  - fann @2.2.0_0
  - boost @1.54.0_0+no_single+no_static+python27+universal

Version @2.4.6_1 of OpenCV from MacPorts has a bug which prevents access to the webcam. In such a
case, the preferred version (@2.4.6_0) can be installed with the following steps (steps from [here] (http://www.codedrop.ca/blog/archives/56)):

1. Modify the configuration file of MacPorts:
  - sudo nano /opt/local/etc/macports/sources.conf
  - Add **file:///Users/Shared/dports** before rsync line
  - Save (Conrol+O) and exit (Control+X)
2. Create the same folder in Terminal:
  - mkdir /Users/Shared/dports
3. Download the definitions for the old version of OpenCV:
   - cd /Users/Shared/dports && svn co -r 107771 http://svn.macports.org/repository/macports/trunk/dports/graphics/opencv/ graphics/opencv
4. Update the indices of MacPorts:
  - portindex /Users/Shared/dports
5. Verify that the old version (2.4.6) is now available in MacPorts:
  - port list opencv*
6. Install the old version:
  - sudo port install opencv @2.4.6

Manual Installation on Ubuntu 12.04
-------------------------------
Tested on a clean installation of Ubuntu 12.04 with a Logitech webcam.

1. Install required libraries
  - sudo apt-get install libopencv-* libgtkmm-2.4-dev libcairomm-1.0-dev libboost-dev libgsl0-dev libfann2 libfann-dev libv4l-dev cmake libboost-system-dev libboost-filesystem-dev
2. Install GSL from source
  - wget ftp://ftp.gnu.org/gnu/gsl/gsl-1.15.tar.gz
  - tar zxf gsl-1.15.tar.gz
  - cd gsl-1.15
  - ./configure
  - make
  - make install
3. [Build and install] (https://trac.ffmpeg.org/wiki/UbuntuCompilationGuide) FFMPEG 0.7.13
4. Build eyetracker
  - cd opengazer
  - make

Usage Instructions
------------
1. Put the camera centrally under you monitor, so that the height of your face in the image
is about half of the height of the image, and close to the centre of the image.  Make sure
the background light level is reasonably constant across the image.
2. Learn the necessary command line options by running "./opengazer --help" from terminal:

		Usage:	opengazer --subject=SUBJECT_NAME [--resolution=[480|720]] [--setup=SETUP_FOLDER_NAME] [--headdistance=DISTANCE] [--record=[0|1]] [--overlay=[0|1]] [--input=INPUT_FILE_PATH]


		OPTIONS:
			subject:		Subject's name to be used in the output file name.
			resolution:		Resolution for the camera. 480 for 640x480 resolution, 720 for 1280x720.
			setup:			Experiment setup name and also the folder to read the test and calibration point locations.
			headdistance:	Subject's head distance in cm to be included in the output file for automatic error calculation purposes.
			record:			Whether a video of the experiment should be recorded for offline processing purposes.
			overlay:		Whether target point and estimation pointers are written as an overlay to the recorded video. Should not be used when offline processing is desired on the output video.
			input:			Input video path in case of offline processing.


		SAMPLE USAGES:
			Basic usage without video recording:
				./opengazer --subject=johndoe --resolution=720 --setup=std --headdistance=80 --record=0

			Usage during experiments to enable video recording for offline processing:
				./opengazer --subject=johndoe --resolution=720 --setup=std --headdistance=80 --record=1

			Usage during offline processing:
				./opengazer --subject=johndoe --resolution=720 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/johndoe_std_720_1.avi

			Usage during offline processing with lower resolution:
				./opengazer --subject=johndoe --resolution=480 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/johndoe_std_720_1.avi
3. Initialize the face tracking component by pressing the button "Choose Points".
4. The next step is calibration.  Remember, in order for this gaze
tracker to work, you must keep your head absolutely still.  Press
"Calibrate".  You will now be asked to look at various points on
the screen, indicated by a target sign.  Remember: do not move your
head, only your eyes!
5. When calibration is finished, the sample frog game will be started.
Here, the aim is finding the frog which is hidden in a cluttered
environment. You have to use your gaze to locate the frog and stare
at it for a few seconds until the frog is teleported to another
location.
