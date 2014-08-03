#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

class VideoInput {
private:
	CvCapture *_capture;
	long _lastFrameTime;

public:
	int frameCount;
	IplImage *frame;
	CvSize size;
	bool captureFromVideo;
	std::string resolutionParameter;

	VideoInput();
	VideoInput(std::string resolution);
	VideoInput(std::string resolution, std::string filename, bool dummy);
	~VideoInput();
	void updateFrame();
	double getResolution();
};

class VideoWriter {
public:
	VideoWriter(CvSize size, std::string filename);
	~VideoWriter();
	void write(const IplImage *image);

private:
	CvVideoWriter *_video;
};

