#include <sys/time.h>

#include "Video.h"

VideoInput::VideoInput():
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(0),
	_capture(cv::VideoCapture(0))
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	_capture.read(frame);
	size = cv::Size(frame.size().width, frame.size().height);
	flip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution):
	frameCount(0),
	captureFromVideo(false),
	resolutionParameter(resolution),
	_capture(cv::VideoCapture(0))
{
	if (resolution.compare("720") == 0) {
		_capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	} else if (resolution.compare("1080") == 0) {
		_capture.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
		_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	} else if (resolution.compare("480") == 0) {
		_capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	//cvSetCaptureProperty(_capture, CV_CAP_PROP_FOURCC, CV_FOURCC('M','J','P','G'));

	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	_capture.read(frame);
	size = cv::Size(frame.size().width, frame.size().height);
	flip(frame, frame, 1);
}

VideoInput::VideoInput(std::string resolution, std::string filename, bool dummy):
	_capture(cv::VideoCapture(filename.c_str())),
	frameCount(0),
	captureFromVideo(true),
	resolutionParameter(resolution)
{
	timeval time;
	gettimeofday(&time, NULL);
	_lastFrameTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	_capture.read(frame);

	size = cv::Size(frame.size().width, frame.size().height);

	videoResolution = frame.size().height;
	double trackerResolution = atoi(resolution.c_str());

	// In case the video is 1280/720 and we want to execute 480 (or 1280 -> 720)
	if (videoResolution != trackerResolution) {
		cv::Mat tempimage = cv::Mat(cv::Size(640, 480), CV_8UC3);
		cv::Mat roi;

		if (videoResolution == 720 && trackerResolution == 480) {
			cv::Rect rect(160, 0, 960, 720);
			roi = frame(rect);
		} else if (videoResolution == 1080 && trackerResolution == 480) {
			cv::Rect rect(240, 0, 1440, 1080);
			roi = frame(rect);
		}
		else {
			roi = frame;
		}

		//std::cout << "FRAME: " << frame->height << "x" << frame->width << " " << frame->depth << std::std::endl;
		//std::cout << "TEMP: " << tempImage->height << "x" << tempImage->width << " " << tempImage->depth << std::endl;
		resize(roi, tempimage, tempimage.size());
		frame = tempimage;
		
		size.width = frame.size().width;
		size.height = frame.size().height;


		std::cout << "Successfully resized first frame" << std::endl;
	}
}

VideoInput::~VideoInput() {
	_capture.release();
}

void VideoInput::updateFrame() {
	static double trackerResolution = frame.size().height;
	frameCount++;

	if (!(captureFromVideo && frameCount == 1)) {
		// If capturing from video and video size is not equal to desired resolution, carry on with resizing
		if (captureFromVideo && videoResolution != trackerResolution) {
			cv::Mat temp_image;
			_capture.read(temp_image);

			cv::Mat roi = temp_image;

			if (videoResolution == 720 && trackerResolution == 480) {
				roi = temp_image(cv::Rect(160, 0, 960, 720));
			} else if (videoResolution == 1080 && trackerResolution == 480) {
				roi = temp_image(cv::Rect(240, 0, 1440, 1080));
			}

			resize(roi, frame, frame.size(), 0, 0, CV_INTER_CUBIC);	
		} else {
			_capture.read(frame);
		}
	}

	if (!captureFromVideo) {
		flip(frame, frame, 1);
	}
}

// Returns 480 or 720 depending on camera resolution
double VideoInput::getResolution() {
	double value = atof(resolutionParameter.c_str());
	return value > 0 ? value : _capture.get(CV_CAP_PROP_FRAME_HEIGHT);
}

VideoWriter::VideoWriter(cv::Size size, std::string filename):
	_video(filename.c_str(), 0x58564944, 15.0, size, 1)
{
}

VideoWriter::~VideoWriter() {
	_video.release();
}

void VideoWriter::write(const cv::Mat &image) {
	_video.write(image);
}

