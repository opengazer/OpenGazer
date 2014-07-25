#include "utils.h"
#include <fstream>
#include <stdlib.h>
#include "MainGazeTracker.h"
#include <boost/thread/thread.hpp> 
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <unistd.h>

class VideoWriter {
    CvVideoWriter *video;
public:
    VideoWriter(CvSize size, string filename) : 
	video(cvCreateVideoWriter(filename.c_str(), 0x58564944, 15.0, size, 1))
    {}
    
    void write(const IplImage *image) {
	cvWriteFrame(video, image);
    }

    ~VideoWriter() {
	cvReleaseVideoWriter(&video);
    }
};



CommandLineArguments::CommandLineArguments(int argc, char** argv) {
    for(int i=1; i<argc; i++) {
		string parameter(argv[i]);
		int equal_sign_index = parameter.find("=");
		string option = parameter.substr(2, equal_sign_index - 2);
		string value = parameter.substr(equal_sign_index + 1, parameter.length()-equal_sign_index-1);
	    options.push_back(option);
	    parameters.push_back(value);
	}
}
CommandLineArguments::~CommandLineArguments() {
    options = std::vector<string>();
    parameters = std::vector<string>();
}

bool CommandLineArguments::isoption(string option) {
    xforeach(iter, options)
	if (iter->compare(option) == 0)
	    return true;
    return false;
}

string CommandLineArguments::getoptionvalue(string option) {
    for(int i=0; i<options.size(); i++) {
		if (options[i].compare(option) == 0)
		    return parameters[i];
	}
    return "";
}

vector<int> CommandLineArguments::getoptionvalueasvector(string option) {
    vector<int> return_vector;
    for(int i=0; i<options.size(); i++) {
		if (options[i].compare(option) == 0) {
            string input = parameters[i];
            istringstream ss(input);
            string token;

            while(std::getline(ss, token, ',')) {
                return_vector.push_back(atoi(token.c_str()));
                std::cout << "PARSED: " << token << '\n';
            }
            
		    return return_vector;
        }
	}
    
    return return_vector;
}

VideoInput::VideoInput(): 
    capture(cvCaptureFromCAM(0)), framecount(0), capture_from_video(false), resolution_parameter(0)
{	
	timeval time;
	gettimeofday(&time, NULL);
	last_frame_time = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	
	frame = cvQueryFrame(capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(string resolution):
    capture(cvCaptureFromCAM(0)), framecount(0), capture_from_video(false), resolution_parameter(resolution)
{	
	if(resolution.compare("720") == 0) {
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1280);
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 720);
	}
	else if(resolution.compare("1080") == 0) {
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1920);
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}
	else if(resolution.compare("480") == 0) {
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
	}
	
	//cvSetCaptureProperty(capture, CV_CAP_PROP_FOURCC, CV_FOURCC('M','J','P','G'));
	
	timeval time;
	gettimeofday(&time, NULL);
	last_frame_time = (time.tv_sec * 1000) + (time.tv_usec / 1000);

	frame = cvQueryFrame(capture);
	size = cvSize(frame->width, frame->height);
	cvFlip(frame, frame, 1);
}

VideoInput::VideoInput(string resolution, string filename, bool dummy):
    capture(cvCaptureFromFile(filename.c_str())), framecount(0),
    frame(cvQueryFrame(capture)), size(cvSize(frame->width, frame->height)), capture_from_video(true), resolution_parameter(resolution)
{
	timeval time;
	gettimeofday(&time, NULL);
	last_frame_time = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	
    
	double video_resolution = atoi(resolution.c_str());
	double tracker_resolution = frame->height;
    
	// In case the video is 1280/720 and we want to execute 480 (or 1280 -> 720)
	if(video_resolution != tracker_resolution) {
		IplImage *tempimage = cvCreateImage(cvSize(640, 480), 8, 3);
		
		if(video_resolution == 720 && tracker_resolution == 480)
			cvSetImageROI(frame, cvRect(160, 0, 960, 720));		// Set ROI
		else if(video_resolution == 1080 && tracker_resolution == 480)
			cvSetImageROI(frame, cvRect(240, 0, 1440, 1080));		// Set ROI
		
		//cout << "FRAME: " << frame->height << "x" << frame->width << " " << frame->depth << endl;
		//cout << "TEMP: " << tempimage->height << "x" << tempimage->width << " " << tempimage->depth << endl; 
		cvResize(frame, tempimage);
		frame = tempimage;
		cvResetImageROI(frame);
		size.width = frame->width;
		size.height = frame->height;
		
		
		cout << "Successfully resized first frame" << endl;
	}
}

// Returns 480 or 720 depending on camera resolution
double VideoInput::get_resolution() {
	double value = atof(resolution_parameter.c_str());
	
	if(value > 0)
		return value;
	else
		return cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
}

void VideoInput::updateFrame() {
	static double video_resolution = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    static double tracker_resolution = frame->height;
	framecount++;
	
	
	if(capture_from_video && framecount == 1)
		;
	else {
		// If capturing from video and video size is not equal to desired resolution, carry on with resizing	
		if(capture_from_video && video_resolution != tracker_resolution) {
		
			IplImage *tempimage = cvQueryFrame(capture);
			
			if(video_resolution == 720 && tracker_resolution == 480)
				cvSetImageROI(tempimage, cvRect(160, 0, 960, 720));		// Set ROI
			else if(video_resolution == 1080 && tracker_resolution == 480)
				cvSetImageROI(tempimage, cvRect(240, 0, 1440, 1080));		// Set ROI
				
			cvResize(tempimage, frame, CV_INTER_CUBIC);								// Resize image to 640x480
		}
		else {
    		frame = cvQueryFrame(capture);	
		}
	}
	
	if(!capture_from_video) {
		cvFlip(frame, frame, 1);
	}
}

VideoInput::~VideoInput() {
    cvReleaseCapture( &capture );
}

MainGazeTracker::MainGazeTracker(int argc, char** argv, 
                 const vector<boost::shared_ptr<AbstractStore> >
				 &stores): 
    framestoreload(-1), stores(stores), autoreload(false), videooverlays(false), totalframecount(0), recording(false), commandindex(-1)
