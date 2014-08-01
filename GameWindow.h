#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/box.h>

#include "GazeTracker.h"
#include "GraphicalPointer.h"

class GameArea: public Gtk::DrawingArea {
	friend class GameWindow;

public:
	GameArea(TrackerOutput *output);
	virtual ~GameArea();
	void showContents();
	void calculateNewFrogPosition();
	void clearLastUpdatedRegion();
	void displayImageCentered(IplImage *image);

private:
	TrackerOutput *_output;
	IplImage *_current;
	WindowPointer *_calibrationPointer;
	IplImage *_repositioningImage;
	IplImage *_origImage;
	IplImage *_background;
	IplImage *_frog;
	IplImage *_target;
	//IplImage *_black;
	IplImage *_frogMask;
	IplImage *_gaussianMask;
	IplImage *_clearingImage;
	CvRect *_lastUpdatedRegion;
	int _frogX;
	int _frogY;
	int _frogCounter;
	int _gameAreaX;
	int _gameAreaY;
	int _gameAreaWidth;
	int _gameAreaHeight;
	long _startTime;
	long _futureTime;
	long _tempTime;
	CvScalar _backgroundColor;
	bool _isWindowInitialized;

	bool onIdle();

	// Gtk::DrawingArea;
	virtual bool on_expose_event(GdkEventExpose *event);
	virtual bool on_button_press_event(GdkEventButton *event);
	virtual bool on_button_release_event(GdkEventButton *event);
};

class GameWindow: public Gtk::Window {
public:
	GameWindow(TrackerOutput *output);
	virtual ~GameWindow();
	IplImage *getCurrent();
	void setCalibrationPointer(WindowPointer *pointer);
	void setRepositioningImage(IplImage *image);
	void changeWindowColor(double illuminationLevel);

private:
	GameArea _picture;
	int _grayLevel;

	//Member widgets:
	//Gtk::Button _calibrateButton, _loadButton, _saveButton, _clearButton, _chooseButton;
	Gtk::VBox _vbox;
	//Gtk::HBox _buttonBar;

};
