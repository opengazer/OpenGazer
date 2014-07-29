#include <fstream>
#include <sys/time.h>

#include "MainGazeTracker.h"
#include "Application.h"
#include "utils.h"
#include "FaceDetector.h"

namespace {
	float calculateDistance(CvPoint2D32f pt1, CvPoint2D32f pt2 ) {
		float dx = pt2.x - pt1.x;
		float dy = pt2.y - pt1.y;

		return cvSqrt((float)(dx * dx + dy * dy));
	}

	std::vector<Point> scaleByScreen(const std::vector<Point> &points) {
		int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
		Gdk::Rectangle rect;
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

		if (numMonitors == 1) {
			return Calibrator::scaled(points, screen->get_width(), screen->get_height());
		} else {
			screen->get_monitor_geometry(numMonitors - 1, rect);
			return Calibrator::scaled(points, rect.get_width(), rect.get_height());
		}
	}

	void checkRectSize(IplImage *image, CvRect *rect) {
		if (rect->x < 0) {
			rect->x = 0;
		}

		if (rect->y < 0) {
			rect->y = 0;
		}

		if (rect->x + rect->width >= image->width) {
			rect->width = image->width - rect->x - 1;
		}

		if (rect->y + rect->height >= image->height) {
			rect->height = image->height - rect->y - 1;
		}
	}

	CvPoint2D32f *detectCornersInGrayscale(IplImage *eyeRegionImageGray, int &cornerCount) {
		IplImage *eigImage = 0;
		IplImage *tempImage = 0;

		eigImage = cvCreateImage(cvSize(eyeRegionImageGray->width, eyeRegionImageGray->height), IPL_DEPTH_32F, 1);
		tempImage = cvCreateImage(cvSize(eyeRegionImageGray->width, eyeRegionImageGray->height), IPL_DEPTH_32F, 1);

		CvPoint2D32f *corners = new CvPoint2D32f[cornerCount];
		double qualityLevel = 0.01;
		double minDistance = 2;
		int eigBlockSize = 3;
		int useHarris = false;

		cvGoodFeaturesToTrack(eyeRegionImageGray, eigImage /* output */, tempImage, corners, &cornerCount, qualityLevel, minDistance, NULL, eigBlockSize, useHarris);

		return corners;
	}

	bool detectNose(IplImage *image, double resolution, CvRect noseRect, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		CvRect *nose = new CvRect();
		IplImage *noseRegionImage;

		nose->width = 0;
		nose->height = 0;
		nose->x = 0;
		nose->y = 0;

		CvSize noseSize;

		if (resolution == 720) {
			noseSize = cvSize(36, 30);
		} else if (resolution == 1080) {
			noseSize = cvSize(54, 45);
		} else if (resolution == 480) {
			noseSize = cvSize(24, 20);
		}

		noseRegionImage = cvCreateImage(cvSize(noseRect.width, noseRect.height), image->depth, image->nChannels);
		cvSetImageROI(image, noseRect);
		cvCopy(image, noseRegionImage);
		cvResetImageROI(image);
		//cvSaveImage("nose.png", noseRegionImage);


		// Load the face detector and create empty memory storage
		char *file = "DetectorNose2.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		if (cascade == NULL) {
			std::cout << "CASCADE NOT LOADED" << std::endl;
		} else {
			std::cout << "Loaded" << std::endl;
		}

		// Detect objects
		CvSeq *noseSeq = cvHaarDetectObjects(noseRegionImage, cascade, storage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, noseSize);

		std::cout << noseSeq->total << " NOSES DETECTED" << std::endl;

		// Return the first nose if any eye is detected
		if (noseSeq && noseSeq->total > 0) {
			// If there are multiple matches, choose the one with larger area
			for (int i = 0; i < noseSeq->total; i++) {
				CvRect *dummy = (CvRect *)cvGetSeqElem(noseSeq, i+1);

				if ((dummy->width * dummy->height) > (nose->width*nose->height)) {
					nose = dummy;
				}
			}
		} else {
			return false;
		}

		//cvRectangle(image, cvPoint(nose->x, nose->y), cvPoint(nose->x + nose->width, nose->y + nose->height), CV_RGB(0, 255, 0), 2, 8, 0);

		points[0] = Point(noseRect.x + nose->x + nose->width * 0.33, noseRect.y + nose->y + nose->height * 0.6);
		points[1] = Point(noseRect.x + nose->x + nose->width * 0.67, noseRect.y + nose->y + nose->height * 0.6);

