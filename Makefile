# Get UNAME to decide on which system we are building
UNAME := $(shell uname)

# Command prefix to suppress outputs of executed commands on the terminal
# remove the @ sign if you want to see the executed commands
CMD_PREFIX = @

# Linux systems
# TODO : Change the path of these two folders for Linux systems where VXL is built & installed from source
#        If VXL libraries & include folders are installed to another location after build, more changes are necessary below
ifeq ($(UNAME), Linux)
	# Directory where vxl is built in (where ccmake command is run)
	VXL_BUILD_FOLDER = /home/onur/libs/vxl-1.14.0/build

	# Directory where vxl code is located (Changes.txt, configure, TODO.txt, etc.)
	VXL_SOURCE_FOLDER = /home/onur/libs/vxl-1.14.0
endif



######################################
# NOTHING TO CHANGE AFTER THIS POINT #
######################################
VERSION = eyetracker-1.0.0
CPPFLAGS = -Wall -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-write-strings -g -O3

# Linux linker parameters and include directories
ifeq ($(UNAME), Linux)
	LINKER = -L/usr/local/lib -L/opt/local/lib -L$(VXL_BUILD_FOLDER)/lib -lm -ldl -lvnl -lmvl -lvnl_algo -lvgl -lgthread-2.0  -lfann -lboost_filesystem -lboost_system -lgsl -lgslcblas
	INCLUDES = -I/usr/local/include -I/home/onur/libs/vxl-1.14.0 -I$(VXL_SOURCE_FOLDER)/core -I$(VXL_SOURCE_FOLDER)/vcl -I$(VXL_SOURCE_FOLDER)/contrib/oxl  -I$(VXL_BUILD_FOLDER)/core -I$(VXL_BUILD_FOLDER)/vcl -I$(VXL_BUILD_FOLDER)/contrib/oxl
endif

# Mac OS X linker parameters and include directories
ifeq ($(UNAME), Darwin)
    # VXL library folder (default when installed with MacPorts)
    VXL_LIBRARY_FOLDER = /opt/local/lib/vxl
    # VXL include folder (default when installed with MacPorts)
    VXL_INCLUDE_FOLDER = /opt/local/include/vxl

	LINKER = -L/opt/local/lib -L$(VXL_LIBRARY_FOLDER)  -lm -ldl -lvnl -lmvl -lvnl_algo -lvgl -lgthread-2.0  -lfann -lboost_filesystem-mt -lboost_system-mt -lgsl -lgslcblas
	INCLUDES = -I/usr/local/include -I$(VXL_INCLUDE_FOLDER)/core -I$(VXL_INCLUDE_FOLDER)/vcl -I$(VXL_INCLUDE_FOLDER)/contrib/oxl  -I$(VXL_LIBRARY_FOLDER)/core -I$(VXL_LIBRARY_FOLDER)/vcl -I$(VXL_LIBRARY_FOLDER)/contrib/oxl
endif

sources = opengazer.cpp Calibrator.cpp GazeTrackerGtk.cpp HeadTracker.cpp LeastSquares.cpp EyeExtractor.cpp GazeTracker.cpp MainGazeTracker.cpp OutputMethods.cpp PointTracker.cpp FaceDetector.cpp GazeArea.cpp TrackingSystem.cpp GtkStore.cpp Containers.cpp GraphicalPointer.cpp Point.cpp utils.cpp BlinkDetector.cpp FeatureDetector.cpp mir.cpp GameWindow.cpp
objects = $(patsubst %.cpp,%.o,$(sources))

%.o.depends: %.cpp
	$(CMD_PREFIX)g++ -MM $< > $@

%.o: %.cpp 
	$(CMD_PREFIX)g++ -c -o $@ $(INCLUDES) $<  `pkg-config cairomm-1.0 opencv gtkmm-2.4 --cflags` $(CPPFLAGS)

opengazer: 	$(objects)
	$(CMD_PREFIX)g++ -o $@ $^ `pkg-config cairomm-1.0 opencv gtkmm-2.4 --libs`  $(LINKER) $(CPPFLAGS)

clean:
	$(CMD_PREFIX)rm -rf opengazer
	$(CMD_PREFIX)rm -rf *.o
	$(CMD_PREFIX)rm -rf *.o.depends

