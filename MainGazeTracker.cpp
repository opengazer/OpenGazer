#include <fstream>

#include "MainGazeTracker.h"
#include "Application.h"
#include "utils.h"
#include "FaceDetector.h"
#include "Detection.h"

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
}

MainGazeTracker::MainGazeTracker(int argc, char **argv):
	_frameStoreLoad(-1),
	_stores(Application::getStores()),
	_autoReload(false),
	_videoOverlays(false),
	_totalFrameCount(0),
	_recording(false),
	_commandIndex(-1)
{
	CommandLineArguments args(argc, argv);

	if (args.getOptionValue("help").length()) {
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

	if (args.getOptionValue("input").length()) {
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
		if (args.getOptionValue("resolution").length()) {
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

	if (subject.length() == 0) {
		subject = "default";
	}

	if (setup.length() == 0) {
		setup = "std";
	}

	if (args.getOptionValue("overlay") == "1") {
		_videoOverlays = true;
	}

	// --headdistance parameter
	if (args.getOptionValue("headdistance").length()) {
		_headDistance = atoi(args.getOptionValue("headdistance").c_str());
	} else {
		_headDistance = 70;
	}

	// --dwelltime parameter
	if (args.getOptionValue("dwelltime").length()) {
		Application::dwelltimeParameter = atoi(args.getOptionValue("dwelltime").c_str());
	} else {
		Application::dwelltimeParameter = 30;
	}

	// --testdwelltime parameter
	if (args.getOptionValue("testdwelltime").length()) {
		Application::testDwelltimeParameter = atoi(args.getOptionValue("testdwelltime").c_str());
	} else {
		Application::testDwelltimeParameter = 20;
	}

	// --sleep parameter
	if (args.getOptionValue("sleep").length()) {
		Application::sleepParameter = atoi(args.getOptionValue("sleep").c_str());
	 } else {
		Application::sleepParameter = 0;
	 }

	// --folder parameter
	std::string folderParameter = "outputs";

	if (args.getOptionValue("outputfolder").length()) {
		folderParameter = args.getOptionValue("outputfolder");
	}

	// --subject parameter
	_basePath = Utils::getUniqueFileName(folderParameter, subject + "_" + setup + "_" + args.getOptionValue("resolution"));

	// --record parameter
	if (args.getOptionValue("record") == "1") {
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

		Detection::detectEyeCorners(videoInput->frame, videoInput->getResolution(), eyes);

		CvRect noseRect = cvRect(eyes[0].x, eyes[0].y, fabs(eyes[0].x - eyes[1].x), fabs(eyes[0].x - eyes[1].x));
		checkRectSize(videoInput->frame, &noseRect);
		//std::cout << "Nose rect: " << noseRect.x << ", " << noseRect.y << " - " << noseRect.width << ", " << noseRect.height << std::endl;

		if (!Detection::detectNose(videoInput->frame, videoInput->getResolution(), noseRect, nose)) {
			std::cout << "NO NOSE" << std::endl;
			return;
		}

		CvRect mouthRect = cvRect(eyes[0].x, nose[0].y, fabs(eyes[0].x - eyes[1].x), 0.8 * fabs(eyes[0].x - eyes[1].x));
		checkRectSize(videoInput->frame, &mouthRect);

		if (!Detection::detectMouth(videoInput->frame, videoInput->getResolution(), mouthRect, mouth)) {
			std::cout << "NO MOUTH" << std::endl;
			return;
		}

		CvRect eyebrowRect = cvRect(eyes[0].x + fabs(eyes[0].x - eyes[1].x) * 0.25, eyes[0].y - fabs(eyes[0].x - eyes[1].x) * 0.40, fabs(eyes[0].x - eyes[1].x) * 0.5, fabs(eyes[0].x - eyes[1].x) * 0.25);
		checkRectSize(videoInput->frame, &eyebrowRect);
		Detection::detectEyebrowCorners(videoInput->frame, videoInput->getResolution(), eyebrowRect, eyebrows);

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

