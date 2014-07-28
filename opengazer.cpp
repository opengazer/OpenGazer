#define BOOST_FILESYSTEM_VERSION 3

//#define EXPERIMENT_MODE
//#define DEBUG

#include <gtkmm.h>

#include "GazeTrackerGtk.h"

int main(int argc, char **argv) {
	try {
		Gtk::Main kit(argc, argv);
		Glib::thread_init();

		//CalibrationWindow calwindow;
		//calwindow.show();

		GazeTrackerGtk helloworld(argc, argv);

		helloworld.show();

		Gtk::Main::run(helloworld);
	}
	catch (Utils::QuitNow) {
		std::cout << "Caught it!\n";
	}

	return 0;
}
