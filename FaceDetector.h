#pragma once

class FaceDetector {
public:
	static FaceDetector faceDetector;

	FaceDetector(char *cascadeName="haarcascade_frontalface_alt.xml");
	~FaceDetector();
	cv::Rect detect(const cv::Mat img);

private:
	cv::CascadeClassifier _cascade;
};


