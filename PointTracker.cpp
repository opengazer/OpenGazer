#include "PointTracker.h"
#include "FaceDetector.h"
#include <opencv/highgui.h>
#include <fstream>


PointTracker::PointTracker(const CvSize &size):
    flags(CV_LKFLOW_INITIAL_GUESSES),
    grey(cvCreateImage(size, 8, 1)),
    orig_grey(cvCreateImage(size, 8, 1)),
    pyramid(cvCreateImage(size, 8, 1)),
    orig_pyramid(cvCreateImage(size, 8, 1)),
    last_grey(cvCreateImage(size, 8, 1)),
    last_pyramid(cvCreateImage(size, 8, 1))
//     origpoints(new CvPoint2D32f[MAX_COUNT]),
//     currentpoints(new CvPoint2D32f[MAX_COUNT]),
//     status(new char[MAX_COUNT]),
{}

static Point pointbetweenrects(const Point &point, CvRect source, CvRect dest) {
    return Point((point.x-source.x)*(double(dest.width)/source.width)+dest.x,
		 (point.y-source.y)*(double(dest.height)/source.height)+dest.y);

}

static vector<Point> pointbetweenrects(const vector<Point> &points,
				       CvRect source, CvRect dest)
{
    vector<Point> result;
    result.reserve(points.size());
    xforeach(iter, points)
	result.push_back(pointbetweenrects(*iter, source, dest));
    return result;
}

void PointTracker::save(string filename, string newpoints, 
			const IplImage *frame) 
{
    vector<CvRect> faces = FaceDetector::facedetector.detect(frame);
    if (faces.size() == 1) {
	cvSaveImage((filename + "-orig-grey.png").c_str(), orig_grey.get());
	cvSaveImage((filename + "-orig-pyramid.png").c_str(), orig_pyramid.get());
    
	ofstream origfile((filename + "-orig-points.txt").c_str());
	origfile << origpoints;

	CvRect face = faces[0];
	ofstream facefile(newpoints.c_str());
	vector<Point> temppoints;
	convert(currentpoints, temppoints);
	facefile << pointbetweenrects(temppoints, face, cvRect(0, 0, 1, 1));
    }
    else 
	throw ios_base::failure("No face found in the image");
}


void PointTracker::save_image() 
{
	cvSaveImage("point-selection-frame.png", orig_grey.get());
}


void PointTracker::load(string filename, string newpoints, 
			const IplImage *frame) 
{
    vector<CvRect> faces = FaceDetector::facedetector.detect(frame);

    if (faces.size() == 1) {
	ifstream origfile((filename + "-orig-points.txt").c_str());
	ifstream facefile(newpoints.c_str());
	if (!origfile.is_open() || !facefile.is_open())
	    throw ios_base::failure("File not found");

	// todo: memory leak here, change to scoped_ptr!
	orig_grey.reset(cvLoadImage((filename + "-orig-grey.png").c_str(), 0));
	orig_pyramid.reset(cvLoadImage((filename + "-orig-pyramid.png").c_str(), 0));
    
	vector<Point> temppoints;
	origfile >> temppoints;
	convert(temppoints, origpoints);

	facefile >> temppoints;
	temppoints = pointbetweenrects(temppoints, cvRect(0,0,1,1), faces[0]);
	convert(temppoints, currentpoints);
	lastpoints = currentpoints;
    }
    else
	throw ios_base::failure("No face found in the image");
}

int PointTracker::getClosestTracker(const Point &point) {
    vector<Point> points;
    convert(currentpoints, points);
    return point.closestPoint(points);
}

void PointTracker::removetracker(int id) {
    currentpoints.erase(currentpoints.begin()+id);
    lastpoints.erase(lastpoints.begin()+id);
    origpoints.erase(origpoints.begin()+id);
}

void PointTracker::synchronizepoints() {
    swap(orig_grey, grey);
    swap(orig_pyramid, pyramid);
    origpoints = lastpoints = currentpoints;
}

void PointTracker::updatetracker(int id, const Point &point) {
    currentpoints[id] = point.cvpoint32();
    synchronizepoints();
}

void PointTracker::addtracker(const Point &point) {
    currentpoints.push_back(point.cvpoint32());
    synchronizepoints();
}

void PointTracker::cleartrackers() {
    currentpoints.clear();
    synchronizepoints();
}

