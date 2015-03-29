#include <opencv/highgui.h>
#include <sys/time.h>

#include "GameWindow.h"
#include "Application.h"
#include "utils.h"

namespace {
	cv::Scalar backgroundColor2(255, 255, 255);

	bool buttonEvents() {
		static int mode = 1;

		//backgroundColor2 = mode == 1 ? CV_RGB(0, 0, 0) : CV_RGB(255, 255, 255);
		//mode = 1 - mode;

		return true;
	}
}

GameArea::GameArea(TrackerOutput *output):
	_output(output), 
	_backgroundColor(153, 75, 75)
{
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	Gdk::Rectangle rect;
	screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);

	set_size_request(rect.get_width(), rect.get_height());

	//this->move(0, 0);
	Glib::signal_idle().connect(sigc::mem_fun(*this, &GameArea::onIdle));

	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);

	Glib::RefPtr<Gdk::Window> window = get_window();
	//this->signal_key_press_event().connect(sigc::mem_fun(*this, &GameArea::onIdle));

	_origImage = cv::imread("./background_full.png", CV_LOAD_IMAGE_COLOR);
	cvtColor(_origImage, _origImage, CV_RGB2BGR);

	_frog.create(cv::Size(180, 180), CV_8UC3);
	_target.create(cv::Size(50, 50), CV_8UC3);

	_background.create(cv::Size(rect.get_width(), rect.get_height()), CV_8UC3);
	_current.create(cv::Size(_background.size().width, _background.size().height), CV_8UC3);
	//_black.create(cv::Size(_background.size().width, _background.size().height), CV_8UC3);
	_clearingImage.create(cv::Size(2000, 1500), CV_8UC3);

	std::cout << "IMAGES CREATED WITH SIZE: " << _background.size().width << "x" << _background.size().height << std::endl;

	_background.setTo(cv::Scalar(0,0,0));

	//_background.setTo(_backgroundColor);
	//_black.setTo(backgroundColor2);
	
	// Clearing image is filled with white
	//_clearingImage.setTo(backgroundColor2);
	_clearingImage.setTo(cv::Scalar(255, 255, 255));

	_gameAreaX = (rect.get_width() - _origImage.size().width) / 2;
	_gameAreaY = (rect.get_height() - _origImage.size().height) / 2;
	_gameAreaWidth = _origImage.size().width;
	_gameAreaHeight = _origImage.size().height;

	_frog = cv::imread("./frog.png", CV_LOAD_IMAGE_COLOR);
	cvtColor(_frog, _frog, CV_RGB2BGR);

	_frogMask = cv::imread("./frog-mask.png", CV_LOAD_IMAGE_COLOR);
	_gaussianMask = cv::imread("./gaussian-mask.png", CV_LOAD_IMAGE_COLOR);

	_target = cv::imread("./target.png", CV_LOAD_IMAGE_COLOR);

	_frogCounter = 0;
	calculateNewFrogPosition();

	timeval time;
	gettimeofday(&time, NULL);
	_futureTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	_futureTime += 1000 * 20000;

	_startTime = _futureTime;

	_calibrationPointer = NULL;

	_lastUpdatedRegion.x = 0;
	_lastUpdatedRegion.y = 0;
	_lastUpdatedRegion.width = rect.get_width();// - 54;
	_lastUpdatedRegion.height = rect.get_height();// - 24;

	_isWindowInitialized = false;
}

GameArea::~GameArea() {}

void GameArea::showContents() {
	static double estimationXRight = 0;
	static double estimationYRight = 0;
	static double estimationXLeft = 0;
	static double estimationYLeft = 0;

	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		if (Application::status == Application::STATUS_PAUSED) {
			std::cout << "PAUSED, DRAWING HERE" << std::endl;

			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
			cv::Rect bounds = cv::Rect((_background.size().width - _repositioningImage->size().width) / 2, (_background.size().height - _repositioningImage->size().height) / 2, _repositioningImage->size().width, _repositioningImage->size().height);

			Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
				(guint8 *)_repositioningImage->data,
				Gdk::COLORSPACE_RGB,
				false,
				8,
				_repositioningImage->size().width,
				_repositioningImage->size().height,
				(int) _repositioningImage->step[0]
			);

			window->draw_pixbuf(gc, pixbuf, 0, 0, bounds.x, bounds.y, bounds.width, bounds.height, Gdk::RGB_DITHER_NONE, 0, 0);

			_lastUpdatedRegion.x = bounds.x;
			_lastUpdatedRegion.y = bounds.y;
			_lastUpdatedRegion.width = bounds.width;
			_lastUpdatedRegion.height = bounds.height;
		}

