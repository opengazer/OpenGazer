#include <fann.h>

#include "GazeTracker.h"
#include "EyeExtractor.h"
#include "Point.h"
#include "mir.h"

namespace {
	fann_type *allInputs[1000];
	fann_type *allOutputs[1000];
	fann_type *allInputsLeft[1000];
	fann_type *allOutputsLeft[1000];
	cv::Mat *allImages[1000];
	cv::Mat *allImagesLeft[1000];
	double allOutputCoords[1000][2];
}

template <class T, class S> std::vector<S> getSubVector(std::vector<T> const &input, S T::*ptr) {
	std::vector<S> output(input.size());

	for (int i = 0; i < input.size(); i++) {
		output[i] = input[i].*ptr;
	}

	return output;
}

static void ignore(const cv::Mat *) {
}

void FANN_API getTrainingData(unsigned int row, unsigned int inputSize, unsigned int outputSize, fann_type *input, fann_type *output) {
	//std::cout << "GTD: row=" << row << ", inp. size=" << inputSize << ", op. size=" << outputSize << std::endl;
	for (int i = 0; i < inputSize; i++) {
		input[i] = allInputs[row][i];
	}

	for (int i = 0; i < outputSize; i++) {
		output[i] = allOutputs[row][i];
	}

	//memcpy(input, allInputs[row], inputSize * sizeof(fann_type));
	//memcpy(output, allOutputs[row], outputSize * sizeof(fann_type));
}

void FANN_API getTrainingDataLeft(unsigned int row, unsigned int inputSize, unsigned int outputSize, fann_type *input, fann_type *output) {
	//std::cout << "GTD: row=" << row << ", inp. size=" << inputSize << ", op. size=" << outputSize << std::endl;
	for (int i = 0; i < inputSize; i++) {
		input[i] = allInputsLeft[row][i];
	}

	for (int i = 0; i < outputSize; i++) {
		output[i] = allOutputsLeft[row][i];
	}

	//memcpy(input, allInputsLeft[row], inputSize * sizeof(fann_type));
	//memcpy(output, allOutputsLeft[row], outputSize * sizeof(fann_type));
}

Targets::Targets() {}

Targets::Targets(std::vector<Point> const &targets):
	targets(targets)
{
}

int Targets::getCurrentTarget(Point point) {
	std::vector<double> distances(targets.size());
	//debugTee(targets);
	transform(targets.begin(), targets.end(), distances.begin(), sigc::mem_fun(point, &Point::distance));
	//debugTee(distances);
	return min_element(distances.begin(), distances.end()) - distances.begin();
	//for (int i = 0; i < targets.size(); i++) {
	//	if (point.distance(targets[i]) < 30) {
	// 		return i;
	// 	}
	//}
	//return -1;
}

CalTarget::CalTarget() {}

CalTarget::CalTarget(Point point, const cv::Mat *image, const cv::Mat *origImage):
	point(point),
	image(new cv::Mat(image->size(), image->type()), Utils::releaseImage),
	origImage(new cv::Mat(origImage->size(), origImage->type()), Utils::releaseImage)
{
	image->copyTo(*this->image);
	origImage->copyTo(*this->origImage);
}

void CalTarget::save(CvFileStorage *out, const char *name) {
	cvStartWriteStruct(out, name, CV_NODE_MAP);
	point.save(out, "point");
	cvWrite(out, "image", image.get());
	cvWrite(out, "origImage", origImage.get());
	cvEndWriteStruct(out);
}

void CalTarget::load(CvFileStorage *in, CvFileNode *node) {
	point.load(in, cvGetFileNodeByName(in, node, "point"));
	image.reset((cv::Mat*) cvReadByName(in, node, "image"));
	origImage.reset((cv::Mat*) cvReadByName(in, node, "origImage"));
}

TrackerOutput::TrackerOutput(Point gazePoint, Point target, int targetId):
	gazePoint(gazePoint),
	target(target),
	targetId(targetId)
{
}

