#include "GazeTrackerGtk.h"
#include <gtkmm.h>
#include <iostream>
#include "GtkStore.h"

static vector<shared_ptr<AbstractStore> > getStores() {
    vector<shared_ptr<AbstractStore> > stores;

    stores.push_back(shared_ptr<AbstractStore>(new SocketStore()));
    stores.push_back(shared_ptr<AbstractStore>(new StreamStore(cout)));
    stores.push_back(shared_ptr<AbstractStore>
      (new WindowStore(WindowPointer::PointerSpec(20, 20, 0, 0, 1),
		       WindowPointer::PointerSpec(30, 30, 1, 0, 1))));

    return stores;
}

GazeTrackerGtk::GazeTrackerGtk(int argc, char **argv):
    calibratebutton("Calibrate"), 
    loadbutton("Load points"),
    savebutton("Save points"),
    clearbutton("Clear points"),
    picture(argc, argv, getStores())
{
    set_title("opengazer 0.1.1");

    add(vbox);
    vbox.pack_start(picture);
    vbox.pack_start(buttonbar);

    buttonbar.pack_start(calibratebutton);
    Gtk::Button *testbutton = manage(new Gtk::Button("Test"));
    buttonbar.pack_start(*testbutton);
    buttonbar.pack_start(savebutton);
    buttonbar.pack_start(loadbutton);
    buttonbar.pack_start(clearbutton);
    
    calibratebutton.signal_clicked().
 	connect(sigc::mem_fun(&picture.gazetracker,
			      &MainGazeTracker::startCalibration));
    testbutton->signal_clicked().
 	connect(sigc::mem_fun(&picture.gazetracker,
			      &MainGazeTracker::startTesting));
    savebutton.signal_clicked().
	connect(sigc::mem_fun(&picture.gazetracker,
			      &MainGazeTracker::savepoints));
    loadbutton.signal_clicked().
	connect(sigc::mem_fun(&picture.gazetracker,
			      &MainGazeTracker::loadpoints));
    clearbutton.signal_clicked().
	connect(sigc::mem_fun(&picture.gazetracker,
			      &MainGazeTracker::clearpoints));

    picture.show();
    testbutton->show();
    calibratebutton.show();
    savebutton.show();
    loadbutton.show();
    clearbutton.show();
    buttonbar.show();
    vbox.show();
}


GazeTrackerGtk::~GazeTrackerGtk() {
}


