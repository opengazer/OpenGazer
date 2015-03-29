#pragma once

#include "Point.h"

namespace Detection {
	void loadCascades();
	bool detectLargestObject(cv::CascadeClassifier cascade, cv::Mat image, cv::Rect &largestObject, double scaleFactor = 1.1, int minNeighbors = 3, int flags = 0, cv::Size minSize = cv::Size());
	bool detectNose(cv::Mat image, double resolution, cv::Rect noseRect, Point points[]);
	bool detectMouth(cv::Mat image, double resolution, cv::Rect mouthRect, Point points[]);
	void detectEyeCorners(cv::Mat image, double resolution, Point points[]);
	void detectEyebrowCorners(cv::Mat image, double resolution, cv::Rect eyebrowRect, Point points[]);
}
