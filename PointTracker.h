#pragma once

#include <boost/scoped_ptr.hpp>

#include "Point.h"

class TrackingException: public std::exception {};

class PointTracker {
public:
	static const int eyePoint1 = 0;
	static const int eyePoint2 = 1;
	std::vector<uchar> status;
	std::vector<cv::Point2f> origPoints, currentPoints, lastPoints;

	PointTracker(const cv::Size &size);
	void clearTrackers();
	void addTracker(const Point &point);
	void updateTracker(int id, const Point &point);
	void removeTracker(int id);
	int getClosestTracker(const Point &point);
	void track(const cv::Mat &frame, int pyramidDepth=1);
	void retrack(const cv::Mat &frame, int pyramidDepth=1);
	int countActivePoints();
	bool areAllPointsActive();
	int pointCount();
	std::vector<Point> getPoints(const std::vector<cv::Point2f> PointTracker::*points, bool allPoints=true);
	void draw(cv::Mat &canvas);
	void normalizeOriginalGrey();

	void save(std::string filename, std::string newname, const cv::Mat frame);
	void load(std::string filename, std::string newname, const cv::Mat frame);
	void saveImage();

private:
	static const int _winSize = 11;
	int _flags;
	cv::Mat _grey, _origGrey, _lastGrey;
	
	void synchronizePoints();
};

