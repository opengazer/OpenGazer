#pragma once

#include <opencv/highgui.h>

class VideoInput {
private:
	cv::VideoCapture _capture;
	long _lastFrameTime;

public:
	int frameCount;
	cv::Mat frame;
	IplImage *cFrame;
	cv::Size size;
	bool captureFromVideo;
	std::string resolutionParameter;
    double videoResolution;

	VideoInput();
	VideoInput(std::string resolution);
	VideoInput(std::string resolution, std::string filename, bool dummy);
	~VideoInput();
	void updateFrame();
	double getResolution();
};

class VideoWriter {
public:
	VideoWriter(cv::Size, std::string filename);
	~VideoWriter();
	void write(const cv::Mat &image);

private:
	cv::VideoWriter _video;
};

