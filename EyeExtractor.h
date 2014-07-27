#pragma once

#include "utils.h"
#include "PointTracker.h"
#include "BlinkDetector.h"

class EyeExtractor {
public:
	static const int eyeDX;
	static const int eyeDY;
	static const CvSize eyeSize;

	scoped_ptr<IplImage> eyeGrey, eyeFloat, eyeImage;
	scoped_ptr<IplImage> eyeGreyLeft, eyeFloatLeft, eyeImageLeft;

	EyeExtractor(const PointTracker &pointTracker);
	~EyeExtractor();
	void extractEyes(const IplImage *originalImage);
	bool isBlinking();

private:
	const PointTracker &_pointTracker; /* dangerous */
	scoped_ptr<IplImage> _eyeFloat2;
	scoped_ptr<IplImage> _eyeFloat2Left;
	BlinkDetector _blinkDetector;
	BlinkDetector _blinkDetectorLeft;
	bool _isBlinking;

	CvMat pointMatrix() throw (TrackingException);
	void extractEye(const IplImage *originalImage, IplImage *eyeGrey, IplImage *eyeImage);
	void processEyes();
};

