#pragma once

#include "Point.h"

namespace Detection {
	bool detectNose(IplImage *image, double resolution, CvRect noseRect, Point points[]);
	bool detectMouth(IplImage *image, double resolution, CvRect mouthRect, Point points[]);
	void detectEyeCorners(IplImage *image, double resolution, Point points[]);
	void detectEyebrowCorners(IplImage *image, double resolution, CvRect eyebrowRect, Point points[]);
}
