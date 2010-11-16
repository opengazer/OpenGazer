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
	Cairo::RefPtr<Cairo::Context> cr(new Cairo::Context(gdk_cairo_create(window->gobj()), true));
	if (event) {
	    cr->rectangle(event->area.x, event->area.y,
			  event->area.width, event->area.height);
	    cr->clip();
	}
	cr->set_source_rgb(1.0, 1.0, 1.0);
	cr->paint();
	cr->set_source_rgb(spec.red, spec.green, spec.blue);
	cr->arc(get_width()/2, get_height()/2, get_width()/3, 0, 2*M_PI);
	cr->fill();
// 	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
// 	color.set_blue(100);
// 	gc->set_foreground(color);
// 	window->draw_arc(gc, true, 0, 0, get_width(), get_height(), 0, 360*64);
    }
    return false;
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
    pointerwindow.show();
}

void WindowPointer::setPosition(int x, int y) {
    pointerwindow.move(x, y);
}
