#include "FeatureDetector.h"
#include "utils.h"

FeatureDetector::FeatureDetector(CvSize eyeSize):
	_eyeSize(eyeSize),
	_sumImage(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	_sum2Image(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	_temp(cvCreateImage(eyeSize, IPL_DEPTH_32F, 1)),
	_samples(0)
{
	cvZero(_sumImage.get());
	cvZero(_sum2Image.get());
}

void FeatureDetector::addSample(const IplImage *source) {
	cvConvertScale(source, _temp.get());
	cvAcc(_temp.get(), _sumImage.get());
	cvSquareAcc(_temp.get(), _sum2Image.get());
	_samples++;
}

boost::shared_ptr<IplImage> FeatureDetector::getMean() {
	boost::shared_ptr<IplImage> mean(Utils::createImage(_eyeSize, IPL_DEPTH_32F, 1));
	cvConvertScale(_sumImage.get(), mean.get(), 1.0 / _samples);
	return mean;
}

boost::shared_ptr<IplImage> FeatureDetector::getVariance() {
	boost::shared_ptr<IplImage> variance(Utils::createImage(_eyeSize, IPL_DEPTH_32F, 1));
	cvMul(_sumImage.get(), _sumImage.get(), _temp.get(), -1.0 / _samples);
	cvAdd(_temp.get(), _sum2Image.get(), _temp.get());
	cvScale(_temp.get(), variance.get(), 1.0 / _samples);
	return variance;
}
