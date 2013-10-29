#include "utils.h"

int tracker_status = STATUS_IDLE;
bool is_tracker_calibrated = false;
int dwelltime_parameter = 20;
int test_dwelltime_parameter = 20;
int sleep_parameter = 0;
CvRect* face_rectangle = NULL;

void releaseImage(IplImage *image) {
//     cout << "deleting shared image" << endl;
    cvReleaseImage(&image);
}

shared_ptr<IplImage> createImage(const CvSize &size, int depth, int channels) {
    return shared_ptr<IplImage>(cvCreateImage(size, depth, channels),
				releaseImage);
}



void mapToFirstMonitorCoordinates(Point monitor2point, Point& monitor1point)
{	
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitor1geometry;
	Gdk::Rectangle monitor2geometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	
	screen->get_monitor_geometry(0, monitor1geometry);
	screen->get_monitor_geometry(num_of_monitors - 1, monitor2geometry);
	
	monitor1point.x = (monitor2point.x / monitor2geometry.get_width()) * (monitor1geometry.get_width() - 40) + monitor1geometry.get_x();
	monitor1point.y = (monitor2point.y / monitor2geometry.get_height()) * monitor1geometry.get_height() + monitor1geometry.get_y();
}


void mapToVideoCoordinates(Point monitor2point, double resolution, Point& videoPoint, bool reverse_x)
{	
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitor1geometry(0, 0, 1280, 720);
	Gdk::Rectangle monitor2geometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	
	screen->get_monitor_geometry(num_of_monitors - 1, monitor2geometry);
	
	if(resolution == 480) {
		monitor1geometry.set_width(640);
		monitor1geometry.set_height(480);
	}
	else if(resolution == 1080) {
		monitor1geometry.set_width(1920);
		monitor1geometry.set_height(1080);
	}
	
	if(reverse_x) {
		videoPoint.x = monitor1geometry.get_width() - (monitor2point.x / monitor2geometry.get_width()) * monitor1geometry.get_width();	
	}
	else {
		videoPoint.x = (monitor2point.x / monitor2geometry.get_width()) * monitor1geometry.get_width();	
	}
	
	videoPoint.y = (monitor2point.y / monitor2geometry.get_height()) * monitor1geometry.get_height();
}

// Neural network
void mapToNeuralNetworkCoordinates(Point point, Point& nnpoint)
{	
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitor1geometry(0, 0, 1, 1);
	Gdk::Rectangle monitor2geometry;
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	
	screen->get_monitor_geometry(num_of_monitors - 1, monitor2geometry);
	
	nnpoint.x = ((point.x - monitor2geometry.get_x()) / monitor2geometry.get_width()) * monitor1geometry.get_width() + monitor1geometry.get_x();
	
	nnpoint.y = ((point.y - monitor2geometry.get_y()) / monitor2geometry.get_height()) * monitor1geometry.get_height() + monitor1geometry.get_y();
	
	//cout << "ORIG: " << point.x << ", " << point.y << " MAP: " << nnpoint.x << ", " << nnpoint.y << endl;
}


void mapFromNeuralNetworkToScreenCoordinates(Point nnpoint, Point& point)
{	
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	Gdk::Rectangle monitor1geometry;
	Gdk::Rectangle monitor2geometry(0, 0, 1, 1);
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	
	// Geometry of main monitor
	screen->get_monitor_geometry(num_of_monitors - 1, monitor1geometry);

	
	point.x = ((nnpoint.x - monitor2geometry.get_x()) / monitor2geometry.get_width()) * monitor1geometry.get_width() + monitor1geometry.get_x();
	
	point.y = ((nnpoint.y - monitor2geometry.get_y()) / monitor2geometry.get_height()) * monitor1geometry.get_height() + monitor1geometry.get_y();
	
	//cout << "ORIG: " << point.x << ", " << point.y << " MAP: " << nnpoint.x << ", " << nnpoint.y << endl;
}

string getUniqueFileName(string directory, string base_file_name)
{
	string file_abs_name;
	boost::filesystem::path current_dir(directory);
	int maximum_existing_no = 0;
	//boost::regex pattern(base_file_name + ".*"); // list all files having this base file name

	// Check all the files matching this base file name and find the maximum serial number until now
	for (boost::filesystem::directory_iterator iter(current_dir),end; iter!=end; ++iter) {
		string name = iter->path().filename().string();
		if (strncmp(base_file_name.c_str(), name.c_str(), base_file_name.length()) == 0) {// regex_match(name, pattern)) {
			
			//cout << "MATCH: base=" << base_file_name << ", file=" << name << endl;
			int current_no = 0;
			name = name.substr(base_file_name.length() + 1);
			
			//cout << "After substr=" << name << endl;
			sscanf(name.c_str(), "%d", &current_no);
			//cout << "NO= " << current_no << endl;
			
			if(current_no > maximum_existing_no) {
				maximum_existing_no = current_no;
			}
		}
	}
	
	//cout << "Max. existing no = " << maximum_existing_no << endl;
	
	// Return the next serial number
	return directory + "/" + base_file_name +  "_" + boost::lexical_cast<std::string>(maximum_existing_no + 1) + ".txt";
}

namespace boost {
    template<>
    void checked_delete(IplImage *image) {
// 	cout << "deleting scoped image" << endl;
	if (image)
	    cvReleaseImage(&image);
    }
}



// Normalize to 50-200 interval
void normalizeGrayScaleImage2(IplImage *image, double standard_mean, double standard_std) {
    double minVal, maxVal;
    double scale = 1;
    
    double interval_start = 25;
    double interval_end = 230;
    
    cvMinMaxLoc(image, &minVal, &maxVal);
    cvConvertScale(image, image, 1, -1 * minVal);   // Subtract the minimum value

    // If pixel intensities are between 0 and 1
    if(maxVal < 2) {
		interval_start = interval_start/255.0;
		interval_end = interval_end/255.0;
    }
    
    scale = (interval_end-interval_start) / (maxVal - minVal);  
    
    // Scale the image to the selected interval
    cvConvertScale(image, image, scale, 0);
    cvConvertScale(image, image, 1, interval_start);
}

// Normalize by making mean and standard deviation equal in all images
void normalizeGrayScaleImage(IplImage *image, double standard_mean, double standard_std) {
    CvScalar scalar_mean, scalar_std;
    double mean, std;

    cvAvgSdv(image, &scalar_mean, &scalar_std);
    
    mean = scalar_mean.val[0];
    std = scalar_std.val[0];
    
    //cout << "Image mean and std is " << mean << ", " << std << endl;
        
	double ratio = standard_std/std;
	double shift = standard_mean - mean*ratio;

    cvConvertScale(image, image, ratio, shift);   // Move the mean from 0 to 127
}


