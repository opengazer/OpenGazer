#include "FeatureDetector.h"

FeatureDetector::FeatureDetector(CvSize eyesize):
    eyesize(eyesize),
    sumimage(cvCreateImage(eyesize, IPL_DEPTH_32F, 1)),
    sum2image(cvCreateImage(eyesize, IPL_DEPTH_32F, 1)),
    temp(cvCreateImage(eyesize, IPL_DEPTH_32F, 1)),
    samples(0)
{
    cvZero(sum2image.get());
    cvZero(sumimage.get());
}

void FeatureDetector::addSample(const IplImage *source) {
    cvConvertScale(source, temp.get());
    cvAcc(temp.get(), sumimage.get());
    cvSquareAcc(temp.get(), sum2image.get());
    samples++;
}

shared_ptr<IplImage> FeatureDetector::getMean() {
    shared_ptr<IplImage> mean(createImage(eyesize, IPL_DEPTH_32F, 1));
    cvConvertScale(sumimage.get(), mean.get(), 1.0 / samples);
    return mean;
}

shared_ptr<IplImage> FeatureDetector::getVariance() {
    shared_ptr<IplImage> variance(createImage(eyesize, IPL_DEPTH_32F, 1));
    cvMul(sumimage.get(), sumimage.get(), temp.get(), -1.0/samples);
    cvAdd(temp.get(), sum2image.get(), temp.get());
    cvScale(temp.get(), variance.get(), 1.0/samples);
    return variance;
}
