#include "GazeTrackerGtk.h"
#include "GtkStore.h"

static std::vector<boost::shared_ptr<AbstractStore> > getStores() {
	std::vector<boost::shared_ptr<AbstractStore> > stores;

	stores.push_back(boost::shared_ptr<AbstractStore>(new SocketStore()));
	stores.push_back(boost::shared_ptr<AbstractStore>(new StreamStore(std::cout)));
	stores.push_back(boost::shared_ptr<AbstractStore>(new WindowStore(WindowPointer::PointerSpec(20, 30, 0, 0, 1), WindowPointer::PointerSpec(20, 20, 0, 1, 1), WindowPointer::PointerSpec(30, 30, 1, 0, 1))));

	return stores;
}

GazeTrackerGtk::GazeTrackerGtk(int argc, char **argv):
	_picture(argc, argv, getStores()),
	_calibrateButton("Calibrate"),
	_loadButton("Load points"),
	_saveButton("Save points"),
	_chooseButton("Choose points"),
	_pauseButton("Pause"),
	_clearButton("Clear points")
{
	try {
		set_title("opengazer 0.1.1");
		move(0, 0);

		add(_vbox);
		_vbox.pack_start(_picture);
		_vbox.pack_start(_buttonBar);

		_buttonBar.pack_start(_chooseButton);
		_buttonBar.pack_start(_clearButton);
		_buttonBar.pack_start(_calibrateButton);
		Gtk::Button *testButton = manage(new Gtk::Button("Test"));
		_buttonBar.pack_start(*testButton);
		_buttonBar.pack_start(_pauseButton);
		_buttonBar.pack_start(_saveButton);
		_buttonBar.pack_start(_loadButton);

		_calibrateButton.signal_clicked().
		connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::startCalibration));
		testButton->signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::startTesting));
		_saveButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::savepoints));
		_loadButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::loadpoints));
		_chooseButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::choosepoints));
		_pauseButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::pauseOrRepositionHead));
		_pauseButton.signal_clicked().connect(sigc::mem_fun(this, &GazeTrackerGtk::changePauseButtonText));
		_clearButton.signal_clicked().connect(sigc::mem_fun(&_picture.gazeTracker, &MainGazeTracker::clearpoints));

		_picture.show();
		testButton->show();
		_calibrateButton.show();
		//_saveButton.show();
		//_loadButton.show();
		_chooseButton.show();
		_pauseButton.show();
		_clearButton.show();
		_buttonBar.show();
		_vbox.show();
	}
	catch (Utils::QuitNow) {
		std::cout << "Caught it!\n";
	}
}

GazeTrackerGtk::~GazeTrackerGtk() {}

void GazeTrackerGtk::changePauseButtonText() {
	if(_pauseButton.get_label().compare("Pause") == 0) {
		_pauseButton.set_label("Unpause");
	} else {
		_pauseButton.set_label("Pause");
	}
}

