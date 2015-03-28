#include "FeatureDetector.h"
#include "utils.h"

FeatureDetector::FeatureDetector(cv::Size eyeSize):
	_eyeSize(eyeSize),
	_sumImage(new cv::Mat(eyeSize, CV_32FC1)),
	_sum2Image(new cv::Mat(eyeSize, CV_32FC1)),
	_temp(new cv::Mat(eyeSize, CV_32FC1)),
	_samples(0)
{
	_sumImage->setTo(cv::Scalar(0,0,0));
	_sum2Image->setTo(cv::Scalar(0,0,0));
}

void FeatureDetector::addSample(const cv::Mat *source) {
	source->convertTo(*_temp, _temp->type());
	cv::accumulate(*_temp, *_sumImage);
	cv::accumulateSquare(*_temp, *_sum2Image);
	_samples++;
}

boost::shared_ptr<cv::Mat> FeatureDetector::getMean() {
	boost::shared_ptr<cv::Mat> mean(Utils::createImage(_eyeSize, CV_32FC1));
	_sumImage->convertTo(*mean, mean->type(), 1.0 / _samples, 0);
	return mean;
}

boost::shared_ptr<cv::Mat> FeatureDetector::getVariance() {
	boost::shared_ptr<cv::Mat> variance(Utils::createImage(_eyeSize, CV_32FC1));
	cv::multiply(*_sumImage, *_sumImage, *_temp, -1.0 / _samples);
	cv::add(*_temp, *_sum2Image, *_temp);
	_temp->convertTo(*variance, variance->type(), 1.0 / _samples, 0);
	return variance;
}
