#pragma once

#include <opencv/cv.h>
#include <vector>

#include "utils.h"

class TrackingException: public std::exception {};

class PointTracker {
public:
	static const int eyePoint1 = 0;
	static const int eyePoint2 = 1;
	vector<char> status;
	vector<CvPoint2D32f> origPoints, currentPoints, lastPoints;

	PointTracker(const CvSize &size);
	void clearTrackers();
	void addTracker(const Point &point);
	void updateTracker(int id, const Point &point);
	void removeTracker(int id);
	int getClosestTracker(const Point &point);
	void track(const IplImage *frame, int pyramidDepth=1);
	void retrack(const IplImage *frame, int pyramidDepth=1);
	int countActivePoints();
	bool areAllPointsActive();
	int pointCount();
	vector<Point> getPoints(const vector<CvPoint2D32f> PointTracker::*points, bool allPoints=true);
	void draw(IplImage *canvas);
	void normalizeOriginalGrey();

	void save(string filename, string newname, const IplImage *frame);
	void load(string filename, string newname, const IplImage *frame);
	void saveImage();

private:
	static const int _winSize = 11;
	int _flags;
	boost::scoped_ptr<IplImage> _grey, _origGrey, _lastGrey;
	boost::scoped_ptr<IplImage> _pyramid, _origPyramid, _lastPyramid;

	void synchronizePoints();
};

