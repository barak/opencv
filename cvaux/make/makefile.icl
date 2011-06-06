# Makefile for Intel Proton Compiler >=5.0

CXX = icl
LINK = link

!ifdef debug

SUFFIX = d
DR = _Dbg

!else

SUFFIX = 
DR = _Rls

!endif

OUTBIN = ..\..\bin\cvaux$(SUFFIX).dll
OUTLIB = ..\..\lib\cvaux$(SUFFIX).lib

OBJS = ..\..\_temp\cvaux$(DR)_icl\cv3dtracker.obj \
..\..\_temp\cvaux$(DR)_icl\cvaux.obj ..\..\_temp\cvaux$(DR)_icl\cvauxutils.obj \
..\..\_temp\cvaux$(DR)_icl\cvcalibfilter.obj \
..\..\_temp\cvaux$(DR)_icl\cvdpstereo.obj ..\..\_temp\cvaux$(DR)_icl\cvepilines.obj \
..\..\_temp\cvaux$(DR)_icl\cvface.obj ..\..\_temp\cvaux$(DR)_icl\cvfacedetection.obj \
..\..\_temp\cvaux$(DR)_icl\cvfacetemplate.obj \
..\..\_temp\cvaux$(DR)_icl\cvfindface.obj ..\..\_temp\cvaux$(DR)_icl\cvhaar.obj \
..\..\_temp\cvaux$(DR)_icl\cvhaarfacecascade.obj \
..\..\_temp\cvaux$(DR)_icl\cvhmm1d.obj ..\..\_temp\cvaux$(DR)_icl\cvlcm.obj \
..\..\_temp\cvaux$(DR)_icl\cvlee.obj ..\..\_temp\cvaux$(DR)_icl\cvmat.obj \
..\..\_temp\cvaux$(DR)_icl\cvmorphcontours.obj \
..\..\_temp\cvaux$(DR)_icl\cvsegment.obj ..\..\_temp\cvaux$(DR)_icl\cvsubdiv2.obj \
..\..\_temp\cvaux$(DR)_icl\cvtexture.obj \
..\..\_temp\cvaux$(DR)_icl\cvvecfacetracking.obj \
..\..\_temp\cvaux$(DR)_icl\cvvideo.obj ..\..\_temp\cvaux$(DR)_icl\precomp.obj 



INC = ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h \
../include/cvaux.hpp ../include/cvmat.hpp 


CXXFLAGS2 = /I"../src" /I"../include" /I"../../cv/include" /I"../../cv/src" /nologo /GX /G6 /W4 "/Qwd68,171,424,444,869,981,522,9" /Qaxi /Qxi /c /Fo 
LINKFLAGS2 = /libpath:"../../lib" /nologo /subsystem:windows /dll /pdb:none /machine:I386 /out:$(OUTBIN) /implib:$(OUTLIB) /nodefaultlib:libm /nodefaultlib:libirc 

!ifdef debug

CXXFLAGS = /D"WIN32" /D"_WINDOWS" /D"_DEBUG" /D"CVAUX_DLL" /MDd /Gm /Zi /Od /FD /GZ $(CXXFLAGS2)
LIBS = cvd.lib kernel32.lib user32.lib gdi32.lib 
LINKFLAGS = $(LINKFLAGS2) /debug

!else

CXXFLAGS = /D"WIN32" /D"NDEBUG" /D"_WINDOWS" /D"CVAUX_DLL" /MD /O3 /Ob2 $(CXXFLAGS2)
LIBS = cv.lib kernel32.lib user32.lib gdi32.lib 
LINKFLAGS = $(LINKFLAGS2) 

!endif


$(OUTBIN): $(OBJS)
	-mkdir ..\..\bin 2> nul
	-mkdir ..\..\lib 2> nul
	$(LINK) $(LINKFLAGS) $** $(LIBS) 
	

all: $(OUTBIN)

