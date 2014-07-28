#pragma once

#include <boost/scoped_ptr.hpp>
#include <opencv/cv.h>
#include <vector>

#include "Point.h"

class TrackingException: public std::exception {};

class PointTracker {
public:
	static const int eyePoint1 = 0;
	static const int eyePoint2 = 1;
	std::vector<char> status;
	std::vector<CvPoint2D32f> origPoints, currentPoints, lastPoints;

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
	std::vector<Point> getPoints(const std::vector<CvPoint2D32f> PointTracker::*points, bool allPoints=true);
	void draw(IplImage *canvas);
	void normalizeOriginalGrey();

	void save(std::string filename, std::string newname, const IplImage *frame);
	void load(std::string filename, std::string newname, const IplImage *frame);
	void saveImage();

private:
	static const int _winSize = 11;
	int _flags;
	boost::scoped_ptr<IplImage> _grey, _origGrey, _lastGrey;
	boost::scoped_ptr<IplImage> _pyramid, _origPyramid, _lastPyramid;

	void synchronizePoints();
};

