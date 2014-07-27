#include "GtkStore.h"

WindowStore::WindowStore(const WindowPointer::PointerSpec &pointerSpec, const WindowPointer::PointerSpec &pointerSpecLeft, const WindowPointer::PointerSpec &targetSpec):
	_pointer(pointerSpec),
	_pointerLeft(pointerSpecLeft),
	_target(targetSpec)
{
}

void WindowStore::store(const TrackerOutput &output) {
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();

	if (numMonitors == 1) {
		_pointer.setPosition((int)output.gazepoint.x, (int)output.gazepoint.y);
		_pointerLeft.setPosition((int)output.gazepoint_left.x, (int)output.gazepoint_left.y);
		_target.setPosition((int)output.target.x, (int)output.target.y);

		//_pointer.setPosition((int)output.nn_gazepoint.x, (int)output.nn_gazepoint.y);
		//_pointerLeft.setPosition((int)output.nn_gazepoint_left.x, (int)output.nn_gazepoint_left.y);
		//_target.setPosition(0, 0);
	} else {
		Point gazepoint1, gazepoint2, targetpoint;

		// Show Gaussian process outputs
		Utils::mapToFirstMonitorCoordinates(output.gazepoint, gazepoint1);
		Utils::mapToFirstMonitorCoordinates(output.gazepoint_left, gazepoint2);

		// Show neural network results
		//Utils::mapToFirstMonitorCoordinates(output.nn_gazepoint, gazepoint1);
		//Utils::mapToFirstMonitorCoordinates(output.nn_gazepoint_left, gazepoint2);

		Utils::mapToFirstMonitorCoordinates(output.target, targetpoint);

		_pointer.setPosition((int)gazepoint1.x, (int)gazepoint1.y);
		_pointerLeft.setPosition((int)gazepoint2.x, (int)gazepoint2.y);
		_target.setPosition((int)targetpoint.x, (int)targetpoint.y);
	}
}
