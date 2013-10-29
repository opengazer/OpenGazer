#include "GtkStore.h"

WindowStore::WindowStore(const WindowPointer::PointerSpec& pointerspec,
			 const WindowPointer::PointerSpec& pointerspec_left,
			 const WindowPointer::PointerSpec& targetspec):
    pointer(pointerspec), pointer_left(pointerspec_left), target(targetspec)
{
}

void WindowStore::store(const TrackerOutput &output) {
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	
	if(num_of_monitors == 1) {
	    pointer.setPosition((int) output.gazepoint.x, (int) output.gazepoint.y);
		pointer_left.setPosition((int) output.gazepoint_left.x, (int) output.gazepoint_left.y);
	    target.setPosition((int) output.target.x, (int) output.target.y);
	

	    //pointer.setPosition((int) output.nn_gazepoint.x, (int) output.nn_gazepoint.y);
		//pointer_left.setPosition((int) output.nn_gazepoint_left.x, (int) output.nn_gazepoint_left.y);
	    //target.setPosition(0, 0);
	}
	else {
		Point gazepoint1, gazepoint2, targetpoint;
		
		// Show Gaussian process outputs
		mapToFirstMonitorCoordinates(output.gazepoint, gazepoint1);
		mapToFirstMonitorCoordinates(output.gazepoint_left, gazepoint2);
		
		// Show neural network results
		//mapToFirstMonitorCoordinates(output.nn_gazepoint, gazepoint1);
		//mapToFirstMonitorCoordinates(output.nn_gazepoint_left, gazepoint2);
		
		mapToFirstMonitorCoordinates(output.target, targetpoint);
		
	    pointer.setPosition((int) gazepoint1.x, (int) gazepoint1.y);
		pointer_left.setPosition((int) gazepoint2.x, (int) gazepoint2.y);
	    target.setPosition((int) targetpoint.x, (int) targetpoint.y);
	}
}
