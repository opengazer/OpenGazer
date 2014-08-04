#pragma once

#include "PointTracker.h"
#include "EyeExtractor.h"
#include "GazeTracker.h"
#include "HeadTracker.h"
#include "HeadCompensation.cpp"

class TrackingSystem {
public:
	PointTracker pointTracker;
	EyeExtractor eyeExtractor;
	GazeTracker gazeTracker;

	TrackingSystem(CvSize size);
	void process(const IplImage *frame, IplImage *image);
	void displayEye(IplImage *image, int baseX, int baseY, int stepX, int stepY);

private:
	HeadTracker _headTracker;
	HeadCompensation _headCompensation;
};
