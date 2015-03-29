#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class FeatureDetector {
public:
	FeatureDetector(cv::Size eyeSize);
	void addSample(const cv::Mat *source);
	boost::shared_ptr<cv::Mat> getMean();
	boost::shared_ptr<cv::Mat> getVariance();

private:
	cv::Size _eyeSize;
	boost::scoped_ptr<cv::Mat> _sumImage, _sum2Image, _temp;
	int _samples;
};
