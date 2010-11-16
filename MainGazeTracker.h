#pragma once
#include "utils.h"
#include "TrackingSystem.h"
//#include "Alert.h"
#include "Calibrator.h"
#include <opencv/highgui.h>
#include <gtkmm.h>

struct CommandLineArguments {
    vector<char*> parameters;
    vector<char*> options;
     
    CommandLineArguments(int argc, char **argv);
    bool isoption(const char* option);
};



class VideoInput {
    CvCapture* capture;

 public:
    int framecount;
    const IplImage* frame;
    const CvSize size;
    VideoInput();
    VideoInput(const char* avifile);
    ~VideoInput();
    void updateFrame();
};

class VideoWriter;
/* class FileInput; */

class MainGazeTracker {
    scoped_ptr<VideoWriter> video;
    int framestoreload;
    vector<shared_ptr<AbstractStore> > stores;
    int framecount;
    bool autoreload;
//    StateMachine<void> statemachine;

 public:
    shared_ptr<TrackingSystem> tracking;

    FrameProcessing framefunctions;
    scoped_ptr<IplImage> canvas;
    scoped_ptr<VideoInput> videoinput;

    MainGazeTracker(int argc, char** argv,
		    const vector<shared_ptr<AbstractStore> > &stores);
    void doprocessing(void);
    ~MainGazeTracker(void);
    void addTracker(Point point);
    void addExemplar(Point exemplar);
    void startCalibration();
    void startTesting();
    void savepoints();
    void loadpoints();
    void clearpoints();
};