void PointTracker::normalizeOriginalGrey() {
    cvSetImageROI(orig_grey.get(), *face_rectangle);
	normalizeGrayScaleImage2(orig_grey.get(), 90, 160);
	cvResetImageROI(orig_grey.get());
}
void PointTracker::track(const IplImage *frame, int pyramiddepth) 
{
	try {
	    assert(lastpoints.size() == currentpoints.size());
	    assert(origpoints.size() == currentpoints.size());
	    status.resize(currentpoints.size());
	    cvCvtColor(frame, grey.get(), CV_BGR2GRAY );

		if(face_rectangle != NULL) {
		    cvSetImageROI(grey.get(), *face_rectangle);
			normalizeGrayScaleImage2(grey.get(), 90, 160);
			cvResetImageROI(grey.get());
		}

		// Apply median filter of 5x5
	    cvSmooth(grey.get(), grey.get(), CV_MEDIAN, 5);
	
	    if (!currentpoints.empty()) {

			// then calculate the position based on the original
			// template without any pyramids
			cvCalcOpticalFlowPyrLK(orig_grey.get(), grey.get(), 
					       orig_pyramid.get(), pyramid.get(),
					       &origpoints[0], &currentpoints[0], pointcount(), 
					       cvSize(win_size, win_size), 
					       pyramiddepth*3, &status[0], 0,
					       cvTermCriteria(CV_TERMCRIT_EPS,20,0.03), 
					       flags);

		//	}

			flags |= CV_LKFLOW_PYR_A_READY;
	    }

	    cvCopy(grey.get(), last_grey.get(), 0);
	    cvCopy(pyramid.get(), last_pyramid.get(), 0);
	    lastpoints = currentpoints;
	}
    catch (std::exception &ex) {
		cout << ex.what() << endl;
		cleartrackers();
    }
}

void PointTracker::retrack(const IplImage *frame, int pyramiddepth) 
{
	try {
		currentpoints = origpoints;
		
		cout << "RETRACKING" << endl;
		for(int i=0; i< (int) currentpoints.size(); i++) {
			cout << "CP["<< i <<"]" << currentpoints[i].x << ", " << currentpoints[i].y << endl;
		}
			
		flags = 0;
	    cvCvtColor(frame, grey.get(), CV_BGR2GRAY );
	
		// Apply median filter of 5x5
	    cvSmooth(grey.get(), grey.get(), CV_MEDIAN, 5);
			// then calculate the position based on the original
			// template without any pyramids
			cvCalcOpticalFlowPyrLK(orig_grey.get(), grey.get(), 
					       orig_pyramid.get(), pyramid.get(),
					       &origpoints[0], &currentpoints[0], pointcount(), 
					       cvSize(win_size, win_size), 
					       pyramiddepth*3, &status[0], 0,
					       cvTermCriteria(CV_TERMCRIT_EPS,200,0.0001), 
					       flags);

		//	}

            flags = CV_LKFLOW_INITIAL_GUESSES;
			flags |= CV_LKFLOW_PYR_A_READY;
	    
	    cvCopy(grey.get(), last_grey.get(), 0);
	    cvCopy(pyramid.get(), last_pyramid.get(), 0);
	    lastpoints = currentpoints;
	
			cout << endl << "AFTER RETRACKING" << endl;
			for(int i=0; i< (int) currentpoints.size(); i++) {
				cout << "CP["<< i <<"]" << currentpoints[i].x << ", " << currentpoints[i].y << endl;
			}
	}
    catch (std::exception &ex) {
		cout << ex.what() << endl;
		cleartrackers();
    }
}
int PointTracker::countactivepoints(void) {
    return count_if(status.begin(), status.end(), 
		    bind1st(not_equal_to<char>(), 0));
}

bool PointTracker::areallpointsactive(void) {
    return count(status.begin(), status.end(), 0) == 0;
}

void PointTracker::draw(IplImage *canvas) {
	try{
    for(int i=0; i< (int) currentpoints.size(); i++)
	cvCircle( canvas, cvPointFrom32f(currentpoints[i]), 3, 
		  status[i]?(i == eyepoint1 || i == eyepoint2 ? 
			     CV_RGB(255,0,0):
			     CV_RGB(0,255,0)):
		  CV_RGB(0,0,255), 
		  -1, 8,0);
		}
	    catch (std::exception &ex) {
			cout << ex.what() << endl;
			cleartrackers();
	    }
}

int PointTracker::pointcount() {
    return currentpoints.size();
}

vector<Point> 
PointTracker::getpoints(const vector<CvPoint2D32f> PointTracker::*points, 
			bool allpoints) 
{
    vector<Point> vec;
    for(int i=0; i<pointcount(); i++)
	if (allpoints || status[i])
	    vec.push_back(Point((this->*points)[i].x, 
				   (this->*points)[i].y));
    return vec;
}
