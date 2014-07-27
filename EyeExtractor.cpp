#include "utils.h"
#include "EyeExtractor.h"

const int EyeExtractor::eyeDX = 32;
const int EyeExtractor::eyeDY = 16;
const CvSize EyeExtractor::eyeSize = cvSize(eyeDX * 2, eyeDY * 2);

EyeExtractor::EyeExtractor(const PointTracker &pointTracker):
	_pointTracker(pointTracker),

	_eyeFloat2(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	eyeGrey(cvCreateImage(eyeSize, 8, 1)),
	eyeFloat(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	eyeImage(cvCreateImage(eyeSize, 8, 3)),

 	// ONUR DUPLICATED CODE FOR LEFT EYE
	_eyeFloat2Left(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	eyeGreyLeft(cvCreateImage(eyeSize, 8, 1)),
	eyeFloatLeft(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	eyeImageLeft(cvCreateImage(eyeSize, 8, 3)),

	_isBlinking(false)
{
}

EyeExtractor::~EyeExtractor() {}

void EyeExtractor::extractEyes(const IplImage *originalImage) {
	extractEye(originalImage, eyeGrey.get(), eyeImage.get());
	extractEye(originalImage, eyeGreyLeft.get(), eyeImageLeft.get());

	processEyes();
}

bool EyeExtractor::isBlinking() {
	return _isBlinking;
}

CvMat EyeExtractor::pointMatrix() throw (TrackingException) {
	//if (!_pointTracker.status[_pointTracker.eyepoint1]) {
	//	throw TrackingException();
	//}

	double x0 = _pointTracker.currentPoints[_pointTracker.eyePoint1].x;
	double y0 = _pointTracker.currentPoints[_pointTracker.eyePoint1].y;
	double x1 = _pointTracker.currentPoints[_pointTracker.eyePoint2].x;
	double y1 = _pointTracker.currentPoints[_pointTracker.eyePoint2].y;
	double factor = 0.17;
	double xFactor = 0.05;
	double yFactor = 0.20 * (x0 < x1 ? -1 : 1);
	double L = factor / eyeDX;
	double LL = x0 < x1? L : -L;
	float matrix[6] = {
		LL * (x1 - x0),
		LL * (y0 - y1),
		x0 + factor * ((1 - xFactor) * (x1 - x0) + yFactor * (y0 - y1)),
		LL * (y1 - y0),
		LL * (x1 - x0),
		y0 + factor * ((1 - xFactor) * (y1 - y0) + yFactor * (x1 - x0))
	};

	CvMat M = cvMat(2, 3, CV_32F, matrix);

	return M;
}


void EyeExtractor::extractEye(const IplImage *originalImage, IplImage *eyeGrey, IplImage *eyeImage) {
	CvMat M = pointMatrix();

	cvGetQuadrangleSubPix(originalImage, eyeImage, &M);
	cvCvtColor(eyeImage, eyeGrey, CV_RGB2GRAY);
}

void EyeExtractor::processEyes() {
	normalizeGrayScaleImage(eyeGrey.get(), 127, 50);
	cvConvertScale(eyeGrey.get(), _eyeFloat2.get());
	// todo: equalize it somehow first!
	cvSmooth(_eyeFloat2.get(), eyeFloat.get(), CV_GAUSSIAN, 3);
	cvEqualizeHist(eyeGrey.get(), eyeGrey.get());

	// ONUR DUPLICATED CODE FOR LEFT EYE
	normalizeGrayScaleImage(eyeGreyLeft.get(), 127, 50);
	cvConvertScale(eyeGreyLeft.get(), _eyeFloat2Left.get());
	// todo: equalize it somehow first!
	cvSmooth(_eyeFloat2Left.get(), eyeFloatLeft.get(), CV_GAUSSIAN, 3);
	cvEqualizeHist(eyeGreyLeft.get(), eyeGreyLeft.get());

	// Blink detection trials
	scoped_ptr<IplImage> temp(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1));
	scoped_ptr<IplImage> temp2(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1));

	cvConvertScale(eyeGrey.get(), temp.get());
	_blinkDetector.update(eyeFloat);
	cvConvertScale(eyeGreyLeft.get(), temp2.get());
	_blinkDetectorLeft.update(eyeFloatLeft);

	if (_blinkDetector.getState() >= 2 && _blinkDetectorLeft.getState() >= 2) {
		_isBlinking = true;
		//cout << "BLINK!! RIGHT EYE STATE: " << _blinkDetector.getState() << "LEFT EYE STATE: " << _blinkDetectorLeft.getState() <<endl;
	} else {
		_isBlinking = false;
	}
}