//     , statemachine(shared_ptr<AlertWindow>(new AlertWindow("start")))
{
    CommandLineArguments args(argc, argv);

	if (args.getoptionvalue("help").compare("") != 0) {
		cout << endl;
		cout << "CVC Machine Vision Group Eye-Tracker" << endl;
		cout << endl << endl;
		cout << "Usage:\teyetracker --subject=SUBJECT_NAME [--resolution=[480|720]] [--setup=SETUP_FOLDER_NAME] [--headdistance=DISTANCE] [--record=[0|1]] [--overlay=[0|1]] [--input=INPUT_FILE_PATH]" << endl;
		cout << endl << endl;
		cout << "OPTIONS:" << endl;
		cout << "\tsubject:\tSubject\'s name to be used in the output file name." << endl;
		cout << "\tresolution:\tResolution for the camera. 480 for 640x480 resolution, 720 for 1280x720." << endl;
		cout << "\tsetup:\t\tExperiment setup name and also the folder to read the test and calibration point locations." << endl;
		cout << "\theaddistance:\tSubject\'s head distance in cm to be included in the output file for automatic error calculation purposes." << endl;
		cout << "\trecord:\t\tWhether a video of the experiment should be recorded for offline processing purposes." << endl;
		cout << "\toverlay:\tWhether target point and estimation pointers are written as an overlay to the recorded video. Should not be used when offline processing is desired on the output video." << endl;
		cout << "\tinput:\t\tInput video path in case of offline processing." << endl;
		cout << endl << endl;
		cout << "SAMPLE USAGES:" << endl;
		cout << "\tBasic usage without video recording:" << endl;
		cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=0" << endl;
		cout << endl;
		cout << "\tUsage during experiments to enable video recording for offline processing:" << endl;
		cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=1" << endl;
		cout << endl;
		cout << "\tUsage during offline processing:" << endl;
		cout << "\t\t./eyetracker --subject=david --resolution=720 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/david_std_720_1.avi" << endl;
		cout << endl;
		cout << "\tUsage during offline processing with lower resolution:" << endl;
		cout << "\t\t./eyetracker --subject=david --resolution=480 --setup=std --headdistance=80 --record=1 --overlay=1 --input=../outputs/david_std_720_1.avi" << endl;
		cout << endl << endl;
		exit(0);
	}
	
	if (args.getoptionvalue("input").compare("") != 0) {
		videoinput.reset(new VideoInput(args.getoptionvalue("resolution"), args.getoptionvalue("input"), true));
		
		// Read the commands (SELECT, CLEAR, CALIBRATE, TEST)
		string inputcommandfilename = args.getoptionvalue("input");
		inputcommandfilename = inputcommandfilename.substr(0, inputcommandfilename.length()-4) + "_commands.txt";
		
		commandinputfile = new ifstream(inputcommandfilename.c_str());

	    for(;;) {
			long no;
			string name;
			*commandinputfile >> no >> name;
			
			if (commandinputfile->rdstate())
				break; // break if any error
			
			commands.push_back(Command(no, name));
	    }
	
		commandindex = 0;
	
		cout << commands.size() << " commands read." << endl;
	}
	else {
		// --resolution parameter
		if (args.getoptionvalue("resolution").compare("") != 0) {
			videoinput.reset(new VideoInput(args.getoptionvalue("resolution")));
		} 
		else {
			videoinput.reset(new VideoInput("480"));
		}
	}
	
	if(videoinput.get()->get_resolution() == 720) {
		conversionimage = cvCreateImage(cvSize(1280, 720), 8, 3 );
	}
	else if(videoinput.get()->get_resolution() == 1080) {
		conversionimage = cvCreateImage(cvSize(1920, 1080), 8, 3 );
	}
	else if(videoinput.get()->get_resolution() == 480) {
		conversionimage = cvCreateImage(cvSize(640, 480), 8, 3 );
	}
	
	string subject = args.getoptionvalue("subject");
	string setup = args.getoptionvalue("setup");

	if(subject.compare("") == 0) {
		subject = "default";
	}

	if(setup.compare("") == 0) {
		setup = "std";
	}
		

	if (args.getoptionvalue("overlay").compare("1") == 0)
		videooverlays = true;
	
	// --headdistance parameter
	if (args.getoptionvalue("headdistance").compare("") != 0)
		headdistance = atoi(args.getoptionvalue("headdistance").c_str());
	else
		headdistance = 70;
	
	// --dwelltime parameter
	if (args.getoptionvalue("dwelltime").compare("") != 0)
		dwelltime_parameter = atoi(args.getoptionvalue("dwelltime").c_str());
	else
		dwelltime_parameter = 30;
	
	// --testdwelltime parameter
	if (args.getoptionvalue("testdwelltime").compare("") != 0)
		test_dwelltime_parameter = atoi(args.getoptionvalue("testdwelltime").c_str());
	else
		test_dwelltime_parameter = 20;
	
	// --sleep parameter
	if (args.getoptionvalue("sleep").compare("") != 0)
		sleep_parameter = atoi(args.getoptionvalue("sleep").c_str());
	else
		sleep_parameter = 0;
		
	// --folder parameter
    string folder_parameter = "outputs";
    
	if (args.getoptionvalue("outputfolder").compare("") != 0)
		folder_parameter = args.getoptionvalue("outputfolder");

	// --subject parameter
	base_path = getUniqueFileName(folder_parameter, subject + "_" + setup + "_" + args.getoptionvalue("resolution"));
	
    	// --record parameter
	if (args.getoptionvalue("record").compare("1") == 0) {
	    video.reset(new VideoWriter(videoinput->size, base_path.substr(0, base_path.length() - 4) + ".avi"));
	    recording = true;
	}

	outputfile = new ofstream((base_path + "_").c_str());

	// First write the system time
    time_t current_time = time(NULL);
    *outputfile << ctime(&current_time) << endl;

	// Then write the setup parameters
	*outputfile << "--input=" << args.getoptionvalue("input") << endl;
	*outputfile << "--record=" << args.getoptionvalue("record") << endl;
	*outputfile << "--overlay=" << (videooverlays ? "true" : "false") << endl;
	*outputfile << "--headdistance=" << headdistance << endl;
	*outputfile << "--resolution=" << args.getoptionvalue("resolution") << endl;
	*outputfile << "--setup=" << setup << endl;
	*outputfile << "--subject=" << subject << endl << endl;

	// Finally the screen resolution
    Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	Gdk::Rectangle rect;
	screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);
	*outputfile << "Screen resolution: " << rect.get_width() << " x " << rect.get_height() << " (Position: "<< rect.get_x() << ", "<< rect.get_y() << ")" << endl << endl;
	outputfile->flush();
	
	// If recording, create the file to write the commands for button events
	if (recording) {
		string commandfilename = base_path.substr(0, base_path.length()-4) + "_commands.txt";
		commandoutputfile = new ofstream(commandfilename.c_str());
	}	
	
	directory = setup;
	
    canvas.reset(cvCreateImage(videoinput->size, 8, 3));
    tracking.reset(new TrackingSystem(videoinput->size));
	
	tracking->gazetracker.output_file = outputfile;
	isCalibrationOutputWritten = true;
	
	game_win = new GameWindow (&(tracking->gazetracker.output));
	game_win->show();

    if(videoinput.get()->get_resolution() == 720) {
        repositioning_image = cvCreateImage(cvSize(1280, 720), 8, 3 );
    }
    else if(videoinput.get()->get_resolution() == 1080) {
        repositioning_image = cvCreateImage(cvSize(1920, 1080), 8, 3 );
    }
    else if(videoinput.get()->get_resolution() == 480) {
        repositioning_image = cvCreateImage(cvSize(640, 480), 8, 3 );
    }
    
    game_win->setRepositioningImage(repositioning_image);
    face_rectangle = NULL;
}

