#pragma once
#include <gtkmm.h>
#include "Containers.h"

/* represents the pointer as a small window and moves that window */
class WindowPointer {
 public:
    struct PointerSpec {
	int width, height;
	double red, green, blue;
	PointerSpec(int width, int height, 
		    double red, double green, double blue);
    };
 private:
    class GtkPointerDrawingArea: public Gtk::DrawingArea {
	PointerSpec spec;
    public:
        GtkPointerDrawingArea(const PointerSpec &pointerspec);
        virtual bool on_expose_event(GdkEventExpose *event);
    };
    class GtkPointerWindow: public Gtk::Window {
        GtkPointerDrawingArea area;
    public:
        GtkPointerWindow(const PointerSpec &pointerspec);
    };  
    GtkPointerWindow pointerwindow;
 public:
    WindowPointer(const PointerSpec &pointerspec);
    void setPosition(int x, int y);
};

/* class PointerMark: public Gtk::DrawingArea { */
/*     virtual bool on_expose_event(GdkEventExpose *event); */
/* }; */

/* class CalibrationMark: public Gtk::DrawingArea { */
/*     virtual bool on_expose_event(GdkEventExpose *event); */
/* }; */

