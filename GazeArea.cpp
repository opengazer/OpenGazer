#include "GazeArea.h"
#include "OutputMethods.h"
#include <opencv/cv.h>

GazeArea::GazeArea(int argc, char **argv, 
		   const vector<shared_ptr<AbstractStore> > &stores):
    lastPointId(-1), gazetracker(argc, argv, stores)
{
    set_size_request(gazetracker.canvas->width, gazetracker.canvas->height);
    Glib::signal_idle().connect(sigc::mem_fun(*this, &GazeArea::on_idle));
    add_events(Gdk::BUTTON_PRESS_MASK);
    add_events(Gdk::BUTTON_RELEASE_MASK);
}

GazeArea::~GazeArea(void) {}

bool GazeArea::on_idle() {
    gazetracker.doprocessing();
    queue_draw();
    return true;
}

bool GazeArea::on_expose_event(GdkEventExpose *event) {
    Glib::RefPtr<Gdk::Window> window = get_window();
    if (window) {
	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
	const IplImage *image = gazetracker.canvas.get();
	Glib::RefPtr<Gdk::Pixbuf> pixbuf =
	    Gdk::Pixbuf::create_from_data((guint8*) image->imageData,
					  Gdk::COLORSPACE_RGB,
					  false,
					  image->depth,
					  image->width,
					  image->height,
					  image->widthStep);
	window->draw_pixbuf(gc, pixbuf, 0,0,0,0, width, height,
			    Gdk::RGB_DITHER_NONE, 0, 0);
    }
    return true;
}

bool GazeArea::on_button_press_event(GdkEventButton *event) {
    if (event->button == 1) {
	switch(event->type) {
	case GDK_BUTTON_PRESS: clickCount = 1; break;
	case GDK_2BUTTON_PRESS: clickCount = 2; break;
	case GDK_3BUTTON_PRESS: clickCount = 3; break;
	default: break;
	}

	if (event->type == GDK_BUTTON_PRESS) {
	    Point point(event->x, event->y);
	    PointTracker &tracker = gazetracker.tracking->tracker;
	    int closest = tracker.getClosestTracker(point);
	    if (closest >= 0 &&
		point.distance(tracker.currentpoints[closest]) <= 10)
		lastPointId = closest;
	    else
		lastPointId = -1;
	}
	return true;
    }
    else 
	return false;
}

bool GazeArea::on_button_release_event(GdkEventButton *event) {
    if (event->button == 1) {
	PointTracker &tracker = gazetracker.tracking->tracker;
	Point point(event->x, event->y);
	if (lastPointId >= 0)
	    switch(clickCount) {
	    case 1: tracker.updatetracker(lastPointId, point); break;
	    case 2: tracker.removetracker(lastPointId); break;
	    }
	else
	    tracker.addtracker(point);
	return true;
    }
    else
	return false;
}
