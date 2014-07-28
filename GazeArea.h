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

	bool onExposeEvent(GdkEventExpose *event);
	bool onButtonPressEvent(GdkEventButton *event);
	bool onButtonReleaseEvent(GdkEventButton *event);
	bool onIdle();
};
