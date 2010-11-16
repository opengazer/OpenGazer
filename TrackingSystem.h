#pragma once
#include "utils.h"
#include "PointTracker.h"
#include "HeadTracker.h"
#include "HeadCompensation.cpp"
#include "EyeExtractor.h"
#include "OutputMethods.h"
#include "GazeTracker.h"

struct TrackingSystem {
    PointTracker tracker;
    HeadTracker headtracker;
    HeadCompensation headcomp;
    EyeExtractor eyex;
    GazeTracker gazetracker;

    TrackingSystem(CvSize size);
    void doprocessing(const IplImage *frame, 
		      IplImage *image);
    void displayeye(IplImage *image, int basex, int basey, 
		    int stepx, int stepy);

};