#ifdef EXPERIMENT_MODE
		else if (false) {		// For the experiment mode, never show the game
#else
		else if(Application::status == Application::STATUS_CALIBRATED) {
#endif
			//Gtk::Allocation allocation = get_allocation();
			const int width = _background.size().width;
			const int height = _background.size().height;

			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);

			double alpha = 0.6;
			estimationXRight = (1 - alpha) * _output->gazePoint.x + alpha * estimationXRight;
			estimationYRight = (1 - alpha) * _output->gazePoint.y + alpha * estimationYRight;
			estimationXLeft = (1 - alpha) * _output->gazePointLeft.x + alpha * estimationXLeft;
			estimationYLeft = (1 - alpha) * _output->gazePointLeft.y + alpha * estimationYLeft;

			int estimationX = (estimationXRight + estimationXLeft) / 2;
			int estimationY = (estimationYRight + estimationYLeft) / 2;

			//int estimationX = output->gazePoint.x;
			//int estimationY = output->gazePoint.y;
			//std::cout << "INIT EST: " << estimationX << ", " << estimationY << std::endl;

			// Map estimation to window coordinates
			int windowX, windowY;

			//window->get_geometry(windowX, windowY, dummy, dummy, dummy);
			//GtkWidget *topWindow;
			//gint x,y;
			//topWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

			//gdk_window_get_position(topWindow->window, &x, &y);
			//std::cout << "WINDOW POSITION: " << x << ", " << y << std::endl;
			//estimationX -= windowX;
			//estimationY -= windowY;

			//// REMOVE
			//estimationX = 800;
			//estimationY = 500;

			//std::cout << "EST: " << estimationX << ", " << estimationY << std::endl;
			//std::cout << "separately: (" << output->gazepoint.x << ", " << output->gazepoint.y << ") and (" << output->gazepoint_left.x << ", " << output->gazepoint_left.y << ")" << std::endl;
			//std::cout << "  --errors: (" << output->nnGazePoint.x << ", " << output->nnGazePoint.y << ") and (" << output->nnGazePointLeft.x << ", " << output->nnGazePointLeft.y << ")" << std::endl;

			//_black.setTo(backgroundColor2);

			cv::Rect bounds = cv::Rect(estimationX - 100, estimationY - 100, 200, 200);
			cv::Rect gaussianBounds = cv::Rect(0, 0, 200, 200);

			if (bounds.x < 0) {
				bounds.width += bounds.x;		// Remove the amount from the width
				gaussianBounds.x -= bounds.x;
				gaussianBounds.width += bounds.x;
				bounds.x = 0;
			}

			if (bounds.y < 0) {
				bounds.height += bounds.y;		// Remove the amount from the height
				gaussianBounds.y -= bounds.y;
				gaussianBounds.height += bounds.y;
				bounds.y = 0;
			}

			if (bounds.width + bounds.x > _background.size().width) {
				bounds.width = _background.size().width - bounds.x;
			}

			if (bounds.height + bounds.y > _background.size().height) {
				bounds.height = _background.size().height - bounds.y;
			}
			gaussianBounds.width = bounds.width;
			gaussianBounds.height = bounds.height;

			if (estimationX <= 0) {
				estimationX = 1;
			}

			if (estimationY <= 0) {
				estimationY = 1;
			}

			if (estimationX >= _background.size().width) {
				estimationX = _background.size().width -1;
			}
			if (estimationY >= _background.size().height) {
				estimationY = _background.size().height -1;
			}
			
			//_black.copyTo(_current);
			_current.setTo(backgroundColor2);

			//std::cout << "Bounds: " << bounds.x << ", " << bounds.y << "," << bounds.width << ", " << bounds.height << std::endl;
			//std::cout << "Gaussian Bounds: " << gaussianBounds.x << ", " << gaussianBounds.y << "," << gaussianBounds.width << ", " << gaussianBounds.height << std::endl;

			if (bounds.width > 0 && bounds.height > 0) {
				if (estimationX != 0 || estimationY != 0) {
					_background(bounds).copyTo(_current(bounds), _gaussianMask(gaussianBounds));
				}
			}

			clearLastUpdatedRegion();

			// Draw only the region which is to be updated
			Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
				(guint8 *)_current.data,
				Gdk::COLORSPACE_RGB,
				false,
				8,
				_current.size().width,
				_current.size().height,
				(int) _current.step[0]
			);

			window->draw_pixbuf(gc, pixbuf, bounds.x, bounds.y, bounds.x, bounds.y, bounds.width, bounds.height, Gdk::RGB_DITHER_NONE, 0, 0);

			_lastUpdatedRegion.x = bounds.x;
			_lastUpdatedRegion.y = bounds.y;
			_lastUpdatedRegion.width = bounds.width;
			_lastUpdatedRegion.height = bounds.height;

			int diff = ((estimationX - _frogX) * (estimationX - _frogX)) + ((estimationY - _frogY) * (estimationY - _frogY));
			if (diff < 35000) {	// If less than 150 pix, count
				//std::cout << "Difference is fine!" << std::endl;
				timeval time;
				gettimeofday(&time, NULL);
				_tempTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);

				if (_startTime == _futureTime) {
					_startTime = _tempTime;
				} else if (_tempTime - _startTime > 1500) {	// If fixes on the same point for 3 seconds
					_frogCounter++;
					calculateNewFrogPosition();
					_startTime = _futureTime;
				}
			} else {	// If cannot focus for a while, reset
				_startTime = _futureTime;
			}
		} else if (!_isWindowInitialized) {
			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);

			Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
				(guint8 *)_clearingImage.data,
				Gdk::COLORSPACE_RGB,
				false,
				8,
				_clearingImage.size().width,
				_clearingImage.size().height,
				(int) _clearingImage.step[0]
			);

			window->draw_pixbuf(gc, pixbuf, 0, 0, 0, 0, 1920, 1080, Gdk::RGB_DITHER_NONE , 0, 0);

			_isWindowInitialized = true;
		} else if (_calibrationPointer != NULL){	// Calibration
			const int width = _background.size().width;
			const int height = _background.size().height;
			cv::Rect currentlyUpdatedRegion = cv::Rect(0, 0, 0, 0);
			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
			Point calibrationPoint = _calibrationPointer->getPosition();

			//if (Application::status == Application::STATUS_TESTING) {
			//	std::cout << "EXPECTED OUTPUT: ("<< calibrationPoint.x << ", " << calibrationPoint.y << ")" << std::endl << std::endl;
			//}

			// If the coordinates are beyond bounds, calibration is finished
			if (calibrationPoint.x > 3000 || calibrationPoint.y > 3000) {
				_calibrationPointer = NULL;
				return;
			}

			if (calibrationPoint.x > 0 && calibrationPoint.y > 0) {
				cv::Rect currentBounds = cv::Rect(calibrationPoint.x - 25, calibrationPoint.y - 25, 50, 50);
				cv::Rect targetBounds = cv::Rect(0, 0, 50, 50);

				if (currentBounds.x < 0) {
					currentBounds.width += currentBounds.x;		// Remove the amount from the width
					targetBounds.x -= currentBounds.x;
					targetBounds.width += currentBounds.x;
					currentBounds.x = 0;
				}

				if (currentBounds.y < 0) {
					currentBounds.height += currentBounds.y;		// Remove the amount from the height
					targetBounds.y -= currentBounds.y;
					targetBounds.height += currentBounds.y;
					currentBounds.y = 0;
				}

				if (currentBounds.width + currentBounds.x > _background.size().width) {
					currentBounds.width = _background.size().width - currentBounds.x;
				}

				if (currentBounds.height + currentBounds.y > _background.size().height) {
					currentBounds.height = _background.size().height - currentBounds.y;
				}
				
				// Clear only previously updated region
				clearLastUpdatedRegion();

				// Draw only the region which is to be updated
				Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
					(guint8 *)_target.data,
					Gdk::COLORSPACE_RGB,
					false,
					8,
					_target.size().width,
					_target.size().height,
					(int) _target.step[0]
				);

				window->draw_pixbuf(gc, pixbuf, targetBounds.x,targetBounds.y, currentBounds.x, currentBounds.y, targetBounds.width, targetBounds.height, Gdk::RGB_DITHER_NONE, 0, 0);

				_lastUpdatedRegion.x = currentBounds.x;
				_lastUpdatedRegion.y = currentBounds.y;
				_lastUpdatedRegion.width = targetBounds.width;
				_lastUpdatedRegion.height = targetBounds.height;
			}
		} else {
			clearLastUpdatedRegion();
		}
	}
}

