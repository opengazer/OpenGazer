#include "Calibrator.h"

Calibrator::~Calibrator() {
#ifdef DEBUG
    cout << "Destroying calibrator" << endl;
#endif
}

FrameFunction::~FrameFunction() {
#ifdef DEBUG
    cout << "Destroying framefunction" << endl;
#endif
}

MovingTarget::MovingTarget(const int &frameno, 
			   const vector<Point>& points, 
			   const shared_ptr<WindowPointer> &pointer,
			   int dwelltime):
    FrameFunction(frameno), 
    points(points), dwelltime(dwelltime), pointer(pointer)
{    
};

MovingTarget::~MovingTarget() {
    int id = getFrame() / dwelltime;
}

void MovingTarget::process() {
    if (getPointNo() != points.size() && active()) {
        int id = getPointNo();
        
        if (getPointFrame() == 1) 
            pointer->setPosition((int)points[id].x, (int)points[id].y); 
    }
    else {
        if(getPointNo() == points.size() && tracker_status == STATUS_TESTING) {
            tracker_status = STATUS_CALIBRATED;
        }
        detach();
    }
}

bool MovingTarget::active() {
    if(parent == NULL) 
        return false;
    
    return getPointNo() < (int) points.size();
}

bool MovingTarget::isLast() {
    return getPointNo() == ((int) points.size()) - 1;
}

int MovingTarget::getPointNo() {
    return getFrame() / dwelltime;
}

int MovingTarget::getPointFrame() {
    return getFrame() % dwelltime;
}

int MovingTarget::getDwellTime() {
    return dwelltime;
}

Point MovingTarget::getActivePoint() {
    int id = getPointNo();
    
    return points[id];
}

Calibrator::Calibrator(const int &framecount, 
               const shared_ptr<TrackingSystem> &trackingsystem,
               const vector<Point>& points, 
               const shared_ptr<WindowPointer> &pointer,
               int dwelltime): 
    MovingTarget(framecount, points, pointer, dwelltime),
    trackingsystem(trackingsystem)
{
    trackingsystem->gazetracker.clear();
    // todo: remove all calibration points
}


void Calibrator::process() {
    static int dummy = 0;
    
    if (active()) {
        int id = getPointNo();
        int frame = getPointFrame();
        if (frame == 1) {// start
            averageeye.reset(new FeatureDetector(EyeExtractor::eyesize));
            averageeye_left.reset(new FeatureDetector(EyeExtractor::eyesize));
        }
        if (frame >= 11) { // middle    ONUR dwelltime/2 changed to 11
            if(!trackingsystem->eyex.isBlinking()) {
                averageeye->addSample(trackingsystem->eyex.eyefloat.get());
                averageeye_left->addSample(trackingsystem->eyex.eyefloat_left.get());
                
                // Neural network 
                //if(dummy % 8 == 0) {  // Only add samples on the 11-19-27-35 frames
                //for(int i=0; i<1000; i++) {   // Train 100 times with each frame
                    trackingsystem->gazetracker.
                    addSampleToNN(points[id], trackingsystem->eyex.eyefloat.get(), trackingsystem->eyex.eyegrey.get());
                    trackingsystem->gazetracker.
                    addSampleToNN_left(points[id], trackingsystem->eyex.eyefloat_left.get(), trackingsystem->eyex.eyegrey_left.get());
                    
                    dummy++;
                //}
            }
            else {
                cout << "Skipped adding sample!!!!" << endl;
            }
        }
    
        if (frame == dwelltime-1) { // end
            trackingsystem->gazetracker.
            addExemplar(points[id], averageeye->getMean().get(),
                    trackingsystem->eyex.eyegrey.get());
            // ONUR DUPLICATED CODE
            trackingsystem->gazetracker.
            addExemplar_left(points[id], averageeye_left->getMean().get(),
                    trackingsystem->eyex.eyegrey_left.get());
                
            if(id == points.size()-1) {
                tracker_status = STATUS_CALIBRATED;
                is_tracker_calibrated = true;
                
                //trackingsystem->gazetracker.trainNN();
                //trackingsystem->gazetracker.calculateTrainingErrors();
            }
        
            // If we have processed the last target
            // Calculate training error and output on screen
            //if(isLast()) {
            //  trackingsystem->gazetracker.calculateTrainingErrors();
            //}
        }
    }
    MovingTarget::process();
}

const Point Calibrator::defaultpointarr[] = {Point(0.5, 0.5), 
					     Point(0.1, 0.5), Point(0.9, 0.5),
					     Point(0.5, 0.1), Point(0.5, 0.9), 
					     Point(0.1, 0.1), Point(0.1, 0.9), 
					     Point(0.9, 0.9), Point(0.9, 0.1), 
					     Point(0.3, 0.3), Point(0.3, 0.7), 
					     Point(0.7, 0.7), Point(0.7, 0.3)};

vector<Point> 
Calibrator::defaultpoints(Calibrator::defaultpointarr, 
			  Calibrator::defaultpointarr+
			  (sizeof(Calibrator::defaultpointarr) / 
			   sizeof(Calibrator::defaultpointarr[0])));

vector<Point> Calibrator::loadpoints(istream& in) {
    vector<Point> result;

    for(;;) {
	double x, y;
	in >> x >> y;
	if (in.rdstate()) break; // break if any error
	result.push_back(Point(x, y));
    }

    return result;
}

vector<Point> Calibrator::scaled(const vector<Point> &points,
				      double x, double y) 
{
//     double dx = x > y ? (x-y)/2 : 0.0;
//     double dy = y > x ? (y-x)/2 : 0.0;
//     double scale = x > y ? y : x;

    vector<Point> result;

    xforeach(iter, points)
	result.push_back(Point(iter->x * x, iter->y * y));
// 	result.push_back(Point(iter->x * scale + dx, iter->y * scale + dy));

    return result;
}

vector<Point> Calibrator::scaled(const vector<Point> &points,
                          int x, int y, double width, double height) 
{
    vector<Point> result;

    xforeach(iter, points) {
    result.push_back(Point(iter->x * width + x, iter->y * height + y));
        //cout << "ADDED POINT (" << iter->x * width + x << ", " << iter->y * height + y << ")" << endl;
    }
    return result;
}