void TrackerOutput::setActualTarget(Point actual) {
	actualTarget = actual;
}

//void TrackerOutput::setErrorOutput(bool show) {
//	outputError = show;
//}

void TrackerOutput::setFrameId(int id) {
	frameId = id;
}

GazeTracker::GazeTracker():
	output(Point(0,0), Point(0,0), -1),
	_targets(new Targets),
	_nnEye(new cv::Mat(cv::Size(16, 8), CV_8UC1)),
	_inputCount(0),
	_inputCountLeft(0)
{
}

bool GazeTracker::isActive() {
	return _gaussianProcessX.get() && _gaussianProcessY.get();
}

void GazeTracker::clear() {
	_calTargets.clear();
	_calTargetsLeft.clear();
	_betaX = -1;
	_gammaX = -1;

	_ANN = fann_create_standard(2, _nnEyeWidth * _nnEyeHeight, 2);
	fann_set_activation_function_output(_ANN, FANN_SIGMOID);

	_ANNLeft = fann_create_standard(2, _nnEyeWidth * _nnEyeHeight, 2);
	fann_set_activation_function_output(_ANNLeft, FANN_SIGMOID);

	// updateGaussianProcesses()
}

void GazeTracker::addExemplar(Point point, const cv::Mat *eyeFloat, const cv::Mat *eyeGrey) {
	_calTargets.push_back(CalTarget(point, eyeFloat, eyeGrey));
	updateGaussianProcesses();
}

void GazeTracker::addExemplarLeft(Point point, const cv::Mat *eyeFloat, const cv::Mat *eyeGrey) {
	_calTargetsLeft.push_back(CalTarget(point, eyeFloat, eyeGrey));
	updateGaussianProcessesLeft();
}

void GazeTracker::addSampleToNN(Point point, const cv::Mat *eyeFloat, const cv::Mat *eyeGrey) {
	// Save the entire grey image for later use
	cv::Mat *savedImage = new cv::Mat(EyeExtractor::eyeSize, CV_32FC1);
	eyeFloat->copyTo(*savedImage);

	allImages[_inputCount] = savedImage;
	allOutputCoords[_inputCount][0] = point.x;
	allOutputCoords[_inputCount][1] = point.y;

	// Resize image to 16x8 and equalize histogram
	cv::resize(*eyeGrey, *_nnEye, _nnEye->size());
	//cv::equalizeHist(_nnEye, _nnEye);

	// Convert image to interval [0, 1]
	fann_type *inputs = new fann_type[_nnEyeWidth * _nnEyeHeight];
	for (int i = 0; i < _nnEyeWidth * _nnEyeHeight; ++i) {
		inputs[i] = (float)(_nnEye->data[i] + 129) / 257.0f;

		if (inputs[i] < 0 || inputs[i] > 1) {
			std::cout << "IMPOSSIBLE INPUT!" << std::endl;
		}

		//if (((int)eyeGrey->data[i] >= 127) || ((int)eyeGrey->data[i] <= -127)) {
		//	std::cout << "INPUT[" << i << "] = " << inputs[i] << ", image data = " << (int)eyeGrey->data[i] << std::endl;
		//}
	}

	// Convert coordinates to interval [0, 1]
	Point nnPoint;
	Utils::mapToNeuralNetworkCoordinates(point, nnPoint);

	fann_type *outputs = new fann_type[2];
	outputs[0] = nnPoint.x;
	outputs[1] = nnPoint.y;

	allOutputs[_inputCount] = &(outputs[0]);
	allInputs[_inputCount] = &(inputs[0]);
	_inputCount++;

	//std::cout << "Added sample # " << inputCount << std::endl;
	//for (int j = 0; j < 100; j++) {
	//	fann_train(_ANN, inputs, outputs);	// Moved training to batch
	//}
}