void MainGazeTracker::addTracker(Point point) {
    tracking->tracker.addtracker(point);
}

void MainGazeTracker::savepoints() {
    try {
	tracking->tracker.save("tracker", "points.txt", videoinput->frame);
	autoreload = true;
    }
    catch (ios_base::failure &e) {
	cout << e.what() << endl;
    }
}

void MainGazeTracker::loadpoints() {
    try {
	tracking->tracker.load("tracker", "points.txt", videoinput->frame);
	autoreload = true;
    }
    catch (ios_base::failure &e) {
	cout << e.what() << endl;
    }

}

void MainGazeTracker::choosepoints() {
    try {
		Point eyes[2];
		Point nose[2];
		Point mouth[2];
		Point eyebrows[2];
		
	    if (recording) {
			*commandoutputfile << totalframecount << " SELECT" << endl;
		}
		
		detect_eye_corners(videoinput->frame, videoinput->get_resolution(), eyes);
		
		
		CvRect nose_rect = cvRect(eyes[0].x, eyes[0].y, fabs(eyes[0].x-eyes[1].x), fabs(eyes[0].x-eyes[1].x));
		check_rect_size(videoinput->frame, &nose_rect);
		//cout << "Nose rect: " << nose_rect.x << ", " << nose_rect.y << " - " << nose_rect.width << ", " << nose_rect.height << endl;
		
		if(!detect_nose(videoinput->frame, videoinput->get_resolution(), nose_rect, nose)) {
			cout << "NO NOSE" << endl;
			return;
		}
			
		CvRect mouth_rect = cvRect(eyes[0].x, nose[0].y, fabs(eyes[0].x-eyes[1].x), 0.8*fabs(eyes[0].x-eyes[1].x));
		check_rect_size(videoinput->frame, &mouth_rect);
		
		if(!detect_mouth(videoinput->frame, videoinput->get_resolution(), mouth_rect, mouth)) {
			cout << "NO MOUTH" << endl;
			return;
		}
		
		CvRect eyebrow_rect = cvRect(eyes[0].x  + fabs(eyes[0].x-eyes[1].x) * 0.25, eyes[0].y - fabs(eyes[0].x-eyes[1].x) * 0.40,  fabs(eyes[0].x-eyes[1].x) * 0.5, fabs(eyes[0].x-eyes[1].x) * 0.25);
		check_rect_size(videoinput->frame, &eyebrow_rect);
		detect_eyebrow_corners(videoinput->frame, videoinput->get_resolution(), eyebrow_rect, eyebrows);
		
		tracking->tracker.cleartrackers();
	    autoreload = false;
	
		tracking->tracker.addtracker(eyes[0]);
		tracking->tracker.addtracker(eyes[1]);
		
		tracking->tracker.addtracker(nose[0]);
		tracking->tracker.addtracker(nose[1]);
		tracking->tracker.addtracker(mouth[0]);
		tracking->tracker.addtracker(mouth[1]);
		tracking->tracker.addtracker(eyebrows[0]);
		tracking->tracker.addtracker(eyebrows[1]);
		
			
		cout << "EYES: " << eyes[0] << " + " << eyes[1] << endl;
		cout << "NOSE: " << nose[0] << " + " << nose[1] << endl;
		cout << "MOUTH: " << mouth[0] << " + " << mouth[1] << endl;
		cout << "EYEBROWS: " << eyebrows[0] << " + " << eyebrows[1] << endl;
		

		// Save point selection image 
		tracking->tracker.save_image();

		// Calculate the area containing the face
		extract_face_region_rectangle(videoinput->frame, tracking->tracker.getpoints(&PointTracker::lastpoints, true));
		tracking->tracker.normalizeOriginalGrey();
    }
    catch (ios_base::failure &e) {
	cout << e.what() << endl;
    }
    catch (std::exception &ex) {
	cout << ex.what() << endl;
    }
}

void MainGazeTracker::clearpoints() {
    if (recording) {
		*commandoutputfile << totalframecount << " CLEAR" << endl;
	}
	
    tracking->tracker.cleartrackers();
    autoreload = false;
}


