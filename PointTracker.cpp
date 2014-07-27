#include <opencv/highgui.h>
#include <fstream>

#include "Application.h"
#include "PointTracker.h"
#include "FaceDetector.h"
#include "utils.h"

using Utils::operator<<;
using Utils::operator>>;

static Point pointBetweenRects(const Point &point, CvRect source, CvRect dest) {
	return Point((point.x - source.x) * (double(dest.width) / source.width) + dest.x, (point.y - source.y) * (double(dest.height) / source.height) + dest.y);
}

static vector<Point> pointBetweenRects(const vector<Point> &points, CvRect source, CvRect dest) {
	vector<Point> result;

	result.reserve(points.size());
	xForEach(iter, points)
	result.push_back(pointBetweenRects(*iter, source, dest));

	return result;
}

PointTracker::PointTracker(const CvSize &size):
	_flags(CV_LKFLOW_INITIAL_GUESSES),
	_grey(cvCreateImage(size, 8, 1)),
	_origGrey(cvCreateImage(size, 8, 1)),
	_lastGrey(cvCreateImage(size, 8, 1)),
	_pyramid(cvCreateImage(size, 8, 1)),
	_origPyramid(cvCreateImage(size, 8, 1)),
	_lastPyramid(cvCreateImage(size, 8, 1))
	//origPoints(new CvPoint2D32f[MAX_COUNT]),
	//currentPoints(new CvPoint2D32f[MAX_COUNT]),
	//status(new char[MAX_COUNT]),
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
	vector<Point> points;
	Utils::convert(currentPoints, points);
	return point.closestPoint(points);
}

void PointTracker::track(const IplImage *frame, int pyramidDepth) {
	try {
		assert(lastPoints.size() == currentPoints.size());
		assert(origPoints.size() == currentPoints.size());
		status.resize(currentPoints.size());
		cvCvtColor(frame, _grey.get(), CV_BGR2GRAY );

		if (Application::faceRectangle != NULL) {
			cvSetImageROI(_grey.get(), *Application::faceRectangle);
			Utils::normalizeGrayScaleImage2(_grey.get(), 90, 160);
			cvResetImageROI(_grey.get());
		}

		// Apply median filter of 5x5
		cvSmooth(_grey.get(), _grey.get(), CV_MEDIAN, 5);

		if (!currentPoints.empty()) {
			// then calculate the position based on the original
			// template without any pyramids
			cvCalcOpticalFlowPyrLK(
				_origGrey.get(), _grey.get(),
				_origPyramid.get(), _pyramid.get(),
				&origPoints[0], &currentPoints[0], pointCount(),
				cvSize(_winSize, _winSize),
				pyramidDepth * 3, &status[0], 0,
				cvTermCriteria(CV_TERMCRIT_EPS,20,0.03),
				_flags);
		//}
			_flags |= CV_LKFLOW_PYR_A_READY;
		}

		cvCopy(_grey.get(), _lastGrey.get(), 0);
		cvCopy(_pyramid.get(), _lastPyramid.get(), 0);
		lastPoints = currentPoints;
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		clearTrackers();
	}
}

void PointTracker::retrack(const IplImage *frame, int pyramidDepth) {
	try {
		currentPoints = origPoints;

		cout << "RETRACKING" << endl;
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			cout << "CP["<< i <<"]" << currentPoints[i].x << ", " << currentPoints[i].y << endl;
		}

		_flags = 0;
		cvCvtColor(frame, _grey.get(), CV_BGR2GRAY );

		// Apply median filter of 5x5
		cvSmooth(_grey.get(), _grey.get(), CV_MEDIAN, 5);

		// then calculate the position based on the original
		// template without any pyramids
		cvCalcOpticalFlowPyrLK(
			_origGrey.get(), _grey.get(),
			_origPyramid.get(), _pyramid.get(),
			&origPoints[0], &currentPoints[0], pointCount(),
			cvSize(_winSize, _winSize),
			pyramidDepth * 3, &status[0], 0,
			cvTermCriteria(CV_TERMCRIT_EPS,200,0.0001),
			_flags);

		//}

		_flags = CV_LKFLOW_INITIAL_GUESSES;
		_flags |= CV_LKFLOW_PYR_A_READY;

		cvCopy(_grey.get(), _lastGrey.get(), 0);
		cvCopy(_pyramid.get(), _lastPyramid.get(), 0);
		lastPoints = currentPoints;

		cout << endl << "AFTER RETRACKING" << endl;
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			cout << "CP["<< i <<"]" << currentPoints[i].x << ", " << currentPoints[i].y << endl;
		}
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		clearTrackers();
	}
}

