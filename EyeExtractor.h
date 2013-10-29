#pragma once
#include "utils.h"
#include "PointTracker.h"
#include "BlinkDetector.h"


class EyeExtractor {
    const PointTracker &tracker; /* dangerous */
    scoped_ptr<IplImage> eyefloat2;
    scoped_ptr<IplImage> eyefloat2_left;
	BlinkDetector blinkdet;
	BlinkDetector blinkdet_left;
	bool blink;
    void processEye(void);

public:
    static const int eyedx;
    static const int eyedy;
    static const CvSize eyesize;

    scoped_ptr<IplImage> eyegrey, eyefloat, eyeimage;
    scoped_ptr<IplImage> eyegrey_left, eyefloat_left, eyeimage_left;

    EyeExtractor(const PointTracker &tracker);
    void extractEye(const IplImage *origimage) throw (TrackingException);
    void extractLeftEye(const IplImage *origimage) throw (TrackingException);
	bool isBlinking();
    ~EyeExtractor(void);
};