..\..\_temp\cvaux$(DR)_icl\cv3dtracker.obj: ..\src\cv3dtracker.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	@-mkdir ..\..\_temp\cvaux$(DR)_icl 2>nul
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cv3dtracker.obj ..\src\cv3dtracker.cpp
..\..\_temp\cvaux$(DR)_icl\cvaux.obj: ..\src\cvaux.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvaux.obj ..\src\cvaux.cpp
..\..\_temp\cvaux$(DR)_icl\cvauxutils.obj: ..\src\cvauxutils.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvauxutils.obj ..\src\cvauxutils.cpp
..\..\_temp\cvaux$(DR)_icl\cvcalibfilter.obj: ..\src\cvcalibfilter.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvcalibfilter.obj ..\src\cvcalibfilter.cpp
..\..\_temp\cvaux$(DR)_icl\cvdpstereo.obj: ..\src\cvdpstereo.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvdpstereo.obj ..\src\cvdpstereo.cpp
..\..\_temp\cvaux$(DR)_icl\cvepilines.obj: ..\src\cvepilines.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvepilines.obj ..\src\cvepilines.cpp
..\..\_temp\cvaux$(DR)_icl\cvface.obj: ..\src\cvface.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvface.obj ..\src\cvface.cpp
..\..\_temp\cvaux$(DR)_icl\cvfacedetection.obj: ..\src\cvfacedetection.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvfacedetection.obj ..\src\cvfacedetection.cpp
..\..\_temp\cvaux$(DR)_icl\cvfacetemplate.obj: ..\src\cvfacetemplate.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvfacetemplate.obj ..\src\cvfacetemplate.cpp
..\..\_temp\cvaux$(DR)_icl\cvfindface.obj: ..\src\cvfindface.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvfindface.obj ..\src\cvfindface.cpp
..\..\_temp\cvaux$(DR)_icl\cvhaar.obj: ..\src\cvhaar.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvhaar.obj ..\src\cvhaar.cpp
..\..\_temp\cvaux$(DR)_icl\cvhaarfacecascade.obj: ..\src\cvhaarfacecascade.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvhaarfacecascade.obj ..\src\cvhaarfacecascade.cpp
..\..\_temp\cvaux$(DR)_icl\cvhmm1d.obj: ..\src\cvhmm1d.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvhmm1d.obj ..\src\cvhmm1d.cpp
..\..\_temp\cvaux$(DR)_icl\cvlcm.obj: ..\src\cvlcm.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvlcm.obj ..\src\cvlcm.cpp
..\..\_temp\cvaux$(DR)_icl\cvlee.obj: ..\src\cvlee.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvlee.obj ..\src\cvlee.cpp
..\..\_temp\cvaux$(DR)_icl\cvmat.obj: ..\src\cvmat.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvmat.obj ..\src\cvmat.cpp
..\..\_temp\cvaux$(DR)_icl\cvmorphcontours.obj: ..\src\cvmorphcontours.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvmorphcontours.obj ..\src\cvmorphcontours.cpp
..\..\_temp\cvaux$(DR)_icl\cvsegment.obj: ..\src\cvsegment.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvsegment.obj ..\src\cvsegment.cpp
..\..\_temp\cvaux$(DR)_icl\cvsubdiv2.obj: ..\src\cvsubdiv2.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvsubdiv2.obj ..\src\cvsubdiv2.cpp
..\..\_temp\cvaux$(DR)_icl\cvtexture.obj: ..\src\cvtexture.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvtexture.obj ..\src\cvtexture.cpp
..\..\_temp\cvaux$(DR)_icl\cvvecfacetracking.obj: ..\src\cvvecfacetracking.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvvecfacetracking.obj ..\src\cvvecfacetracking.cpp
..\..\_temp\cvaux$(DR)_icl\cvvideo.obj: ..\src\cvvideo.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\cvvideo.obj ..\src\cvvideo.cpp
..\..\_temp\cvaux$(DR)_icl\precomp.obj: ..\src\precomp.cpp ../src/_cvaux.h ../src/_cvfacedetection.h ../include/cvaux.h ../include/cvaux.hpp ../include/cvmat.hpp
	-$(CXX) $(CXXFLAGS)..\..\_temp\cvaux$(DR)_icl\precomp.obj ..\src\precomp.cpp
