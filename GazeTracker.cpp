#include "GazeTracker.h"

int Targets::getCurrentTarget(Point point) {
    vector<double> distances(targets.size());
    //    debugtee(targets);
    transform(targets.begin(), targets.end(), distances.begin(),
	      sigc::mem_fun(point, &Point::distance));
    //    debugtee(distances);
    return min_element(distances.begin(), distances.end()) - distances.begin();
//     for(int i=0; i<targets.size(); i++)
// 	if (point.distance(targets[i]) < 30)
// 	    return i;
//     return -1;
}


CalTarget::CalTarget() {}

CalTarget::CalTarget(Point point, 
		     const IplImage* image, const IplImage* origimage):
    point(point), 
    image(cvCloneImage(image), releaseImage), 
    origimage(cvCloneImage(origimage), releaseImage) 
{
}

void CalTarget::save(CvFileStorage* out, const char* name) {
    cvStartWriteStruct(out, name, CV_NODE_MAP);
    point.save(out, "point");
    cvWrite(out, "image", image.get());
    cvWrite(out, "origimage", origimage.get());
    cvEndWriteStruct(out);
}

void CalTarget::load(CvFileStorage* in, CvFileNode *node) {
    point.load(in, cvGetFileNodeByName(in, node, "point"));
    image.reset((IplImage*) cvReadByName(in, node, "image"));
    origimage.reset((IplImage*) cvReadByName(in, node, "origimage"));
}

TrackerOutput::TrackerOutput(Point gazepoint, Point target, int targetid):
    gazepoint(gazepoint), target(target), targetid(targetid)
{
}

template <class T, class S>
vector<S> getsubvector(vector<T> const& input, S T::*ptr) {
    vector<S> output(input.size());
    for(int i=0; i<input.size(); i++)
	output[i] = input[i].*ptr;
    return output;
}

double GazeTracker::imagedistance(const IplImage *im1, const IplImage *im2) {
    double norm = cvNorm(im1, im2, CV_L2);
    return norm*norm;
}

double GazeTracker::covariancefunction(SharedImage const& im1, 
				       SharedImage const& im2)
{
    const double sigma = 1.0;
    const double lscale = 500.0;
    return sigma*sigma*exp(-imagedistance(im1.get(),im2.get())/(2*lscale*lscale));
}

void GazeTracker::updateGPs(void) {
    Vector xlabels(caltargets.size());
    Vector ylabels(caltargets.size());
		
    for(int i=0; i<caltargets.size(); i++) {
	xlabels[i] = caltargets[i].point.x;
	ylabels[i] = caltargets[i].point.y;
    }

    vector<SharedImage> images = 
	getsubvector(caltargets, &CalTarget::image);

    gpx.reset(new ImProcess(images, xlabels, covariancefunction, 0.01));
    gpy.reset(new ImProcess(images, ylabels, covariancefunction, 0.01));  
    targets.reset(new Targets(getsubvector(caltargets, &CalTarget::point)));
}

void GazeTracker::clear() {
    caltargets.clear();
    // updateGPs()
}

void GazeTracker::addExemplar(Point point, 
			      const IplImage *eyefloat, 
			      const IplImage *eyegrey) 
{
    caltargets.push_back(CalTarget(point, eyefloat, eyegrey));
    updateGPs();
}

// void GazeTracker::updateExemplar(int id, 
// 				 const IplImage *eyefloat, 
// 				 const IplImage *eyegrey)
// {
//     cvConvertScale(eyegrey, caltargets[id].origimage.get());
//     cvAdd(caltargets[id].image.get(), eyefloat, caltargets[id].image.get());
//     cvConvertScale(caltargets[id].image.get(), caltargets[id].image.get(), 0.5);
//     updateGPs();
// }

void GazeTracker::draw(IplImage *destimage, int eyedx, int eyedy) {
//     for(int i=0; i<caltargets.size(); i++) {
// 	Point p = caltargets[i].point;
// 	cvSetImageROI(destimage, cvRect((int)p.x - eyedx, (int)p.y - eyedy, 
// 					2*eyedx, 2*eyedy));
// 	cvCvtColor(caltargets[i].origimage, destimage, CV_GRAY2RGB);
// 	cvRectangle(destimage, cvPoint(0,0), cvPoint(2*eyedx-1,2*eyedy-1),
// 		    CV_RGB(255,0,255));
//     }
//     cvResetImageROI(destimage);
}

void GazeTracker::save(void) {
    CvFileStorage *out = 
	cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_WRITE);
    save(out, "GazeTracker");
    cvReleaseFileStorage(&out);
}

void GazeTracker::save(CvFileStorage *out, const char *name) {
    cvStartWriteStruct(out, name, CV_NODE_MAP);
    savevector(out, "caltargets", caltargets);
    cvEndWriteStruct(out);
}


void GazeTracker::load(void) {
    CvFileStorage *in = 
	cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_READ);
    CvFileNode *root = cvGetRootFileNode(in);
    load(in, cvGetFileNodeByName(in, root, "GazeTracker"));
    cvReleaseFileStorage(&in);
    updateGPs();
}

void GazeTracker::load(CvFileStorage *in, CvFileNode *node) {
    caltargets = loadvector<CalTarget>(in, cvGetFileNodeByName(in, node, 
							       "caltargets"));
}

static void ignore(const IplImage *) {}

void GazeTracker::update(const IplImage *image) {
    if (isActive()) {
	output.gazepoint = Point(gpx->getmean(SharedImage(image, &ignore)), 
				 gpy->getmean(SharedImage(image, &ignore)));
	output.targetid = getTargetId(output.gazepoint);
	output.target = getTarget(output.targetid);
    }
}

int GazeTracker::getTargetId(Point point) {
    return targets->getCurrentTarget(point);
}

Point GazeTracker::getTarget(int id) {
    return targets->targets[id];
}


