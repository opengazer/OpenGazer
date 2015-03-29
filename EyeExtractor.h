#pragma once

#include "PointTracker.h"
#include "BlinkDetector.h"

class EyeExtractor {
public:
	static const int eyeDX;
	static const int eyeDY;
	static const cv::Size eyeSize;

	boost::scoped_ptr<cv::Mat> eyeGrey, eyeFloat, eyeImage;
	boost::scoped_ptr<cv::Mat> eyeGreyLeft, eyeFloatLeft, eyeImageLeft;
	
	EyeExtractor(const PointTracker &pointTracker);
	~EyeExtractor();
	void extractEyes(const cv::Mat originalImage);
	bool isBlinking();

private:
	const PointTracker &_pointTracker; /* dangerous */
	BlinkDetector _blinkDetector;
	BlinkDetector _blinkDetectorLeft;
	bool _isBlinking;

	void extractEye(const cv::Mat originalImage) throw (TrackingException);
	void extractEyeLeft(const cv::Mat originalImage) throw (TrackingException);
	void processEyes();
};