void GameArea::calculateNewFrogPosition() {
	//static int frogXS[] = {350, 640, 640, 350};
	//static int frogYS[] = {680, 95, 680, 680};
	//static int counter = 0;

	//_frogX = frogXS[counter];
	//_frogY = frogYS[counter];

	//counter++;

	_frogX = rand() % (_gameAreaWidth - 90) + _gameAreaX + 90;
	_frogY = rand() % (_gameAreaHeight - 90) + _gameAreaY + 90;

	std::cout << "Preparing bg image" << std::endl;

	// Copy the initial background image to "background"
	_background.setTo(cv::Scalar(0,0,0));
	_origImage.copyTo(_background(cv::Rect(_gameAreaX, _gameAreaY, _gameAreaWidth, _gameAreaHeight)));
	std::cout << "Original image copied" << std::endl;

	// Copy the frog image to the background
	_frog.copyTo(_background(cv::Rect(_frogX - 90, _frogY - 90, 180, 180)), _frogMask);
	std::cout << "Background prepared" << std::endl;
}

void GameArea::clearLastUpdatedRegion() {
	if (_lastUpdatedRegion.width > 0) {
		Glib::RefPtr<Gdk::Window> window = get_window();
		Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);

		Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
			(guint8 *)_clearingImage.data,
			Gdk::COLORSPACE_RGB,
			false,
			8,
			_clearingImage.size().width,
			_clearingImage.size().height,
			(int) _clearingImage.step[0]
		);

		window->draw_pixbuf(gc, pixbuf, 0,0, _lastUpdatedRegion.x, _lastUpdatedRegion.y, _lastUpdatedRegion.width, _lastUpdatedRegion.height, Gdk::RGB_DITHER_NORMAL , 0, 0);

		//std::cout << "CLEARING THE AREA: " << _lastUpdatedRegion.x << ", " << _lastUpdatedRegion.y << ", " << _lastUpdatedRegion.width << ", " << _lastUpdatedRegion.height << "." << std::endl;

		_lastUpdatedRegion.x = 0;
		_lastUpdatedRegion.y = 0;
		_lastUpdatedRegion.width = 0;
		_lastUpdatedRegion.height = 0;
	}
}

