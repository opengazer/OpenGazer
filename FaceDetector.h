#pragma once

#include <opencv/cv.h>

#include "utils.h"

class FaceDetector {
public:
	static FaceDetector faceDetector;

	FaceDetector(char *cascadeName="haarcascade_frontalface_alt.xml");
	~FaceDetector();
	vector<CvRect> detect(const IplImage *img);
	vector<CvRect> detectInGrayscale(const IplImage *img);

private:
	CvMemStorage *_storage;
	CvHaarClassifierCascade *_cascade ;
};