void GazeTracker::addSampleToNNLeft(Point point, const cv::Mat *eyeFloat, const cv::Mat *eyeGrey) {
	// Save the entire grey image for later use
	cv::Mat *savedImage = new cv::Mat(EyeExtractor::eyeSize, CV_32FC1);
	eyeFloat->copyTo(*savedImage);

	allImagesLeft[_inputCountLeft] = savedImage;

	// Resize image to 16x8
	cv::resize(*eyeGrey, *_nnEye, _nnEye->size());
	//cv::equalizeHist(_nnEye, _nnEye);

	// Convert image to interval [0, 1]
	fann_type* inputs = new fann_type[_nnEyeWidth * _nnEyeHeight];
	for (int i = 0; i < _nnEyeWidth * _nnEyeHeight; ++i) {
		inputs[i] = (float)(_nnEye->data[i] + 129) / 257.0f;

		if (inputs[i] < 0 || inputs[i] > 1) {
			std::cout << "IMPOSSIBLE INPUT!" << std::endl;
		}
	}

	// Convert coordinates to interval [0, 1]
	Point nnPoint;
	Utils::mapToNeuralNetworkCoordinates(point, nnPoint);

	fann_type *outputs = new fann_type[2];
	outputs[0] = nnPoint.x;
	outputs[1] = nnPoint.y;

	allOutputsLeft[_inputCountLeft] = outputs;
	allInputsLeft[_inputCountLeft] = inputs;
	_inputCountLeft++;

	//std::cout << "(Left) Added sample # " << _inputCountLeft << std::endl;
	//for (int j = 0; j < 100; j++) {
	//	fann_train(_ANNLeft, inputs, outputs);	// Moved training to batch
	//}
}

void GazeTracker::trainNN() {
	std::cout << "Getting data" << std::endl;
	struct fann_train_data *data = fann_create_train_from_callback(_inputCount, _nnEyeWidth * _nnEyeHeight, 2, getTrainingData);
	//fann_save_train(data, "data.txt");

	std::cout << "Getting left data" << std::endl;
	struct fann_train_data *dataLeft = fann_create_train_from_callback(_inputCount, _nnEyeWidth * _nnEyeHeight, 2, getTrainingDataLeft);
	//fann_save_train(dataLeft, "dataLeft.txt");

	fann_set_training_algorithm(_ANN, FANN_TRAIN_RPROP);
	fann_set_learning_rate(_ANN, 0.75);
	fann_set_training_algorithm(_ANNLeft, FANN_TRAIN_RPROP);
	fann_set_learning_rate(_ANNLeft, 0.75);

	std::cout << "Training" << std::endl;
	fann_train_on_data(_ANN, data, 200, 20, 0.01);

	std::cout << "Training left" << std::endl;
	fann_train_on_data(_ANNLeft, dataLeft, 200, 20, 0.01);

	double mse = fann_get_MSE(_ANN);
	double mseLeft = fann_get_MSE(_ANNLeft);

	std::cout << "MSE: " << mse << ", MSE left: " << mseLeft << std::endl;
}

void GazeTracker::removeCalibrationError(Point &estimate) {
	double x[1][2];
	double output[1];
	double sigma[1];
	int pointCount = _calTargets.size() + 4;

	if (_betaX == -1 && _gammaX == -1) {
		return;
	}

	x[0][0] = estimate.x;
	x[0][1] = estimate.y;

	//std::cout << "INSIDE CAL ERR REM. BETA = " << _betaX << ", " << _betaY << ", GAMMA IS " << _gammaX << ", " << _gammaY << std::endl;
	//for (int i = 0; i < pointCount; i++) {
	//	std::cout << _xv[i][0] << ", " << _xv[i][1] << std::endl;
	//}

	int N = pointCount;
	N = binomialInv(N, 2) - 1;

	//std::cout << "CALIB. ERROR REMOVAL. Target size: " << pointCount << ", " << N << std::endl;

	mirEvaluate(1, 2, 1, (double *)x, pointCount, (double *)_xv, _fvX, _sigv, 0, NULL, NULL, NULL, _betaX, _gammaX, N, 2, output, sigma);

	if (output[0] >= -100) {
		estimate.x = output[0];
	}

	mirEvaluate(1, 2, 1, (double *)x, pointCount, (double *)_xv, _fvY, _sigv, 0, NULL, NULL, NULL, _betaY, _gammaY, N, 2, output, sigma);

	if (output[0] >= -100) {
		estimate.y = output[0];
	}

	//std::cout << "Estimation corrected from: (" << x[0][0] << ", " << x[0][1] << ") to (" << estimate.x << ", " << estimate.y << ")" << std::endl;

	boundToScreenCoordinates(estimate);

	//std::cout << "Estimation corrected from: (" << x[0][0] << ", " << x[0][1] << ") to (" << estimate.x << ", " << estimate.y << ")" << std::endl;
}