		return true;
	}

	bool detectMouth(IplImage *image, double resolution, CvRect mouthRect, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		CvRect *mouth = new CvRect();
		IplImage *mouthRegionImage;

		mouth->width = 0;
		mouth->height = 0;
		mouth->x = 0;
		mouth->y = 0;

		CvSize mouthSize;

		if (resolution == 720) {
			mouthSize = cvSize(50, 30);
		} else if (resolution == 1080) {
			mouthSize = cvSize(74, 45);
		} else if (resolution == 480) {
			mouthSize = cvSize(25, 15);
		}

		mouthRegionImage = cvCreateImage(cvSize(mouthRect.width, mouthRect.height), image->depth, image->nChannels);
		cvSetImageROI(image, mouthRect);
		cvCopy(image, mouthRegionImage);
		cvResetImageROI(image);
		//cvSaveImage("mouth.png", mouthRegionImage);

		// Load the face detector and create empty memory storage
		char *file = "DetectorMouth.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		if (cascade == NULL) {
			std::cout << "CASCADE NOT LOADED" << std::endl;
		} else {
			std::cout << "Loaded" << std::endl;
		}

		// Detect objects
		CvSeq *mouthSeq = cvHaarDetectObjects(mouthRegionImage, cascade, storage, 1.1, 3, 0 /* CV_HAAR_DO_CANNY_PRUNING */, mouthSize);

		std::cout << mouthSeq->total << " MOUTHS DETECTED" << std::endl;

		// Return the first mouth if any eye is detected
		if (mouthSeq && mouthSeq->total > 0) {
			// If there are multiple matches, choose the one with larger area
			for (int i = 0; i < mouthSeq->total; i++) {
				CvRect *dummy = (CvRect *)cvGetSeqElem(mouthSeq, i+1);

				if ((dummy->width * dummy->height) > (mouth->width * mouth->height)) {
					mouth = dummy;
				}
			}
		} else {
			return false;
		}

		//cvRectangle( image, cvPoint(mouth->x, mouth->y), cvPoint(mouth->x + mouth->width, mouth->y + mouth->height), CV_RGB(0, 255, 0), 2, 8, 0 );

		points[0] = Point(mouthRect.x + mouth->x + mouth->width * 0.1, mouthRect.y + mouth->y + mouth->height * 0.4);
		points[1] = Point(mouthRect.x + mouth->x + mouth->width * 0.9, mouthRect.y + mouth->y + mouth->height * 0.4);

		return true;
	}

	void detectEyeCorners(IplImage *image, double resolution, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		IplImage *eyeRegionImage;
		IplImage *eyeRegionImageGray;
		//IplImage *dummy;
		CvRect *bothEyes;
		//CvRect *rightEye;
		//CvRect *leftEye;
		CvSize bothEyesSize;
		CvSize singleEyeSize;

		if (resolution == 720) {
			bothEyesSize = cvSize(100, 25);
			singleEyeSize = cvSize(18, 12);
		} else if (resolution == 1080) {
			bothEyesSize = cvSize(150, 38);
			singleEyeSize = cvSize(27, 18);
		} else if (resolution == 480) {
			bothEyesSize = cvSize(64, 16);
			singleEyeSize = cvSize(6, 4);
		}

		// Load the face detector and create empty memory storage
		char *file = "DetectorEyes.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		// Detect objects
		CvSeq *eyeRegions = cvHaarDetectObjects(image, cascade, storage, 1.1, 10, CV_HAAR_DO_CANNY_PRUNING, bothEyesSize);

		std::cout << eyeRegions->total << " eye regions detected" << std::endl;

		// Return the first eye if any eye is detected
		if (eyeRegions && eyeRegions->total > 0) {
			bothEyes = (CvRect *)cvGetSeqElem(eyeRegions, 1);
		} else {
			return;
		}

		std::cout << "Resolution: " << resolution << ", both eye reg.:" << bothEyes->width << ", " << bothEyes->height << std::endl;

		//cvRectangle(image, cvPoint(bothEyes->x, bothEyes->y), cvPoint(bothEyes->x + bothEyes->width, bothEyes->y + bothEyes->height), CV_RGB(0, 255, 0), 2, 8, 0);

		int cornerCount = 100;
		eyeRegionImage = cvCreateImage(cvSize(bothEyes->width, bothEyes->height), image->depth, image->nChannels);
		eyeRegionImageGray = cvCreateImage(cvSize(bothEyes->width, bothEyes->height), 8, 1);

		CvRect leftRect = cvRect(bothEyes->x, bothEyes->y, bothEyes->width, bothEyes->height);
		cvSetImageROI(image, leftRect);
		cvCopy(image, eyeRegionImage);
		cvResetImageROI(image);

		cvCvtColor(eyeRegionImage, eyeRegionImageGray, CV_RGB2GRAY);

		Utils::normalizeGrayScaleImage(eyeRegionImageGray);

		CvPoint2D32f *corners = detectCornersInGrayscale(eyeRegionImageGray, cornerCount);

		int leftEyeCornersXSum = 0;
		int leftEyeCornersYSum = 0;
		int leftEyeCornersCount = 0;

		int rightEyeCornersXSum = 0;
		int rightEyeCornersYSum = 0;
		int rightEyeCornersCount = 0;

		/// Drawing a circle around corners
		for (int j = 0; j < cornerCount; j++ ) {
			if (corners[j].x < bothEyes->width * 0.4) {
				leftEyeCornersXSum += corners[j].x;
				leftEyeCornersYSum += corners[j].y;
				leftEyeCornersCount++;
				cvCircle(eyeRegionImage, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
			} else if (corners[j].x > bothEyes->width * 0.6) {
				rightEyeCornersXSum += corners[j].x;
				rightEyeCornersYSum += corners[j].y;
				rightEyeCornersCount++;
				cvCircle(eyeRegionImage, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
			}
		}

		double leftEyeCenterX = bothEyes->x + (leftEyeCornersXSum / (double)leftEyeCornersCount);
		double leftEyeCenterY = bothEyes->y + (leftEyeCornersYSum / (double)leftEyeCornersCount);

		double rightEyeCenterX = bothEyes->x + (rightEyeCornersXSum / (double)rightEyeCornersCount);
		double rightEyeCenterY = bothEyes->y + (rightEyeCornersYSum / (double)rightEyeCornersCount);

		double xDiff = rightEyeCenterX - leftEyeCenterX;
		double yDiff = rightEyeCenterY - leftEyeCenterY;

		//points[0] = Point(bothEyes->x + leftEyeCornersXSum, bothEyes->y + leftEyeCornersYSum);
		//points[1] = Point(bothEyes->x + rightEyeCornersXSum, bothEyes->y + rightEyeCornersYSum);
		points[0] = Point(leftEyeCenterX - 0.29 * xDiff, leftEyeCenterY - 0.29 * yDiff);// + xDiff/40);
		points[1] = Point(rightEyeCenterX + 0.29 * xDiff, rightEyeCenterY + 0.29 * yDiff);// + xDiff/40);

		/// Drawing a circle around corners
		//for (int i = 0; i < cornerCount; i++) {
		//	cvCircle(eyeRegionImage, cvPoint(corners[i].x, corners[i].y), 3, CV_RGB(255,0,0), -1, 8, 0);
		//}

		cvCircle(eyeRegionImage, cvPoint(points[0].x, points[0].y), 3, CV_RGB(0,255,0), -1, 8, 0);
		cvCircle(eyeRegionImage, cvPoint(points[1].x, points[1].y), 3, CV_RGB(0,255,0), -1, 8, 0);

		//cvSaveImage("eye_corners.png", eyeRegionImage);
		//cvSaveImage((_basePath.substr(0, _basePath.length() - 4) + "_eye_corners.png").c_str(), eyeRegionImage);
	}

	void detectEyebrowCorners(IplImage *image, double resolution, CvRect eyebrowRect, Point points[]) {
		IplImage *eyebrowRegionImage;
		IplImage *eyebrowRegionImageGray;
		IplImage *eyebrowRegionImage2;
		IplImage *eyebrowRegionImageGray2;

		eyebrowRect.width = eyebrowRect.width / 2;
		eyebrowRegionImage = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), image->depth, image->nChannels);
		eyebrowRegionImageGray = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), 8, 1);
		eyebrowRegionImage2 = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), image->depth, image->nChannels);
		eyebrowRegionImageGray2 = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), 8, 1);

		std::cout << "EYEBROW x, y = " << eyebrowRect.x << " - " << eyebrowRect.y << " width, height =" << eyebrowRect.width << " - " << eyebrowRect.height << std::endl;

		cvSetImageROI(image, eyebrowRect);
		cvCopy(image, eyebrowRegionImage);

		std::cout << "Copied first" << std::endl;

		CvRect eyebrowRect2 = cvRect(eyebrowRect.x + eyebrowRect.width, eyebrowRect.y, eyebrowRect.width, eyebrowRect.height);
		cvSetImageROI(image, eyebrowRect2);
		cvCopy(image, eyebrowRegionImage2);
		cvResetImageROI(image);

		//cvSaveImage("eyebrows.png", eyebrowRegionImage);

		cvCvtColor(eyebrowRegionImage, eyebrowRegionImageGray, CV_RGB2GRAY);
		cvCvtColor(eyebrowRegionImage2, eyebrowRegionImageGray2, CV_RGB2GRAY);

		int cornerCount = 1;
		CvPoint2D32f *corners = detectCornersInGrayscale(eyebrowRegionImageGray, cornerCount);

		cornerCount = 1;
		CvPoint2D32f *corners2 = detectCornersInGrayscale(eyebrowRegionImageGray2, cornerCount);

		points[0] = Point(eyebrowRect.x + corners[0].x, eyebrowRect.y + corners[0].y);
		points[1] = Point(eyebrowRect2.x + corners2[0].x, eyebrowRect2.y + corners2[0].y);

		std::cout << "Finished eyebrows" << std::endl;
	}
}

