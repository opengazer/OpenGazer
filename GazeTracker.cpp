#include "GazeTracker.h"
#include "EyeExtractor.h"
#include "mir.h"

fann_type *all_inputs[1000], *all_outputs[1000];
fann_type *all_inputs_left[1000], *all_outputs_left[1000];
IplImage *all_images[1000];
IplImage *all_images_left[1000];
double all_output_coords[1000][2];

static void ignore(const IplImage *) {}

int Targets::getCurrentTarget(Point point) {
    vector<double> distances(targets.size());
    //    debugtee(targets);
    transform(targets.begin(), targets.end(), distances.begin(),
	      sigc::mem_fun(point, &Point::distance));
    //    debugtee(distances);
    return min_element(distances.begin(), distances.end()) - distances.begin();
//     for(int i=0; i<targets.size(); i++)
// 	if (point.distance(targets[i]) < 30)
// 	    return i;
//     return -1;
}


CalTarget::CalTarget() {}

CalTarget::CalTarget(Point point, 
		     const IplImage* image, const IplImage* origimage):
    point(point), 
    image(cvCloneImage(image), Utils::releaseImage), 
    origimage(cvCloneImage(origimage), Utils::releaseImage) 
{
}

void CalTarget::save(CvFileStorage* out, const char* name) {
    cvStartWriteStruct(out, name, CV_NODE_MAP);
    point.save(out, "point");
    cvWrite(out, "image", image.get());
    cvWrite(out, "origimage", origimage.get());
    cvEndWriteStruct(out);
}

void CalTarget::load(CvFileStorage* in, CvFileNode *node) {
    point.load(in, cvGetFileNodeByName(in, node, "point"));
    image.reset((IplImage*) cvReadByName(in, node, "image"));
    origimage.reset((IplImage*) cvReadByName(in, node, "origimage"));
}

TrackerOutput::TrackerOutput(Point gazepoint, Point target, int targetid):
    gazepoint(gazepoint), target(target), targetid(targetid)
{
}

void TrackerOutput::setActualTarget(Point actual) {
	actualTarget = actual;
}

//void TrackerOutput::setErrorOutput(bool show) {
//	outputError = show;
//}

void TrackerOutput::setFrameId(int id) {
	frameid = id;
}
 
template <class T, class S>
vector<S> getsubvector(vector<T> const& input, S T::*ptr) {
    vector<S> output(input.size());
    for(int i=0; i<input.size(); i++)
	output[i] = input[i].*ptr;
    return output;
}

double GazeTracker::imagedistance(const IplImage *im1, const IplImage *im2) {
    double norm = cvNorm(im1, im2, CV_L2);
    return norm*norm;
}

double GazeTracker::covariancefunction(Utils::SharedImage const& im1, 
				       Utils::SharedImage const& im2)
{
    const double sigma = 1.0;
    const double lscale = 500.0;
    return sigma*sigma*exp(-imagedistance(im1.get(),im2.get())/(2*lscale*lscale));
}

void GazeTracker::updateGPs(void) {
    vector<double> xlabels;
    vector<double> ylabels;
		
    for(int i=0; i<caltargets.size(); i++) {
	    xlabels.push_back(caltargets[i].point.x);
	    ylabels.push_back(caltargets[i].point.y);
    }

    vector<Utils::SharedImage> images = 
	getsubvector(caltargets, &CalTarget::image);
	
	/*
cout << "INSIDE updateGPs" << endl;
cout << "labels size: " << xlabels.size();
cout << "images size: " << images.size();
*/
    gpx.reset(new ImProcess(images, xlabels, covariancefunction, 0.01));
    gpy.reset(new ImProcess(images, ylabels, covariancefunction, 0.01));  
    targets.reset(new Targets(getsubvector(caltargets, &CalTarget::point)));
}

void GazeTracker::updateGPs_left(void) {
    vector<double> xlabels;
	vector<double> ylabels;

    for(int i=0; i<caltargets_left.size(); i++) {
	    xlabels.push_back(caltargets_left[i].point.x);
	    ylabels.push_back(caltargets_left[i].point.y);
    }

    vector<Utils::SharedImage> images = 
	getsubvector(caltargets_left, &CalTarget::image);


    gpx_left.reset(new ImProcess(images, xlabels, covariancefunction, 0.01));
    gpy_left.reset(new ImProcess(images, ylabels, covariancefunction, 0.01));  
    //targets_left.reset(new Targets(getsubvector(caltargets_left, &CalTarget::point)));
}

