#include "GameWindow.h"
#include <gtkmm.h>
#include <iostream>

#include "OutputMethods.h"
#include <stdlib.h>
#include <opencv2/opencv.hpp>

CvScalar background_color2;

bool button_events() {
	static int mode = 1;

/*
	if(mode==1)
		background_color2 = CV_RGB(0, 0, 0);
	else
		background_color2 = CV_RGB(255, 255, 255);
		
	mode = 1 - mode;
*/	
	return true;
}

GameArea::GameArea(TrackerOutput* op)  
{
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
	Gdk::Rectangle rect;
	screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);

	set_size_request(rect.get_width(), rect.get_height());
	
	
		
//	this->move(0, 0);
    Glib::signal_idle().connect(sigc::mem_fun(*this, &GameArea::on_idle));
	output = op;
	
	
	add_events(Gdk::BUTTON_PRESS_MASK);
	add_events(Gdk::BUTTON_RELEASE_MASK);
	
	Glib::RefPtr<Gdk::Window> window = get_window();
    //this->signal_key_press_event().connect(sigc::mem_fun(*this, &GameArea::on_idle));
    	

	orig_image = (IplImage*) cvLoadImage("./background_full.png");
    cvCvtColor(orig_image, orig_image, CV_RGB2BGR);
	
	frog = (IplImage*) cvCreateImage(cvSize(180, 180), 8, 3 );
	target = (IplImage*) cvCreateImage(cvSize(50, 50), 8, 3 );
	
	background = (IplImage*) cvCreateImage(cvSize(rect.get_width(), rect.get_height()), 8, 3 );
	current = (IplImage*) cvCreateImage(cvSize(background->width, background->height), 8, 3 );
	//black = (IplImage*) cvCreateImage(cvSize(background->width, background->height), 8, 3 );
    clearing_image = (IplImage*) cvCreateImage(cvSize(2000, 1500), 8, 3 );
	
	cout << "IMAGES CREATED WITH SIZE: " << background->width << "x" << background->height << endl;
	
	cvSetZero(background);
	//cvSetZero(black);
	background_color = CV_RGB(153, 75, 75);
	background_color2 = CV_RGB(255, 255, 255);
	//cvSet(background, background_color);
    //cvSet(black, background_color2);

    // Clearing image is filled with white
	//cvSet(clearing_image, background_color2);
    cvSet(clearing_image, CV_RGB(255, 255, 255));
	
	
	
	game_area_x = (rect.get_width()-orig_image->width)/2;
	game_area_y = (rect.get_height()-orig_image->height)/2;
	game_area_width = orig_image->width;
	game_area_height = orig_image->height;
	
		
	frog = (IplImage*) cvLoadImage("./frog.png");
	cvCvtColor(frog, frog, CV_RGB2BGR);
	
	frog_mask = (IplImage*) cvLoadImage("./frog-mask.png");
	gaussian_mask = (IplImage*) cvLoadImage("./gaussian-mask.png");
	
	
	target = (IplImage*) cvLoadImage("./target.png");
	
	frog_counter=0;
	calculateNewFrogPosition();
	
	
	timeval time;
	gettimeofday(&time, NULL);
	future_time = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	future_time += 1000*20000;
	
	start_time = future_time;
	
	calibrationPointer = NULL;

    last_updated_region = new CvRect();
    last_updated_region->x = 0;
    last_updated_region->y = 0;
    last_updated_region->width = rect.get_width()-54;
    last_updated_region->height = rect.get_height()-24;
    is_window_initialized = false;
}

GameArea::~GameArea(void) {}

bool GameArea::on_idle() {
	showContents();
	//queue_draw();
	return true;
}

bool GameArea::on_expose_event(GdkEventExpose *event) {
	//showContents();
    return true;
}

void GameArea::clearLastUpdatedRegion() {
    if(last_updated_region->width > 0) {
	    Glib::RefPtr<Gdk::Window> window = get_window();
		Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);

        Glib::RefPtr<Gdk::Pixbuf> pixbuf2 =
        Gdk::Pixbuf::create_from_data((guint8*) clearing_image->imageData,
			            Gdk::COLORSPACE_RGB,
			            false,
			            clearing_image->depth,
			            clearing_image->width,
			            clearing_image->height,
			            clearing_image->widthStep);

        window->draw_pixbuf(gc, pixbuf2, 0,0, last_updated_region->x,last_updated_region->y,last_updated_region->width, last_updated_region->height,
	            Gdk::RGB_DITHER_NORMAL , 0, 0);
                
                
        //cout << "CLEARING THE AREA: " << last_updated_region->x << ", " << last_updated_region->y << ", " << last_updated_region->width << ", " << last_updated_region->height << "." << endl;
        
                
        last_updated_region->x = 0;
        last_updated_region->y = 0;
        last_updated_region->width = 0;
        last_updated_region->height = 0;
    }
}

