#pragma once

#include "TrackingSystem.h"
#include "Calibrator.h"
#include "GameWindow.h"
#include "OutputMethods.h"
#include "Video.h"
#include "Command.h"

class MainGazeTracker {
public:
	//bool isTesting;
	bool isCalibrationOutputWritten;
	boost::shared_ptr<TrackingSystem> trackingSystem;
	MovingTarget *target;
	FrameProcessing frameFunctions;
	boost::scoped_ptr<cv::Mat> canvas;
	boost::scoped_ptr<VideoInput> videoInput;

	MainGazeTracker(int argc, char **argv);
	~MainGazeTracker();
	void process();
	void simulateClicks();
	void cleanUp();
	void addTracker(Point point);
	void addExemplar(Point exemplar);
	void startCalibration();
	void startTesting();
	void startPlaying();
	void savePoints();
	void loadPoints();
	void choosePoints();
	void clearPoints();
	void pauseOrRepositionHead();
	void extractFaceRegionRectangle(cv::Mat &frame, std::vector<Point> featurePoints);

private:
	boost::scoped_ptr<VideoWriter> _video;
	int _frameStoreLoad;
	std::vector<boost::shared_ptr<AbstractStore> > _stores;
	int _frameCount;
	bool _autoReload;
	std::string _directory;
	std::string _basePath;
	std::ofstream *_outputFile;
	std::ofstream *_commandOutputFile;
	std::ifstream *_commandInputFile;
	cv::Mat _conversionImage;
	cv::Mat _overlayImage;
	cv::Mat _repositioningImage;
	cv::Rect _face;
	Calibrator *_calibrator;
	int _headDistance;
	bool _videoOverlays;
	long _totalFrameCount;
	bool _recording;
	std::vector<Command> _commands;
	int _commandIndex;
	GameWindow *_gameWin;
};