CommandLineArguments::CommandLineArguments(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		std::string parameter(argv[i]);
		int equalSignIndex = parameter.find("=");
		std::string option = parameter.substr(2, equalSignIndex - 2);
		std::string value = parameter.substr(equalSignIndex + 1, parameter.length() - equalSignIndex - 1);
		options.push_back(option);
		parameters.push_back(value);
	}
}
CommandLineArguments::~CommandLineArguments() {
	options = std::vector<std::string>();
	parameters = std::vector<std::string>();
}

bool CommandLineArguments::isOption(std::string option) {
	xForEach(iter, options) {
		if (iter->compare(option) == 0) {
			return true;
		}
	}

	return false;
}

std::string CommandLineArguments::getOptionValue(std::string option) {
	for (int i = 0; i < options.size(); i++) {
		if (options[i].compare(option) == 0) {
			return parameters[i];
		}
	}

	return "";
}

std::vector<int> CommandLineArguments::getOptionValueAsVector(std::string option) {
	std::vector<int> returnVector;

	for (int i = 0; i < options.size(); i++) {
		if (options[i].compare(option) == 0) {
			std::string input = parameters[i];
			std::istringstream ss(input);
			std::string token;

			while(std::getline(ss, token, ',')) {
				returnVector.push_back(atoi(token.c_str()));
				std::cout << "PARSED: " << token << '\n';
			}

			return returnVector;
		}
	}

	return returnVector;
}

Command::Command(long number, std::string name):
	frameNumber(number),
	commandName(name)
{
}

VideoInput::VideoInput():
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(0),
	_capture(cvCaptureFromCAM(0))
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	frame = cvQueryFrame(_capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution):
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(resolution),
	_capture(cvCaptureFromCAM(0))
{
	if (resolution.compare("720") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 1280);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 720);
	} else if (resolution.compare("1080") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 1920);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 1080);
	} else if (resolution.compare("480") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	//cvSetCaptureProperty(_capture, CV_CAP_PROP_FOURCC, CV_FOURCC('M','J','P','G'));

	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	frame = cvQueryFrame(_capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution, std::string filename, bool dummy):
	frameCount(0),
	frame(cvQueryFrame(_capture)),
	size(cvSize(frame->width, frame->height)),
	captureFromVideo(true),
	resolutionParameter(resolution),
	_capture(cvCaptureFromFile(filename.c_str()))
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);


	double videoResolution = atoi(resolution.c_str());
	double trackerResolution = frame->height;

	// In case the video is 1280/720 and we want to execute 480 (or 1280 -> 720)
	if (videoResolution != trackerResolution) {
		IplImage *tempImage = cvCreateImage(cvSize(640, 480), 8, 3);

		if (videoResolution == 720 && trackerResolution == 480) {
			cvSetImageROI(frame, cvRect(160, 0, 960, 720));		// Set ROI
		} else if (videoResolution == 1080 && trackerResolution == 480) {
			cvSetImageROI(frame, cvRect(240, 0, 1440, 1080));	// Set ROI
		}

		//std::cout << "FRAME: " << frame->height << "x" << frame->width << " " << frame->depth << std::std::endl;
		//std::cout << "TEMP: " << tempImage->height << "x" << tempImage->width << " " << tempImage->depth << std::endl;
		cvResize(frame, tempImage);
		frame = tempImage;
		cvResetImageROI(frame);
		size.width = frame->width;
		size.height = frame->height;


		std::cout << "Successfully resized first frame" << std::endl;
	}
}

VideoInput::~VideoInput() {
	cvReleaseCapture(&_capture);
}

void VideoInput::updateFrame() {
	static double videoResolution = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
	static double trackerResolution = frame->height;
	frameCount++;

	if (!(captureFromVideo && frameCount == 1)) {
		// If capturing from video and video size is not equal to desired resolution, carry on with resizing
		if (captureFromVideo && videoResolution != trackerResolution) {
			IplImage *tempImage = cvQueryFrame(_capture);

			if (videoResolution == 720 && trackerResolution == 480) {
				cvSetImageROI(tempImage, cvRect(160, 0, 960, 720));		// Set ROI
			} else if (videoResolution == 1080 && trackerResolution == 480) {
				cvSetImageROI(tempImage, cvRect(240, 0, 1440, 1080));	// Set ROI
			}

			cvResize(tempImage, frame, CV_INTER_CUBIC);		// Resize image to 640x480
		} else {
			frame = cvQueryFrame(_capture);
		}
	}

	if (!captureFromVideo) {
		cvFlip(frame, frame, 1);
	}
}

