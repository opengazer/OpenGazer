#pragma once

#include <gtkmm/drawingarea.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "MainGazeTracker.h"
#include "OutputMethods.h"

class GazeArea: public Gtk::DrawingArea {
public:
	MainGazeTracker gazeTracker;

	GazeArea(int argc, char **argv, const std::vector<boost::shared_ptr<AbstractStore> > &stores);
	virtual ~GazeArea();

private:
	friend class GazeTrackerGtk;

	int _lastPointId;
	int _clickCount;

	bool onIdle();

	// Gtk::DrawingArea
	bool on_expose_event(GdkEventExpose *event);
	bool on_button_press_event(GdkEventButton *event);
	bool on_button_release_event(GdkEventButton *event);
};
