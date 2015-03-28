#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "utils.h"

namespace Utils {
	boost::shared_ptr<IplImage> createImage(const CvSize &size, int depth, int channels) {
		return boost::shared_ptr<IplImage>(cvCreateImage(size, depth, channels), releaseImage);
	}

	void releaseImage(IplImage *image) {
		//cout << "deleting shared image" << endl;
		cvReleaseImage(&image);
	}

	void mapToFirstMonitorCoordinates(Point monitor2Point, Point &monitor1Point) {
		int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
		Gdk::Rectangle monitor1Geometry;
		Gdk::Rectangle monitor2Geometry;
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

		screen->get_monitor_geometry(0, monitor1Geometry);
		screen->get_monitor_geometry(numMonitors - 1, monitor2Geometry);

		monitor1Point.x = (monitor2Point.x / monitor2Geometry.get_width()) * (monitor1Geometry.get_width() - 40) + monitor1Geometry.get_x();
		monitor1Point.y = (monitor2Point.y / monitor2Geometry.get_height()) * monitor1Geometry.get_height() + monitor1Geometry.get_y();
	}


	void mapToVideoCoordinates(Point monitor2Point, double resolution, Point &videoPoint, bool reverseX) {
		int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
		Gdk::Rectangle monitor1Geometry(0, 0, 1280, 720);
		Gdk::Rectangle monitor2Geometry;
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

		screen->get_monitor_geometry(numMonitors - 1, monitor2Geometry);

		if (resolution == 480) {
			monitor1Geometry.set_width(640);
			monitor1Geometry.set_height(480);
		} else if (resolution == 1080) {
			monitor1Geometry.set_width(1920);
			monitor1Geometry.set_height(1080);
		}

		if (reverseX) {
			videoPoint.x = monitor1Geometry.get_width() - (monitor2Point.x / monitor2Geometry.get_width()) * monitor1Geometry.get_width();
		} else {
			videoPoint.x = (monitor2Point.x / monitor2Geometry.get_width()) * monitor1Geometry.get_width();
		}

		videoPoint.y = (monitor2Point.y / monitor2Geometry.get_height()) * monitor1Geometry.get_height();
	}

	// Neural network
	void mapToNeuralNetworkCoordinates(Point point, Point &nnPoint) {
		int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
		Gdk::Rectangle monitor1Geometry(0, 0, 1, 1);
		Gdk::Rectangle monitor2Geometry;
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

		screen->get_monitor_geometry(numMonitors - 1, monitor2Geometry);

		nnPoint.x = ((point.x - monitor2Geometry.get_x()) / monitor2Geometry.get_width()) * monitor1Geometry.get_width() + monitor1Geometry.get_x();
		nnPoint.y = ((point.y - monitor2Geometry.get_y()) / monitor2Geometry.get_height()) * monitor1Geometry.get_height() + monitor1Geometry.get_y();

		//cout << "ORIG: " << point.x << ", " << point.y << " MAP: " << nnPoint.x << ", " << nnPoint.y << endl;
	}


	void mapFromNeuralNetworkToScreenCoordinates(Point nnPoint, Point &point) {
		int numMonitors = Gdk::Screen::get_default()->get_n_monitors();
		Gdk::Rectangle monitor1Geometry;
		Gdk::Rectangle monitor2Geometry(0, 0, 1, 1);
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();

		// Geometry of main monitor
		screen->get_monitor_geometry(numMonitors - 1, monitor1Geometry);

		point.x = ((nnPoint.x - monitor2Geometry.get_x()) / monitor2Geometry.get_width()) * monitor1Geometry.get_width() + monitor1Geometry.get_x();
		point.y = ((nnPoint.y - monitor2Geometry.get_y()) / monitor2Geometry.get_height()) * monitor1Geometry.get_height() + monitor1Geometry.get_y();

		//cout << "ORIG: " << point.x << ", " << point.y << " MAP: " << nnPoint.x << ", " << nnPoint.y << endl;
	}

	std::string getUniqueFileName(std::string directory, std::string baseFileName) {
		std::string fileAbsName;
		boost::filesystem::path currentDir(directory);
		int maximumExistingNumber = 0;
		//boost::regex pattern(base_file_name + ".*"); // list all files having this base file name

		// Check all the files matching this base file name and find the maximum serial number until now
		for (boost::filesystem::directory_iterator iter(currentDir),end; iter != end; ++iter) {
			std::string name = iter->path().filename().string();
			if (strncmp(baseFileName.c_str(), name.c_str(), baseFileName.length()) == 0) { // regex_match(name, pattern)) {
				//cout << "MATCH: base=" << baseFileName << ", file=" << name << endl;
				int currentNumber = 0;
				name = name.substr(baseFileName.length() + 1);

				//cout << "After substr=" << name << endl;
				sscanf(name.c_str(), "%d", &currentNumber);
				//cout << "NO= " << currentNumber << endl;

				if (currentNumber > maximumExistingNumber) {
					maximumExistingNumber = currentNumber;
				}
			}
		}

		//cout << "Max. existing no = " << maximum_existing_no << endl;

		// Return the next serial number
		return directory + "/" + baseFileName +  "_" + boost::lexical_cast<std::string>(maximumExistingNumber + 1) + ".txt";
	}

	// Normalize by making mean and standard deviation equal in all images
	void normalizeGrayScaleImage(IplImage *image, double standardMean, double standardStd) {
		CvScalar scalarMean;
		CvScalar scalarStd;
		double mean;
		double std;

		cvAvgSdv(image, &scalarMean, &scalarStd);

		mean = scalarMean.val[0];
		std = scalarStd.val[0];

		//cout << "Image mean and std is " << mean << ", " << std << endl;

		double ratio = standardStd / std;
		double shift = standardMean - mean * ratio;

		cvConvertScale(image, image, ratio, shift);   // Move the mean from 0 to 127
	}

	// Normalize to 50-200 interval
	void normalizeGrayScaleImage2(IplImage *image, double standardMean, double standardStd) {
		double minVal;
		double maxVal;
		double scale = 1;
		double intervalStart = 25;
		double intervalEnd = 230;

		cvMinMaxLoc(image, &minVal, &maxVal);
		cvConvertScale(image, image, 1, -1 * minVal);   // Subtract the minimum value

		// If pixel intensities are between 0 and 1
		if (maxVal < 2) {
			intervalStart = intervalStart / 255.0;
			intervalEnd = intervalEnd / 255.0;
		}

		scale = (intervalEnd - intervalStart) / (maxVal - minVal);

		// Scale the image to the selected interval
		cvConvertScale(image, image, scale, 0);
		cvConvertScale(image, image, 1, intervalStart);
	}

	void printMat(CvMat *mat) {
		printf("(%dx%d)\n", mat->cols, mat->rows);
		for (int i = 0; i < mat->rows; i++) {
			if (i == 0) {
				for (int j = 0; j < mat->cols; j++) {
					printf("%10d", j + 1);
				}
			}

			printf("\n%4d: ", i + 1);
			for (int j = 0; j < mat->cols; j++) {
				printf("%10.2f", cvGet2D(mat, i, j).val[0]);
			}
		}

		printf("\n");
	}
}

namespace boost {
	template <> void checked_delete(IplImage *image) {
		//cout << "deleting scoped image" << endl;
		if (image) {
			cvReleaseImage(&image);
		}
	}
}
