UNAME := $(shell uname)

# Command prefix to suppress outputs of executed commands on the terminal
# remove the @ sign if you want to see the executed commands
CMD_PREFIX = @

# Linux systems
ifeq ($(UNAME), Linux)
	# Directory where vxl is built in (where ccmake command is run)
	VXLBUILD = /home/onur/libs/vxl-1.14.0/build

	# Directory where vxl code is located (Changes.txt, configure, TODO.txt, etc.)
	VXLSRC = /home/onur/libs/vxl-1.14.0
endif

# Mac OS X systems
ifeq ($(UNAME), Darwin)
	# Directory where vxl is built in (where ccmake command is run)
	VXLBUILD = /opt/local/lib/vxl
	# Directory where vxl code is located (Changes.txt, configure, TODO.txt, etc.)
	VXLSRC = /opt/local/include/vxl
endif

VERSION = eyetracker-1.0.0
CPPFLAGS = -Wall -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-write-strings -g -O3

ifeq ($(UNAME), Linux)
	LINKER = -L$(VXLBUILD)/lib -L/usr/local/lib -L/opt/local/lib -lm -ldl -lvnl -lmvl -lvnl_algo -lvgl -lgthread-2.0  -lfann -lboost_filesystem -lboost_system -lgsl -lgslcblas
endif

ifeq ($(UNAME), Darwin)
	LINKER = -L$(VXLBUILD)  -L/opt/local/lib -lm -ldl -lvnl -lmvl -lvnl_algo -lvgl -lgthread-2.0  -lfann -lboost_filesystem-mt -lboost_system-mt -lgsl -lgslcblas
endif


INCLUDES = -L/usr/local/lib -L/opt/local/lib/vxl/ -I/usr/local/include -I/home/onur/libs/vxl-1.14.0 -I$(VXLSRC)/core -I$(VXLSRC)/vcl -I$(VXLSRC)/contrib/oxl  -I$(VXLBUILD)/core -I$(VXLBUILD)/vcl -I$(VXLBUILD)/contrib/oxl

sources = opengazer.cpp Calibrator.cpp GazeTrackerGtk.cpp HeadTracker.cpp LeastSquares.cpp EyeExtractor.cpp GazeTracker.cpp MainGazeTracker.cpp OutputMethods.cpp PointTracker.cpp FaceDetector.cpp GazeArea.cpp TrackingSystem.cpp GtkStore.cpp Containers.cpp GraphicalPointer.cpp Point.cpp utils.cpp BlinkDetector.cpp FeatureDetector.cpp mir.cpp GameWindow.cpp

objects = $(patsubst %.cpp,%.o,$(sources))

%.o.depends: %.cpp
	$(CMD_PREFIX)g++ -MM $< > $@

%.o: %.cpp 
	$(CMD_PREFIX)g++ -c -o $@ $(INCLUDES) $<  `pkg-config cairomm-1.0 opencv gtkmm-2.4 --cflags` $(CPPFLAGS)

opengazer: 	$(objects)
	$(CMD_PREFIX)g++ -o $@ $^ `pkg-config cairomm-1.0 opencv gtkmm-2.4 --libs`  $(LINKER) $(CPPFLAGS)

# include $(patsubst %.cpp,%.o.depends,$(sources))
clean:
	$(CMD_PREFIX)rm -rf opengazer
	$(CMD_PREFIX)rm -rf *.o
	$(CMD_PREFIX)rm -rf *.o.depends

