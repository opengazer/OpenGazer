#pragma once
#include "utils.h"
#include "PointTracker.h"

class EyeExtractor {
    const PointTracker &tracker; /* dangerous */
    scoped_ptr<IplImage> eyefloat2;

    void processEye(void);

public:
    static const int eyedx;
    static const int eyedy;
    static const CvSize eyesize;

    scoped_ptr<IplImage> eyegrey, eyefloat, eyeimage;

    EyeExtractor(const PointTracker &tracker);
    void extractEye(const IplImage *origimage) throw (TrackingException);
    ~EyeExtractor(void);
};

