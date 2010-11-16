#pragma once
#include <vgl/vgl_homg_point_2d.h>
#include <opencv/cv.h>
#include <vector>
#include "PointTracker.h"

using namespace std;

class HeadTracker {
    vector<double> depths;

    vector<bool> detectinliers(vector<HomPoint> const &prev, 
			       vector<HomPoint> const &now,
			       double radius = 30.0);

    void predictpoints(double xx0, double yy0, double xx1, double yy1,
		       double rotx, double roty, double atx, double aty);

 public:
    PointTracker &tracker;

    double rotx, roty, atx, aty;

    void draw(IplImage *image);
    void updatetracker(void);

    HeadTracker(PointTracker &tracker) : tracker(tracker) {}
};
