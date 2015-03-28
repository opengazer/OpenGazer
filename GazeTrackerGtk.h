#pragma once

#include "GazeArea.h"

class GazeTrackerGtk: public Gtk::Window {
public:
	GazeTrackerGtk(int argc, char **argv);
	virtual ~GazeTrackerGtk();

private:
	GazeArea _picture;
	Gtk::VBox _vbox;
	Gtk::HBox _buttonBar;
	Gtk::Button _calibrateButton;
	Gtk::Button _loadButton;
	Gtk::Button _saveButton;
	Gtk::Button _clearButton;
	Gtk::Button _chooseButton;
	Gtk::Button _pauseButton;
	Gtk::Button _testButton;

	void changePauseButtonText();
};
