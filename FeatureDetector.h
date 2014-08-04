#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class FeatureDetector {
public:
	FeatureDetector(CvSize eyeSize);
	void addSample(const IplImage *source);
	boost::shared_ptr<IplImage> getMean();
	boost::shared_ptr<IplImage> getVariance();

private:
	CvSize _eyeSize;
	boost::scoped_ptr<IplImage> _sumImage, _sum2Image, _temp;
	int _samples;
};
