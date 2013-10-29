#include "utils.h"
#include "EyeExtractor.h"

const int EyeExtractor::eyedx = 32;
const int EyeExtractor::eyedy = 16;
const CvSize EyeExtractor::eyesize = cvSize(eyedx*2, eyedy*2);

void EyeExtractor::processEye(void) {

    normalizeGrayScaleImage(eyegrey.get(), 127, 50);
    cvConvertScale(eyegrey.get(), eyefloat2.get());
    // todo: equalize it somehow first!
    cvSmooth(eyefloat2.get(), eyefloat.get(), CV_GAUSSIAN, 3);
    cvEqualizeHist(eyegrey.get(), eyegrey.get());

	// ONUR DUPLICATED CODE FOR LEFT EYE

    normalizeGrayScaleImage(eyegrey_left.get(), 127, 50);

    cvConvertScale(eyegrey_left.get(), eyefloat2_left.get());
    // todo: equalize it somehow first!
    cvSmooth(eyefloat2_left.get(), eyefloat_left.get(), CV_GAUSSIAN, 3);
    cvEqualizeHist(eyegrey_left.get(), eyegrey_left.get());

	// Blink detection trials
	scoped_ptr<IplImage> temp(cvCreateImage(eyesize, IPL_DEPTH_32F, 1));
	scoped_ptr<IplImage> temp2(cvCreateImage(eyesize, IPL_DEPTH_32F, 1));
	cvConvertScale(eyegrey.get(), temp.get());
	blinkdet.update(eyefloat);
	
	cvConvertScale(eyegrey_left.get(), temp2.get());
	blinkdet_left.update(eyefloat_left);
	
	if(blinkdet.getState() >= 2 && blinkdet_left.getState() >= 2) {
		blink = true;
		//cout << "BLINK!! RIGHT EYE STATE: " << blinkdet.getState() << "LEFT EYE STATE: " << blinkdet_left.getState() <<endl;
	}
	else {
		blink = false;
	}
}

bool EyeExtractor::isBlinking() {
	return blink;
}


EyeExtractor::EyeExtractor(const PointTracker &tracker):
    tracker(tracker), 
    eyefloat2(cvCreateImage( eyesize, IPL_DEPTH_32F, 1 )),
    eyegrey(cvCreateImage( eyesize, 8, 1 )),
    eyefloat(cvCreateImage( eyesize, IPL_DEPTH_32F, 1 )),
    eyeimage(cvCreateImage( eyesize, 8, 3 )),
 	// ONUR DUPLICATED CODE FOR LEFT EYE
    eyefloat2_left(cvCreateImage( eyesize, IPL_DEPTH_32F, 1 )),
    eyegrey_left(cvCreateImage( eyesize, 8, 1 )),
    eyefloat_left(cvCreateImage( eyesize, IPL_DEPTH_32F, 1 )),
    eyeimage_left(cvCreateImage( eyesize, 8, 3 )),
	blink(false)
{
}

void EyeExtractor::extractEye(const IplImage *origimage) 
    throw (TrackingException) 
{
    if (!tracker.status[tracker.eyepoint1])
	throw TrackingException();

    double x0 = tracker.currentpoints[tracker.eyepoint1].x;
    double y0 = tracker.currentpoints[tracker.eyepoint1].y;
    double x1 = tracker.currentpoints[tracker.eyepoint2].x;
    double y1 = tracker.currentpoints[tracker.eyepoint2].y;
    double factor = 0.17;
    double xfactor = 0.05;
    double yfactor = 0.20 * (x0 < x1 ? -1 : 1);
    double L = factor / eyedx;
    double LL = x0 < x1? L : -L;
    float matrix[6] = 
	{LL*(x1-x0), LL*(y0-y1), 
	 x0 + factor * ((1-xfactor)*(x1-x0) + yfactor * (y0-y1)),
	 LL*(y1-y0), LL*(x1-x0), 
	 y0 + factor * ((1-xfactor)*(y1-y0) + yfactor * (x1-x0))};
    CvMat M = cvMat( 2, 3, CV_32F, matrix );

    cvGetQuadrangleSubPix( origimage, eyeimage.get(), &M);
    cvCvtColor(eyeimage.get(), eyegrey.get(), CV_RGB2GRAY);

	extractLeftEye(origimage);
	
    processEye();
}

void EyeExtractor::extractLeftEye(const IplImage *origimage) 
    throw (TrackingException) 
{
    if (!tracker.status[tracker.eyepoint1])
	throw TrackingException();

    double x0 = tracker.currentpoints[tracker.eyepoint2].x;
    double y0 = tracker.currentpoints[tracker.eyepoint2].y;
    double x1 = tracker.currentpoints[tracker.eyepoint1].x;
    double y1 = tracker.currentpoints[tracker.eyepoint1].y;
    double factor = 0.17;
    double xfactor = 0.05;
    double yfactor = 0.20 * (x0 < x1 ? -1 : 1);
    double L = factor / eyedx;
    double LL = x0 < x1? L : -L;
    float matrix[6] = 
	{LL*(x1-x0), LL*(y0-y1), 
	 x0 + factor * ((1-xfactor)*(x1-x0) + yfactor * (y0-y1)),
	 LL*(y1-y0), LL*(x1-x0), 
	 y0 + factor * ((1-xfactor)*(y1-y0) + yfactor * (x1-x0))};
    CvMat M = cvMat( 2, 3, CV_32F, matrix );

	float matrix2[6] = 
	{LL*(x1-x0), LL*(y0-y1), 
	 x0 + 64 + factor * ((1-xfactor)*(x1-x0) + yfactor * (y0-y1)),
	 LL*(y1-y0), LL*(x1-x0), 
	 y0 + factor * ((1-xfactor)*(y1-y0) + yfactor * (x1-x0))};
    
    cvGetQuadrangleSubPix( origimage, eyeimage_left.get(), &M);
    cvCvtColor(eyeimage_left.get(), eyegrey_left.get(), CV_RGB2GRAY);
}
EyeExtractor::~EyeExtractor(void) {
}


