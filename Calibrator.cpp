#include "Calibrator.h"
#include "Application.h"

FrameFunction::FrameFunction(const int &frameNumber):
	_frameNumber(frameNumber),
	_startFrame(frameNumber)
{
}

FrameFunction::~FrameFunction() {
#ifdef DEBUG
	std::cout << "Destroying framefunction" << std::endl;
#endif
}

int FrameFunction::getFrame() {
	return _frameNumber - _startFrame;
}

MovingTarget::MovingTarget(const int &frameNumber, const std::vector<Point> &points, const boost::shared_ptr<WindowPointer> &windowPointer, int dwellTime):
	FrameFunction(frameNumber),
	_points(points),
	_dwellTime(dwellTime),
	_windowPointer(windowPointer)
{
}

MovingTarget::~MovingTarget() {
	int id = getFrame() / _dwellTime;
}

void MovingTarget::process() {
	if (getPointNumber() != _points.size() && isActive()) {
		int id = getPointNumber();

		if (getPointFrame() == 1) {
			_windowPointer->setPosition((int)_points[id].x, (int)_points[id].y);
		}
	} else {
		if (getPointNumber() == _points.size() && Application::status == Application::STATUS_TESTING) {
			Application::status = Application::STATUS_CALIBRATED;
		}
		detach();
	}
}

bool MovingTarget::isActive() {
	if (parent == NULL) {
		return false;
	}

	return getPointNumber() < (int)_points.size();
}

bool MovingTarget::isLast() {
	return getPointNumber() == ((int)_points.size()) - 1;
}

int MovingTarget::getPointNumber() {
	return getFrame() / _dwellTime;
}

int MovingTarget::getPointFrame() {
	return getFrame() % _dwellTime;
}

int MovingTarget::getDwellTime() {
	return _dwellTime;
}

Point MovingTarget::getActivePoint() {
	return _points[getPointNumber()];
}

const Point Calibrator::_defaultPointArray[] = {
	Point(0.5, 0.5),
	Point(0.1, 0.5),
	Point(0.9, 0.5),
	Point(0.5, 0.1),
	Point(0.5, 0.9),
	Point(0.1, 0.1),
	Point(0.1, 0.9),
	Point(0.9, 0.9),
	Point(0.9, 0.1),
	Point(0.3, 0.3),
	Point(0.3, 0.7),
	Point(0.7, 0.7),
	Point(0.7, 0.3)
};

std::vector<Point> Calibrator::defaultPoints(_defaultPointArray, _defaultPointArray + (sizeof(_defaultPointArray) / sizeof(_defaultPointArray[0])));

Calibrator::Calibrator(const int &frameNumber, const boost::shared_ptr<TrackingSystem> &trackingSystem, const std::vector<Point> &points,  const boost::shared_ptr<WindowPointer> &windowPointer, int dwellTime):
	MovingTarget(frameNumber, points, windowPointer, dwellTime),
	_trackingSystem(trackingSystem)
{
	_trackingSystem->gazeTracker.clear();
	// todo: remove all calibration points
}

Calibrator::~Calibrator() {
#ifdef DEBUG
	std::cout << "Destroying calibrator" << std::endl;
#endif
}

void Calibrator::process() {
	static int dummy = 0;

	if (isActive()) {
		int id = getPointNumber();
		int frame = getPointFrame();

		if (frame == 1) { // start
			_averageEye.reset(new FeatureDetector(EyeExtractor::eyeSize));
			_averageEyeLeft.reset(new FeatureDetector(EyeExtractor::eyeSize));
		}

		if (frame >= 11) { // middle	ONUR _dwellTime/2 changed to 11
			if (!_trackingSystem->eyeExtractor.isBlinking()) {
				_averageEye->addSample(_trackingSystem->eyeExtractor.eyeFloat.get());
				_averageEyeLeft->addSample(_trackingSystem->eyeExtractor.eyeFloatLeft.get());

				// Neural network
				//if (dummy % 8 == 0) {  // Only add samples on the 11-19-27-35 frames
				//	for (int i = 0; i < 1000; i++) {   // Train 100 times with each frame
					_trackingSystem->gazeTracker.addSampleToNN(_points[id], _trackingSystem->eyeExtractor.eyeFloat.get(), _trackingSystem->eyeExtractor.eyeGrey.get());
					_trackingSystem->gazeTracker.addSampleToNN_left(_points[id], _trackingSystem->eyeExtractor.eyeFloatLeft.get(), _trackingSystem->eyeExtractor.eyeGreyLeft.get());

					dummy++;
				//}
			} else {
				std::cout << "Skipped adding sample!!!!" << std::endl;
			}
		}

		if (frame == _dwellTime - 1) { // end
			_trackingSystem->gazeTracker.addExemplar(_points[id], _averageEye->getMean().get(), _trackingSystem->eyeExtractor.eyeGrey.get());
			// ONUR DUPLICATED CODE
			_trackingSystem->gazeTracker.addExemplar_left(_points[id], _averageEyeLeft->getMean().get(), _trackingSystem->eyeExtractor.eyeGreyLeft.get());

			if(id == _points.size() - 1) {
				Application::status = Application::STATUS_CALIBRATED;
				Application::isTrackerCalibrated = true;

				//_trackingSystem->gazeTracker.trainNN();
				//_trackingSystem->gazeTracker.calculateTrainingErrors();
			}

			// If we have processed the last target
			// Calculate training error and output on screen
			//if (isLast()) {
			//	_trackingSystem->gazeTracker.calculateTrainingErrors();
			//}
		}
	}
	MovingTarget::process();
}

std::vector<Point> Calibrator::loadPoints(std::istream &in) {
	std::vector<Point> result;

	for(;;) {
		double x, y;
		in >> x >> y;
		if (in.rdstate()) {
			// break if any error
			break;
		}
		result.push_back(Point(x, y));
	}

	return result;
}

std::vector<Point> Calibrator::scaled(const std::vector<Point> &points, double x, double y) {
	//double dx = x > y ? (x-y)/2 : 0.0;
	//double dy = y > x ? (y-x)/2 : 0.0;
	//double scale = x > y ? y : x;

	std::vector<Point> result;

	xForEach(iter, points) {
		result.push_back(Point(iter->x * x, iter->y * y));
		//result.push_back(Point(iter->x * scale + dx, iter->y * scale + dy));
	}

	return result;
}

std::vector<Point> Calibrator::scaled(const std::vector<Point> &points, int x, int y, double width, double height) {
	std::vector<Point> result;

	xForEach(iter, points) {
		result.push_back(Point(iter->x * width + x, iter->y * height + y));
		//std::cout << "ADDED POINT (" << iter->x * width + x << ", " << iter->y * height + y << ")" << std::endl;
	}

	return result;
}
