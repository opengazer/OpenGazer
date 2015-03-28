#define BOOST_FILESYSTEM_VERSION 3

//#define EXPERIMENT_MODE
//#define DEBUG

#include "GazeTrackerGtk.h"

int main(int argc, char **argv) {
	try {
		Gtk::Main kit(argc, argv);
		Glib::thread_init();

		GazeTrackerGtk window(argc, argv);
		window.show();

		Gtk::Main::run(window);
	}
	catch (Utils::QuitNow) {
		std::cout << "Caught it!\n";
	}

	return 0;
}
