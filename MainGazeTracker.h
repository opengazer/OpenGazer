#pragma once

#include <opencv/highgui.h>

#include "TrackingSystem.h"
#include "Calibrator.h"
#include "GameWindow.h"
#include "OutputMethods.h"

bool detect_nose(IplImage* img, double resolution, CvRect nose_rect, Point points[]);
bool detect_mouth(IplImage* img, double resolution, CvRect nose_rect, Point points[]);
void detect_eye_corners(IplImage* img, double resolution, Point points[]);
void detect_eyebrow_corners(IplImage* img, double resolution, CvRect eyebrow_rect, Point points[]);
void check_rect_size(IplImage* image, CvRect* rect);
CvPoint2D32f* detect_corners_in_grayscale(IplImage* eye_region_image_gray, int& corner_count);

struct CommandLineArguments {
    std::vector<std::string> parameters;
    std::vector<std::string> options;
     
    CommandLineArguments(int argc, char **argv);
    ~CommandLineArguments();
    bool isoption(std::string option);
	std::string getoptionvalue(std::string option);
    std::vector<int> getoptionvalueasvector(std::string option);
};


struct Command {
	long frameno;
	std::string commandname;
	
    Command(long no, std::string name): frameno(no), commandname(name) {}
};


class VideoInput {
    CvCapture* capture;
	long last_frame_time;

 public:
    int framecount;
    IplImage* frame;
    CvSize size;
	bool capture_from_video;
	std::string resolution_parameter;
    VideoInput();
    VideoInput(std::string resolution);
    VideoInput(std::string resolution, std::string filename, bool dummy);
    ~VideoInput();
    void updateFrame();
	double get_resolution();
};

class VideoWriter;
/* class FileInput; */

class MainGazeTracker {
	boost::scoped_ptr<VideoWriter> video;
    int framestoreload;
    std::vector<boost::shared_ptr<AbstractStore> > stores;
    int framecount;
    bool autoreload;
	std::string directory;
	std::string base_path;
	std::ofstream* outputfile;
	std::ofstream* commandoutputfile;
	std::ifstream* commandinputfile;
	IplImage* conversionimage;
	IplImage* overlayimage;
	IplImage* repositioning_image;
	std::vector<CvRect> faces;
	
	Calibrator* calibrator;
	int headdistance;
	bool videooverlays;
	long totalframecount;
	bool recording;
	std::vector<Command> commands;
	int commandindex;

	GameWindow* game_win;
//    StateMachine<void> statemachine;

 public:	
	//bool isTesting;
	bool isCalibrationOutputWritten;
    boost::shared_ptr<TrackingSystem> tracking;
	MovingTarget* target;

    FrameProcessing framefunctions;
	boost::scoped_ptr<IplImage> canvas;
	boost::scoped_ptr<VideoInput> videoinput;
	

    MainGazeTracker(int argc, char** argv,
            const std::vector<boost::shared_ptr<AbstractStore> > &stores);
    void doprocessing(void);
	void simulateClicks(void);
    ~MainGazeTracker(void);
	void cleanUp(void);
    void addTracker(Point point);
    void addExemplar(Point exemplar);
    void startCalibration();
    void startTesting();
    void startPlaying();
    void savepoints();
    void loadpoints();
    void choosepoints();
	void pauseOrRepositionHead();
    void clearpoints();
    void extract_face_region_rectangle(IplImage* frame, std::vector<Point> feature_points);
};