void GazeTracker::calculateTrainingErrors() {
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorgeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

	// Geometry of main monitor
	screen->get_monitor_geometry(num_of_monitors - 1, monitorgeometry);
	
	vector<Point> points = getsubvector(caltargets, &CalTarget::point);
	
	int j = 0;
	
	//cout << "Input count: " << input_count;
	//cout << ", Target size: " << caltargets.size() << endl;
	
	for(int i=0; i<caltargets.size(); i++) {
		double x_total = 0;
		double y_total = 0;
		double sample_count = 0;
		
		//cout << points[i].x << ", " << points[i].y << " x " << all_output_coords[j][0] << ", " << all_output_coords[j][1] << endl;
		
		while(j < input_count && points[i].x == all_output_coords[j][0] && points[i].y == all_output_coords[j][1]) {
			double x_estimate = (gpx->getmean(Utils::SharedImage(all_images[j], &ignore)) + gpx_left->getmean(Utils::SharedImage(all_images_left[j], &ignore))) / 2;
			double y_estimate = (gpy->getmean(Utils::SharedImage(all_images[j], &ignore)) + gpy_left->getmean(Utils::SharedImage(all_images_left[j], &ignore))) / 2;
			
			//cout << "i, j = (" << i << ", " << j << "), est: " << x_estimate << "("<< gpx->getmean(SharedImage(all_images[j], &ignore)) << ","<< gpx_left->getmean(SharedImage(all_images_left[j], &ignore)) << ")" << ", " << y_estimate << "("<< gpy->getmean(SharedImage(all_images[j], &ignore)) <<","<< gpy_left->getmean(SharedImage(all_images_left[j], &ignore)) << ")"<< endl;
			
			x_total += x_estimate;
			y_total += y_estimate;
			sample_count++;
			j++;
		}
		
		x_total /= sample_count;
		y_total /= sample_count;
	
		*output_file << "TARGET: (" << caltargets[i].point.x << "\t, " << caltargets[i].point.y << "\t),\tESTIMATE: ("<< x_total << "\t, " << y_total <<")" << endl;
		//cout << "TARGET: (" << caltargets[i].point.x << "\t, " << caltargets[i].point.y << "\t),\tESTIMATE: ("<< x_total << "\t, " << y_total <<"),\tDIFF: ("<< fabs(caltargets[i].point.x- x_total) << "\t, " << fabs(caltargets[i].point.y - y_total) <<")" << endl;
		
		// Calibration error removal
		xv[i][0] = x_total;		// Source
		xv[i][1] = y_total;
		
		// Targets
		fv_x[i] = caltargets[i].point.x;
		fv_y[i] = caltargets[i].point.y;
		sigv[i] = 0;
		
		int targetId = getTargetId(Point(x_total, y_total));
		
		if(targetId != i) {
			cout << "Target id is not the expected one!! (Expected: "<< i<< ", Current: "<< targetId << ")" << endl;
		}
		
	}
	
	// Add the corners of the monitor as 4 extra data points. This helps the correction for points that are near the edge of monitor
	xv[caltargets.size()][0] = monitorgeometry.get_x();
	xv[caltargets.size()][1] = monitorgeometry.get_y();
	fv_x[caltargets.size()] = monitorgeometry.get_x()-40;
	fv_y[caltargets.size()] = monitorgeometry.get_y()-40;
	
	xv[caltargets.size()+1][0] = monitorgeometry.get_x() + monitorgeometry.get_width();
	xv[caltargets.size()+1][1] = monitorgeometry.get_y();
	fv_x[caltargets.size()+1] = monitorgeometry.get_x() + monitorgeometry.get_width() + 40;
	fv_y[caltargets.size()+1] = monitorgeometry.get_y() - 40;
	
	xv[caltargets.size()+2][0] = monitorgeometry.get_x() + monitorgeometry.get_width();
	xv[caltargets.size()+2][1] = monitorgeometry.get_y() + monitorgeometry.get_height();
	fv_x[caltargets.size()+2] = monitorgeometry.get_x() + monitorgeometry.get_width() + 40;
	fv_y[caltargets.size()+2] = monitorgeometry.get_y() + monitorgeometry.get_height() + 40;
	
	xv[caltargets.size()+3][0] = monitorgeometry.get_x();
	xv[caltargets.size()+3][1] = monitorgeometry.get_y() + monitorgeometry.get_height();
	fv_x[caltargets.size()+3] = monitorgeometry.get_x() - 40;
	fv_y[caltargets.size()+3] = monitorgeometry.get_y() + monitorgeometry.get_height() + 40;
	
	int point_count = caltargets.size() + 4;
    int N = point_count;
    N = binomialInv(N, 2) - 1;
	
	// Find the best beta and gamma parameters for interpolation
    mirBetaGamma(1, 2, point_count, (double*)xv, fv_x, sigv, 0, NULL, NULL, NULL,
                 N, 2, 50.0, &beta_x, &gamma_x);
    mirBetaGamma(1, 2, point_count, (double*)xv, fv_y, sigv, 0, NULL, NULL, NULL,
                 N, 2, 50.0, &beta_y, &gamma_y);
	
	*output_file << endl << endl;
	cout << endl << endl;
	
	output_file->flush();
	
	
	cout << "ERROR CALCULATION FINISHED. BETA = " << beta_x << ", " << beta_y << ", GAMMA IS " << gamma_x << ", " << gamma_y << endl;
	for(int j=0; j<point_count; j++) {
			cout << xv[j][0] << ", " << xv[j][1] << endl;
	}
	

    //checkErrorCorrection();
}


