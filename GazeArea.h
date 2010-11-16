#pragma once
#include <gtkmm/drawingarea.h>
#include "MainGazeTracker.h"

class GazeArea: public Gtk::DrawingArea {
    friend class GazeTrackerGtk;
    int lastPointId;
    int clickCount;
 public:
    MainGazeTracker gazetracker;

    GazeArea(int argc, char **argv,
	     const vector<shared_ptr<AbstractStore> > &stores);
    virtual ~GazeArea();

 protected:
    virtual bool on_expose_event(GdkEventExpose *event);
    virtual bool on_button_press_event(GdkEventButton *event);
    virtual bool on_button_release_event(GdkEventButton *event);
    bool on_idle();
};
