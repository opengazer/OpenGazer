#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/box.h>

#include "GazeArea.h"

class GazeTrackerGtk: public Gtk::Window {
 protected:
  //Member widgets:
    Gtk::Button calibratebutton, loadbutton, savebutton, clearbutton;
    Gtk::VBox vbox;
    Gtk::HBox buttonbar;

 public:
    GazeArea picture;
    GazeTrackerGtk(int argc, char **argv);
    virtual ~GazeTrackerGtk();
    
};
