#include "GraphicalPointer.h"
#include <cairomm/context.h>

WindowPointer::PointerSpec::PointerSpec(int width, int height, 
					double red, double green, double blue):
    width(width), height(height), red(red), green(green), blue(blue) 
{
}

WindowPointer::GtkPointerDrawingArea::
GtkPointerDrawingArea(const PointerSpec &pointerspec):
    spec(pointerspec)
{
    set_size_request(spec.width, spec.height);
}

bool WindowPointer::GtkPointerDrawingArea::
on_expose_event(GdkEventExpose *event) 
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		draw_on_window(window, event);
	}
	return false;
}

void WindowPointer::GtkPointerDrawingArea::draw_on_window(Glib::RefPtr<Gdk::Window> window, GdkEventExpose *event) {
	Cairo::RefPtr<Cairo::Context> cr(new Cairo::Context(gdk_cairo_create(window->gobj()), true));
	if (event) {
		//cout << "WIDTH, HEIGHT: " << event->area.width << ", " << event->area.height << endl;
		//cout << "GET WIDTH, HEIGHT: " << get_width() << ", " << get_height() << endl;
	    cr->rectangle(event->area.x, event->area.y,
				event->area.width, event->area.height);
	    cr->clip();
		//cout << "AFTER GET WIDTH, HEIGHT: " << get_width() << ", " << get_height() << endl;
	}
	
	cr->set_source_rgb(0.647, 0.7294, 0.8314);//(1.0, 1.0, 1.0);
	cr->paint();
	cr->set_source_rgb(spec.red, spec.green, spec.blue);
	cr->arc(get_width()/2, get_height()/2, get_height()/3, 0, 2*M_PI);
	cr->fill();
// 	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
// 	color.set_blue(100);
// 	gc->set_foreground(color);
// 	window->draw_arc(gc, true, 0, 0, get_width(), get_height(), 0, 360*64);
}

WindowPointer::GtkPointerWindow::
GtkPointerWindow(const PointerSpec &pointerspec):
    area(pointerspec)
{
    add(area);
    area.show();
    set_decorated(false);
    set_keep_above(true);
}

WindowPointer::WindowPointer(const PointerSpec &pointerspec):
    pointerwindow(pointerspec) 
{
	width = pointerspec.width;
	height = pointerspec.height;
	last_x= 0;
	last_y= 0;
    //pointerwindow.show();
}

void WindowPointer::setPosition(int x, int y) {
	//cout << "SETTING POSITION TO (" << x << ", " << y << ")" << endl;
	int num_of_monitors = Gdk::Screen::get_default()->get_n_monitors();
	
	// ONUR Fixed the coordinate problem. Previously the center of the window did not correspond to actual test point
	//pointerwindow.move(x, y);
	
	pointerwindow.move(x - pointerwindow.get_width()/2, y - pointerwindow.get_height()/2);
	
	// Save the last position
	last_x = x;
	last_y = y;
		
	if(num_of_monitors > 1 && mirror != NULL) {
		Point pt(x, y);
		Point pt2;
		mapToFirstMonitorCoordinates(pt, pt2);
		mirror->setPosition(pt2.x, pt2.y);
	}

		//int w, h;
		//pointerwindow.get_size(w, h);
		//cout <<  "MOVE TO: " << x - pointerwindow.get_width()/2 << " , " << y - pointerwindow.get_height()/2 << endl;
}

Point WindowPointer::getPosition() {
	Point pt(last_x, last_y);
	
	return pt;
}
