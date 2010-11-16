#include "LeastSquares.h"

class HeadCompensation {
    LeastSquares xparams, yparams;
    const HeadTracker &head;

    double xa0, xa1, xa2, ya0, ya1, ya2;
    int samples;
public:

    HeadCompensation(HeadTracker const& head): 
	xparams(3), yparams(3), head(head), 
	xa0(0.0), xa1(0.0), xa2(0.0),
	ya0(0.0), ya1(0.0), ya2(0.0), samples(0)
    {}

    void addCorrection(Point correction) {
	xparams.addSample(head.rotx, head.roty, 1.0, correction.x);
	yparams.addSample(head.rotx, head.roty, 1.0, correction.y);
	samples++;
    }

    void updateFactors(void) {
	if (samples > 0) {
	    xparams.solve(xa0, xa1, xa2);
	    yparams.solve(ya0, ya1, ya2);
	}
    }

    Point estimateCorrection(void) {
	return Point(xa0 * head.rotx + xa1 * head.roty + xa2, 
		     ya0 * head.rotx + ya1 * head.roty + ya2);
    }
};