void GazeTracker::boundToScreenCoordinates(Point &estimate) {
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorGeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

	// Geometry of main monitor
	screen->get_monitor_geometry(numMonitors - 1, monitorGeometry);

	// If x or y coordinates are outside screen boundaries, correct them
	if (estimate.x < monitorGeometry.get_x()) {
		estimate.x = monitorGeometry.get_x();
	}

	if (estimate.y < monitorGeometry.get_y()) {
		estimate.y = monitorGeometry.get_y();
	}

	if (estimate.x >= monitorGeometry.get_x() + monitorGeometry.get_width()) {
		estimate.x = monitorGeometry.get_x() + monitorGeometry.get_width();
	}

	if (estimate.y >= monitorGeometry.get_y() + monitorGeometry.get_height()) {
		estimate.y = monitorGeometry.get_y() + monitorGeometry.get_height();
	}
}

void GazeTracker::checkErrorCorrection() {
}

void GazeTracker::draw(cv::Mat &destImage, int eyeDX, int eyeDY) {
//	for (int i = 0; i < _calTargets.size(); i++) {
//		Point p = _calTargets[i].point;
//		cvSetImageROI(destImage, cvRect((int)p.x - eyeDX, (int)p.y - eyeDY, 2 * eyeDX, 2 * eyeDY));
//		cvCvtColor(_calTargets[i].origImage, destImage, CV_GRAY2RGB);
//		cvRectangle(destImage, cvPoint(0, 0), cvPoint(2 * eyeDX - 1, 2 * eyeDY - 1), CV_RGB(255, 0, 255));
//	}
//	cvResetImageROI(destImage);
}

void GazeTracker::save() {
	CvFileStorage *out = cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_WRITE);
	save(out, "GazeTracker");
	cvReleaseFileStorage(&out);
}

void GazeTracker::save(CvFileStorage *out, const char *name) {
	cvStartWriteStruct(out, name, CV_NODE_MAP);
	Utils::saveVector(out, "calTargets", _calTargets);
	cvEndWriteStruct(out);
}

void GazeTracker::load() {
	CvFileStorage *in = cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_READ);
	CvFileNode *root = cvGetRootFileNode(in);
	load(in, cvGetFileNodeByName(in, root, "GazeTracker"));
	cvReleaseFileStorage(&in);
	updateGaussianProcesses();
}

void GazeTracker::load(CvFileStorage *in, CvFileNode *node) {
	_calTargets = Utils::loadVector<CalTarget>(in, cvGetFileNodeByName(in, node, "calTargets"));
}

void GazeTracker::update(const cv::Mat *image, const cv::Mat *eyeGrey) {
	if (isActive()) {
		output.gazePoint = Point(_gaussianProcessX->getmean(Utils::SharedImage(image, &ignore)), _gaussianProcessY->getmean(Utils::SharedImage(image, &ignore)));
		output.targetId = getTargetId(output.gazePoint);
		output.target = getTarget(output.targetId);

		// Neural network
		// Resize image to 16x8
		cv::resize(*eyeGrey, *_nnEye, _nnEye->size());
		cv::equalizeHist(*_nnEye, *_nnEye);

		fann_type inputs[_nnEyeWidth * _nnEyeHeight];
		for (int i = 0; i < _nnEyeWidth * _nnEyeHeight; ++i) {
			inputs[i] = (float)(_nnEye->data[i] + 129) / 257.0f;
		}

		fann_type *outputs = fann_run(_ANN, inputs);
		Utils::mapFromNeuralNetworkToScreenCoordinates(Point(outputs[0], outputs[1]), output.nnGazePoint);
	}
}

