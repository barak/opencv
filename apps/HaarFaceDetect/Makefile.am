# here it is our binary program
bin_PROGRAMS = HaarFaceDetect

HaarFaceDetect_SOURCES = facedetect.cpp

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = cvfltk.h

EXTRA_DIST = SampleBase HaarFaceDetect.dsp

# add here the libraries which have to be include
# for building the shared lib
#
#  please consider also the usage of the make environment variables
#  like
#  $(LIB_KDEUI), $(LIB_QT), $(LIB_HTML), etc.
#  instead of the hardcoded entries, which have placed
#  in here.
#  The following line is only a suggestion!
#
HaarFaceDetect_LDADD = 

# INCLUDES +=  -I./ -I../../cv/include -I../../cvaux/include @DEBUG@ $(X_CFLAGS)
INCLUDES = -I../../cv/include -I./ -I../../cvaux/include -I../../otherlibs/highgui `@FLTKCONFIG@ --cxxflags`
LIBS = -L../../cv/src -lopencv -L../../cvaux/src -lcvaux -L../../otherlibs/highgui -lhighgui `@FLTKCONFIG@ --ldflags`