void MainGazeTracker::doprocessing(void) {	
	totalframecount++;
	
    framecount++;
    videoinput->updateFrame();
	
	// Wait a little so that the marker stays on the screen for a longer time
	if((tracker_status == STATUS_CALIBRATING || tracker_status == STATUS_TESTING) && !videoinput->capture_from_video) {
		usleep(sleep_parameter);
	}
	else {
		;
	}
	
	const IplImage *frame = videoinput->frame;

    canvas->origin = frame->origin;
	double image_norm = 0.0;
	
	if(tracker_status == STATUS_PAUSED) {
		cvAddWeighted(frame, 0.5, overlayimage, 0.5, 0.0, canvas.get());
		
		// Only calculate norm in the area containing the face
	    if (faces.size() == 1) {
			cvSetImageROI(const_cast<IplImage*>(frame), faces[0]);
			cvSetImageROI(overlayimage, faces[0]);
		
			image_norm = cvNorm(frame, overlayimage, CV_L2);
			image_norm = (10000 * image_norm) / (faces[0].width * faces[0].height);
			
			// To be able to use the same threshold for VGA and 720 cameras
			if(videoinput->get_resolution() == 720) {
				image_norm *= 1.05;
			}
			
			cout << "ROI NORM: " << image_norm << " (" << faces[0].width << "x" << faces[0].height << ")" << endl;
			cvResetImageROI(const_cast<IplImage*>(frame));
			cvResetImageROI(overlayimage);
		}
		else {
			image_norm = cvNorm(frame, overlayimage, CV_L2);
			image_norm = (15000 * image_norm) / (videoinput->get_resolution() * videoinput->get_resolution());
			
			// To be able to use the same threshold for only-face method and all-image method
			image_norm *= 1.2;
			
			// To be able to use the same threshold for VGA and 720 cameras
			if(videoinput->get_resolution() == 720) {
				image_norm *= 1.05;
			}
			//cout << "WHOLE NORM: " << image_norm << endl;
		}
	
		
	}
	else {
        cvCopy( frame, canvas.get(), 0 );	
	}

    try {
	tracking->doprocessing(frame, canvas.get());
	if (tracking->gazetracker.isActive()) {
		if(tracker_status != STATUS_TESTING) {
			tracking->gazetracker.output.setActualTarget(Point(0, 0));
			tracking->gazetracker.output.setFrameId(0);
		}
		else {
			tracking->gazetracker.output.setActualTarget(Point(target->getActivePoint().x, target->getActivePoint().y));
			tracking->gazetracker.output.setFrameId(target->getPointFrame());
	    }
	
		//tracking->gazetracker.output.setErrorOutput(tracker_status == STATUS_TESTING);	// No longer necessary, TODO REMOVE
		
		xforeach(iter, stores)
		(*iter)->store(tracking->gazetracker.output);
		
		// Write the same info to the output text file
		if(outputfile != NULL) {
			TrackerOutput output = tracking->gazetracker.output;
			if(tracker_status == STATUS_TESTING) {
                cout << "TESTING, WRITING OUTPUT!!!!!!!!!!!!!!!!!" << endl;
				if(!tracking->eyex.isBlinking()) {
					*outputfile << output.frameid + 1 << "\t" 
							<< output.actualTarget.x << "\t" << output.actualTarget.y << "\t"
							<< output.gazepoint.x << "\t" << output.gazepoint.y << "\t"
							<< output.gazepoint_left.x << "\t" << output.gazepoint_left.y 
							// << "\t" << output.nn_gazepoint.x << "\t" << output.nn_gazepoint.y << "\t"
							// << output.nn_gazepoint_left.x << "\t" << output.nn_gazepoint_left.y 
							<< endl;
				}
				else {
					*outputfile << output.frameid + 1 << "\t" 
							<< output.actualTarget.x << "\t" << output.actualTarget.y << "\t"
							<< 0 << "\t" << 0 << "\t"
							<< 0 << "\t" << 0 
							// << "\t" << 0 << "\t" << 0 << "\t"
							//<< 0 << "\t" << 0 
							<< endl;
				}
			}

		    outputfile->flush();
		}
	}
	
// 	if (!tracking->tracker.areallpointsactive())
// 	    throw TrackingException();
	framestoreload = 20;
    }
    catch (TrackingException&) {
	framestoreload--;
    }	

	if(tracker_status == STATUS_PAUSED) {
		int rectangle_thickness = 15;
		CvScalar color;
		
		if(image_norm < 1500) {
			color = CV_RGB(0, 255, 0);
		}
		else if(image_norm < 2500) {
			color = CV_RGB(0, 165, 255);
		}
		else {
			color = CV_RGB(0, 0, 255);
		}
		
		cvRectangle(canvas.get(), cvPoint(0, 0), cvPoint(rectangle_thickness, videoinput->size.height), color, CV_FILLED);	//left
		cvRectangle(canvas.get(), cvPoint(0, 0), cvPoint(videoinput->size.width, rectangle_thickness), color, CV_FILLED);	//top
		cvRectangle(canvas.get(), cvPoint(videoinput->size.width-rectangle_thickness, 0), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//right
		cvRectangle(canvas.get(), cvPoint(0, videoinput->size.height-rectangle_thickness), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//bottm
		
		
		// Fill the repositioning image so that it can be displayed on the subject's monitor too
		cvAddWeighted(frame, 0.5, overlayimage, 0.5, 0.0, repositioning_image);
		
		cvRectangle(repositioning_image, cvPoint(0, 0), cvPoint(rectangle_thickness, videoinput->size.height), color, CV_FILLED);	//left
		cvRectangle(repositioning_image, cvPoint(0, 0), cvPoint(videoinput->size.width, rectangle_thickness), color, CV_FILLED);	//top
		cvRectangle(repositioning_image, cvPoint(videoinput->size.width-rectangle_thickness, 0), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//right
		cvRectangle(repositioning_image, cvPoint(0, videoinput->size.height-rectangle_thickness), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//bottm
	}

    framefunctions.process();

	// If video output is requested
    if (recording) {
		if(videooverlays) {
			//cout << "VIDEO EXISTS" << endl;
			TrackerOutput output = tracking->gazetracker.output;
		
			Point actualtarget(0, 0);
			Point estimation(0, 0);

			cvCopy(canvas.get(), conversionimage);
			
			if(tracker_status == STATUS_TESTING) {
				//cout << "TARGET: " << output.actualTarget.x << ", " << output.actualTarget.y << endl;
				mapToVideoCoordinates(output.actualTarget, videoinput->get_resolution(), actualtarget);
				//cout << "MAPPING: " << actualtarget.x << ", " << actualtarget.y << endl << endl;

				cvCircle((CvArr*) conversionimage, cvPoint(actualtarget.x, actualtarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);
			
				// If not blinking, show the estimation in video
				if(!tracking->eyex.isBlinking()) {
					mapToVideoCoordinates(output.gazepoint, videoinput->get_resolution(), estimation);
					cvCircle((CvArr*) conversionimage, cvPoint(estimation.x, estimation.y), 8, cvScalar(0, 255, 0), -1, 8, 0);
					
					mapToVideoCoordinates(output.gazepoint_left, videoinput->get_resolution(), estimation);
					cvCircle((CvArr*) conversionimage, cvPoint(estimation.x, estimation.y), 8, cvScalar(255, 0, 0), -1, 8, 0);
				}
			}

            if(tracker_status == STATUS_PAUSED) {
                int rectangle_thickness = 15;
                CvScalar color;
                
                if(image_norm < 1900) {
                    color = CV_RGB(0, 255, 0);
                }
                else if(image_norm < 3000) {
                    color = CV_RGB(255, 165, 0);
                }
                else {
                    color = CV_RGB(255, 0, 0);
                }
                                
                cvRectangle(conversionimage, cvPoint(0, 0), cvPoint(rectangle_thickness, videoinput->size.height), color, CV_FILLED);	//left
                cvRectangle(conversionimage, cvPoint(0, 0), cvPoint(videoinput->size.width, rectangle_thickness), color, CV_FILLED);	//top
                cvRectangle(conversionimage, cvPoint(videoinput->size.width-rectangle_thickness, 0), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//right
                cvRectangle(conversionimage, cvPoint(0, videoinput->size.height-rectangle_thickness), cvPoint(videoinput->size.width, videoinput->size.height), color, CV_FILLED);	//bottm
            }

			video->write(conversionimage); //conversionimage);
		}
		else {
			//cout << "Trying to write video image" << endl;
			cvCopy(videoinput->frame, conversionimage);
			//cout << "Image copied" << endl;
			video->write(conversionimage);
			//cout << "Image written" << endl << endl;
		}
	}
	
	// Show the current target & estimation points on the main window
	if(tracker_status == STATUS_CALIBRATING || tracker_status == STATUS_TESTING || tracker_status == STATUS_CALIBRATED) {
		TrackerOutput output = tracking->gazetracker.output;
		Point actualtarget(0, 0);
		Point estimation(0, 0);
		
		if(tracker_status == STATUS_TESTING) {
			mapToVideoCoordinates(target->getActivePoint(), videoinput->get_resolution(), actualtarget, false);
			cvCircle((CvArr*) canvas.get(), cvPoint(actualtarget.x, actualtarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);
		}
		else if(tracker_status == STATUS_CALIBRATING) {
			mapToVideoCoordinates(calibrator->getActivePoint(), videoinput->get_resolution(), actualtarget, false);
			cvCircle((CvArr*) canvas.get(), cvPoint(actualtarget.x, actualtarget.y), 8, cvScalar(0, 0, 255), -1, 8, 0);
		}
		
		// If not blinking, show the estimation in video
		if(!tracking->eyex.isBlinking()) {
			mapToVideoCoordinates(output.gazepoint, videoinput->get_resolution(), estimation, false);
			cvCircle((CvArr*) canvas.get(), cvPoint(estimation.x, estimation.y), 8, cvScalar(0, 255, 0), -1, 8, 0);
			
			mapToVideoCoordinates(output.gazepoint_left, videoinput->get_resolution(), estimation, false);
			cvCircle((CvArr*) canvas.get(), cvPoint(estimation.x, estimation.y), 8, cvScalar(255, 0, 0), -1, 8, 0);
		}
	}
//     statemachine.handleEvent(EVENT_TICK);

    if (autoreload && framestoreload <= 0 && tracker_status != STATUS_PAUSED) 
 		loadpoints();	
}

void MainGazeTracker::simulateClicks(void) {
	if(commands.size() > 0) {
	while(commandindex >= 0 && commandindex <= (commands.size()-1) && commands[commandindex].frameno == totalframecount) {
		cout << "Command: " << commands[commandindex].commandname << endl;
		if(strcmp(commands[commandindex].commandname.c_str(), "SELECT") == 0) {
			cout << "Choosing points automatically" << endl;
			choosepoints();
		}
		else if(strcmp(commands[commandindex].commandname.c_str(), "CLEAR") == 0) {
			cout << "Clearing points automatically" << endl;
			clearpoints();
		}
		else if(strcmp(commands[commandindex].commandname.c_str(), "CALIBRATE") == 0) {
			cout << "Calibrating automatically" << endl;
			startCalibration();
		}
		else if(strcmp(commands[commandindex].commandname.c_str(), "TEST") == 0) {
			cout << "Testing automatically" << endl;
			startTesting();
		}
		else if(strcmp(commands[commandindex].commandname.c_str(), "UNPAUSE") == 0 || strcmp(commands[commandindex].commandname.c_str(), "PAUSE") == 0) {
			cout << "Pausing/unpausing automatically" << endl;
			pauseOrRepositionHead();
		}
		
		commandindex++;
	}
	
		if(commandindex == commands.size() && (tracker_status == STATUS_IDLE || tracker_status == STATUS_CALIBRATED)) {
			throw QuitNow();	
		}
}
}

MainGazeTracker::~MainGazeTracker(void) {
	cleanUp();
}

void MainGazeTracker::cleanUp(void) {
	outputfile->close();
	rename((base_path + "_").c_str(), (base_path).c_str());
	//rename("out.avi", (base_path.substr(0, base_path.length() - 4) + ".avi").c_str());
}


void MainGazeTracker::addExemplar(Point exemplar) {
    if (exemplar.x >= EyeExtractor::eyedx && 
	exemplar.x + EyeExtractor::eyedx < videoinput->size.width &&
	exemplar.y >= EyeExtractor::eyedy && 
	exemplar.y + EyeExtractor::eyedy < videoinput->size.height) {
		tracking->gazetracker.addExemplar(exemplar, 
						  tracking->eyex.eyefloat.get(), 
						  tracking->eyex.eyegrey.get());
						
		tracking->gazetracker.addExemplar_left(exemplar, 
						  tracking->eyex.eyefloat_left.get(), 
						  tracking->eyex.eyegrey_left.get());
	}
}

static vector<Point> scalebyscreen(const vector<Point> &points) {
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle rect;
	
    Glib::RefPtr<Gdk::Screen> screen = 
	Gdk::Display::get_default()->get_default_screen();
	
	if(num_of_monitors == 1) {
    	return Calibrator::scaled(points, screen->get_width(), screen->get_height());
	}
	else {
		screen->get_monitor_geometry(num_of_monitors - 1, rect);
	    return Calibrator::scaled(points, rect.get_width(), rect.get_height());
	}
}

void MainGazeTracker::startCalibration() {
	tracker_status = STATUS_CALIBRATING;
	
	if(game_win == NULL) {
		game_win = new GameWindow (&(tracking->gazetracker.output));
	}
	game_win->show();
	
	if (recording) {
		*commandoutputfile << totalframecount << " CALIBRATE" << endl;
	}
	
    boost::shared_ptr<WindowPointer> 
	pointer(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0.2)));
	
	game_win->setCalibrationPointer(pointer.get());
	
	if(Gdk::Screen::get_default()->get_n_monitors() > 1) {
	    boost::shared_ptr<WindowPointer> 
		mirror(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0)));
	
		pointer->mirror = mirror;
	}

    ifstream calfile((directory + "/calpoints.txt").c_str());

    boost::shared_ptr<Calibrator> 
	cal(new Calibrator(framecount, tracking, 
				  scalebyscreen(Calibrator::loadPoints(calfile)),
				  pointer, dwelltime_parameter));
				
	calibrator = cal.operator->();
	
	isCalibrationOutputWritten = false;
    
    framefunctions.clear();
    framefunctions.addChild(&framefunctions, cal);

}

void MainGazeTracker::startTesting() {
	tracker_status = STATUS_TESTING;

    if (recording) {
		*commandoutputfile << totalframecount << " TEST" << endl;
	}
	
    vector<Point> points;

    boost::shared_ptr<WindowPointer> 
	pointer(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0.2)));
	
	game_win->setCalibrationPointer(pointer.get());
	
	if(Gdk::Screen::get_default()->get_n_monitors() > 1) {
        boost::shared_ptr<WindowPointer>
		mirror(new WindowPointer(WindowPointer::PointerSpec(30,30,1,0,0)));
	
		pointer->mirror = mirror;
	}
	
	// ONUR Modified code to read the test points from a text file
    ifstream calfile((directory + "/testpoints.txt").c_str());
	points = Calibrator::loadPoints(calfile);
	
    boost::shared_ptr<MovingTarget>
	moving(new MovingTarget(framecount, scalebyscreen(points), pointer, test_dwelltime_parameter));
	
	target = moving.operator->();
	
	//MovingTarget* target = new MovingTarget(framecount, scalebyscreen(points), pointer);
	//shared_ptr<MovingTarget> moving((const boost::shared_ptr<MovingTarget>&) *target);
    
	*outputfile << "TESTING" << endl << endl;
	
    framefunctions.clear();
    framefunctions.addChild(&framefunctions, moving);
}

void MainGazeTracker::startPlaying() {
	if(game_win == NULL) {
		game_win = new GameWindow (&(tracking->gazetracker.output));
	}
	game_win->show();
}
void MainGazeTracker::pauseOrRepositionHead() {
	if(tracker_status == STATUS_PAUSED) {
	    if (recording) {
			*commandoutputfile << totalframecount << " UNPAUSE" << endl;
		}
		
		if(is_tracker_calibrated) {
			tracker_status = STATUS_CALIBRATED;
		}
		else {
			tracker_status = STATUS_IDLE;
		}
		tracking->tracker.retrack(videoinput->frame, 2);
		//choosepoints();
	}
	else {
	    if (recording) {
			*commandoutputfile << totalframecount << " PAUSE" << endl;
		}
		
		tracker_status = STATUS_PAUSED;
		
		overlayimage = cvLoadImage("point-selection-frame.png", CV_LOAD_IMAGE_COLOR);
	    faces = FaceDetector::facedetector.detect(overlayimage);
	}
}


bool detect_nose(IplImage* img, double resolution, CvRect nose_rect, Point points[]){
	CvHaarClassifierCascade* cascade = 0;
	CvMemStorage* storage = 0;
	CvRect* nose = new CvRect();
	IplImage* nose_region_image;
	
	nose->width = 0;
	nose->height = 0;
	nose->x = 0;
	nose->y = 0;
		
	CvSize nose_size;
	
	if(resolution == 720) {
		nose_size = cvSize(36, 30);
	}
	else if(resolution == 1080) {
		nose_size = cvSize(54, 45);
	}
	else if(resolution == 480) {
		nose_size = cvSize(24, 20);
	}

	nose_region_image = cvCreateImage(cvSize(nose_rect.width, nose_rect.height),
	                               img->depth,
	                               img->nChannels);
	cvSetImageROI(img, nose_rect);
	cvCopy(img, nose_region_image);
	cvResetImageROI(img);

	//cvSaveImage("nose.png", nose_region_image);


	// Load the face detector and create empty memory storage
	char* file = "DetectorNose2.xml";
	cascade = (CvHaarClassifierCascade*) cvLoad(file, 0, 0, 0);
	storage = cvCreateMemStorage(0);
	
	if(cascade == NULL)
		cout << "CASCADE NOT LOADED" << endl;
	else
		cout << "Loaded" << endl;

	// Detect objects
	CvSeq* nose_seq = cvHaarDetectObjects(
					nose_region_image,
					cascade,
					storage,
					1.1,
					3,
					CV_HAAR_DO_CANNY_PRUNING,
					nose_size
					);
		
				cout << nose_seq->total << " NOSES DETECTED" << endl;
	// Return the first nose if any eye is detected
	if((nose_seq ? nose_seq->total : 0) > 0) {
		// If there are multiple matches, choose the one with larger area
		for(int i=0; i<nose_seq->total; i++) {
			CvRect *dummy = (CvRect*)cvGetSeqElem(nose_seq, i+1);

			if((dummy->width * dummy->height) > (nose->width*nose->height))
				nose = dummy;
		}
	}
	else {
		return false;
	}
	
	/*
	cvRectangle(
	  img,
	  cvPoint(nose->x, nose->y),
	  cvPoint(nose->x + nose->width, nose->y + nose->height),
	  CV_RGB(0, 255, 0),
	  2, 8, 0
	);
	*/
	
	points[0] = Point(nose_rect.x + nose->x + nose->width*0.33, nose_rect.y + nose->y + nose->height*0.6);
	points[1] = Point(nose_rect.x + nose->x + nose->width*0.67, nose_rect.y + nose->y + nose->height*0.6);
	
	return true;
}

bool detect_mouth(IplImage* img, double resolution, CvRect mouth_rect, Point points[]){
	CvHaarClassifierCascade* cascade = 0;
	CvMemStorage* storage = 0;
	CvRect* mouth = new CvRect();
	IplImage* mouth_region_image;
	
	mouth->width = 0;
	mouth->height = 0;
	mouth->x = 0;
	mouth->y = 0;
	
	CvSize mouth_size;
	
	if(resolution == 720) {
		mouth_size = cvSize(50, 30);
	}
	else if(resolution == 1080) {
		mouth_size = cvSize(74, 45);
	}
	else if(resolution == 480) {
		mouth_size = cvSize(25, 15);
	}

	mouth_region_image = cvCreateImage(cvSize(mouth_rect.width, mouth_rect.height),
	                               img->depth,
	                               img->nChannels);
	cvSetImageROI(img, mouth_rect);
	cvCopy(img, mouth_region_image);
	cvResetImageROI(img);

	//cvSaveImage("mouth.png", mouth_region_image);


	// Load the face detector and create empty memory storage
	char* file = "DetectorMouth.xml";
	cascade = (CvHaarClassifierCascade*) cvLoad(file, 0, 0, 0);
	storage = cvCreateMemStorage(0);
	
	if(cascade == NULL)
		cout << "CASCADE NOT LOADED" << endl;
	else
		cout << "Loaded" << endl;

	// Detect objects
	CvSeq* mouth_seq = cvHaarDetectObjects(
					mouth_region_image,
					cascade,
					storage,
					1.1,
					3,
					0, //CV_HAAR_DO_CANNY_PRUNING,
					mouth_size
					);
	
	cout << mouth_seq->total << " MOUTHS DETECTED" << endl;
		
	// Return the first mouth if any eye is detected
	if((mouth_seq ? mouth_seq->total : 0) > 0) {
		// If there are multiple matches, choose the one with larger area
		for(int i=0; i<mouth_seq->total; i++) {
			CvRect *dummy = (CvRect*)cvGetSeqElem(mouth_seq, i+1);
			
			if((dummy->width * dummy->height) > (mouth->width*mouth->height))
				mouth = dummy;
		}
	}
	else {
		return false;
	}
	
	/*
	cvRectangle(
	  img,
	  cvPoint(mouth->x, mouth->y),
	  cvPoint(mouth->x + mouth->width, mouth->y + mouth->height),
	  CV_RGB(0, 255, 0),
	  2, 8, 0
	);
	*/
	
	points[0] = Point(mouth_rect.x + mouth->x + mouth->width*0.1, mouth_rect.y + mouth->y + mouth->height*0.4);
	points[1] = Point(mouth_rect.x + mouth->x + mouth->width*0.9, mouth_rect.y + mouth->y + mouth->height*0.4);
	
	return true;
}

float calculateDistance(CvPoint2D32f pt1, CvPoint2D32f pt2 ) { 
    float dx = pt2.x - pt1.x; 
    float dy = pt2.y - pt1.y; 
 
    return cvSqrt( (float)(dx*dx + dy*dy)); 
}

void detect_eye_corners(IplImage* img, double resolution, Point points[]){
	CvHaarClassifierCascade* cascade = 0;
	CvMemStorage* storage = 0;
	IplImage* eye_region_image;
	IplImage* eye_region_image_gray;
	//IplImage* dummy;
	CvRect* both_eyes;
	//CvRect* right_eye;
	//CvRect* left_eye;
	
	CvSize both_eyes_size;
	CvSize single_eye_size;
	
	if(resolution == 720) {
		both_eyes_size = cvSize(100, 25);
		single_eye_size = cvSize(18, 12);
	}
	else if(resolution == 1080) {
		both_eyes_size = cvSize(150, 38);
		single_eye_size = cvSize(27, 18);
	}
	else if(resolution == 480) {
		both_eyes_size = cvSize(64, 16);
		single_eye_size = cvSize(6, 4);
	}
	
	// Load the face detector and create empty memory storage
	char* file = "DetectorEyes.xml";
	cascade = (CvHaarClassifierCascade*) cvLoad(file, 0, 0, 0);
	storage = cvCreateMemStorage(0);
	
	// Detect objects
	CvSeq* eye_regions = cvHaarDetectObjects(
					img,
					cascade,
					storage,
					1.1,
					10,
					CV_HAAR_DO_CANNY_PRUNING,
					both_eyes_size
					);
		
	cout << eye_regions->total << " eye regions detected" << endl;
				
	// Return the first eye if any eye is detected
	if((eye_regions ? eye_regions->total : 0) > 0) {
		both_eyes = (CvRect*)cvGetSeqElem(eye_regions, 1);
	}
	else {
		return;
	}
	
	cout << "Resolution: " << resolution << ", both eye reg.:" << both_eyes->width << ", " << both_eyes->height << endl;
	
	/*
	cvRectangle(
	  img,
	  cvPoint(both_eyes->x, both_eyes->y),
	  cvPoint(both_eyes->x + both_eyes->width, both_eyes->y + both_eyes->height),
	  CV_RGB(0, 255, 0),
	  2, 8, 0
	);
	*/
	
	int corner_count = 100;
	eye_region_image = cvCreateImage(cvSize(both_eyes->width, both_eyes->height),
	                               img->depth,
	                               img->nChannels);
	eye_region_image_gray = cvCreateImage(cvSize(both_eyes->width, both_eyes->height), 8, 1);
	
	CvRect left_rect = cvRect(both_eyes->x, both_eyes->y, both_eyes->width, both_eyes->height);	
	cvSetImageROI(img, left_rect);
	cvCopy(img, eye_region_image);
    cvResetImageROI(img);

	cvCvtColor(eye_region_image, eye_region_image_gray, CV_RGB2GRAY);
	
	normalizeGrayScaleImage(eye_region_image_gray);

	CvPoint2D32f* corners = detect_corners_in_grayscale(eye_region_image_gray, corner_count);

	int left_eye_corners_x_sum = 0;
	int left_eye_corners_y_sum = 0;
	int left_eye_corners_count = 0;
	
	int right_eye_corners_x_sum = 0;
	int right_eye_corners_y_sum = 0;
	int right_eye_corners_count = 0;
	
	/// Drawing a circle around corners
  	for( int j = 0; j < corner_count; j++ )
	{
		if(corners[j].x < both_eyes->width*0.4) {
			left_eye_corners_x_sum += corners[j].x;
			left_eye_corners_y_sum += corners[j].y;
			left_eye_corners_count++;
			cvCircle(eye_region_image, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
		}
		else if(corners[j].x > both_eyes->width*0.6){
			right_eye_corners_x_sum += corners[j].x;
			right_eye_corners_y_sum += corners[j].y;
			right_eye_corners_count++;
			cvCircle(eye_region_image, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
		}
	}
	
	double left_eye_center_x = both_eyes->x + (left_eye_corners_x_sum / (double) left_eye_corners_count);
	double left_eye_center_y = both_eyes->y + (left_eye_corners_y_sum / (double) left_eye_corners_count);
	
	double right_eye_center_x = both_eyes->x + (right_eye_corners_x_sum / (double) right_eye_corners_count);
	double right_eye_center_y = both_eyes->y + (right_eye_corners_y_sum / (double) right_eye_corners_count);
	
	double x_diff = right_eye_center_x - left_eye_center_x;
	double y_diff = right_eye_center_y - left_eye_center_y;
	
	//points[0] = Point(both_eyes->x + left_eye_corners_x_sum, both_eyes->y + left_eye_corners_y_sum);
	//points[1] = Point(both_eyes->x + right_eye_corners_x_sum, both_eyes->y + right_eye_corners_y_sum);
	points[0] = Point(left_eye_center_x - 0.29*x_diff, left_eye_center_y - 0.29*y_diff);// + x_diff/40);
	points[1] = Point(right_eye_center_x + 0.29*x_diff, right_eye_center_y + 0.29*y_diff);// + x_diff/40);

	
	/// Drawing a circle around corners
  	//for( int j = 0; j < corner_count; j++ )
	//{ 
	 //   cvCircle(eye_region_image, cvPoint(corners[j].x, corners[j].y), 3, CV_RGB(255,0,0), -1, 8,0);
//	}
	
	cvCircle(eye_region_image, cvPoint(points[0].x, points[0].y), 3, CV_RGB(0,255,0), -1, 8,0);
	cvCircle(eye_region_image, cvPoint(points[1].x, points[1].y), 3, CV_RGB(0,255,0), -1, 8,0);
	
	//cvSaveImage("eye_corners.png", eye_region_image);
	//cvSaveImage((base_path.substr(0, base_path.length() - 4) + "_eye_corners.png").c_str(), eye_region_image);
}

void detect_eyebrow_corners(IplImage* img, double resolution, CvRect eyebrow_rect, Point points[]){
	IplImage* eyebrow_region_image;
	IplImage* eyebrow_region_image_gray;
	IplImage* eyebrow_region_image_2;
	IplImage* eyebrow_region_image_gray_2;

	eyebrow_rect.width = eyebrow_rect.width/2;
	eyebrow_region_image = cvCreateImage(cvSize(eyebrow_rect.width, eyebrow_rect.height),
	                               img->depth,
	                               img->nChannels);
	eyebrow_region_image_gray = cvCreateImage(cvSize(eyebrow_rect.width, eyebrow_rect.height), 8, 1);
	
	eyebrow_region_image_2 = cvCreateImage(cvSize(eyebrow_rect.width, eyebrow_rect.height),
	                               img->depth,
	                               img->nChannels);
	eyebrow_region_image_gray_2 = cvCreateImage(cvSize(eyebrow_rect.width, eyebrow_rect.height), 8, 1);
	
	cout << "EYEBROW x, y = " << eyebrow_rect.x << " - " << eyebrow_rect.y << " width, height =" << eyebrow_rect.width << " - " << eyebrow_rect.height << endl;
	cvSetImageROI(img, eyebrow_rect);
	cvCopy(img, eyebrow_region_image);
	
	cout << "Copied first" << endl;
	
	CvRect eyebrow_rect_2 = cvRect(eyebrow_rect.x + eyebrow_rect.width, eyebrow_rect.y, eyebrow_rect.width, eyebrow_rect.height);
	cvSetImageROI(img, eyebrow_rect_2);
	cvCopy(img, eyebrow_region_image_2);
	cvResetImageROI(img);
	
	//cvSaveImage("eyebrows.png", eyebrow_region_image);

    cvCvtColor(eyebrow_region_image, eyebrow_region_image_gray, CV_RGB2GRAY);
    cvCvtColor(eyebrow_region_image_2, eyebrow_region_image_gray_2, CV_RGB2GRAY);

	int corner_count = 1;
	CvPoint2D32f* corners = detect_corners_in_grayscale(eyebrow_region_image_gray, corner_count);
	
	corner_count = 1;
	CvPoint2D32f* corners_2 = detect_corners_in_grayscale(eyebrow_region_image_gray_2, corner_count);

	points[0] = Point(eyebrow_rect.x + corners[0].x, eyebrow_rect.y + corners[0].y);
	points[1] = Point(eyebrow_rect_2.x + corners_2[0].x, eyebrow_rect_2.y + corners_2[0].y);
	
	cout << "Finished eyebrows" << endl;
}


CvPoint2D32f* detect_corners_in_grayscale(IplImage* eye_region_image_gray, int& corner_count) {
	IplImage* eig_image = 0;
	IplImage* temp_image = 0;

	eig_image = cvCreateImage(cvSize(eye_region_image_gray->width, eye_region_image_gray->height), IPL_DEPTH_32F, 1);
	temp_image = cvCreateImage(cvSize(eye_region_image_gray->width, eye_region_image_gray->height), IPL_DEPTH_32F, 1);
			
	CvPoint2D32f* corners = new CvPoint2D32f[corner_count];
	double quality_level = 0.01;
	double min_distance = 2;
	int eig_block_size = 3;
	int use_harris = false;

	cvGoodFeaturesToTrack(eye_region_image_gray,
			eig_image,                    // output
			temp_image,
			corners,
			&corner_count,
			quality_level,
			min_distance,
			NULL,
			eig_block_size,
			use_harris);
			
	return corners;
}

void check_rect_size(IplImage* image, CvRect* rect) {
	if(rect->x < 0) {
		rect->x = 0;
	}
	
	if(rect->y < 0) {
		rect->y = 0;
	}
	
	if(rect->x + rect->width >= image->width) {
		rect->width = image->width - rect->x - 1;
	}

	if(rect->y + rect->height >= image->height) {
		rect->height = image->height - rect->y - 1;
	}
}


void MainGazeTracker::extract_face_region_rectangle(IplImage* frame, vector<Point> feature_points) {
	int min_x = 10000;
	int max_x = 0;
	int min_y = 10000;
	int max_y = 0;
	
	// Find the boundaries of the feature points
	for(int i=0; i< (int) feature_points.size(); i++) {
		min_x = feature_points[i].x < min_x ? feature_points[i].x : min_x;
		min_y = feature_points[i].y < min_y ? feature_points[i].y : min_y;
		max_x = feature_points[i].x > max_x ? feature_points[i].x : max_x;
		max_y = feature_points[i].y > max_y ? feature_points[i].y : max_y;
	}
	
	int dif_x = max_x - min_x;
	int dif_y = max_y - min_y;
	
	min_x -= 0.4 * dif_x;
	max_x += 0.4 * dif_x;
	min_y -= 0.5 * dif_y;
	max_y += 0.5 * dif_y;
	
	face_rectangle = new CvRect();
	face_rectangle->x = min_x;
	face_rectangle->y = min_y;
	face_rectangle->width = max_x - min_x;
	face_rectangle->height = max_y - min_y;
}