void GameArea::calculateNewFrogPosition() {
	/*
	static int frog_x_s[] = {350, 640, 640, 350};
	static int frog_y_s[] = {680, 95, 680, 680};
	static int counter = 0;
	
	frog_x = frog_x_s[counter];
	frog_y = frog_y_s[counter];
	
	counter++;
	*/
	
	
	frog_x = rand() % (game_area_width-90) + game_area_x + 90;
	frog_y = rand() % (game_area_height-90) + game_area_y + 90;


    cout << "Preparing bg image" << endl;
    // Copy the initial background image to "background"
	cvSetZero(background);
	cvSetImageROI(background, cvRect(game_area_x, game_area_y, game_area_width, game_area_height));
	cvCopy(orig_image, background);
    cout << "Original image copied" << endl;

    // Copy the frog image to the background
	cvSetImageROI(background, cvRect(frog_x-90, frog_y-90, 180, 180));
	cvCopy(frog, background, frog_mask);
	cvResetImageROI(background);
    cout << "Background prepared" << endl;
}

void GameArea::showContents() {
	static double estimation_x_right = 0, estimation_y_right = 0, estimation_x_left = 0, estimation_y_left = 0;
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {

		if(tracker_status == STATUS_PAUSED) {
			cout << "PAUSED, DRAWING HERE" << endl;
			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
			IplImage* repimg = repositioningImage;
			CvRect bounds = cvRect((background->width - repimg->width)/2, (background->height - repimg->height)/2, repimg->width, repimg->height);
			
		    Glib::RefPtr<Gdk::Pixbuf> pixbuf =
		    Gdk::Pixbuf::create_from_data((guint8*) repimg->imageData,
						    Gdk::COLORSPACE_RGB,
						    false,
						    repimg->depth,
						    repimg->width,
						    repimg->height,
						    repimg->widthStep);
		    window->draw_pixbuf(gc, pixbuf, 0, 0, bounds.x, bounds.y, bounds.width, bounds.height,
				    Gdk::RGB_DITHER_NONE, 0, 0);

            last_updated_region->x = bounds.x;
            last_updated_region->y = bounds.y;
            last_updated_region->width = bounds.width;
            last_updated_region->height = bounds.height;
		}
#ifdef EXPERIMENT_MODE
		else if(false) {		// For the experiment mode, never show the game
#else
		else if(tracker_status == STATUS_CALIBRATED) {
#endif
		
			//Gtk::Allocation allocation = get_allocation();
			const int width = background->width;
			const int height = background->height;

			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
			
			double alpha = 0.6;
			estimation_x_right = (1-alpha)*output->gazepoint.x + alpha*estimation_x_right;
			estimation_y_right = (1-alpha)*output->gazepoint.y + alpha*estimation_y_right;
			estimation_x_left = (1-alpha)*output->gazepoint_left.x + alpha*estimation_x_left;
			estimation_y_left = (1-alpha)*output->gazepoint_left.y + alpha*estimation_y_left;
			
			int estimation_x = (estimation_x_right + estimation_x_left) / 2;
			int estimation_y = (estimation_y_right + estimation_y_left) / 2;
			
			//int estimation_x = output->gazepoint.x;
			//int estimation_y = output->gazepoint.y;
			//cout << "INIT EST: " << estimation_x << ", " << estimation_y << endl;
			
			// Map estimation to window coordinates
			int window_x, window_y;
			
			//window->get_geometry(window_x, window_y, dummy, dummy, dummy); 
			//GtkWidget *top_window;
			//gint x,y;
			//top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			
			//gdk_window_get_position(top_window->window, &x, &y);
			//cout << "WINDOW POSITION: " << x<< ", " << y << endl;
			//estimation_x -= window_x;
			//estimation_y -= window_y;
			
			//// REMOVE
			//estimation_x = 800;
			//estimation_y = 500;
			
			//cout << "EST: " << estimation_x << ", " << estimation_y << endl;
			//cout << "separately: (" << output->gazepoint.x << ", " << output->gazepoint.y << ") and (" << output->gazepoint_left.x << ", " << output->gazepoint_left.y << ")" << endl;
			//cout << "  --errors: (" << output->nn_gazepoint.x << ", " << output->nn_gazepoint.y << ") and (" << output->nn_gazepoint_left.x << ", " << output->nn_gazepoint_left.y << ")" << endl;
			
			//cvSet(black, background_color2);
	
			cvResetImageROI(background);
			cvResetImageROI(current);
			cvResetImageROI(gaussian_mask);
			
			CvRect bounds = cvRect(estimation_x - 100, estimation_y - 100, 200, 200);
			CvRect gaussian_bounds = cvRect(0, 0, 200, 200);
			
			if(bounds.x < 0) {
				bounds.width += bounds.x;		// Remove the amount from the width
				gaussian_bounds.x -= bounds.x;
				gaussian_bounds.width += bounds.x;
				bounds.x = 0;
			}
			if(bounds.y < 0) {
				bounds.height += bounds.y;		// Remove the amount from the height
				gaussian_bounds.y -= bounds.y;
				gaussian_bounds.height += bounds.y;
				bounds.y = 0;
			}
			if(bounds.width + bounds.x > background->width) {
				bounds.width = background->width - bounds.x; 
			}
			if(bounds.height + bounds.y > background->height) {
				bounds.height = background->height - bounds.y; 
			}	
			gaussian_bounds.width = bounds.width;
			gaussian_bounds.height = bounds.height;
			
			if(estimation_x <= 0) {
				estimation_x = 1;
				//cout << "IMPOSSIBLE CHANGE 1" << endl;
			}
			if(estimation_y <= 0) {
				estimation_y = 1;
				//cout << "IMPOSSIBLE CHANGE 2" << endl;
			}
			if(estimation_x >= background->width) {
				estimation_x = background->width -1;
				//cout << "IMPOSSIBLE CHANGE 3" << endl;
			}
			if(estimation_y >= background->height) {
				estimation_y = background->height -1;
				//cout << "IMPOSSIBLE CHANGE 4" << endl;
			}
			
			//cout << "4" << endl;
			//cvCopy(black, current);
            cvSet(current, background_color2);
			//cout << "44" << endl;
			
			//cout << "Bounds: " << bounds.x << ", " << bounds.y << "," << bounds.width << ", " << bounds.height << endl;
			//cout << "Gaussian Bounds: " << gaussian_bounds.x << ", " << gaussian_bounds.y << "," << gaussian_bounds.width << ", " << gaussian_bounds.height << endl;
			
			if(bounds.width > 0 && bounds.height > 0) {
				if(estimation_x != 0 || estimation_y != 0) {
					cvSetImageROI(background, bounds);
					cvSetImageROI(current, bounds);
					cvSetImageROI(gaussian_mask, gaussian_bounds);
					//cout << "5" << endl;
					cvCopy(background, current, gaussian_mask);
					//cout << "6" << endl;
				}
			}
			
			cvResetImageROI(background);
			cvResetImageROI(current);
			cvResetImageROI(gaussian_mask);

            clearLastUpdatedRegion();
			
            // Draw only the region which is to be updated
		    Glib::RefPtr<Gdk::Pixbuf> pixbuf =
		    Gdk::Pixbuf::create_from_data((guint8*) current->imageData,
						    Gdk::COLORSPACE_RGB,
						    false,
						    current->depth,
						    current->width,
						    current->height,
						    current->widthStep);
		    window->draw_pixbuf(gc, pixbuf, bounds.x, bounds.y, bounds.x, bounds.y, bounds.width, bounds.height,
				    Gdk::RGB_DITHER_NONE, 0, 0);

            last_updated_region->x = bounds.x;
            last_updated_region->y = bounds.y;
            last_updated_region->width = bounds.width;
            last_updated_region->height = bounds.height;
			
			int diff = ((estimation_x - frog_x) * (estimation_x - frog_x)) + ((estimation_y - frog_y) * (estimation_y - frog_y));
					
					
			// If less than 150 pix, count
			if(diff < 35000) {
				//cout << "Difference is fine!" << endl;
				timeval time;
				gettimeofday(&time, NULL);
				temp_time = (time.tv_sec * 1000) + (time.tv_usec / 1000);
				
				
				if(start_time == future_time) {
					start_time = temp_time;
				}
				// If fixes on the same point for 3 seconds
				else if(temp_time - start_time > 1500) {
					frog_counter++;
					calculateNewFrogPosition();
					start_time = future_time;
				}
				
			}
			// If cannot focus for a while, reset
			else {
				start_time = future_time;
			}
		}
        else if(!is_window_initialized) {
			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);

            Glib::RefPtr<Gdk::Pixbuf> pixbuf2 =
            Gdk::Pixbuf::create_from_data((guint8*) clearing_image->imageData,
                            Gdk::COLORSPACE_RGB,
                            false,
                            clearing_image->depth,
                            clearing_image->width,
                            clearing_image->height,
                            clearing_image->widthStep);

            window->draw_pixbuf(gc, pixbuf2, 0,0, 0,0,1920, 1080,
                    Gdk::RGB_DITHER_NONE , 0, 0);
            
            is_window_initialized = true;
        }
		else if (calibrationPointer != NULL){	// Calibration
			const int width = background->width;
			const int height = background->height;
            CvRect currently_updated_region = cvRect(0, 0, 0, 0);

			Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
			
			Point calibrationPoint = calibrationPointer->getPosition();
			
			//if(tracker_status == STATUS_TESTING) {
			//	cout << "EXPECTED OUTPUT: ("<< calibrationPoint.x << ", " << calibrationPoint.y << ")" << endl << endl;
			//}
			
			// If the coordinates are beyond bounds, calibration is finished
			if(calibrationPoint.x > 3000 || calibrationPoint.y > 3000) {
				calibrationPointer = NULL;
                return;
			}
			
			if(calibrationPoint.x > 0 && calibrationPoint.y > 0) {
				CvRect currentBounds = cvRect(calibrationPoint.x - 25, calibrationPoint.y - 25, 50, 50);
				CvRect targetBounds = cvRect(0, 0, 50, 50);
				
				if(currentBounds.x < 0) {
					currentBounds.width += currentBounds.x;		// Remove the amount from the width
					targetBounds.x -= currentBounds.x;
					targetBounds.width += currentBounds.x;
					currentBounds.x = 0;
				}
				if(currentBounds.y < 0) {
					currentBounds.height += currentBounds.y;		// Remove the amount from the height
					targetBounds.y -= currentBounds.y;
					targetBounds.height += currentBounds.y;
					currentBounds.y = 0;
				}
				if(currentBounds.width + currentBounds.x > background->width) {
					currentBounds.width = background->width - currentBounds.x; 
				}
				if(currentBounds.height + currentBounds.y > background->height) {
					currentBounds.height = background->height - currentBounds.y; 
				}
				
				//cvSetImageROI(current, currentBounds);
				//cvSetImageROI(target, targetBounds);
				//cout << "7" << endl;
				//cvCopy(target, current);
				//cout << "8" << endl;
				
				//cvResetImageROI(current);
				//cvResetImageROI(target);

                // Clear only previously updated region
                clearLastUpdatedRegion();
	
                // Draw only the region which is to be updated
			    Glib::RefPtr<Gdk::Pixbuf> pixbuf =
			    Gdk::Pixbuf::create_from_data((guint8*) target->imageData,
							    Gdk::COLORSPACE_RGB,
							    false,
							    target->depth,
							    target->width,
							    target->height,
							    target->widthStep);
			    window->draw_pixbuf(gc, pixbuf, targetBounds.x,targetBounds.y, currentBounds.x,currentBounds.y,targetBounds.width, targetBounds.height,
					    Gdk::RGB_DITHER_NONE, 0, 0);

                last_updated_region->x = currentBounds.x;
                last_updated_region->y = currentBounds.y;
                last_updated_region->width = targetBounds.width;
                last_updated_region->height = targetBounds.height;
			}
		}
		else {
			clearLastUpdatedRegion();
		}
	}
}





