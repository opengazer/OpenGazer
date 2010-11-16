#pragma once
#include "utils.h"
#include "GaussianProcess.cpp"

typedef MeanAdjustedGaussianProcess<SharedImage> ImProcess;

struct Targets {
    vector<Point> targets;

    Targets(void) {};
    Targets(vector<Point> const& targets): targets(targets) {}
    int getCurrentTarget(Point point);
};

struct CalTarget {
    Point point;
    SharedImage image, origimage;

    CalTarget();
    CalTarget(Point point, const IplImage* image, const IplImage* origimage);

    void save(CvFileStorage* out, const char* name=NULL);
    void load(CvFileStorage* in, CvFileNode *node);
};

struct TrackerOutput {
    Point gazepoint;
    Point target;
    int targetid;

    TrackerOutput(Point gazepoint, Point target, int targetid);
};

class GazeTracker {
    scoped_ptr<ImProcess> gpx, gpy;
    vector<CalTarget> caltargets;
    scoped_ptr<Targets> targets;
    
    static double imagedistance(const IplImage *im1, const IplImage *im2);
    static double covariancefunction(const SharedImage& im1, 
				     const SharedImage& im2);

    void updateGPs(void);

public:
    TrackerOutput output;

    GazeTracker(): targets(new Targets), 
	output(Point(0,0), Point(0,0), -1) {}

    bool isActive() { return gpx.get() && gpy.get(); }

    void clear();
    void addExemplar(Point point, 
		     const IplImage *eyefloat, const IplImage *eyegrey);
    void draw(IplImage *canvas, int eyedx, int eyedy);
    void save(void);
    void save(CvFileStorage *out, const char *name);
    void load(void);
    void load(CvFileStorage *in, CvFileNode *node);
    void update(const IplImage *image);
    int getTargetId(Point point);
    Point getTarget(int id);
};
