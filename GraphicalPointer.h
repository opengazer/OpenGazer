#pragma once
#include <gtkmm.h>
#include "Containers.h"

/* represents the pointer as a small window and moves that window */
class WindowPointer {
 public:
	shared_ptr<WindowPointer> mirror;
    struct PointerSpec {
	int width, height;
	double red, green, blue;
	PointerSpec(int width, int height, 
		    double red, double green, double blue);
    };
 private:
	 int last_x, last_y;
    class GtkPointerDrawingArea: public Gtk::DrawingArea {
	PointerSpec spec;
    public:
        GtkPointerDrawingArea(const PointerSpec &pointerspec);
        virtual bool on_expose_event(GdkEventExpose *event);
	void draw_on_window(Glib::RefPtr<Gdk::Window> window, GdkEventExpose *event);
    };
    class GtkPointerWindow: public Gtk::Window {
        GtkPointerDrawingArea area;
    public:
        GtkPointerWindow(const PointerSpec &pointerspec);
    };  
    GtkPointerWindow pointerwindow;
	int width, height;
 public:
    WindowPointer(const PointerSpec &pointerspec);
    void setPosition(int x, int y);
    Point getPosition();
};

/* class PointerMark: public Gtk::DrawingArea { */
/*     virtual bool on_expose_event(GdkEventExpose *event); */
/* }; */

/* class CalibrationMark: public Gtk::DrawingArea { */
/*     virtual bool on_expose_event(GdkEventExpose *event); */
/* }; */

