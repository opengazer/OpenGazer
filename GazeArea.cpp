#include "GazeArea.h"

GazeArea::GazeArea(int argc, char **argv):
	_lastPointId(-1),
	gazeTracker(argc, argv)
{
	set_size_request(gazeTracker.canvas->width, gazeTracker.canvas->height);
	Glib::signal_idle().connect(sigc::mem_fun(*this, &GazeArea::onIdle));
	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);
}

GazeArea::~GazeArea() {}

bool GazeArea::onIdle() {
	try {
		gazeTracker.process();
		queue_draw();
		gazeTracker.simulateClicks();
	}
	catch (Utils::QuitNow) {
		gazeTracker.cleanUp();
		exit(0);
	}

	return true;
}

bool GazeArea::on_expose_event(GdkEventExpose *event) {
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		Gtk::Allocation allocation = get_allocation();
		const int width = allocation.get_width();
		const int height = allocation.get_height();

		Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
		const IplImage *image = gazeTracker.canvas.get();
		Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data((guint8*) image->imageData, Gdk::COLORSPACE_RGB, false, image->depth, image->width, image->height, image->widthStep);

		window->draw_pixbuf(gc, pixbuf, 0, 0, 0, 0, width, height, Gdk::RGB_DITHER_NONE, 0, 0);
	}

	return true;
}

bool GazeArea::on_button_press_event(GdkEventButton *event) {
	if (event->button == 1) {
		switch(event->type) {
		case GDK_BUTTON_PRESS:
			_clickCount = 1;
			break;
		case GDK_2BUTTON_PRESS:
			_clickCount = 2;
			break;
		case GDK_3BUTTON_PRESS:
			_clickCount = 3;
			break;
		default:
			break;
		}

		if (event->type == GDK_BUTTON_PRESS) {
			PointTracker &pointTracker = gazeTracker.trackingSystem->pointTracker;
			Point point(event->x, event->y);

			int closest = pointTracker.getClosestTracker(point);
			if (closest >= 0 && point.distance(pointTracker.currentPoints[closest]) <= 25) {
				_lastPointId = closest;
			} else {
				_lastPointId = -1;
			}
		}

		return true;
	}

	return false;
}

bool GazeArea::on_button_release_event(GdkEventButton *event) {
	if (event->button == 1) {
		PointTracker &pointTracker = gazeTracker.trackingSystem->pointTracker;
		Point point(event->x, event->y);

		if (_lastPointId >= 0) {
			switch(_clickCount) {
			case 1:
				pointTracker.updateTracker(_lastPointId, point);
				break;
			case 2:
				pointTracker.removeTracker(_lastPointId);
				break;
			default:
				break;
			}
		} else {
			pointTracker.addTracker(point);
		}

		return true;
	}

	return false;
}

