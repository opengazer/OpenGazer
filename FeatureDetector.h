#pragma once
#include "utils.h"

class FeatureDetector {
    CvSize eyesize;
    scoped_ptr<IplImage> sumimage, sum2image, temp;
    int samples;
public:
    FeatureDetector(CvSize eyesize);
    void addSample(const IplImage *source);
    shared_ptr<IplImage> getMean();
    shared_ptr<IplImage> getVariance();
};
