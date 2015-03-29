#include "GazeTrackerGtk.h"
#include "Application.h"

GazeTrackerGtk::GazeTrackerGtk(int argc, char **argv):
	_picture(argc, argv),
	_vbox(false, 0),
	_buttonBar(true, 0),
	_calibrateButton("Calibrate"),
	_loadButton("Load points"),
	_saveButton("Save points"),
	_chooseButton("Choose points"),
	_pauseButton("Pause"),
	_clearButton("Clear points"),
	_testButton("Test")
{
	try {
		set_title("opengazer 0.1.1");
		move(0, 0);

		// Construct view
		add(_vbox);

		_vbox.pack_start(_buttonBar, false, true, 0);
		_vbox.pack_start(_picture);

		_buttonBar.pack_start(_chooseButton);
		_buttonBar.pack_start(_clearButton);
		_buttonBar.pack_start(_calibrateButton);
		_buttonBar.pack_start(_testButton);
		_buttonBar.pack_start(_pauseButton);
		_buttonBar.pack_start(_saveButton);
		_buttonBar.pack_start(_loadButton);

		// Connect buttons
		_calibrateButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::startCalibration));
		_testButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::startTesting));
		_saveButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::savePoints));
		_loadButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::loadPoints));
		_chooseButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::choosePoints));
		_pauseButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::pauseOrRepositionHead));
		_pauseButton.signal_clicked().connect(sigc::mem_fun(this, &GazeTrackerGtk::changePauseButtonText));
		_clearButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::clearPoints));

		// Display view
		_vbox.show();
		_buttonBar.show();
		_picture.show();
		_calibrateButton.show();
		//_saveButton.show();
		//_loadButton.show();
		_chooseButton.show();
		_pauseButton.show();
		_clearButton.show();
		_testButton.show();
	}
	catch (Utils::QuitNow) {
		std::cout << "Caught it!\n";
	}
}

GazeTrackerGtk::~GazeTrackerGtk() {}

void GazeTrackerGtk::changePauseButtonText() {
	if(_pauseButton.get_label() == "Pause") {
		_pauseButton.set_label("Unpause");
	} else {
		_pauseButton.set_label("Pause");
	}
}