int PointTracker::countActivePoints() {
	return count_if(status.begin(), status.end(), bind1st(not_equal_to<char>(), 0));
}

bool PointTracker::areAllPointsActive() {
	return count(status.begin(), status.end(), 0) == 0;
}

int PointTracker::pointCount() {
	return currentPoints.size();
}

vector<Point> PointTracker::getPoints(const vector<CvPoint2D32f> PointTracker::*points, bool allPoints) {
	vector<Point> vec;
	for (int i = 0; i < pointCount(); i++) {
		if (allPoints || status[i]) {
			vec.push_back(Point((this->*points)[i].x, (this->*points)[i].y));
		}
	}
	return vec;
}

void PointTracker::draw(IplImage *canvas) {
	try {
		for (int i = 0; i < (int)currentPoints.size(); i++) {
			cvCircle(canvas, cvPointFrom32f(currentPoints[i]), 3, status[i] ? (i == eyePoint1 || i == eyePoint2 ? CV_RGB(255,0,0) : CV_RGB(0,255,0)) : CV_RGB(0,0,255), -1, 8, 0);
		}
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		clearTrackers();
	}
}

void PointTracker::normalizeOriginalGrey() {
	cvSetImageROI(_origGrey.get(), *Application::faceRectangle);
	Utils::normalizeGrayScaleImage2(_origGrey.get(), 90, 160);
	cvResetImageROI(_origGrey.get());
}

void PointTracker::save(string filename, string newname, const IplImage *frame) {
	vector<CvRect> faces = FaceDetector::faceDetector.detect(frame);
	if (faces.size() == 1) {
		cvSaveImage((filename + "-orig-grey.png").c_str(), _origGrey.get());
		cvSaveImage((filename + "-orig-pyramid.png").c_str(), _origPyramid.get());

		ofstream origFile((filename + "-orig-points.txt").c_str());
		origFile << origPoints;

		CvRect face = faces[0];
		ofstream facefile(newname.c_str());
		vector<Point> tempPoints;
		Utils::convert(currentPoints, tempPoints);
		facefile << pointBetweenRects(tempPoints, face, cvRect(0, 0, 1, 1));
	} else {
		throw ios_base::failure("No face found in the image");
	}
}

void PointTracker::load(string filename, string newname, const IplImage *frame) {
	vector<CvRect> faces = FaceDetector::faceDetector.detect(frame);

	if (faces.size() == 1) {
		ifstream origFile((filename + "-orig-points.txt").c_str());
		ifstream faceFile(newname.c_str());
		if (!origFile.is_open() || !faceFile.is_open()) {
			throw ios_base::failure("File not found");
		}

		// todo: memory leak here, change to scoped_ptr!
		_origGrey.reset(cvLoadImage((filename + "-orig-grey.png").c_str(), 0));
		_origPyramid.reset(cvLoadImage((filename + "-orig-pyramid.png").c_str(), 0));

		vector<Point> tempPoints;
		origFile >> tempPoints;
		Utils::convert(tempPoints, origPoints);

		faceFile >> tempPoints;
		tempPoints = pointBetweenRects(tempPoints, cvRect(0,0,1,1), faces[0]);
		Utils::convert(tempPoints, currentPoints);
		lastPoints = currentPoints;
	} else {
		throw ios_base::failure("No face found in the image");
	}
}

void PointTracker::saveImage() {
	cvSaveImage("point-selection-frame.png", _origGrey.get());
}

void PointTracker::synchronizePoints() {
	swap(_origGrey, _grey);
	swap(_origPyramid, _pyramid);
	origPoints = lastPoints = currentPoints;
}

