#pragma once

#include <opencv/cv.h>
#include <vector>
#include <vgl/vgl_homg_point_2d.h>
#include "utils.h"

using namespace std;
typedef vgl_vector_2d<double> HomPoint;

class TrackingException: public std::exception {};

class PointTracker {
public:
    static const int eyepoint1 = 0;
    static const int eyepoint2 = 1;
    vector<char> status;
    vector<CvPoint2D32f> origpoints, currentpoints, lastpoints;

 private:
    static const int win_size = 11;
    int flags;

    scoped_ptr<IplImage> grey, orig_grey, pyramid, orig_pyramid,
	last_grey, last_pyramid;

    void synchronizepoints();

public:
    PointTracker(const CvSize &size);
    void cleartrackers();
    void addtracker(const Point &point);
    void updatetracker(int id, const Point &point);
    void removetracker(int id);
    int getClosestTracker(const Point &point);
    void track(const IplImage *frame, int pyramiddepth=1);
	void retrack(const IplImage *frame, int pyramiddepth=1);
    int countactivepoints(void);
    bool areallpointsactive(void);
    int pointcount();
    void draw(IplImage *canvas);
    void normalizeOriginalGrey();

    vector<HomPoint> 
	getpoints(const vector<CvPoint2D32f> PointTracker::*points, 
		  bool allpoints=true);

    void save(string filename, string newname, const IplImage *frame);
    void load(string filename, string newname, const IplImage *frame);
    void save_image();
};
