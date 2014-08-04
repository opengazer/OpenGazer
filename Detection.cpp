#include "Detection.h"
#include "utils.h"

namespace Detection {
	CvPoint2D32f *detectCornersInGrayscale(IplImage *eyeRegionImageGray, int &cornerCount) {
		IplImage *eigImage = 0;
		IplImage *tempImage = 0;

		eigImage = cvCreateImage(cvSize(eyeRegionImageGray->width, eyeRegionImageGray->height), IPL_DEPTH_32F, 1);
		tempImage = cvCreateImage(cvSize(eyeRegionImageGray->width, eyeRegionImageGray->height), IPL_DEPTH_32F, 1);

		CvPoint2D32f *corners = new CvPoint2D32f[cornerCount];
		double qualityLevel = 0.01;
		double minDistance = 2;
		int eigBlockSize = 3;
		int useHarris = false;

		cvGoodFeaturesToTrack(eyeRegionImageGray, eigImage /* output */, tempImage, corners, &cornerCount, qualityLevel, minDistance, NULL, eigBlockSize, useHarris);

		return corners;
	}

	bool detectNose(IplImage *image, double resolution, CvRect noseRect, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		CvRect *nose = new CvRect();
		IplImage *noseRegionImage;

		nose->width = 0;
		nose->height = 0;
		nose->x = 0;
		nose->y = 0;

		CvSize noseSize;

		if (resolution == 720) {
			noseSize = cvSize(36, 30);
		} else if (resolution == 1080) {
			noseSize = cvSize(54, 45);
		} else if (resolution == 480) {
			noseSize = cvSize(24, 20);
		}

		noseRegionImage = cvCreateImage(cvSize(noseRect.width, noseRect.height), image->depth, image->nChannels);
		cvSetImageROI(image, noseRect);
		cvCopy(image, noseRegionImage);
		cvResetImageROI(image);
		//cvSaveImage("nose.png", noseRegionImage);


		// Load the face detector and create empty memory storage
		char *file = "DetectorNose2.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		if (cascade == NULL) {
			std::cout << "CASCADE NOT LOADED" << std::endl;
		} else {
			std::cout << "Loaded" << std::endl;
		}

		// Detect objects
		CvSeq *noseSeq = cvHaarDetectObjects(noseRegionImage, cascade, storage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, noseSize);

		std::cout << noseSeq->total << " NOSES DETECTED" << std::endl;

		// Return the first nose if any eye is detected
		if (noseSeq && noseSeq->total > 0) {
			// If there are multiple matches, choose the one with larger area
			for (int i = 0; i < noseSeq->total; i++) {
				CvRect *dummy = (CvRect *)cvGetSeqElem(noseSeq, i+1);

				if ((dummy->width * dummy->height) > (nose->width*nose->height)) {
					nose = dummy;
				}
			}
		} else {
			return false;
		}

		//cvRectangle(image, cvPoint(nose->x, nose->y), cvPoint(nose->x + nose->width, nose->y + nose->height), CV_RGB(0, 255, 0), 2, 8, 0);

		points[0] = Point(noseRect.x + nose->x + nose->width * 0.33, noseRect.y + nose->y + nose->height * 0.6);
		points[1] = Point(noseRect.x + nose->x + nose->width * 0.67, noseRect.y + nose->y + nose->height * 0.6);

