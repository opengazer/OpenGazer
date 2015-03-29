#include "GazeTracker.h"
#include "WindowPointer.h"

class GameArea: public Gtk::DrawingArea {
	friend class GameWindow;

public:
	GameArea(TrackerOutput *output);
	virtual ~GameArea();
	void showContents();
	void calculateNewFrogPosition();
	void clearLastUpdatedRegion();

private:
	TrackerOutput *_output;
	cv::Mat _current;
	WindowPointer *_calibrationPointer;
	cv::Mat *_repositioningImage;
	cv::Mat _origImage;
	cv::Mat _background;
	cv::Mat _frog;
	cv::Mat _target;
	cv::Mat _frogMask;
	cv::Mat _gaussianMask;
	cv::Mat _clearingImage;
	cv::Rect _lastUpdatedRegion;
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
	cv::Scalar _backgroundColor;
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
	cv::Mat *getCurrent();
	void setCalibrationPointer(WindowPointer *pointer);
	void setRepositioningImage(cv::Mat *image);
	void changeWindowColor(double illuminationLevel);

private:
	GameArea _picture;
	int _grayLevel;

	//Member widgets:
	//Gtk::Button _calibrateButton, _loadButton, _saveButton, _clearButton, _chooseButton;
	Gtk::VBox _vbox;
	//Gtk::HBox _buttonBar;

};
