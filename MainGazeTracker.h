#pragma once
#include "utils.h"
#include "TrackingSystem.h"
//#include "Alert.h"
#include "Calibrator.h"
#include "FaceDetector.h"
#include <opencv/highgui.h>
#include <gtkmm.h>
#include "fann.h"
#include "GameWindow.h"

#include <sys/time.h>

bool detect_nose(IplImage* img, double resolution, CvRect nose_rect, Point points[]);
bool detect_mouth(IplImage* img, double resolution, CvRect nose_rect, Point points[]);
void detect_eye_corners(IplImage* img, double resolution, Point points[]);
void detect_eyebrow_corners(IplImage* img, double resolution, CvRect eyebrow_rect, Point points[]);
void check_rect_size(IplImage* image, CvRect* rect);
CvPoint2D32f* detect_corners_in_grayscale(IplImage* eye_region_image_gray, int& corner_count);

struct CommandLineArguments {
    vector<string> parameters;
    vector<string> options;
     
    CommandLineArguments(int argc, char **argv);
    ~CommandLineArguments();
    bool isoption(string option);
	string getoptionvalue(string option);
    vector<int> getoptionvalueasvector(string option);
};


struct Command {
	long frameno;
	string commandname;
	
    Command(long no, string name): frameno(no), commandname(name) {}
};


class VideoInput {
    CvCapture* capture;
	long last_frame_time;

 public:
    int framecount;
    IplImage* frame;
    CvSize size;
	bool capture_from_video;
	string resolution_parameter;
    VideoInput();
    VideoInput(string resolution);
    VideoInput(string resolution, string filename, bool dummy);
    ~VideoInput();
    void updateFrame();
	double get_resolution();
};

class VideoWriter;
/* class FileInput; */

class MainGazeTracker {
    scoped_ptr<VideoWriter> video;
    int framestoreload;
    vector<shared_ptr<AbstractStore> > stores;
    int framecount;
    bool autoreload;
	string directory;
	string base_path;
	ofstream* outputfile;
	ofstream* commandoutputfile;
	ifstream* commandinputfile;
	IplImage* conversionimage;
	IplImage* overlayimage;
	IplImage* repositioning_image;
	vector<CvRect> faces;
	
	Calibrator* calibrator;
	int headdistance;
	bool videooverlays;
	long totalframecount;
	bool recording;
	vector<Command> commands;
	int commandindex;

	GameWindow* game_win;
//    StateMachine<void> statemachine;

 public:	
	//bool isTesting;
	bool isCalibrationOutputWritten;
    shared_ptr<TrackingSystem> tracking;
	MovingTarget* target;

    FrameProcessing framefunctions;
    scoped_ptr<IplImage> canvas;
    scoped_ptr<VideoInput> videoinput;
	

    MainGazeTracker(int argc, char** argv,
		    const vector<shared_ptr<AbstractStore> > &stores);
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
    void extract_face_region_rectangle(IplImage* frame, vector<HomPoint> feature_points);
};
