#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "FaceDetector.h"

FaceDetector FaceDetector::faceDetector;

FaceDetector::FaceDetector(char *cascadeName):
	_cascade((CvHaarClassifierCascade *)cvLoad(cascadeName, 0, 0, 0)),
	_storage(cvCreateMemStorage(0))
{
}


FaceDetector::~FaceDetector() {
	cvReleaseMemStorage(&_storage);
	// fixme: release the cascade somehow
}

vector<CvRect> FaceDetector::detect(const IplImage *img) {
	vector<CvRect> result;

	try {
		double scale = 1.3;
		IplImage *grayImage = cvCreateImage(cvSize(img->width, img->height), 8, 1 );
		IplImage *smallImage = cvCreateImage(cvSize(cvRound(img->width / scale), cvRound(img->height / scale)), 8, 1 );

		cvCvtColor(img, grayImage, CV_BGR2GRAY);
		cvResize(grayImage, smallImage, CV_INTER_LINEAR);
		cvEqualizeHist(smallImage, smallImage);
		cvClearMemStorage(_storage);

		CvSeq *faces = cvHaarDetectObjects(smallImage, _cascade, _storage, 1.1, 2, 0 /*CV_HAAR_DO_CANNY_PRUNING*/, cvSize(30, 30));

		for (int i = 0; i < (faces ? faces->total : 0); i++) {
			CvRect *rect = (CvRect *)cvGetSeqElem(faces, i);
			result.push_back(cvRect((int)(rect->x * scale), (int)(rect->y * scale), (int)(rect->width * scale), (int)(rect->height * scale)));
		}

		cvReleaseImage(&grayImage);
		cvReleaseImage(&smallImage);
		//cvReleaseSeq(&faces);
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		return result;
	}

	return result;
}

vector<CvRect> FaceDetector::detectInGrayscale(const IplImage *grayImage) {
	vector<CvRect> result;

	try {
		double scale = 1.3;
		IplImage *smallImage = cvCreateImage(cvSize(cvRound(grayImage->width / scale), cvRound(grayImage->height / scale)), 8, 1);

		cvResize(grayImage, smallImage, CV_INTER_LINEAR);
		cvEqualizeHist(smallImage, smallImage);
		cvClearMemStorage(_storage);

		CvSeq *faces = cvHaarDetectObjects(smallImage, _cascade, _storage, 1.1, 2, 0 /*CV_HAAR_DO_CANNY_PRUNING*/, cvSize(30, 30));

		for (int i = 0; i < (faces ? faces->total : 0); i++) {
			CvRect *rect = (CvRect *)cvGetSeqElem(faces, i);
			result.push_back(cvRect((int)(rect->x * scale), (int)(rect->y * scale), (int)(rect->width * scale), (int)(rect->height * scale)));
		}

		cvReleaseImage(&smallImage);
		//cvReleaseSeq(&faces);
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		return result;
	}

	return result;
}

