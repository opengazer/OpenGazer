#include <cairomm/context.h>

#include "GraphicalPointer.h"

WindowPointer::WindowPointer(const PointerSpec &pointerSpec):
	_pointerWindow(pointerSpec)
{
	_width = pointerSpec.width;
	_height = pointerSpec.height;
	_lastX = 0;
	_lastY = 0;
	//_pointerWindow.show();
}

void WindowPointer::setPosition(int x, int y) {
	//cout << "SETTING POSITION TO (" << x << ", " << y << ")" << endl;
	int numMonitors = Gdk::Screen::get_default()->get_n_monitors();

	// ONUR Fixed the coordinate problem. Previously the center of the window did not correspond to actual test point
	//_pointerWindow.move(x, y);

	_pointerWindow.move(x - _pointerWindow.get_width()/2, y - _pointerWindow.get_height()/2);

	// Save the last position
	_lastX = x;
	_lastY = y;

	if (numMonitors > 1 && mirror != NULL) {
		Point pt(x, y);
		Point pt2;
		Utils::mapToFirstMonitorCoordinates(pt, pt2);
		mirror->setPosition(pt2.x, pt2.y);
	}

	//int w, h;
	//_pointerWindow.get_size(w, h);
	//cout << "MOVE TO: " << x - _pointerWindow.get_width()/2 << " , " << y - _pointerWindow.get_height()/2 << endl;
}

Point WindowPointer::getPosition() {
	Point pt(_lastX, _lastY);
	return pt;
}

WindowPointer::PointerSpec::PointerSpec(int width, int height, double red, double green, double blue):
	width(width),
	height(height),
	red(red),
	green(green),
	blue(blue)
{
}

WindowPointer::GtkPointerDrawingArea::
GtkPointerDrawingArea(const PointerSpec &pointerspec):
	_spec(pointerspec)
{
	set_size_request(_spec.width, _spec.height);
}

void WindowPointer::GtkPointerDrawingArea::drawOnWindow(Glib::RefPtr<Gdk::Window> window, GdkEventExpose *event) {
	Cairo::RefPtr<Cairo::Context> cr(new Cairo::Context(gdk_cairo_create(window->gobj()), true));
	if (event) {
		//cout << "WIDTH, HEIGHT: " << event->area.width << ", " << event->area.height << endl;
		//cout << "GET WIDTH, HEIGHT: " << get_width() << ", " << get_height() << endl;
		cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
		cr->clip();
		//cout << "AFTER GET WIDTH, HEIGHT: " << get_width() << ", " << get_height() << endl;
	}

	cr->set_source_rgb(0.647, 0.7294, 0.8314);//(1.0, 1.0, 1.0);
	cr->paint();
	cr->set_source_rgb(_spec.red, _spec.green, _spec.blue);
	cr->arc(get_width()/2, get_height()/2, get_height()/3, 0, 2*M_PI);
	cr->fill();
	//Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
	//color.set_blue(100);
	//gc->set_foreground(color);
	//window->draw_arc(gc, true, 0, 0, get_width(), get_height(), 0, 360*64);
}

bool WindowPointer::GtkPointerDrawingArea::on_expose_event(GdkEventExpose *event)
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		drawOnWindow(window, event);
	}

	return false;
}

WindowPointer::GtkPointerWindow::GtkPointerWindow(const PointerSpec &pointerspec):
	_area(pointerspec)
{
	add(_area);
	_area.show();
	set_decorated(false);
	set_keep_above(true);
}

