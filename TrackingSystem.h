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

	TrackingSystem(cv::Size size);
	void process(const cv::Mat &frame, cv::Mat *image);
	void displayEye(cv::Mat &image, int baseX, int baseY, int stepX, int stepY);

private:
	HeadTracker _headTracker;
	HeadCompensation _headCompensation;
};
