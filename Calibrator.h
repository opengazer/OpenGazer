#pragma once
#include <glibmm.h>
#include <gdkmm.h>
#include "utils.h"
#include "TrackingSystem.h"
#include "Containers.h"
#include "GraphicalPointer.h"
#include "FeatureDetector.h"

class FrameProcessing;

class FrameFunction: 
public Containee<FrameProcessing, FrameFunction> 
{
    const int &frameno;
    int startframe;
 protected:
    int getFrame() { return frameno - startframe; }
public:
    FrameFunction(const int &frameno): frameno(frameno), startframe(frameno) {}
    virtual void process()=0;
    virtual ~FrameFunction();
};

class FrameProcessing: 
public ProcessContainer<FrameProcessing,FrameFunction> {};

class MovingTarget: public FrameFunction {
    boost::shared_ptr<WindowPointer> pointer;
 public:
    MovingTarget(const int &frameno, 
		 const vector<Point>& points, 
		 const boost::shared_ptr<WindowPointer> &pointer,
		 int dwelltime=20);
    virtual ~MovingTarget();
    virtual void process();
	Point getActivePoint();
	int getDwellTime();
    int getPointFrame();
    bool active();
    bool isLast();
 protected:
    vector<Point> points;
    const int dwelltime;
    int getPointNo();
};

class Calibrator: public MovingTarget {
    static const Point defaultpointarr[];
    boost::shared_ptr<TrackingSystem> trackingsystem;
    scoped_ptr<FeatureDetector> averageeye;
    scoped_ptr<FeatureDetector> averageeye_left;
public:
    static vector<Point> defaultpoints;
    static vector<Point> loadpoints(istream& in);
    Calibrator(const int &frameno, 
	       const boost::shared_ptr<TrackingSystem> &trackingsystem, 
	       const vector<Point>& points, 
	       const boost::shared_ptr<WindowPointer> &pointer,
	       int dwelltime=20);
    virtual ~Calibrator();
    virtual void process();
    static vector<Point> scaled(const vector<Point>& points, double x, double y);
    static vector<Point> scaled(const vector<Point>& points, int x, int y, double width, double height);
};


