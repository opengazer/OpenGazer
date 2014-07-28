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
		_pointer.setPosition((int)output.gazePoint.x, (int)output.gazePoint.y);
		_pointerLeft.setPosition((int)output.gazePointLeft.x, (int)output.gazePointLeft.y);
		_target.setPosition((int)output.target.x, (int)output.target.y);

		//_pointer.setPosition((int)output.nnGazePoint.x, (int)output.nnGazePoint.y);
		//_pointerLeft.setPosition((int)output.nnGazePointLeft.x, (int)output.nnGazePointLeft.y);
		//_target.setPosition(0, 0);
	} else {
		Point gazePoint1, gazePoint2, targetPoint;

		// Show Gaussian process outputs
		Utils::mapToFirstMonitorCoordinates(output.gazePoint, gazePoint1);
		Utils::mapToFirstMonitorCoordinates(output.gazePointLeft, gazePoint2);

		// Show neural network results
		//Utils::mapToFirstMonitorCoordinates(output.nnGazePoint, gazePoint1);
		//Utils::mapToFirstMonitorCoordinates(output.nnGazePointLeft, gazePoint2);

		Utils::mapToFirstMonitorCoordinates(output.target, targetPoint);

		_pointer.setPosition((int)gazePoint1.x, (int)gazePoint1.y);
		_pointerLeft.setPosition((int)gazePoint2.x, (int)gazePoint2.y);
		_target.setPosition((int)targetPoint.x, (int)targetPoint.y);
	}
}