void GazeTracker::printTrainingErrors() {
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorgeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	
	//return;

	// Geometry of main monitorGazeTracker.cpp:233:136: error: expected ‘;’ before string constant

	screen->get_monitor_geometry(num_of_monitors - 1, monitorgeometry);
	
	vector<Point> points = getsubvector(caltargets, &CalTarget::point);
	
	int j = 0;
	
	/*
	cout << "PRINTING TRAINING ESTIMATIONS: " << endl;
	for(int i=0; i<15; i++) {
		int image_index = 0;
		
		while(j < input_count && points[i].x == all_output_coords[j][0] && points[i].y == all_output_coords[j][1]) {
			cout << "X, Y: '" << gpx->getmean(SharedImage(all_images[j], &ignore)) << ", " << gpy->getmean(SharedImage(all_images[j], &ignore)) << "' and '" << gpx_left->getmean(SharedImage(all_images_left[j], &ignore)) << ", " << gpy_left->getmean(SharedImage(all_images_left[j], &ignore)) << "' "<< endl;
			
			image_index++;
			j++;
		}
		
	}
	* */
}


void GazeTracker::clear() {
    caltargets.clear();
    caltargets_left.clear();
    
    beta_x = -1;
    gamma_x = -1;

	ANN = fann_create_standard(2, nn_eyewidth * nn_eyeheight, 2);
	fann_set_activation_function_output(ANN, FANN_SIGMOID);
	
	ANN_left = fann_create_standard(2, nn_eyewidth * nn_eyeheight, 2);
	fann_set_activation_function_output(ANN_left, FANN_SIGMOID);
    // updateGPs()
}

void GazeTracker::addExemplar(Point point, 
			      const IplImage *eyefloat, 
			      const IplImage *eyegrey) 
{
    caltargets.push_back(CalTarget(point, eyefloat, eyegrey));
    updateGPs();
}

void GazeTracker::addExemplar_left(Point point, 
			      const IplImage *eyefloat, 
			      const IplImage *eyegrey) 
{
    caltargets_left.push_back(CalTarget(point, eyefloat, eyegrey));
    updateGPs_left();
}

