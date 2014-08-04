#pragma once

class FaceDetector {
public:
	static FaceDetector faceDetector;

	FaceDetector(char *cascadeName="haarcascade_frontalface_alt.xml");
	~FaceDetector();
	std::vector<CvRect> detect(const IplImage *img);
	std::vector<CvRect> detectInGrayscale(const IplImage *img);

private:
	CvMemStorage *_storage;
	CvHaarClassifierCascade *_cascade ;
};


