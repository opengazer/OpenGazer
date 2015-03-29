#include <opencv/highgui.h>
#include <fstream>

#include "PointTracker.h"
#include "Application.h"
#include "FaceDetector.h"
#include "utils.h"

using Utils::operator<<;
using Utils::operator>>;

static Point pointBetweenRects(const Point &point, cv::Rect source, cv::Rect dest) {
	return Point((point.x - source.x) * (double(dest.width) / source.width) + dest.x, 
		(point.y - source.y) * (double(dest.height) / source.height) + dest.y);
}

static std::vector<Point> pointBetweenRects(const std::vector<Point> &points, cv::Rect source, cv::Rect dest) {
	std::vector<Point> result;
	result.reserve(points.size());

	xForEach(iter, points) {
		result.push_back(pointBetweenRects(*iter, source, dest));
	}

	return result;
}

PointTracker::PointTracker(const cv::Size &size):
	_flags(CV_LKFLOW_INITIAL_GUESSES),
	_grey(size, 1),
	_origGrey(size, 1),
	_lastGrey(size, 1)
{
}

void PointTracker::clearTrackers() {
	currentPoints.clear();
	synchronizePoints();
}

void PointTracker::addTracker(const Point &point) {
	currentPoints.push_back(point.cvPoint32());
	synchronizePoints();
}

void PointTracker::updateTracker(int id, const Point &point) {
	currentPoints[id] = point.cvPoint32();
	synchronizePoints();
}

void PointTracker::removeTracker(int id) {
	currentPoints.erase(currentPoints.begin() + id);
	lastPoints.erase(lastPoints.begin() + id);
	origPoints.erase(origPoints.begin() + id);
}

int PointTracker::getClosestTracker(const Point &point) {
	std::vector<Point> points;
	Utils::convert(currentPoints, points);
	return point.closestPoint(points);
}

void PointTracker::track(const cv::Mat &frame, int pyramidDepth) {
	try {
		assert(lastPoints.size() == currentPoints.size());
		assert(origPoints.size() == currentPoints.size());
		status.resize(currentPoints.size());
		cvtColor(frame, _grey, CV_BGR2GRAY);

/*
		if (Application::faceRectangle != NULL) {
			Utils::normalizeGrayScaleImage_NEW(_grey(*Application::faceRectangle), 90, 160);
		}
*/
		// Apply median filter of 5x5
		medianBlur(_grey, _grey, 5);

		if (!currentPoints.empty()) {
			// then calculate the position based on the original
			// template without any pyramids
			std::vector<float> err;

/*
			std::cout << "BEFORE TRACKING:" << std::endl;
			for(int i=0; i<currentPoints.size(); i++) {
				std::cout << "Point " << i+1 << ": " << currentPoints[i].x << ", " << currentPoints[i].y << std::endl;
			}
*/
			calcOpticalFlowPyrLK(_origGrey, _grey, 
				origPoints, currentPoints, 
				status, err, 
				cv::Size(_winSize,_winSize), 
				pyramidDepth * 3, 
				cv::TermCriteria(CV_TERMCRIT_EPS, 20, 0.03), 
				_flags);

			_flags |= CV_LKFLOW_PYR_A_READY;
/*
			std::cout << "AFTER TRACKING:" << std::endl;
			for(int i=0; i<currentPoints.size(); i++) {
				std::cout << "Point " << i+1 << ": " << currentPoints[i].x << ", " << currentPoints[i].y << std::endl;
			}
*/
		}

		_grey.copyTo(_lastGrey);
		lastPoints = currentPoints;
	}
	catch (std::exception &ex) {
		std::cout << ex.what() << std::endl;
		clearTrackers();
	}
}