// Neural network
void GazeTracker::addSampleToNN(Point point, 
				const IplImage *eyefloat,
  				const IplImage *eyegrey)
{
	// Save the entire grey image for later use
	IplImage *savedimage = cvCreateImage(cvSize(64, 32), IPL_DEPTH_32F, 1);
	cvCopy(eyefloat, savedimage);
	
	all_images[input_count] = savedimage;
	all_output_coords[input_count][0] = point.x;
	all_output_coords[input_count][1] = point.y;
	
	// Resize image to 16x8 and equalize histogram
	cvResize(eyegrey, nn_eye);
	//cvEqualizeHist(nn_eye, nn_eye);
	
	// Convert image to interval [0, 1]
	fann_type* inputs = new fann_type[nn_eyewidth * nn_eyeheight];
	for (int i = 0; i < nn_eyewidth * nn_eyeheight; ++i) 
	{ 
		inputs[i] = (float)(nn_eye->imageData[i] + 129) / 257.0f;
		
		if(inputs[i] <0 || inputs[i] > 1) {
			cout << "IMPOSSIBLE INPUT!" << endl;
		}
//		if(((int) eyegrey->imageData[i] >= 127) || ((int) eyegrey->imageData[i] <= -127))
//			cout << "INPUT[" << i << "] = " << inputs[i] << ", image data = " << (int) eyegrey->imageData[i] << endl;
	}
	
	// Convert coordinates to interval [0, 1]
	Point nnpoint;
	Utils::mapToNeuralNetworkCoordinates(point, nnpoint);
	
	fann_type* outputs = new fann_type[2];
	outputs[0] = nnpoint.x;
	outputs[1] = nnpoint.y;
	
	all_outputs[input_count] = &(outputs[0]);
	all_inputs[input_count] = &(inputs[0]);
	input_count++;
	
	//cout << "Added sample # " << input_count << endl;
	//for(int j=0; j<100; j++)
	//fann_train(ANN, inputs, outputs);	// Moved training to batch
}


void GazeTracker::addSampleToNN_left(Point point, 
				const IplImage *eyefloat,
  				const IplImage *eyegrey)
{
	// Save the entire grey image for later use
	IplImage *savedimage = cvCreateImage(cvSize(64, 32), IPL_DEPTH_32F, 1);
	cvCopy(eyefloat, savedimage);
	
	all_images_left[input_count_left] = savedimage;
	
	// Resize image to 16x8
	cvResize(eyegrey, nn_eye);
	//cvEqualizeHist(nn_eye, nn_eye);

	// Convert image to interval [0, 1]
	fann_type* inputs = new fann_type[nn_eyewidth * nn_eyeheight];
	for (int i = 0; i < nn_eyewidth * nn_eyeheight; ++i)
	{ 
		inputs[i] = (float)(nn_eye->imageData[i] + 129) / 257.0f;
		
		if(inputs[i] <0 || inputs[i] > 1) {
			cout << "IMPOSSIBLE INPUT!" << endl;
		}
	}
	
	// Convert coordinates to interval [0, 1]
	Point nnpoint;
	Utils::mapToNeuralNetworkCoordinates(point, nnpoint);
	
	fann_type* outputs = new fann_type[2];
	outputs[0] = nnpoint.x;
	outputs[1] = nnpoint.y;
	
	all_outputs_left[input_count_left] = outputs;
	all_inputs_left[input_count_left] = inputs;
	input_count_left++;
	
	//cout << "(Left) Added sample # " << input_count_left << endl;
	//for(int j=0; j<100; j++)
	//fann_train(ANN_left, inputs, outputs);	// Moved training to batch
}

void FANN_API getTrainingData(unsigned int row, unsigned int input_size, unsigned int output_size, fann_type* input, fann_type* output)
{
	//cout << "GTD: row=" << row << ", inp. size=" << input_size << ", op. size=" << output_size << endl;
	int i;
	for(i=0; i<input_size; i++)
		input[i] = all_inputs[row][i];
		
	for(i=0; i<output_size; i++) 
		output[i] = all_outputs[row][i];
		
	//memcpy(input, all_inputs[row], input_size * sizeof(fann_type));
	//memcpy(output, all_outputs[row], output_size * sizeof(fann_type));
}

void FANN_API getTrainingData_left(unsigned int row, unsigned int input_size, unsigned int output_size, fann_type* input, fann_type* output)
{
	//cout << "GTD: row=" << row << ", inp. size=" << input_size << ", op. size=" << output_size << endl;
	int i;
	for(i=0; i<input_size; i++)
		input[i] = all_inputs_left[row][i];
		
	for(i=0; i<output_size; i++) 
		output[i] = all_outputs_left[row][i];
		//memcpy(input, all_inputs_left[row], input_size * sizeof(fann_type));
		//memcpy(output, all_outputs_left[row], output_size * sizeof(fann_type));
}

