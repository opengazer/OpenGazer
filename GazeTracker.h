#pragma once

#include <boost/scoped_ptr.hpp>

#include "utils.h"
#include "GaussianProcess.cpp"

struct Targets {
	std::vector<Point> targets;

	Targets();
	Targets(std::vector<Point> const &targets);
	int getCurrentTarget(Point point);
};

struct CalTarget {
	Point point;
	Utils::SharedImage image;
	Utils::SharedImage origImage;

	CalTarget();
	CalTarget(Point point, const IplImage *image, const IplImage *origImage);
	void save(CvFileStorage *out, const char *name=NULL);
	void load(CvFileStorage *in, CvFileNode *node);
};

struct TrackerOutput {
	Point gazePoint;
	Point gazePointLeft;

	// Neural network
	Point nnGazePoint;
	Point nnGazePointLeft;
	Point target;
	Point actualTarget;
	int targetId;
	int frameId;
	//bool outputError;

	TrackerOutput(Point gazePoint, Point target, int targetId);
	void setActualTarget(Point actual);
	//void setErrorOutput(bool show);
	void setFrameId(int id);
};

class GazeTracker {
	typedef MeanAdjustedGaussianProcess<Utils::SharedImage> ImProcess;

	static const int _nnEyeWidth = 16;
	static const int _nnEyeHeight = 8;

public:
	TrackerOutput output;
	std::ostream* outputFile;

	GazeTracker();
	bool isActive();
	void clear();
	void addExemplar(Point point, const IplImage *eyeFloat, const IplImage *eyeGrey);
	void addExemplarLeft(Point point, const IplImage *eyeFloat, const IplImage *eyeGrey);

	// Neural network
	void addSampleToNN(Point point, const IplImage *eyeFloat, const IplImage *eyeGrey);
	void addSampleToNNLeft(Point point, const IplImage *eyeFloat, const IplImage *eyeGrey);
	void trainNN();

	// Calibration error removal
	void removeCalibrationError(Point &estimate);
	void boundToScreenCoordinates(Point &estimate);
	void checkErrorCorrection();

	void draw(IplImage *canvas, int eyeDX, int eyeDY);
	void save();
	void save(CvFileStorage *out, const char *name);
	void load();
	void load(CvFileStorage *in, CvFileNode *node);
	void update(const IplImage *image, const IplImage *eyeGrey);
	void updateLeft(const IplImage *image, const IplImage *eyeGrey);
	Point getTarget(int id);
	int getTargetId(Point point);
	void calculateTrainingErrors();
	void printTrainingErrors();

private:
	boost::scoped_ptr<ImProcess> _gaussianProcessX;
	boost::scoped_ptr<ImProcess> _gaussianProcessY;
	std::vector<CalTarget> _calTargets;
	boost::scoped_ptr<Targets> _targets;

	// ONUR DUPLICATED CODE FOR LEFT EYE
	boost::scoped_ptr<ImProcess> _gaussianProcessXLeft;
	boost::scoped_ptr<ImProcess> _gaussianProcessYLeft;
	std::vector<CalTarget> _calTargetsLeft;
	//boost::scoped_ptr<Targets> _targetsLeft;

	// Neural network
	struct fann *_ANN;
	struct fann *_ANNLeft;
	int _inputCount;
	int _inputCountLeft;

	// Calibration error removal
	double _betaX, _gammaX, _betaY, _gammaY, _sigv[100];	// Max 100 calibration points
	double _xv[100][2], _fvX[100], _fvY[100];

	IplImage *_nnEye;

	static double imageDistance(const IplImage *image1, const IplImage *image2);
	static double covarianceFunction(const Utils::SharedImage &image1, const Utils::SharedImage &image2);

	void updateGaussianProcesses();
	void updateGaussianProcessesLeft();
};
