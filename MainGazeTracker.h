#pragma once

#include <opencv/highgui.h>

#include "TrackingSystem.h"
#include "Calibrator.h"
#include "GameWindow.h"
#include "OutputMethods.h"

struct CommandLineArguments {
	std::vector<std::string> parameters;
	std::vector<std::string> options;

	CommandLineArguments(int argc, char **argv);
	~CommandLineArguments();
	bool isOption(std::string option);
	std::string getOptionValue(std::string option);
	std::vector<int> getOptionValueAsVector(std::string option);
};

struct Command {
	long frameNumber;
	std::string commandName;

	Command(long number, std::string name);
};

class VideoInput {
private:
	CvCapture *_capture;
	long _lastFrameTime;

public:
	int frameCount;
	IplImage *frame;
	CvSize size;
	bool captureFromVideo;
	std::string resolutionParameter;

	VideoInput();
	VideoInput(std::string resolution);
	VideoInput(std::string resolution, std::string filename, bool dummy);
	~VideoInput();
	void updateFrame();
	double getResolution();
};

class VideoWriter {
public:
	VideoWriter(CvSize size, std::string filename);
	~VideoWriter();
	void write(const IplImage *image);

private:
	CvVideoWriter *_video;
};

/* class FileInput; */

class MainGazeTracker {
public:
	//bool isTesting;
	bool isCalibrationOutputWritten;
	boost::shared_ptr<TrackingSystem> trackingSystem;
	MovingTarget *target;
	FrameProcessing frameFunctions;
	boost::scoped_ptr<IplImage> canvas;
	boost::scoped_ptr<VideoInput> videoInput;

	MainGazeTracker(int argc, char** argv, const std::vector<boost::shared_ptr<AbstractStore> > &stores);
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
	void extractFaceRegionRectangle(IplImage *frame, std::vector<Point> featurePoints);

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
	IplImage *_conversionImage;
	IplImage *_overlayImage;
	IplImage *_repositioningImage;
	std::vector<CvRect> _faces;

	Calibrator *_calibrator;
	int _headDistance;
	bool _videoOverlays;
	long _totalFrameCount;
	bool _recording;
	std::vector<Command> _commands;
	int _commandIndex;

	GameWindow *_gameWin;
//	StateMachine<void> _stateMachine;
};