void GazeTracker::trainNN()
{
	cout << "Getting data" << endl;
	struct fann_train_data* data = fann_create_train_from_callback(input_count, nn_eyewidth * nn_eyeheight, 2, getTrainingData);
	//fann_save_train(data, "data.txt");

	cout << "Getting left data" << endl;
	struct fann_train_data* data_left = fann_create_train_from_callback(input_count, nn_eyewidth * nn_eyeheight, 2, getTrainingData_left);
	//fann_save_train(data_left, "data_left.txt");

	fann_set_training_algorithm(ANN, FANN_TRAIN_RPROP);
	fann_set_learning_rate(ANN, 0.75);
	fann_set_training_algorithm(ANN_left, FANN_TRAIN_RPROP);
	fann_set_learning_rate(ANN_left, 0.75);
	
	cout << "Training" << endl;
	fann_train_on_data(ANN, data, 200, 20, 0.01);

	cout << "Training left" << endl;
	fann_train_on_data(ANN_left, data_left, 200, 20, 0.01);
	
	double mse = fann_get_MSE(ANN);
	double mse_left = fann_get_MSE(ANN_left);
	
	cout << "MSE: " << mse << ", MSE left: " << mse_left << endl;
}

// void GazeTracker::updateExemplar(int id, 
// 				 const IplImage *eyefloat, 
// 				 const IplImage *eyegrey)
// {
//     cvConvertScale(eyegrey, caltargets[id].origimage.get());
//     cvAdd(caltargets[id].image.get(), eyefloat, caltargets[id].image.get());
//     cvConvertScale(caltargets[id].image.get(), caltargets[id].image.get(), 0.5);
//     updateGPs();
// }

void GazeTracker::draw(IplImage *destimage, int eyedx, int eyedy) {
//     for(int i=0; i<caltargets.size(); i++) {
// 	Point p = caltargets[i].point;
// 	cvSetImageROI(destimage, cvRect((int)p.x - eyedx, (int)p.y - eyedy, 
// 					2*eyedx, 2*eyedy));
// 	cvCvtColor(caltargets[i].origimage, destimage, CV_GRAY2RGB);
// 	cvRectangle(destimage, cvPoint(0,0), cvPoint(2*eyedx-1,2*eyedy-1),
// 		    CV_RGB(255,0,255));
//     }
//     cvResetImageROI(destimage);
}

void GazeTracker::save(void) {
    CvFileStorage *out = 
	cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_WRITE);
    save(out, "GazeTracker");
    cvReleaseFileStorage(&out);
}

void GazeTracker::save(CvFileStorage *out, const char *name) {
    cvStartWriteStruct(out, name, CV_NODE_MAP);
	Utils::saveVector(out, "caltargets", caltargets);
    cvEndWriteStruct(out);
}


void GazeTracker::load(void) {
    CvFileStorage *in = 
	cvOpenFileStorage("calibration.xml", NULL, CV_STORAGE_READ);
    CvFileNode *root = cvGetRootFileNode(in);
    load(in, cvGetFileNodeByName(in, root, "GazeTracker"));
    cvReleaseFileStorage(&in);
    updateGPs();
}

void GazeTracker::load(CvFileStorage *in, CvFileNode *node) {
    caltargets = Utils::loadVector<CalTarget>(in, cvGetFileNodeByName(in, node, 
							       "caltargets"));
}

void GazeTracker::update(const IplImage *image, const IplImage *eyegrey) {
    if (isActive()) {
		output.gazepoint = Point(gpx->getmean(Utils::SharedImage(image, &ignore)), 
					 gpy->getmean(Utils::SharedImage(image, &ignore)));
		output.targetid = getTargetId(output.gazepoint);
		output.target = getTarget(output.targetid);
	
		// Neural network
		// Resize image to 16x8
		cvResize(eyegrey, nn_eye);
		cvEqualizeHist(nn_eye, nn_eye);
		
		fann_type inputs[nn_eyewidth * nn_eyeheight];
		for (int i = 0; i < nn_eyewidth * nn_eyeheight; ++i) 
		{ 
			inputs[i] = (float)(nn_eye->imageData[i] + 129) / 257.0f;
		}
	
		fann_type* outputs = fann_run(ANN, inputs);
		Utils::mapFromNeuralNetworkToScreenCoordinates(Point(outputs[0], outputs[1]), output.nn_gazepoint); 
    }
}

