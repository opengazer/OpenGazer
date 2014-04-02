#include "GazeTrackerGtk.h"
#include <gtkmm.h>
#include <iostream>
#include "GtkStore.h"

static vector<boost::shared_ptr<AbstractStore> > getStores() {
    vector<boost::shared_ptr<AbstractStore> > stores;

    stores.push_back(boost::shared_ptr<AbstractStore>(new SocketStore()));
    stores.push_back(boost::shared_ptr<AbstractStore>(new StreamStore(cout)));
    stores.push_back(boost::shared_ptr<AbstractStore>
      (new WindowStore(WindowPointer::PointerSpec(20, 30, 0, 0, 1),
			   WindowPointer::PointerSpec(20, 20, 0, 1, 1),
		       WindowPointer::PointerSpec(30, 30, 1, 0, 1))));

    return stores;
}

GazeTrackerGtk::GazeTrackerGtk(int argc, char **argv):
    calibratebutton("Calibrate"), 
    loadbutton("Load points"),
    savebutton("Save points"),
    choosebutton("Choose points"),
    pausebutton("Pause"),
    clearbutton("Clear points"),
    picture(argc, argv, getStores())
{
	try {
	    set_title("opengazer 0.1.1");
		move(0, 0);

	    add(vbox);
	    vbox.pack_start(picture);
	    vbox.pack_start(buttonbar);

	    buttonbar.pack_start(choosebutton);
	    buttonbar.pack_start(clearbutton);
            buttonbar.pack_start(calibratebutton);
	    Gtk::Button *testbutton = manage(new Gtk::Button("Test"));
	    buttonbar.pack_start(*testbutton);
        buttonbar.pack_start(pausebutton);
	    buttonbar.pack_start(savebutton);
	    buttonbar.pack_start(loadbutton);
    
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
	    choosebutton.signal_clicked().
		connect(sigc::mem_fun(&picture.gazetracker,
				      &MainGazeTracker::choosepoints));
	    pausebutton.signal_clicked().
		connect(sigc::mem_fun(&picture.gazetracker,
				      &MainGazeTracker::pauseOrRepositionHead));
	    pausebutton.signal_clicked().
		connect(sigc::mem_fun(this,
				      &GazeTrackerGtk::changePauseButtonText));
	    clearbutton.signal_clicked().
		connect(sigc::mem_fun(&picture.gazetracker,
				      &MainGazeTracker::clearpoints));

	    picture.show();
	    testbutton->show();
	    calibratebutton.show();
	    //savebutton.show();
	    //loadbutton.show();
	    choosebutton.show();
		pausebutton.show();
	    clearbutton.show();
	    buttonbar.show();
	    vbox.show();
	}
	catch (QuitNow)
	{
		cout << "Caught it!\n";
	}
}
void GazeTrackerGtk::changePauseButtonText() {
	if(pausebutton.get_label().compare("Pause") == 0) {
		pausebutton.set_label("Unpause");
	}
	else {
		pausebutton.set_label("Pause");
	}
}

GazeTrackerGtk::~GazeTrackerGtk() {
}