void GazeTracker::updateLeft(const cv::Mat *image, const cv::Mat *eyeGrey) {
	if (isActive()) {
		output.gazePointLeft = Point(_gaussianProcessXLeft->getmean(Utils::SharedImage(image, &ignore)), _gaussianProcessYLeft->getmean(Utils::SharedImage(image, &ignore)));

		// Neural network
		// Resize image to 16x8
		cv::resize(*eyeGrey, *_nnEye, _nnEye->size());
		cv::equalizeHist(*_nnEye, *_nnEye);

		fann_type inputs[_nnEyeWidth * _nnEyeHeight];
		for (int i = 0; i < _nnEyeWidth * _nnEyeHeight; ++i) {
			inputs[i] = (float)(_nnEye->data[i] + 129) / 257.0f;
		}

		fann_type *outputs = fann_run(_ANNLeft, inputs);
		Utils::mapFromNeuralNetworkToScreenCoordinates(Point(outputs[0], outputs[1]), output.nnGazePointLeft);

		if (_gammaX != 0) {
			// Overwrite the NN output with the GP output with calibration errors removed
			output.nnGazePoint.x = (output.gazePoint.x + output.gazePointLeft.x) / 2;
			output.nnGazePoint.y = (output.gazePoint.y + output.gazePointLeft.y) / 2;

			removeCalibrationError(output.nnGazePoint);

			output.nnGazePointLeft.x = output.nnGazePoint.x;
			output.nnGazePointLeft.y = output.nnGazePoint.y;
		}
	}
}

Point GazeTracker::getTarget(int id) {
	return _targets->targets[id];
}

int GazeTracker::getTargetId(Point point) {
	return _targets->getCurrentTarget(point);
}

