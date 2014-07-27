#pragma once

#include <glibmm.h>
#include <gdkmm.h>

#include "utils.h"
#include "TrackingSystem.h"
#include "Containers.h"
#include "GraphicalPointer.h"
#include "FeatureDetector.h"

class FrameProcessing;

class FrameFunction: public Containee<FrameProcessing, FrameFunction> {
public:
	FrameFunction(const int &frameNumber);
	virtual ~FrameFunction();
	virtual void process() = 0;

protected:
	int getFrame();

private:
	const int &_frameNumber;
	int _startFrame;
};

class FrameProcessing: public ProcessContainer<FrameProcessing, FrameFunction> {
};

class MovingTarget: public FrameFunction {
public:
	MovingTarget(const int &frameNumber, const vector<Point> &points, const boost::shared_ptr<WindowPointer> &windowPointer, int dwellTime=20);
	virtual ~MovingTarget();
	virtual void process();
	Point getActivePoint();
	int getDwellTime();
	int getPointFrame();
	bool isActive();
	bool isLast();

protected:
	vector<Point> _points;
	const int _dwellTime;

	int getPointNumber();

private:
	boost::shared_ptr<WindowPointer> _windowPointer;
};

class Calibrator: public MovingTarget {
public:
	static vector<Point> defaultPoints;

	Calibrator(const int &frameNumber, const boost::shared_ptr<TrackingSystem> &trackingSystem, const vector<Point> &points, const boost::shared_ptr<WindowPointer> &windowPointer, int dwellTime=20);
	virtual ~Calibrator();
	virtual void process();
	static vector<Point> loadPoints(istream &in);
	static vector<Point> scaled(const vector<Point> &points, double x, double y);
	static vector<Point> scaled(const vector<Point> &points, int x, int y, double width, double height);

private:
	static const Point _defaultPointArray[];
	boost::shared_ptr<TrackingSystem> _trackingSystem;
	boost::scoped_ptr<FeatureDetector> _averageEye;
	boost::scoped_ptr<FeatureDetector> _averageEyeLeft;
};


