#include "FaceDetector.h"
#include "utils.h"

FaceDetector FaceDetector::faceDetector;

FaceDetector::FaceDetector(char *cascadeName)
{
	_cascade.load(cascadeName);
}


FaceDetector::~FaceDetector() {
	// fixme: release the cascade somehow
}

cv::Rect FaceDetector::detect(const cv::Mat img) {
	cv::Rect largestObject(0, 0, 0, 0);

	try {
		double scale = 1.3;
		cv::Mat grayImage;
		cv::Mat smallImage;
		std::vector<cv::Rect> results;

		Utils::convertAndResize(img, grayImage, smallImage, scale);

		_cascade.detectMultiScale(grayImage, results, 1.1, 2, 0, cv::Size(30, 30));

		for (int i = 0; i < results.size(); i++) {
			cv::Rect temp = results[0];

			if((temp.width * temp.height) > (largestObject.width*largestObject.height)) {
				largestObject = temp;
			}
		}
	}
	catch (std::exception &ex) {
		std::cout << ex.what() << std::endl;
		return largestObject;
	}

	return largestObject;
}