void GazeTracker::calculateTrainingErrors() {
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorGeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

	// Geometry of main monitor
	screen->get_monitor_geometry(numMonitors - 1, monitorGeometry);

	std::vector<Point> points = getSubVector(_calTargets, &CalTarget::point);

	//std::cout << "Input count: " << _inputCount;
	//std::cout << ", Target size: " << _calTargets.size() << std::endl;

	for (int i = 0; i < _calTargets.size(); i++) {
		double xTotal = 0;
		double yTotal = 0;
		double sampleCount = 0;

		//std::cout << points[i].x << ", " << points[i].y << " x " << allOutputCoords[j][0] << ", " << allOutputCoords[j][1] << std::endl;

		int j = 0;
		while (j < _inputCount && points[i].x == allOutputCoords[j][0] && points[i].y == allOutputCoords[j][1]) {
			double xEstimate = (_gaussianProcessX->getmean(Utils::SharedImage(allImages[j], &ignore)) + _gaussianProcessXLeft->getmean(Utils::SharedImage(allImagesLeft[j], &ignore))) / 2;
			double yEstimate = (_gaussianProcessY->getmean(Utils::SharedImage(allImages[j], &ignore)) + _gaussianProcessYLeft->getmean(Utils::SharedImage(allImagesLeft[j], &ignore))) / 2;

			//std::cout << "i, j = (" << i << ", " << j << "), est: " << xEstimate << "(" << _gaussianProcessX->getmean(SharedImage(allImages[j], &ignore)) << "," << _gaussianProcessXLeft->getmean(SharedImage(allImagesLeft[j], &ignore)) << ")" << ", " << yEstimate << "(" << _gaussianProcessY->getmean(SharedImage(allImages[j], &ignore)) << "," << _gaussianProcessYLeft->getmean(SharedImage(allImagesLeft[j], &ignore)) << ")" << std::endl;

			xTotal += xEstimate;
			yTotal += yEstimate;
			sampleCount++;
			j++;
		}

		xTotal /= sampleCount;
		yTotal /= sampleCount;

		*outputFile << "TARGET: (" << _calTargets[i].point.x << "\t, " << _calTargets[i].point.y << "\t),\tESTIMATE: (" << xTotal << "\t, " << yTotal << ")" << std::endl;
		//std::cout << "TARGET: (" << _calTargets[i].point.x << "\t, " << _calTargets[i].point.y << "\t),\tESTIMATE: (" << xTotal << "\t, " << yTotal << "),\tDIFF: (" << fabs(_calTargets[i].point.x- x_total) << "\t, " << fabs(_calTargets[i].point.y - y_total) << ")" << std::endl;

		// Calibration error removal
		_xv[i][0] = xTotal;		// Source
		_xv[i][1] = yTotal;

		// Targets
		_fvX[i] = _calTargets[i].point.x;
		_fvY[i] = _calTargets[i].point.y;
		_sigv[i] = 0;

		int targetId = getTargetId(Point(xTotal, yTotal));

		if (targetId != i) {
			std::cout << "Target id is not the expected one!! (Expected: "<< i << ", Current: " << targetId << ")" << std::endl;
		}
	}

	// Add the corners of the monitor as 4 extra data points. This helps the correction for points that are near the edge of monitor
	_xv[_calTargets.size()][0] = monitorGeometry.get_x();
	_xv[_calTargets.size()][1] = monitorGeometry.get_y();
	_fvX[_calTargets.size()] = monitorGeometry.get_x()-40;
	_fvY[_calTargets.size()] = monitorGeometry.get_y()-40;

	_xv[_calTargets.size()+1][0] = monitorGeometry.get_x() + monitorGeometry.get_width();
	_xv[_calTargets.size()+1][1] = monitorGeometry.get_y();
	_fvX[_calTargets.size()+1] = monitorGeometry.get_x() + monitorGeometry.get_width() + 40;
	_fvY[_calTargets.size()+1] = monitorGeometry.get_y() - 40;

	_xv[_calTargets.size()+2][0] = monitorGeometry.get_x() + monitorGeometry.get_width();
	_xv[_calTargets.size()+2][1] = monitorGeometry.get_y() + monitorGeometry.get_height();
	_fvX[_calTargets.size()+2] = monitorGeometry.get_x() + monitorGeometry.get_width() + 40;
	_fvY[_calTargets.size()+2] = monitorGeometry.get_y() + monitorGeometry.get_height() + 40;

	_xv[_calTargets.size()+3][0] = monitorGeometry.get_x();
	_xv[_calTargets.size()+3][1] = monitorGeometry.get_y() + monitorGeometry.get_height();
	_fvX[_calTargets.size()+3] = monitorGeometry.get_x() - 40;
	_fvY[_calTargets.size()+3] = monitorGeometry.get_y() + monitorGeometry.get_height() + 40;

	int pointCount = _calTargets.size() + 4;
	int N = pointCount;
	N = binomialInv(N, 2) - 1;

	// Find the best beta and gamma parameters for interpolation
	mirBetaGamma(1, 2, pointCount, (double *)_xv, _fvX, _sigv, 0, NULL, NULL, NULL, N, 2, 50.0, &_betaX, &_gammaX);
	mirBetaGamma(1, 2, pointCount, (double *)_xv, _fvY, _sigv, 0, NULL, NULL, NULL, N, 2, 50.0, &_betaY, &_gammaY);

	*outputFile << std::endl << std::endl;
	std::cout << std::endl << std::endl;

	outputFile->flush();

	std::cout << "ERROR CALCULATION FINISHED. BETA = " << _betaX << ", " << _betaY << ", GAMMA IS " << _gammaX << ", " << _gammaY << std::endl;
	for (int i = 0; i < pointCount; i++) {
		std::cout << _xv[i][0] << ", " << _xv[i][1] << std::endl;
	}

	//checkErrorCorrection();
}