void PointTracker::retrack(const cv::Mat &frame, int pyramidDepth) {
	try {
		currentPoints = origPoints;

		std::cout << "RETRACKING" << std::endl;
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			std::cout << "CP["<< i <<"]" << currentPoints[i].x << ", " << currentPoints[i].y << std::endl;
		}

		_flags = 0;
		cvtColor(frame, _grey, CV_BGR2GRAY);

		// Apply median filter of 5x5
		medianBlur(_grey, _grey, 5);

		// then calculate the position based on the original
		// template without any pyramids
		std::vector<float> err;
		
		calcOpticalFlowPyrLK(_origGrey, _grey, 
			origPoints, currentPoints, 
			status, err, 
			cv::Size(_winSize,_winSize), 
			pyramidDepth * 3, 
			cv::TermCriteria(CV_TERMCRIT_EPS, 200, 0.0001), 
			_flags);
		//}

		_flags = CV_LKFLOW_INITIAL_GUESSES;
		_flags |= CV_LKFLOW_PYR_A_READY;

		_grey.copyTo(_lastGrey);

		lastPoints = currentPoints;

		std::cout << std::endl << "AFTER RETRACKING" << std::endl;
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			std::cout << "CP["<< i <<"]" << currentPoints[i].x << ", " << currentPoints[i].y << std::endl;
		}
	}
	catch (std::exception &ex) {
		std::cout << ex.what() << std::endl;
		clearTrackers();
	}
}

int PointTracker::countActivePoints() {
	return count_if(status.begin(), status.end(), bind1st(std::not_equal_to<char>(), 0));
}

bool PointTracker::areAllPointsActive() {
	return count(status.begin(), status.end(), 0) == 0;
}

int PointTracker::pointCount() {
	return currentPoints.size();
}

std::vector<Point> PointTracker::getPoints(const std::vector<cv::Point2f> PointTracker::*points, bool allPoints) {
	std::vector<Point> vec;
	for (int i = 0; i < pointCount(); i++) {
		if (allPoints || status[i]) {
			vec.push_back(Point((this->*points)[i].x, (this->*points)[i].y));
		}
	}
	return vec;
}
// TODO ONUR CHANGE CANVAS TYPE
void PointTracker::draw(cv::Mat &canvas) {
	try {
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			cv::circle(canvas, cvPointFrom32f(currentPoints[i]), 3, status[i] ? (i == eyePoint1 || i == eyePoint2 ? CV_RGB(255,0,0) : CV_RGB(0,255,0)) : CV_RGB(0,0,255), -1, 8, 0);
		}
	}
	catch (std::exception &ex) {
		std::cout << ex.what() << std::endl;
		clearTrackers();
	}
}

void PointTracker::normalizeOriginalGrey() {
	/* TODO ONUR COMMENTED HERE
	cvSetImageROI(_origGrey.get(), *Application::faceRectangle);
	Utils::normalizeGrayScaleImage2(_origGrey.get(), 90, 160);
	cvResetImageROI(_origGrey.get());
	*/
}

void PointTracker::save(std::string filename, std::string newname, const cv::Mat frame) {
	cv::Rect face = FaceDetector::faceDetector.detect(frame);
	if (face.width > 0) {
		cv::imwrite((filename + "-orig-grey.png").c_str(), _origGrey);
		
		std::ofstream origFile((filename + "-orig-points.txt").c_str());
		origFile << origPoints;

		std::ofstream facefile(newname.c_str());
		std::vector<Point> tempPoints;
		Utils::convert(currentPoints, tempPoints);
		facefile << pointBetweenRects(tempPoints, face, cv::Rect(0, 0, 1, 1));
	} else {
		throw std::ios_base::failure("No face found in the image");
	}
}

void PointTracker::load(std::string filename, std::string newname, const cv::Mat frame) {
	cv::Rect face = FaceDetector::faceDetector.detect(frame);

	if (face.width > 0) {
		std::ifstream origFile((filename + "-orig-points.txt").c_str());
		std::ifstream faceFile(newname.c_str());
		if (!origFile.is_open() || !faceFile.is_open()) {
			throw std::ios_base::failure("File not found");
		}

		// todo: memory leak here, change to scoped_ptr!
		_origGrey = cv::imread((filename + "-orig-grey.png").c_str(), 0);
		
		std::vector<Point> tempPoints;
		origFile >> tempPoints;
		Utils::convert(tempPoints, origPoints);

		faceFile >> tempPoints;
		tempPoints = pointBetweenRects(tempPoints, cvRect(0,0,1,1), face);
		Utils::convert(tempPoints, currentPoints);
		lastPoints = currentPoints;
	} else {
		throw std::ios_base::failure("No face found in the image");
	}
}

void PointTracker::saveImage() {
	cv::imwrite("point-selection-frame.png", _origGrey);
}

void PointTracker::synchronizePoints() {
	swap(_origGrey, _grey);
	origPoints = lastPoints = currentPoints;
}