// Returns 480 or 720 depending on camera resolution
double VideoInput::getResolution() {
	double value = atof(resolutionParameter.c_str());
	return value > 0 ? value : cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
}

VideoWriter::VideoWriter(CvSize size, std::string filename):
	_video(cvCreateVideoWriter(filename.c_str(), 0x58564944, 15.0, size, 1))
{
}

VideoWriter::~VideoWriter() {
	cvReleaseVideoWriter(&_video);
}

void VideoWriter::write(const IplImage *image) {
	cvWriteFrame(_video, image);
}

MainGazeTracker::MainGazeTracker(int argc, char **argv, const std::vector<boost::shared_ptr<AbstractStore> > &stores):
	_frameStoreLoad(-1),
	_stores(stores),
	_autoReload(false),
	_videoOverlays(false),
	_totalFrameCount(0),
	_recording(false),
	_commandIndex(-1)
	//_stateMachine(shared_ptr<AlertWindow>(new AlertWindow("start")))
{
	CommandLineArguments args(argc, argv);

	if (args.getOptionValue("help").compare("") != 0) {
		std::cout << std::endl;
		std::cout << "CVC Machine Vision Group Eye-Tracker" << std::endl;
		std::cout << std::endl << std::endl;
		std::cout << "Usage:\teyetracker --subject=SUBJECT_NAME [--resolution=[480|720]] [--setup=SETUP_FOLDER_NAME] [--headdistance=DISTANCE] [--record=[0|1]] [--overlay=[0|1]] [--input=INPUT_FILE_PATH]" << std::endl;
		std::cout << std::endl << std::endl;
		std::cout << "OPTIONS:" << std::endl;
		std::cout << "\tsubject:\tSubject\'s name to be used in the output file name." << std::endl;
		std::cout << "\tresolution:\tResolution for the camera. 480 for 640x480 resolution, 720 for 1280x720." << std::endl;
		std::cout << "\tsetup:\t\tExperiment setup name and also the folder to read the test and calibration point locations." << std::endl;
		std::cout << "\theaddistance:\tSubject\'s head distance in cm to be included in the output file for automatic error calculation purposes." << std::endl;
		std::cout << "\trecord:\t\tWhether a video of the experiment should be recorded for offline processing purposes." << std::endl;
		std::cout << "\toverlay:\tWhether target point and estimation pointers are written as an overlay to the recorded video. Should not be used when offline processing is desired on the output video." << std::endl;
		std::cout << "\tinput:\t\tInput video path in case of offline processing." << std::endl;
		std::cout << std::endl << std::endl;
		std::cout << "SAMPLE USAGES:" << std::endl;
		std::cout << "\tBasic usage without video recording:" << std::endl;
		std::cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=0" << std::endl;
		std::cout << std::endl;
		std::cout << "\tUsage during experiments to enable video recording for offline processing:" << std::endl;
		std::cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=1" << std::endl;
		std::cout << std::endl;
		std::cout << "\tUsage during offline processing:" << std::endl;
		std::cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/david_std_720_1.avi" << std::endl;
		std::cout << std::endl;
		std::cout << "\tUsage during offline processing with lower resolution:" << std::endl;
		std::cout << "\t\t./eyetracker --subject=david --resolution=480 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/david_std_720_1.avi" << std::endl;
		std::cout << std::endl << std::endl;
		exit(0);
	}

	if (args.getOptionValue("input").compare("") != 0) {
		videoInput.reset(new VideoInput(args.getOptionValue("resolution"), args.getOptionValue("input"), true));

		// Read the commands (SELECT, CLEAR, CALIBRATE, TEST)
		std::string inputCommandFilename = args.getOptionValue("input");
		inputCommandFilename = inputCommandFilename.substr(0, inputCommandFilename.length()-4) + "_commands.txt";

		_commandInputFile = new std::ifstream(inputCommandFilename.c_str());

		for(;;) {
			long number;
			std::string name;
			*_commandInputFile >> number >> name;

			if (_commandInputFile->rdstate()) {
				break; // break if any error
			}

			_commands.push_back(Command(number, name));
		}

		_commandIndex = 0;

		std::cout << _commands.size() << " commands read." << std::endl;
	} else {
		// --resolution parameter
		if (args.getOptionValue("resolution").compare("") != 0) {
			videoInput.reset(new VideoInput(args.getOptionValue("resolution")));
		} else {
			videoInput.reset(new VideoInput("480"));
		}
	}

	if (videoInput.get()->getResolution() == 720) {
		_conversionImage = cvCreateImage(cvSize(1280, 720), 8, 3 );
	} else if (videoInput.get()->getResolution() == 1080) {
		_conversionImage = cvCreateImage(cvSize(1920, 1080), 8, 3 );
	} else if (videoInput.get()->getResolution() == 480) {
		_conversionImage = cvCreateImage(cvSize(640, 480), 8, 3 );
	}

	std::string subject = args.getOptionValue("subject");
	std::string setup = args.getOptionValue("setup");

	if (subject.compare("") == 0) {
		subject = "default";
	}

	if (setup.compare("") == 0) {
		setup = "std";
	}

	if (args.getOptionValue("overlay").compare("1") == 0) {
		_videoOverlays = true;
	}

	// --headdistance parameter
	if (args.getOptionValue("headdistance").compare("") != 0) {
		_headDistance = atoi(args.getOptionValue("headdistance").c_str());
	} else {
		_headDistance = 70;
	}

	// --dwelltime parameter
	if (args.getOptionValue("dwelltime").compare("") != 0) {
		Application::dwelltimeParameter = atoi(args.getOptionValue("dwelltime").c_str());
	} else {
		Application::dwelltimeParameter = 30;
	}

	// --testdwelltime parameter
	if (args.getOptionValue("testdwelltime").compare("") != 0) {
		Application::testDwelltimeParameter = atoi(args.getOptionValue("testdwelltime").c_str());
	} else {
		Application::testDwelltimeParameter = 20;
	}

	// --sleep parameter
	if (args.getOptionValue("sleep").compare("") != 0) {
		Application::sleepParameter = atoi(args.getOptionValue("sleep").c_str());
	 } else {
		Application::sleepParameter = 0;
	 }

	// --folder parameter
	std::string folderParameter = "outputs";

	if (args.getOptionValue("outputfolder").compare("") != 0) {
		folderParameter = args.getOptionValue("outputfolder");
	}

	// --subject parameter
	_basePath = Utils::getUniqueFileName(folderParameter, subject + "_" + setup + "_" + args.getOptionValue("resolution"));

	// --record parameter
	if (args.getOptionValue("record").compare("1") == 0) {
		_video.reset(new VideoWriter(videoInput->size, _basePath.substr(0, _basePath.length() - 4) + ".avi"));
		_recording = true;
	}

	_outputFile = new std::ofstream((_basePath + "_").c_str());

	// First write the system time
	time_t currentTime = time(NULL);
	*_outputFile << ctime(&currentTime) << std::endl;

	// Then write the setup parameters
	*_outputFile << "--input=" << args.getOptionValue("input") << std::endl;
	*_outputFile << "--record=" << args.getOptionValue("record") << std::endl;
	*_outputFile << "--overlay=" << (_videoOverlays ? "true" : "false") << std::endl;
	*_outputFile << "--headdistance=" << _headDistance << std::endl;
	*_outputFile << "--resolution=" << args.getOptionValue("resolution") << std::endl;
	*_outputFile << "--setup=" << setup << std::endl;
	*_outputFile << "--subject=" << subject << std::endl << std::endl;

	// Finally the screen resolution
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	Gdk::Rectangle rect;
	screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);
	*_outputFile << "Screen resolution: " << rect.get_width() << " x " << rect.get_height() << " (Position: "<< rect.get_x() << ", "<< rect.get_y() << ")" << std::endl << std::endl;
	_outputFile->flush();

	// If recording, create the file to write the commands for button events
	if (_recording) {
		std::string commandFileName = _basePath.substr(0, _basePath.length() - 4) + "_commands.txt";
		_commandOutputFile = new std::ofstream(commandFileName.c_str());
	}

	_directory = setup;

	canvas.reset(cvCreateImage(videoInput->size, 8, 3));
	trackingSystem.reset(new TrackingSystem(videoInput->size));

	trackingSystem->gazeTracker.outputFile = _outputFile;
	isCalibrationOutputWritten = true;

	_gameWin = new GameWindow(&(trackingSystem->gazeTracker.output));
	_gameWin->show();

	if (videoInput.get()->getResolution() == 720) {
		_repositioningImage = cvCreateImage(cvSize(1280, 720), 8, 3);
	} else if (videoInput.get()->getResolution() == 1080) {
		_repositioningImage = cvCreateImage(cvSize(1920, 1080), 8, 3);
	} else if (videoInput.get()->getResolution() == 480) {
		_repositioningImage = cvCreateImage(cvSize(640, 480), 8, 3);
	}

	_gameWin->setRepositioningImage(_repositioningImage);
	Application::faceRectangle = NULL;
}