void GazeTracker::update_left(const IplImage *image, const IplImage *eyegrey) {
    if (isActive()) {
		output.gazepoint_left = Point(gpx_left->getmean(Utils::SharedImage(image, &ignore)), 
					 gpy_left->getmean(Utils::SharedImage(image, &ignore)));

		// Neural network
		// Resize image to 16x8
		cvResize(eyegrey, nn_eye);
		cvEqualizeHist(nn_eye, nn_eye);

		fann_type inputs[nn_eyewidth * nn_eyeheight];
		for (int i = 0; i < nn_eyewidth * nn_eyeheight; ++i)
		{ 
			inputs[i] = (float)(nn_eye->imageData[i] + 129) / 257.0f;
		}

		fann_type* outputs = fann_run(ANN_left, inputs);
		Utils::mapFromNeuralNetworkToScreenCoordinates(Point(outputs[0], outputs[1]), output.nn_gazepoint_left);
		
		if(gamma_x != 0) {
			// Overwrite the NN output with the GP output with calibration errors removed
			output.nn_gazepoint.x = (output.gazepoint.x + output.gazepoint_left.x) / 2;
			output.nn_gazepoint.y = (output.gazepoint.y + output.gazepoint_left.y) / 2;
		
			removeCalibrationError(output.nn_gazepoint);
		
			output.nn_gazepoint_left.x = output.nn_gazepoint.x;
			output.nn_gazepoint_left.y = output.nn_gazepoint.y;
		}
    }
}

void GazeTracker::removeCalibrationError(Point& estimate) {
	double x[1][2];
	double output[1];
	double sigma[1];
	int point_count = caltargets.size() + 4;
	
	if(beta_x == -1 && gamma_x == -1)
		return;
	
	x[0][0] = estimate.x;
	x[0][1] = estimate.y;
	/*
	cout << "INSIDE CAL ERR REM. BETA = " << beta_x << ", " << beta_y << ", GAMMA IS " << gamma_x << ", " << gamma_y << endl;
	for(int j=0; j<point_count; j++) {
			cout << xv[j][0] << ", " << xv[j][1] << endl;
	}
	*/
    int N = point_count;
    N = binomialInv(N, 2) - 1;
    
    //cout << "CALIB. ERROR REMOVAL. Target size: " << point_count << ", " << N << endl; 
	
    mirEvaluate(1, 2, 1, (double*)x, point_count, (double*)xv, fv_x, sigv,
                0, NULL, NULL, NULL, beta_x, gamma_x, N, 2, output, sigma);
                
    if(output[0] >= -100)
        estimate.x = output[0];
	
	
    mirEvaluate(1, 2, 1, (double*)x, point_count, (double*)xv, fv_y, sigv,
                0, NULL, NULL, NULL, beta_y, gamma_y, N, 2, output, sigma);

    if(output[0] >= -100)
        estimate.y = output[0];
	
	//cout << "Estimation corrected from: ("<< x[0][0] << ", " << x[0][1] << ") to ("<< estimate.x << ", " << estimate.y << ")" << endl;
	
	boundToScreenCoordinates(estimate);
	
	//cout << "Estimation corrected from: ("<< x[0][0] << ", " << x[0][1] << ") to ("<< estimate.x << ", " << estimate.y << ")" << endl;
}

void GazeTracker::boundToScreenCoordinates(Point& estimate) {
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitorgeometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

	// Geometry of main monitor
	screen->get_monitor_geometry(num_of_monitors - 1, monitorgeometry);
	
	// If x or y coordinates are outside screen boundaries, correct them
	if(estimate.x < monitorgeometry.get_x())
		estimate.x = monitorgeometry.get_x();
	
	if(estimate.y < monitorgeometry.get_y())
		estimate.y = monitorgeometry.get_y();
		
	if(estimate.x >= monitorgeometry.get_x() + monitorgeometry.get_width())
		estimate.x = monitorgeometry.get_x() + monitorgeometry.get_width();
		
	if(estimate.y >= monitorgeometry.get_y() + monitorgeometry.get_height())
		estimate.y = monitorgeometry.get_y() + monitorgeometry.get_height();
}

int GazeTracker::getTargetId(Point point) {
    return targets->getCurrentTarget(point);
}

Point GazeTracker::getTarget(int id) {
    return targets->targets[id];
}


