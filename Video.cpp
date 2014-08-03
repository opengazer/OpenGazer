#include <sys/time.h>

#include "Video.h"

VideoInput::VideoInput():
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(0),
	_capture(cvCaptureFromCAM(0))
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	frame = cvQueryFrame(_capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution):
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(resolution),
	_capture(cvCaptureFromCAM(0))
{
	if (resolution.compare("720") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 1280);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 720);
	} else if (resolution.compare("1080") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 1920);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 1080);
	} else if (resolution.compare("480") == 0) {
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	//cvSetCaptureProperty(_capture, CV_CAP_PROP_FOURCC, CV_FOURCC('M','J','P','G'));

	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	frame = cvQueryFrame(_capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution, std::string filename, bool dummy):
	frameCount(0),
	frame(cvQueryFrame(_capture)),
	size(cvSize(frame->width, frame->height)),
	captureFromVideo(true),
	resolutionParameter(resolution),
	_capture(cvCaptureFromFile(filename.c_str()))
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);


	double videoResolution = atoi(resolution.c_str());
	double trackerResolution = frame->height;

	// In case the video is 1280/720 and we want to execute 480 (or 1280 -> 720)
	if (videoResolution != trackerResolution) {
		IplImage *tempImage = cvCreateImage(cvSize(640, 480), 8, 3);

		if (videoResolution == 720 && trackerResolution == 480) {
			cvSetImageROI(frame, cvRect(160, 0, 960, 720));		// Set ROI
		} else if (videoResolution == 1080 && trackerResolution == 480) {
			cvSetImageROI(frame, cvRect(240, 0, 1440, 1080));	// Set ROI
		}

		//std::cout << "FRAME: " << frame->height << "x" << frame->width << " " << frame->depth << std::std::endl;
		//std::cout << "TEMP: " << tempImage->height << "x" << tempImage->width << " " << tempImage->depth << std::endl;
		cvResize(frame, tempImage);
		frame = tempImage;
		cvResetImageROI(frame);
		size.width = frame->width;
		size.height = frame->height;


		std::cout << "Successfully resized first frame" << std::endl;
	}
}

VideoInput::~VideoInput() {
	cvReleaseCapture(&_capture);
}

void VideoInput::updateFrame() {
	static double videoResolution = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
	static double trackerResolution = frame->height;
	frameCount++;

	if (!(captureFromVideo && frameCount == 1)) {
		// If capturing from video and video size is not equal to desired resolution, carry on with resizing
		if (captureFromVideo && videoResolution != trackerResolution) {
			IplImage *tempImage = cvQueryFrame(_capture);

			if (videoResolution == 720 && trackerResolution == 480) {
				cvSetImageROI(tempImage, cvRect(160, 0, 960, 720));		// Set ROI
			} else if (videoResolution == 1080 && trackerResolution == 480) {
				cvSetImageROI(tempImage, cvRect(240, 0, 1440, 1080));	// Set ROI
			}

			cvResize(tempImage, frame, CV_INTER_CUBIC);		// Resize image to 640x480
		} else {
			frame = cvQueryFrame(_capture);
		}
	}

	if (!captureFromVideo) {
		cvFlip(frame, frame, 1);
	}
}

// Returns 480 or 720 depending on camera resolution
double VideoInput::getResolution() {
	double value = atof(resolutionParameter.c_str());
	return value > 0 ? value : cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
}

VideoWriter::VideoWriter(CvSize size, std::string filename):
	_video(cvCreateVideoWriter(filename.c_str(), 0x58564944, 15.0, size, 1))
{
}

VideoWriter::~VideoWriter() {
	cvReleaseVideoWriter(&_video);
}

void VideoWriter::write(const IplImage *image) {
	cvWriteFrame(_video, image);
}

