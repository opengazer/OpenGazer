#include "FaceDetector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

FaceDetector FaceDetector::facedetector;

FaceDetector::FaceDetector(char *cascadename):
    cascade((CvHaarClassifierCascade*)cvLoad(cascadename, 0, 0, 0)),
    storage(cvCreateMemStorage(0))
{}


FaceDetector::~FaceDetector() {
    cvReleaseMemStorage(&storage);
    // fixme: release the cascade somehow
}

vector<CvRect> FaceDetector::detect(const IplImage *img) {
    double scale = 1.3;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = 
	cvCreateImage( cvSize( cvRound (img->width/scale),
			       cvRound (img->height/scale)), 8, 1 );
    int i;

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

    CvSeq* faces = 
	cvHaarDetectObjects( small_img, cascade, storage,
			     1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
			     cvSize(30, 30) );

    vector<CvRect> result;
    for( i = 0; i < (faces ? faces->total : 0); i++ ) {
	CvRect *rect = (CvRect*)cvGetSeqElem( faces, i );
	result.push_back(cvRect((int)(rect->x * scale),
				(int)(rect->y * scale),
				(int)(rect->width * scale),
				(int)(rect->height * scale)));
    }

    cvReleaseImage(&gray);
    cvReleaseImage(&small_img);
//     cvReleaseSeq(&faces);

    return result;
}