bool GameArea::onIdle() {
	showContents();
	//queue_draw();
	return true;
}


bool GameArea::on_expose_event(GdkEventExpose *event) {
	//showContents();
	return true;
}

bool GameArea::on_button_press_event(GdkEventButton *event) {
	return buttonEvents();
}

bool GameArea::on_button_release_event(GdkEventButton *event) {
	return true;
}

GameWindow::GameWindow(TrackerOutput *output) :
	_picture(output)
{
	try {
		set_title("Game Window");

		// Center window
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
		Gdk::Rectangle rect;
		screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);

		//this->set_keep_above(true);
		// Set tracker output
		add(_vbox);
		_vbox.pack_start(_picture);
		//_vbox.pack_start(buttonBar);

		//_buttonbar.pack_start(_calibrateButton);

		//_calibrateButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::startCalibration));

		_picture.show();
		//_calibrateButton.show();
		//_buttonBar.show();
		_vbox.show();

		_picture.showContents();

		move(rect.get_x(), rect.get_y());
		//gtk_window_present((GtkWindow *)this);
		//gtk_window_fullscreen((GtkWindow *)this);
	}
	catch (Utils::QuitNow) {
		std::cout << "Caught it!\n";
	}
}

GameWindow::~GameWindow() {
}


cv::Mat *GameWindow::getCurrent(){
	return &(_picture._current);
}

void GameWindow::setCalibrationPointer(WindowPointer *pointer) {
	_picture._calibrationPointer = pointer;
}

void GameWindow::setRepositioningImage(cv::Mat *image) {
	_picture._repositioningImage = image;
}

void GameWindow::changeWindowColor(double illuminationLevel) {
	_grayLevel = (int)(255 * illuminationLevel);
	_grayLevel = _grayLevel % 256;
	backgroundColor2 = CV_RGB(_grayLevel, _grayLevel, _grayLevel);
}