void GazeTracker::printTrainingErrors() {
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorGeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

	//return;

	// Geometry of main monitorGazeTracker.cpp:233:136: error: expected ‘;’ before string constant

	screen->get_monitor_geometry(numMonitors - 1, monitorGeometry);

	std::vector<Point> points = getSubVector(_calTargets, &CalTarget::point);

	//std::cout << "PRINTING TRAINING ESTIMATIONS: " << std::endl;
	//for (int i = 0; i < 15; i++) {
	//	int imageIndex = 0;
	//	int j = 0;
	//	while (j < inputCount && points[i].x == allOutputCoords[j][0] && points[i].y == allOutputCoords[j][1]) {
	//		std::cout << "X, Y: '" << _gaussianProcessX->getmean(SharedImage(allImages[j], &ignore)) << ", " << _gaussianProcessY->getmean(SharedImage(allImages[j], &ignore)) << "' and '" << _gaussianProcessXLeft->getmean(SharedImage(allImagesLeft[j], &ignore)) << ", " << _gaussianProcessYLeft->getmean(SharedImage(allImagesLeft[j], &ignore)) << "' " << std::endl;
	//		image_index++;
	//		j++;
	//	}
	//}
}


//void GazeTracker::updateExemplar(int id, const cv::Mat &eyeFloat, const cv::Mat &eyeGrey) {
//	cvConvertScale(eyeGrey, _calTargets[id].origImage.get());
//	cvAdd(_calTargets[id].image.get(), eyeFloat, _calTargets[id].image.get());
//	cvConvertScale(_calTargets[id].image.get(), _calTargets[id].image.get(), 0.5);
//	updateGaussianProcesses();
//}

double GazeTracker::imageDistance(const cv::Mat *image1, const cv::Mat *image2) {
	double norm = cv::norm(*image1, *image2, CV_L2);
	return norm * norm;
}

double GazeTracker::covarianceFunction(Utils::SharedImage const &image1, Utils::SharedImage const &image2) {
	const double sigma = 1.0;
	const double lscale = 500.0;
	return sigma * sigma * exp(-imageDistance(image1.get(), image2.get()) / (2 * lscale * lscale));
}

void GazeTracker::updateGaussianProcesses() {
	std::vector<double> xLabels;
	std::vector<double> yLabels;

	for (int i = 0; i < _calTargets.size(); i++) {
		xLabels.push_back(_calTargets[i].point.x);
		yLabels.push_back(_calTargets[i].point.y);
	}

	std::vector<Utils::SharedImage> images = getSubVector(_calTargets, &CalTarget::image);
	//std::cout << "INSIDE updateGPs" << std::endl;
	//std::cout << "labels size: " << xLabels.size();
	//std::cout << "images size: " << images.size();
	_gaussianProcessX.reset(new ImProcess(images, xLabels, covarianceFunction, 0.01));
	_gaussianProcessY.reset(new ImProcess(images, yLabels, covarianceFunction, 0.01));
	_targets.reset(new Targets(getSubVector(_calTargets, &CalTarget::point)));
}

void GazeTracker::updateGaussianProcessesLeft() {
	std::vector<double> xLabels;
	std::vector<double> yLabels;

	for (int i = 0; i < _calTargetsLeft.size(); i++) {
		xLabels.push_back(_calTargetsLeft[i].point.x);
		yLabels.push_back(_calTargetsLeft[i].point.y);
	}

	std::vector<Utils::SharedImage> images = getSubVector(_calTargetsLeft, &CalTarget::image);

	_gaussianProcessXLeft.reset(new ImProcess(images, xLabels, covarianceFunction, 0.01));
	_gaussianProcessYLeft.reset(new ImProcess(images, yLabels, covarianceFunction, 0.01));
	//_targetsLeft.reset(new Targets(getSubVector(_calTargetsLeft, &CalTarget::point)));
}