		return true;
	}

	bool detectMouth(IplImage *image, double resolution, CvRect mouthRect, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		CvRect *mouth = new CvRect();
		IplImage *mouthRegionImage;

		mouth->width = 0;
		mouth->height = 0;
		mouth->x = 0;
		mouth->y = 0;

		CvSize mouthSize;

		if (resolution == 720) {
			mouthSize = cvSize(50, 30);
		} else if (resolution == 1080) {
			mouthSize = cvSize(74, 45);
		} else if (resolution == 480) {
			mouthSize = cvSize(25, 15);
		}

		mouthRegionImage = cvCreateImage(cvSize(mouthRect.width, mouthRect.height), image->depth, image->nChannels);
		cvSetImageROI(image, mouthRect);
		cvCopy(image, mouthRegionImage);
		cvResetImageROI(image);
		//cvSaveImage("mouth.png", mouthRegionImage);

		// Load the face detector and create empty memory storage
		char *file = "DetectorMouth.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		if (cascade == NULL) {
			std::cout << "CASCADE NOT LOADED" << std::endl;
		} else {
			std::cout << "Loaded" << std::endl;
		}

		// Detect objects
		CvSeq *mouthSeq = cvHaarDetectObjects(mouthRegionImage, cascade, storage, 1.1, 3, 0 /* CV_HAAR_DO_CANNY_PRUNING */, mouthSize);

		std::cout << mouthSeq->total << " MOUTHS DETECTED" << std::endl;

		// Return the first mouth if any eye is detected
		if (mouthSeq && mouthSeq->total > 0) {
			// If there are multiple matches, choose the one with larger area
			for (int i = 0; i < mouthSeq->total; i++) {
				CvRect *dummy = (CvRect *)cvGetSeqElem(mouthSeq, i+1);

				if ((dummy->width * dummy->height) > (mouth->width * mouth->height)) {
					mouth = dummy;
				}
			}
		} else {
			return false;
		}

		//cvRectangle( image, cvPoint(mouth->x, mouth->y), cvPoint(mouth->x + mouth->width, mouth->y + mouth->height), CV_RGB(0, 255, 0), 2, 8, 0 );

		points[0] = Point(mouthRect.x + mouth->x + mouth->width * 0.1, mouthRect.y + mouth->y + mouth->height * 0.4);
		points[1] = Point(mouthRect.x + mouth->x + mouth->width * 0.9, mouthRect.y + mouth->y + mouth->height * 0.4);

		return true;
	}

	void detectEyeCorners(IplImage *image, double resolution, Point points[]) {
		CvHaarClassifierCascade *cascade = 0;
		CvMemStorage *storage = 0;
		IplImage *eyeRegionImage;
		IplImage *eyeRegionImageGray;
		//IplImage *dummy;
		CvRect *bothEyes;
		//CvRect *rightEye;
		//CvRect *leftEye;
		CvSize bothEyesSize;
		CvSize singleEyeSize;

		if (resolution == 720) {
			bothEyesSize = cvSize(100, 25);
			singleEyeSize = cvSize(18, 12);
		} else if (resolution == 1080) {
			bothEyesSize = cvSize(150, 38);
			singleEyeSize = cvSize(27, 18);
		} else if (resolution == 480) {
			bothEyesSize = cvSize(64, 16);
			singleEyeSize = cvSize(6, 4);
		}

		// Load the face detector and create empty memory storage
		char *file = "DetectorEyes.xml";
		cascade = (CvHaarClassifierCascade *)cvLoad(file, 0, 0, 0);
		storage = cvCreateMemStorage(0);

		// Detect objects
		CvSeq *eyeRegions = cvHaarDetectObjects(image, cascade, storage, 1.1, 10, CV_HAAR_DO_CANNY_PRUNING, bothEyesSize);

		std::cout << eyeRegions->total << " eye regions detected" << std::endl;

		// Return the first eye if any eye is detected
		if (eyeRegions && eyeRegions->total > 0) {
			bothEyes = (CvRect *)cvGetSeqElem(eyeRegions, 1);
		} else {
			return;
		}

		std::cout << "Resolution: " << resolution << ", both eye reg.:" << bothEyes->width << ", " << bothEyes->height << std::endl;

		//cvRectangle(image, cvPoint(bothEyes->x, bothEyes->y), cvPoint(bothEyes->x + bothEyes->width, bothEyes->y + bothEyes->height), CV_RGB(0, 255, 0), 2, 8, 0);

		int cornerCount = 100;
		eyeRegionImage = cvCreateImage(cvSize(bothEyes->width, bothEyes->height), image->depth, image->nChannels);
		eyeRegionImageGray = cvCreateImage(cvSize(bothEyes->width, bothEyes->height), 8, 1);

		CvRect leftRect = cvRect(bothEyes->x, bothEyes->y, bothEyes->width, bothEyes->height);
		cvSetImageROI(image, leftRect);
		cvCopy(image, eyeRegionImage);
		cvResetImageROI(image);

		cvCvtColor(eyeRegionImage, eyeRegionImageGray, CV_RGB2GRAY);

		Utils::normalizeGrayScaleImage(eyeRegionImageGray);

		CvPoint2D32f *corners = detectCornersInGrayscale(eyeRegionImageGray, cornerCount);

		int leftEyeCornersXSum = 0;
		int leftEyeCornersYSum = 0;
		int leftEyeCornersCount = 0;

		int rightEyeCornersXSum = 0;
		int rightEyeCornersYSum = 0;
		int rightEyeCornersCount = 0;

		/// Drawing a circle around corners
		for (int j = 0; j < cornerCount; j++ ) {
			if (corners[j].x < bothEyes->width * 0.4) {
				leftEyeCornersXSum += corners[j].x;
				leftEyeCornersYSum += corners[j].y;
				leftEyeCornersCount++;
				cvCircle(eyeRegionImage, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
			} else if (corners[j].x > bothEyes->width * 0.6) {
				rightEyeCornersXSum += corners[j].x;
				rightEyeCornersYSum += corners[j].y;
				rightEyeCornersCount++;
				cvCircle(eyeRegionImage, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
			}
		}

		double leftEyeCenterX = bothEyes->x + (leftEyeCornersXSum / (double)leftEyeCornersCount);
		double leftEyeCenterY = bothEyes->y + (leftEyeCornersYSum / (double)leftEyeCornersCount);

		double rightEyeCenterX = bothEyes->x + (rightEyeCornersXSum / (double)rightEyeCornersCount);
		double rightEyeCenterY = bothEyes->y + (rightEyeCornersYSum / (double)rightEyeCornersCount);

		double xDiff = rightEyeCenterX - leftEyeCenterX;
		double yDiff = rightEyeCenterY - leftEyeCenterY;

		//points[0] = Point(bothEyes->x + leftEyeCornersXSum, bothEyes->y + leftEyeCornersYSum);
		//points[1] = Point(bothEyes->x + rightEyeCornersXSum, bothEyes->y + rightEyeCornersYSum);
		points[0] = Point(leftEyeCenterX - 0.29 * xDiff, leftEyeCenterY - 0.29 * yDiff);// + xDiff/40);
		points[1] = Point(rightEyeCenterX + 0.29 * xDiff, rightEyeCenterY + 0.29 * yDiff);// + xDiff/40);

		/// Drawing a circle around corners
		//for (int i = 0; i < cornerCount; i++) {
		//	cvCircle(eyeRegionImage, cvPoint(corners[i].x, corners[i].y), 3, CV_RGB(255,0,0), -1, 8, 0);
		//}

		cvCircle(eyeRegionImage, cvPoint(points[0].x, points[0].y), 3, CV_RGB(0,255,0), -1, 8, 0);
		cvCircle(eyeRegionImage, cvPoint(points[1].x, points[1].y), 3, CV_RGB(0,255,0), -1, 8, 0);

		//cvSaveImage("eye_corners.png", eyeRegionImage);
		//cvSaveImage((_basePath.substr(0, _basePath.length() - 4) + "_eye_corners.png").c_str(), eyeRegionImage);
	}

	void detectEyebrowCorners(IplImage *image, double resolution, CvRect eyebrowRect, Point points[]) {
		IplImage *eyebrowRegionImage;
		IplImage *eyebrowRegionImageGray;
		IplImage *eyebrowRegionImage2;
		IplImage *eyebrowRegionImageGray2;

		eyebrowRect.width = eyebrowRect.width / 2;
		eyebrowRegionImage = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), image->depth, image->nChannels);
		eyebrowRegionImageGray = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), 8, 1);
		eyebrowRegionImage2 = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), image->depth, image->nChannels);
		eyebrowRegionImageGray2 = cvCreateImage(cvSize(eyebrowRect.width, eyebrowRect.height), 8, 1);

		std::cout << "EYEBROW x, y = " << eyebrowRect.x << " - " << eyebrowRect.y << " width, height =" << eyebrowRect.width << " - " << eyebrowRect.height << std::endl;

		cvSetImageROI(image, eyebrowRect);
		cvCopy(image, eyebrowRegionImage);

		std::cout << "Copied first" << std::endl;

		CvRect eyebrowRect2 = cvRect(eyebrowRect.x + eyebrowRect.width, eyebrowRect.y, eyebrowRect.width, eyebrowRect.height);
		cvSetImageROI(image, eyebrowRect2);
		cvCopy(image, eyebrowRegionImage2);
		cvResetImageROI(image);

		//cvSaveImage("eyebrows.png", eyebrowRegionImage);

		cvCvtColor(eyebrowRegionImage, eyebrowRegionImageGray, CV_RGB2GRAY);
		cvCvtColor(eyebrowRegionImage2, eyebrowRegionImageGray2, CV_RGB2GRAY);

		int cornerCount = 1;
		CvPoint2D32f *corners = detectCornersInGrayscale(eyebrowRegionImageGray, cornerCount);

		cornerCount = 1;
		CvPoint2D32f *corners2 = detectCornersInGrayscale(eyebrowRegionImageGray2, cornerCount);

		points[0] = Point(eyebrowRect.x + corners[0].x, eyebrowRect.y + corners[0].y);
		points[1] = Point(eyebrowRect2.x + corners2[0].x, eyebrowRect2.y + corners2[0].y);

		std::cout << "Finished eyebrows" << std::endl;
	}
}