MainGazeTracker::~MainGazeTracker() {
	cleanUp();
}

void MainGazeTracker::process() {
	_totalFrameCount++;
	_frameCount++;
	videoInput->updateFrame();

	// Wait a little so that the marker stays on the screen for a longer time
	if ((Application::status == Application::STATUS_CALIBRATING || Application::status == Application::STATUS_TESTING) && !videoInput->captureFromVideo) {
		usleep(Application::sleepParameter);
	}

	const IplImage *frame = videoInput->frame;
	canvas->origin = frame->origin;

	double imageNorm = 0.0;

	if (Application::status == Application::STATUS_PAUSED) {
		cvAddWeighted(frame, 0.5, _overlayImage, 0.5, 0.0, canvas.get());

		// Only calculate norm in the area containing the face
		if (_faces.size() == 1) {
			cvSetImageROI(const_cast<IplImage*>(frame), _faces[0]);
			cvSetImageROI(_overlayImage, _faces[0]);

			imageNorm = cvNorm(frame, _overlayImage, CV_L2);
			imageNorm = (10000 * imageNorm) / (_faces[0].width * _faces[0].height);

			// To be able to use the same threshold for VGA and 720 cameras
			if (videoInput->getResolution() == 720) {
				imageNorm *= 1.05;
			}

			std::cout << "ROI NORM: " << imageNorm << " (" << _faces[0].width << "x" << _faces[0].height << ")" << std::endl;
			cvResetImageROI(const_cast<IplImage*>(frame));
			cvResetImageROI(_overlayImage);
		} else {
			imageNorm = cvNorm(frame, _overlayImage, CV_L2);
			imageNorm = (15000 * imageNorm) / (videoInput->getResolution() * videoInput->getResolution());

			// To be able to use the same threshold for only-face method and all-image method
			imageNorm *= 1.2;

			// To be able to use the same threshold for VGA and 720 cameras
			if (videoInput->getResolution() == 720) {
				imageNorm *= 1.05;
			}
			//std::cout << "WHOLE NORM: " << imageNorm << std::endl;
		}
	} else {
		cvCopy(frame, canvas.get(), 0);
	}

	try {
		trackingSystem->process(frame, canvas.get());

		if (trackingSystem->gazeTracker.isActive()) {
			if (Application::status != Application::STATUS_TESTING) {
				trackingSystem->gazeTracker.output.setActualTarget(Point(0, 0));
				trackingSystem->gazeTracker.output.setFrameId(0);
			} else {
				trackingSystem->gazeTracker.output.setActualTarget(Point(target->getActivePoint().x, target->getActivePoint().y));
				trackingSystem->gazeTracker.output.setFrameId(target->getPointFrame());
			}

			//trackingSystem->gazeTracker.output.setErrorOutput(Application::status == Application::STATUS_TESTING);	// No longer necessary, TODO REMOVE

			xForEach(iter, _stores) {
				(*iter)->store(trackingSystem->gazeTracker.output);
			}

			// Write the same info to the output text file
			if (_outputFile != NULL) {
				TrackerOutput output = trackingSystem->gazeTracker.output;
				if (Application::status == Application::STATUS_TESTING) {
					std::cout << "TESTING, WRITING OUTPUT!!!!!!!!!!!!!!!!!" << std::endl;
					if (!trackingSystem->eyeExtractor.isBlinking()) {
						*_outputFile << output.frameId + 1 << "\t"
							<< output.actualTarget.x << "\t" << output.actualTarget.y << "\t"
							<< output.gazePoint.x << "\t" << output.gazePoint.y << "\t"
							<< output.gazePointLeft.x << "\t" << output.gazePointLeft.y
							//<< "\t" << output.nnGazePoint.x << "\t" << output.nnGazePoint.y << "\t"
							//<< output.nnGazePointLeft.x << "\t" << output.nnGazePointLeft.y
							<< std::endl;
					} else {
						*_outputFile << output.frameId + 1 << "\t"
							<< output.actualTarget.x << "\t" << output.actualTarget.y << "\t"
							<< 0 << "\t" << 0 << "\t"
							<< 0 << "\t" << 0
							// << "\t" << 0 << "\t" << 0 << "\t"
							//<< 0 << "\t" << 0
							<< std::endl;
					}
				}

				_outputFile->flush();
			}
		}

		//if (!trackingSystem->tracker.areAllPointsActive()) {
		//	throw TrackingException();
		//}
		_frameStoreLoad = 20;
	}
	catch (TrackingException &e) {
		_frameStoreLoad--;
	}

	if (Application::status == Application::STATUS_PAUSED) {
		int rectangleThickness = 15;
		CvScalar color;

		if (imageNorm < 1500) {
			color = CV_RGB(0, 255, 0);
		} else if (imageNorm < 2500) {
			color = CV_RGB(0, 165, 255);
		} else {
			color = CV_RGB(0, 0, 255);
		}

		cvRectangle(canvas.get(), cvPoint(0, 0), cvPoint(rectangleThickness, videoInput->size.height), color, CV_FILLED);	// left
		cvRectangle(canvas.get(), cvPoint(0, 0), cvPoint(videoInput->size.width, rectangleThickness), color, CV_FILLED);	// top
		cvRectangle(canvas.get(), cvPoint(videoInput->size.width - rectangleThickness, 0), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);		// right
		cvRectangle(canvas.get(), cvPoint(0, videoInput->size.height - rectangleThickness), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);	// bottom

		// Fill the repositioning image so that it can be displayed on the subject's monitor too
		cvAddWeighted(frame, 0.5, _overlayImage, 0.5, 0.0, _repositioningImage);

		cvRectangle(_repositioningImage, cvPoint(0, 0), cvPoint(rectangleThickness, videoInput->size.height), color, CV_FILLED);	// left
		cvRectangle(_repositioningImage, cvPoint(0, 0), cvPoint(videoInput->size.width, rectangleThickness), color, CV_FILLED);		// top
		cvRectangle(_repositioningImage, cvPoint(videoInput->size.width - rectangleThickness, 0), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);	// right
		cvRectangle(_repositioningImage, cvPoint(0, videoInput->size.height - rectangleThickness), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);	// bottom
	}

	frameFunctions.process();

	// If video output is requested
	if (_recording) {
		if (_videoOverlays) {
			//std::cout << "VIDEO EXISTS" << std::endl;
			TrackerOutput output = trackingSystem->gazeTracker.output;

			Point actualTarget(0, 0);
			Point estimation(0, 0);

			cvCopy(canvas.get(), _conversionImage);

			if (Application::status == Application::STATUS_TESTING) {
				//std::cout << "TARGET: " << output.actualTarget.x << ", " << output.actualTarget.y << std::endl;
				Utils::mapToVideoCoordinates(output.actualTarget, videoInput->getResolution(), actualTarget);
				//std::cout << "MAPPING: " << actualTarget.x << ", " << actualTarget.y << std::endl << std::endl;

				cvCircle((CvArr *)_conversionImage, cvPoint(actualTarget.x, actualTarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);

				// If not blinking, show the estimation in video
				if (!trackingSystem->eyeExtractor.isBlinking()) {
					Utils::mapToVideoCoordinates(output.gazePoint, videoInput->getResolution(), estimation);
					cvCircle((CvArr *)_conversionImage, cvPoint(estimation.x, estimation.y), 8, cvScalar(0, 255, 0), -1, 8, 0);

					Utils::mapToVideoCoordinates(output.gazePointLeft, videoInput->getResolution(), estimation);
					cvCircle((CvArr *)_conversionImage, cvPoint(estimation.x, estimation.y), 8, cvScalar(255, 0, 0), -1, 8, 0);
				}
			}

			if (Application::status == Application::STATUS_PAUSED) {
				int rectangleThickness = 15;
				CvScalar color;

				if (imageNorm < 1900) {
					color = CV_RGB(0, 255, 0);
				} else if (imageNorm < 3000) {
					color = CV_RGB(255, 165, 0);
				} else {
					color = CV_RGB(255, 0, 0);
				}

				cvRectangle(_conversionImage, cvPoint(0, 0), cvPoint(rectangleThickness, videoInput->size.height), color, CV_FILLED);	// left
				cvRectangle(_conversionImage, cvPoint(0, 0), cvPoint(videoInput->size.width, rectangleThickness), color, CV_FILLED);	// top
				cvRectangle(_conversionImage, cvPoint(videoInput->size.width - rectangleThickness, 0), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);		// right
				cvRectangle(_conversionImage, cvPoint(0, videoInput->size.height - rectangleThickness), cvPoint(videoInput->size.width, videoInput->size.height), color, CV_FILLED);	// bottom
			}

			_video->write(_conversionImage);
		} else {
			//std::cout << "Trying to write video image" << std::endl;
			cvCopy(videoInput->frame, _conversionImage);
			//std::cout << "Image copied" << std::endl;
			_video->write(_conversionImage);
			//std::cout << "Image written" << std::endl << std::endl;
		}
	}

	// Show the current target & estimation points on the main window
	if (Application::status == Application::STATUS_CALIBRATING || Application::status == Application::STATUS_TESTING || Application::status == Application::STATUS_CALIBRATED) {
		TrackerOutput output = trackingSystem->gazeTracker.output;
		Point actualTarget(0, 0);
		Point estimation(0, 0);

		if (Application::status == Application::STATUS_TESTING) {
			Utils::mapToVideoCoordinates(target->getActivePoint(), videoInput->getResolution(), actualTarget, false);
			cvCircle((CvArr *)canvas.get(), cvPoint(actualTarget.x, actualTarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);
		} else if(Application::status == Application::STATUS_CALIBRATING) {
			Utils::mapToVideoCoordinates(_calibrator->getActivePoint(), videoInput->getResolution(), actualTarget, false);
			cvCircle((CvArr *)canvas.get(), cvPoint(actualTarget.x, actualTarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);
		}

		// If not blinking, show the estimation in video
		if (!trackingSystem->eyeExtractor.isBlinking()) {
			Utils::mapToVideoCoordinates(output.gazePoint, videoInput->getResolution(), estimation, false);
			cvCircle((CvArr *)canvas.get(), cvPoint(estimation.x, estimation.y), 8, cvScalar(0, 255, 0), -1, 8, 0);

			Utils::mapToVideoCoordinates(output.gazePointLeft, videoInput->getResolution(), estimation, false);
			cvCircle((CvArr *)canvas.get(), cvPoint(estimation.x, estimation.y), 8, cvScalar(255, 0, 0), -1, 8, 0);
		}
	}

	//_stateMachine.handleEvent(EVENT_TICK);

	if (_autoReload && _frameStoreLoad <= 0 && Application::status != Application::STATUS_PAUSED) {
 		loadPoints();
	}
}

void MainGazeTracker::simulateClicks() {
	if (_commands.size() > 0) {
		while(_commandIndex >= 0 && _commandIndex <= (_commands.size() - 1) && _commands[_commandIndex].frameNumber == _totalFrameCount) {
			std::cout << "Command: " << _commands[_commandIndex].commandName << std::endl;
			if(strcmp(_commands[_commandIndex].commandName.c_str(), "SELECT") == 0) {
				std::cout << "Choosing points automatically" << std::endl;
				choosePoints();
			}
			else if(strcmp(_commands[_commandIndex].commandName.c_str(), "CLEAR") == 0) {
				std::cout << "Clearing points automatically" << std::endl;
				clearPoints();
			}
			else if(strcmp(_commands[_commandIndex].commandName.c_str(), "CALIBRATE") == 0) {
				std::cout << "Calibrating automatically" << std::endl;
				startCalibration();
			}
			else if(strcmp(_commands[_commandIndex].commandName.c_str(), "TEST") == 0) {
				std::cout << "Testing automatically" << std::endl;
				startTesting();
			}
			else if(strcmp(_commands[_commandIndex].commandName.c_str(), "UNPAUSE") == 0 || strcmp(_commands[_commandIndex].commandName.c_str(), "PAUSE") == 0) {
				std::cout << "Pausing/unpausing automatically" << std::endl;
				pauseOrRepositionHead();
			}

			_commandIndex++;
		}

		if (_commandIndex == _commands.size() && (Application::status == Application::STATUS_IDLE || Application::status == Application::STATUS_CALIBRATED)) {
			throw Utils::QuitNow();
		}
	}
}

void MainGazeTracker::cleanUp() {
	_outputFile->close();
	rename((_basePath + "_").c_str(), _basePath.c_str());
	//rename("out.avi", (_basePath.substr(0, _basePath.length() - 4) + ".avi").c_str());
}

void MainGazeTracker::addTracker(Point point) {
	trackingSystem->pointTracker.addTracker(point);
}

void MainGazeTracker::addExemplar(Point exemplar) {
	if (exemplar.x >= EyeExtractor::eyeDX && exemplar.x + EyeExtractor::eyeDX < videoInput->size.width && exemplar.y >= EyeExtractor::eyeDY && exemplar.y + EyeExtractor::eyeDY < videoInput->size.height) {
		trackingSystem->gazeTracker.addExemplar(exemplar, trackingSystem->eyeExtractor.eyeFloat.get(), trackingSystem->eyeExtractor.eyeGrey.get());
		trackingSystem->gazeTracker.addExemplarLeft(exemplar, trackingSystem->eyeExtractor.eyeFloatLeft.get(), trackingSystem->eyeExtractor.eyeGreyLeft.get());
	}
}

void MainGazeTracker::startCalibration() {
	Application::status = Application::STATUS_CALIBRATING;

	if (_gameWin == NULL) {
		_gameWin = new GameWindow(&(trackingSystem->gazeTracker.output));
	}

	_gameWin->show();

	if (_recording) {
		*_commandOutputFile << _totalFrameCount << " CALIBRATE" << std::endl;
	}

	boost::shared_ptr<WindowPointer> pointer(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0.2)));

	_gameWin->setCalibrationPointer(pointer.get());

	if (Gdk::Screen::get_default()->get_n_monitors() > 1) {
		boost::shared_ptr<WindowPointer> mirror(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0)));
		pointer->mirror = mirror;
	}

	std::ifstream calfile((_directory + "/calpoints.txt").c_str());
	boost::shared_ptr<Calibrator> cal(new Calibrator(_frameCount, trackingSystem, scaleByScreen(Calibrator::loadPoints(calfile)), pointer, Application::dwelltimeParameter));

	_calibrator = cal.operator->();
	isCalibrationOutputWritten = false;

	frameFunctions.clear();
	frameFunctions.addChild(&frameFunctions, cal);
}

