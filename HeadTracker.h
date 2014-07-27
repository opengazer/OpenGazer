#pragma once

#include <opencv/cv.h>
#include <vector>

#include "PointTracker.h"

using namespace std;

class HeadTracker {
public:
	PointTracker &pointTracker;
	double rotX;
	double rotY;
	double atX;
	double atY;

	HeadTracker(PointTracker &pointTracker);
	void draw(IplImage *image);
	void updateTracker();

private:
	vector<double> _depths;

	vector<bool> detectInliers(vector<Point> const &prev, vector<Point> const &now, double radius=30.0);
	void predictPoints(double xx0, double yy0, double xx1, double yy1, double rotX, double rotY, double atX, double atY);
};