GameWindow::GameWindow(TrackerOutput* op) :
	picture(op)
{
	try {
		set_title("Game Window");
		
		// Center window
		Glib::RefPtr<Gdk::Screen> screen = Gdk::Display::get_default()->get_default_screen();
		Gdk::Rectangle rect;
		screen->get_monitor_geometry(Gdk::Screen::get_default()->get_n_monitors() - 1, rect);
		
		//this->set_keep_above(true);
		// Set tracker output
		
	    add(vbox);
	    vbox.pack_start(picture);
	    //vbox.pack_start(buttonbar);

	    //buttonbar.pack_start(calibratebutton);
    
	    //calibratebutton.signal_clicked().
		// connect(sigc::mem_fun(&picture.gazetracker,
		//		      &MainGazeTracker::startCalibration));
		

	    picture.show();
	    //calibratebutton.show();
	    //buttonbar.show();
	    vbox.show();
	
		picture.showContents();
		
		
		
		move(rect.get_x(), rect.get_y());
		//gtk_window_present((GtkWindow*) this);
		//gtk_window_fullscreen((GtkWindow*) this);
	}
	catch (QuitNow)
	{
		cout << "Caught it!\n";
	}
}

bool GameArea::on_button_press_event(GdkEventButton *event) {
	return button_events();
}

bool GameArea::on_button_release_event(GdkEventButton *event) {
	return true;
}


GameWindow::~GameWindow() {
}


IplImage* GameWindow::get_current(){
	return picture.current;
}

void GameWindow::setCalibrationPointer(WindowPointer* pointer){
	
	picture.calibrationPointer = pointer;
}	

void GameWindow::setRepositioningImage(IplImage* image){
	picture.repositioningImage = image;
}

void GameWindow::changeWindowColor(double illuminationLevel) {
	gray_level = (int) (255 * illuminationLevel);
	
	gray_level = gray_level % 256;
	
	background_color2 = CV_RGB(gray_level, gray_level, gray_level);
}

