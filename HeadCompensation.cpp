#include "LeastSquares.h"
#include "HeadTracker.h"

class HeadCompensation {
public:
	HeadCompensation(HeadTracker const &head):
		_xParams(3),
		_yParams(3),
		_headTracker(head),
		_xa0(0.0), _xa1(0.0), _xa2(0.0),
		_ya0(0.0), _ya1(0.0), _ya2(0.0),
		_samples(0)
	{
	}

	void addCorrection(Point correction) {
		_xParams.addSample(_headTracker.rotX, _headTracker.rotY, 1.0, correction.x);
		_yParams.addSample(_headTracker.rotX, _headTracker.rotY, 1.0, correction.y);
		_samples++;
	}

	void updateFactors(void) {
		if (_samples > 0) {
			_xParams.solve(_xa0, _xa1, _xa2);
			_yParams.solve(_ya0, _ya1, _ya2);
		}
	}

	Point estimateCorrection(void) {
		return Point(_xa0 * _headTracker.rotX + _xa1 * _headTracker.rotY + _xa2, _ya0 * _headTracker.rotX + _ya1 * _headTracker.rotY + _ya2);
	}

private:
	LeastSquares _xParams;
	LeastSquares _yParams;
	const HeadTracker &_headTracker;
	double _xa0, _xa1, _xa2;
	double _ya0, _ya1, _ya2;
	int _samples;
};