void MainGazeTracker::startTesting() {
	Application::status = Application::STATUS_TESTING;

	if (_recording) {
		*_commandOutputFile << _totalFrameCount << " TEST" << std::endl;
	}

	std::vector<Point> points;
	boost::shared_ptr<WindowPointer> pointer(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0.2)));

	_gameWin->setCalibrationPointer(pointer.get());

	if (Gdk::Screen::get_default()->get_n_monitors() > 1) {
		boost::shared_ptr<WindowPointer> mirror(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0)));
		pointer->mirror = mirror;
	}

	// ONUR Modified code to read the test points from a text file
	std::ifstream calfile((_directory + "/testpoints.txt").c_str());
	points = Calibrator::loadPoints(calfile);

	boost::shared_ptr<MovingTarget> moving(new MovingTarget(_frameCount, scaleByScreen(points), pointer, Application::testDwelltimeParameter));

	target = moving.operator->();

	//MovingTarget *target = new MovingTarget(_frameCount, scaleByScreen(points), pointer);
	//shared_ptr<MovingTarget> moving((const boost::shared_ptr<MovingTarget>&) *target);

	*_outputFile << "TESTING" << std::endl << std::endl;

	frameFunctions.clear();
	frameFunctions.addChild(&frameFunctions, moving);
}

