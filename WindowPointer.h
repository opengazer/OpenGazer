#pragma once

#include <gtkmm.h>

#include "Containers.h"
#include "Point.h"
#include "utils.h"

/* represents the pointer as a small window and moves that window */
class WindowPointer {
public:
	struct PointerSpec {
		int width;
		int height;
		double red;
		double green;
		double blue;

		PointerSpec(int width, int height, double red, double green, double blue);
	};

	boost::shared_ptr<WindowPointer> mirror;

	WindowPointer(const PointerSpec &pointerSpec);
	void setPosition(int x, int y);
	Point getPosition();

private:
	class GtkPointerDrawingArea: public Gtk::DrawingArea {
	public:
		GtkPointerDrawingArea(const PointerSpec &pointerspec);
		void drawOnWindow(Glib::RefPtr<Gdk::Window> window, GdkEventExpose *event);

		// Gtk::DrawingArea
		virtual bool on_expose_event(GdkEventExpose *event);

	private:
		PointerSpec _spec;
	};

	class GtkPointerWindow: public Gtk::Window {
	public:
		GtkPointerWindow(const PointerSpec &pointerspec);

	private:
		GtkPointerDrawingArea _area;
	};

	int _lastX;
	int _lastY;
	int _width;
	int _height;
	GtkPointerWindow _pointerWindow;

};

//class PointerMark: public Gtk::DrawingArea {
//	virtual bool onExposeEvent(GdkEventExpose *event);
//};

//class CalibrationMark: public Gtk::DrawingArea {
//	virtual bool onExposeEvent(GdkEventExpose *event);
//};