void MainGazeTracker::startPlaying() {
	if (_gameWin == NULL) {
		_gameWin = new GameWindow(&(trackingSystem->gazeTracker.output));
	}
	_gameWin->show();
}

void MainGazeTracker::savePoints() {
	try {
		trackingSystem->pointTracker.save("pointTracker", "points.txt", videoInput->frame);
		_autoReload = true;
	}
	catch (std::ios_base::failure &e) {
		std::cout << e.what() << std::endl;
	}
}

void MainGazeTracker::loadPoints() {
	try {
		trackingSystem->pointTracker.load("pointTracker", "points.txt", videoInput->frame);
		_autoReload = true;
	}
	catch (std::ios_base::failure &e) {
		std::cout << e.what() << std::endl;
	}
}

void MainGazeTracker::choosePoints() {
	try {
		Point eyes[2];
		Point nose[2];
		Point mouth[2];
		Point eyebrows[2];

		if (_recording) {
			*_commandOutputFile << _totalFrameCount << " SELECT" << std::endl;
		}

		detectEyeCorners(videoInput->frame, videoInput->getResolution(), eyes);

		CvRect noseRect = cvRect(eyes[0].x, eyes[0].y, fabs(eyes[0].x - eyes[1].x), fabs(eyes[0].x - eyes[1].x));
		checkRectSize(videoInput->frame, &noseRect);
		//std::cout << "Nose rect: " << noseRect.x << ", " << noseRect.y << " - " << noseRect.width << ", " << noseRect.height << std::endl;

		if (!detectNose(videoInput->frame, videoInput->getResolution(), noseRect, nose)) {
			std::cout << "NO NOSE" << std::endl;
			return;
		}

		CvRect mouthRect = cvRect(eyes[0].x, nose[0].y, fabs(eyes[0].x - eyes[1].x), 0.8 * fabs(eyes[0].x - eyes[1].x));
		checkRectSize(videoInput->frame, &mouthRect);

		if (!detectMouth(videoInput->frame, videoInput->getResolution(), mouthRect, mouth)) {
			std::cout << "NO MOUTH" << std::endl;
			return;
		}

		CvRect eyebrowRect = cvRect(eyes[0].x  + fabs(eyes[0].x - eyes[1].x) * 0.25, eyes[0].y - fabs(eyes[0].x - eyes[1].x) * 0.40,  fabs(eyes[0].x - eyes[1].x) * 0.5, fabs(eyes[0].x - eyes[1].x) * 0.25);
		checkRectSize(videoInput->frame, &eyebrowRect);
		detectEyebrowCorners(videoInput->frame, videoInput->getResolution(), eyebrowRect, eyebrows);

		trackingSystem->pointTracker.clearTrackers();
		_autoReload = false;

		trackingSystem->pointTracker.addTracker(eyes[0]);
		trackingSystem->pointTracker.addTracker(eyes[1]);
		trackingSystem->pointTracker.addTracker(nose[0]);
		trackingSystem->pointTracker.addTracker(nose[1]);
		trackingSystem->pointTracker.addTracker(mouth[0]);
		trackingSystem->pointTracker.addTracker(mouth[1]);
		trackingSystem->pointTracker.addTracker(eyebrows[0]);
		trackingSystem->pointTracker.addTracker(eyebrows[1]);

		std::cout << "EYES: " << eyes[0] << " + " << eyes[1] << std::endl;
		std::cout << "NOSE: " << nose[0] << " + " << nose[1] << std::endl;
		std::cout << "MOUTH: " << mouth[0] << " + " << mouth[1] << std::endl;
		std::cout << "EYEBROWS: " << eyebrows[0] << " + " << eyebrows[1] << std::endl;

		// Save point selection image
		trackingSystem->pointTracker.saveImage();

		// Calculate the area containing the face
		extractFaceRegionRectangle(videoInput->frame, trackingSystem->pointTracker.getPoints(&PointTracker::lastPoints, true));
		trackingSystem->pointTracker.normalizeOriginalGrey();
	}
	catch (std::ios_base::failure &e) {
		std::cout << e.what() << std::endl;
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
}

void MainGazeTracker::clearPoints() {
	if (_recording) {
		*_commandOutputFile << _totalFrameCount << " CLEAR" << std::endl;
	}

	trackingSystem->pointTracker.clearTrackers();
	_autoReload = false;
}

void MainGazeTracker::pauseOrRepositionHead() {
	if (Application::status == Application::STATUS_PAUSED) {
		if (_recording) {
			*_commandOutputFile << _totalFrameCount << " UNPAUSE" << std::endl;
		}

		Application::status = Application::isTrackerCalibrated ? Application::STATUS_CALIBRATED : Application::STATUS_IDLE;

		trackingSystem->pointTracker.retrack(videoInput->frame, 2);
		//choosePoints();
	} else {
		if (_recording) {
			*_commandOutputFile << _totalFrameCount << " PAUSE" << std::endl;
		}

		Application::status = Application::STATUS_PAUSED;

		_overlayImage = cvLoadImage("point-selection-frame.png", CV_LOAD_IMAGE_COLOR);
		_faces = FaceDetector::faceDetector.detect(_overlayImage);
	}
}

void MainGazeTracker::extractFaceRegionRectangle(IplImage *frame, std::vector<Point> featurePoints) {
	int minX = 10000;
	int maxX = 0;
	int minY = 10000;
	int maxY = 0;

	// Find the boundaries of the feature points
	for (int i = 0; i < (int)featurePoints.size(); i++) {
		minX = featurePoints[i].x < minX ? featurePoints[i].x : minX;
		minY = featurePoints[i].y < minY ? featurePoints[i].y : minY;
		maxX = featurePoints[i].x > maxX ? featurePoints[i].x : maxX;
		maxY = featurePoints[i].y > maxY ? featurePoints[i].y : maxY;
	}

	int diffX = maxX - minX;
	int diffY = maxY - minY;

	minX -= 0.4 * diffX;
	maxX += 0.4 * diffX;
	minY -= 0.5 * diffY;
	maxY += 0.5 * diffY;

	Application::faceRectangle = new CvRect();
	Application::faceRectangle->x = minX;
	Application::faceRectangle->y = minY;
	Application::faceRectangle->width = maxX - minX;
	Application::faceRectangle->height = maxY - minY;
}

