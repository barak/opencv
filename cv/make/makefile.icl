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

OUTBIN = ..\..\bin\cv$(SUFFIX).dll
OUTLIB = ..\..\lib\cv$(SUFFIX).lib

OBJS = ..\..\_temp\cv$(DR)_icl\cvabsdiff.obj ..\..\_temp\cv$(DR)_icl\cvaccum.obj \
..\..\_temp\cv$(DR)_icl\cvalloc.obj ..\..\_temp\cv$(DR)_icl\cvarithm.obj \
..\..\_temp\cv$(DR)_icl\cvarray.obj ..\..\_temp\cv$(DR)_icl\cvcalccovarmatrix.obj \
..\..\_temp\cv$(DR)_icl\cvcmp.obj ..\..\_temp\cv$(DR)_icl\cvconvert.obj \
..\..\_temp\cv$(DR)_icl\cvcopy.obj ..\..\_temp\cv$(DR)_icl\cvdatastructs.obj \
..\..\_temp\cv$(DR)_icl\cvdxt.obj ..\..\_temp\cv$(DR)_icl\cverror.obj \
..\..\_temp\cv$(DR)_icl\cvjacobieigens.obj ..\..\_temp\cv$(DR)_icl\cvlogic.obj \
..\..\_temp\cv$(DR)_icl\cvmathfuncs.obj ..\..\_temp\cv$(DR)_icl\cvmatmul.obj \
..\..\_temp\cv$(DR)_icl\cvmatrix.obj ..\..\_temp\cv$(DR)_icl\cvmatrix64d.obj \
..\..\_temp\cv$(DR)_icl\cvmean.obj ..\..\_temp\cv$(DR)_icl\cvmeansdv.obj \
..\..\_temp\cv$(DR)_icl\cvminmaxloc.obj ..\..\_temp\cv$(DR)_icl\cvmoments.obj \
..\..\_temp\cv$(DR)_icl\cvnorm.obj ..\..\_temp\cv$(DR)_icl\cvpersistence.obj \
..\..\_temp\cv$(DR)_icl\cvprecomp.obj ..\..\_temp\cv$(DR)_icl\cvrand.obj \
..\..\_temp\cv$(DR)_icl\cvsumpixels.obj ..\..\_temp\cv$(DR)_icl\cvsvd.obj \
..\..\_temp\cv$(DR)_icl\cvswitcher.obj ..\..\_temp\cv$(DR)_icl\cvtables.obj \
..\..\_temp\cv$(DR)_icl\cvutils.obj ..\..\_temp\cv$(DR)_icl\cvadapthresh.obj \
..\..\_temp\cv$(DR)_icl\cvaffine.obj \
..\..\_temp\cv$(DR)_icl\cvcalccontrasthistogram.obj \
..\..\_temp\cv$(DR)_icl\cvcanny.obj ..\..\_temp\cv$(DR)_icl\cvcolor.obj \
..\..\_temp\cv$(DR)_icl\cvcontours.obj ..\..\_temp\cv$(DR)_icl\cvcorner.obj \
..\..\_temp\cv$(DR)_icl\cvcornersubpix.obj ..\..\_temp\cv$(DR)_icl\cvderiv.obj \
..\..\_temp\cv$(DR)_icl\cvdetectwr.obj ..\..\_temp\cv$(DR)_icl\cvdrawing.obj \
..\..\_temp\cv$(DR)_icl\cvfeatureselect.obj ..\..\_temp\cv$(DR)_icl\cvfloodfill.obj \
..\..\_temp\cv$(DR)_icl\cvhistogram.obj ..\..\_temp\cv$(DR)_icl\cvimage.obj \
..\..\_temp\cv$(DR)_icl\cvmineval.obj ..\..\_temp\cv$(DR)_icl\cvmorph.obj \
..\..\_temp\cv$(DR)_icl\cvouttext.obj ..\..\_temp\cv$(DR)_icl\cvprecorner.obj \
..\..\_temp\cv$(DR)_icl\cvpyramids.obj ..\..\_temp\cv$(DR)_icl\cvsamplers.obj \
..\..\_temp\cv$(DR)_icl\cvsmooth.obj ..\..\_temp\cv$(DR)_icl\cvtemplmatch.obj \
..\..\_temp\cv$(DR)_icl\cvthresh.obj ..\..\_temp\cv$(DR)_icl\cvapprox.obj \
..\..\_temp\cv$(DR)_icl\cvcontourtree.obj ..\..\_temp\cv$(DR)_icl\cvconvhull.obj \
..\..\_temp\cv$(DR)_icl\cvdistransform.obj ..\..\_temp\cv$(DR)_icl\cvdominants.obj \
..\..\_temp\cv$(DR)_icl\cvemd.obj ..\..\_temp\cv$(DR)_icl\cvgeometry.obj \
..\..\_temp\cv$(DR)_icl\cvhough.obj ..\..\_temp\cv$(DR)_icl\cvlinefit.obj \
..\..\_temp\cv$(DR)_icl\cvmatchcontours.obj ..\..\_temp\cv$(DR)_icl\cvpgh.obj \
..\..\_temp\cv$(DR)_icl\cvproject.obj ..\..\_temp\cv$(DR)_icl\cvpyrsegmentation.obj \
..\..\_temp\cv$(DR)_icl\cvrotcalipers.obj ..\..\_temp\cv$(DR)_icl\cvshapedescr.obj \
..\..\_temp\cv$(DR)_icl\cvsubdivision2d.obj ..\..\_temp\cv$(DR)_icl\camshift.obj \
..\..\_temp\cv$(DR)_icl\cvcamshift.obj ..\..\_temp\cv$(DR)_icl\cvcondens.obj \
..\..\_temp\cv$(DR)_icl\cvkalman.obj ..\..\_temp\cv$(DR)_icl\cvlkpyramid.obj \
..\..\_temp\cv$(DR)_icl\cvmotempl.obj ..\..\_temp\cv$(DR)_icl\cvoptflowbm.obj \
..\..\_temp\cv$(DR)_icl\cvoptflowhs.obj ..\..\_temp\cv$(DR)_icl\cvoptflowlk.obj \
..\..\_temp\cv$(DR)_icl\cvsnakes.obj \
..\..\_temp\cv$(DR)_icl\cvcalcimagehomography.obj \
..\..\_temp\cv$(DR)_icl\cvcalibinit.obj ..\..\_temp\cv$(DR)_icl\cvcalibration.obj \
..\..\_temp\cv$(DR)_icl\cvcorrespond.obj ..\..\_temp\cv$(DR)_icl\cvcreatehandmask.obj \
..\..\_temp\cv$(DR)_icl\cvfindhandregion.obj ..\..\_temp\cv$(DR)_icl\cvfundam.obj \
..\..\_temp\cv$(DR)_icl\cvlines.obj ..\..\_temp\cv$(DR)_icl\cvlmeds.obj \
..\..\_temp\cv$(DR)_icl\cvmorphing.obj ..\..\_temp\cv$(DR)_icl\cvposit.obj \
..\..\_temp\cv$(DR)_icl\cvprewarp.obj ..\..\_temp\cv$(DR)_icl\cvscanlines.obj \
..\..\_temp\cv$(DR)_icl\cvundistort.obj \
..\..\_temp\cv$(DR)_icl\cvcalceigenobjects.obj \
..\..\_temp\cv$(DR)_icl\cveigendecomposite.obj \
..\..\_temp\cv$(DR)_icl\cveigenobjectswrap.obj \
..\..\_temp\cv$(DR)_icl\cveigenprojection.obj ..\..\_temp\cv$(DR)_icl\cvhmm.obj \
..\..\_temp\cv$(DR)_icl\cvhmmobs.obj ..\..\_temp\cv$(DR)_icl\cvhmmwrap.obj 



INC = ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h \
../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h \
../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h \
../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h \
../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h 


CXXFLAGS2 = /I"../src" /I"../include" /nologo /GX /G6 /W4 "/Qwd68,171,424,444,869,981,522,9" /Qaxi /Qxi /c /Fo 
LINKFLAGS2 = /nologo /subsystem:windows /dll /pdb:none /machine:I386 /out:$(OUTBIN) /implib:$(OUTLIB) /nodefaultlib:libm /nodefaultlib:libirc 

!ifdef debug

CXXFLAGS = /D"WIN32" /D"CV_DLL" /D"_WINDOWS" /D"_DEBUG" /MDd /Gm /Zi /Od /FD /GZ $(CXXFLAGS2)
LIBS = kernel32.lib user32.lib gdi32.lib 
LINKFLAGS = $(LINKFLAGS2) /debug

!else

CXXFLAGS = /D"WIN32" /D"NDEBUG" /D"CV_DLL" /D"_WINDOWS" /MD /O3 /Ob2 $(CXXFLAGS2)
LIBS = kernel32.lib user32.lib gdi32.lib 
LINKFLAGS = $(LINKFLAGS2) 

!endif


$(OUTBIN): $(OBJS)
	-mkdir ..\..\bin 2> nul
	-mkdir ..\..\lib 2> nul
	$(LINK) $(LINKFLAGS) $** $(LIBS) 
	

all: $(OUTBIN)

..\..\_temp\cv$(DR)_icl\cvabsdiff.obj: ..\src\cvabsdiff.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	@-mkdir ..\..\_temp\cv$(DR)_icl 2>nul
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvabsdiff.obj ..\src\cvabsdiff.cpp
..\..\_temp\cv$(DR)_icl\cvaccum.obj: ..\src\cvaccum.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvaccum.obj ..\src\cvaccum.cpp
..\..\_temp\cv$(DR)_icl\cvalloc.obj: ..\src\cvalloc.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvalloc.obj ..\src\cvalloc.cpp
..\..\_temp\cv$(DR)_icl\cvarithm.obj: ..\src\cvarithm.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvarithm.obj ..\src\cvarithm.cpp
..\..\_temp\cv$(DR)_icl\cvarray.obj: ..\src\cvarray.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvarray.obj ..\src\cvarray.cpp
..\..\_temp\cv$(DR)_icl\cvcalccovarmatrix.obj: ..\src\cvcalccovarmatrix.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalccovarmatrix.obj ..\src\cvcalccovarmatrix.cpp
..\..\_temp\cv$(DR)_icl\cvcmp.obj: ..\src\cvcmp.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcmp.obj ..\src\cvcmp.cpp
..\..\_temp\cv$(DR)_icl\cvconvert.obj: ..\src\cvconvert.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvconvert.obj ..\src\cvconvert.cpp
..\..\_temp\cv$(DR)_icl\cvcopy.obj: ..\src\cvcopy.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcopy.obj ..\src\cvcopy.cpp
..\..\_temp\cv$(DR)_icl\cvdatastructs.obj: ..\src\cvdatastructs.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdatastructs.obj ..\src\cvdatastructs.cpp
..\..\_temp\cv$(DR)_icl\cvdxt.obj: ..\src\cvdxt.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdxt.obj ..\src\cvdxt.cpp
..\..\_temp\cv$(DR)_icl\cverror.obj: ..\src\cverror.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cverror.obj ..\src\cverror.cpp
..\..\_temp\cv$(DR)_icl\cvjacobieigens.obj: ..\src\cvjacobieigens.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvjacobieigens.obj ..\src\cvjacobieigens.cpp
..\..\_temp\cv$(DR)_icl\cvlogic.obj: ..\src\cvlogic.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvlogic.obj ..\src\cvlogic.cpp
..\..\_temp\cv$(DR)_icl\cvmathfuncs.obj: ..\src\cvmathfuncs.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmathfuncs.obj ..\src\cvmathfuncs.cpp
..\..\_temp\cv$(DR)_icl\cvmatmul.obj: ..\src\cvmatmul.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmatmul.obj ..\src\cvmatmul.cpp
..\..\_temp\cv$(DR)_icl\cvmatrix.obj: ..\src\cvmatrix.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmatrix.obj ..\src\cvmatrix.cpp
..\..\_temp\cv$(DR)_icl\cvmatrix64d.obj: ..\src\cvmatrix64d.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmatrix64d.obj ..\src\cvmatrix64d.cpp
..\..\_temp\cv$(DR)_icl\cvmean.obj: ..\src\cvmean.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmean.obj ..\src\cvmean.cpp
..\..\_temp\cv$(DR)_icl\cvmeansdv.obj: ..\src\cvmeansdv.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmeansdv.obj ..\src\cvmeansdv.cpp
..\..\_temp\cv$(DR)_icl\cvminmaxloc.obj: ..\src\cvminmaxloc.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvminmaxloc.obj ..\src\cvminmaxloc.cpp
..\..\_temp\cv$(DR)_icl\cvmoments.obj: ..\src\cvmoments.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmoments.obj ..\src\cvmoments.cpp
..\..\_temp\cv$(DR)_icl\cvnorm.obj: ..\src\cvnorm.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvnorm.obj ..\src\cvnorm.cpp
..\..\_temp\cv$(DR)_icl\cvpersistence.obj: ..\src\cvpersistence.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvpersistence.obj ..\src\cvpersistence.cpp
..\..\_temp\cv$(DR)_icl\cvprecomp.obj: ..\src\cvprecomp.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvprecomp.obj ..\src\cvprecomp.cpp
..\..\_temp\cv$(DR)_icl\cvrand.obj: ..\src\cvrand.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvrand.obj ..\src\cvrand.cpp
..\..\_temp\cv$(DR)_icl\cvsumpixels.obj: ..\src\cvsumpixels.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsumpixels.obj ..\src\cvsumpixels.cpp
..\..\_temp\cv$(DR)_icl\cvsvd.obj: ..\src\cvsvd.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsvd.obj ..\src\cvsvd.cpp
..\..\_temp\cv$(DR)_icl\cvswitcher.obj: ..\src\cvswitcher.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvswitcher.obj ..\src\cvswitcher.cpp
..\..\_temp\cv$(DR)_icl\cvtables.obj: ..\src\cvtables.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvtables.obj ..\src\cvtables.cpp
..\..\_temp\cv$(DR)_icl\cvutils.obj: ..\src\cvutils.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvutils.obj ..\src\cvutils.cpp
..\..\_temp\cv$(DR)_icl\cvadapthresh.obj: ..\src\cvadapthresh.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvadapthresh.obj ..\src\cvadapthresh.cpp
..\..\_temp\cv$(DR)_icl\cvaffine.obj: ..\src\cvaffine.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvaffine.obj ..\src\cvaffine.cpp
..\..\_temp\cv$(DR)_icl\cvcalccontrasthistogram.obj: ..\src\cvcalccontrasthistogram.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalccontrasthistogram.obj ..\src\cvcalccontrasthistogram.cpp
..\..\_temp\cv$(DR)_icl\cvcanny.obj: ..\src\cvcanny.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcanny.obj ..\src\cvcanny.cpp
..\..\_temp\cv$(DR)_icl\cvcolor.obj: ..\src\cvcolor.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcolor.obj ..\src\cvcolor.cpp
..\..\_temp\cv$(DR)_icl\cvcontours.obj: ..\src\cvcontours.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcontours.obj ..\src\cvcontours.cpp
..\..\_temp\cv$(DR)_icl\cvcorner.obj: ..\src\cvcorner.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcorner.obj ..\src\cvcorner.cpp
..\..\_temp\cv$(DR)_icl\cvcornersubpix.obj: ..\src\cvcornersubpix.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcornersubpix.obj ..\src\cvcornersubpix.cpp
..\..\_temp\cv$(DR)_icl\cvderiv.obj: ..\src\cvderiv.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvderiv.obj ..\src\cvderiv.cpp
..\..\_temp\cv$(DR)_icl\cvdetectwr.obj: ..\src\cvdetectwr.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdetectwr.obj ..\src\cvdetectwr.cpp
..\..\_temp\cv$(DR)_icl\cvdrawing.obj: ..\src\cvdrawing.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdrawing.obj ..\src\cvdrawing.cpp
..\..\_temp\cv$(DR)_icl\cvfeatureselect.obj: ..\src\cvfeatureselect.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvfeatureselect.obj ..\src\cvfeatureselect.cpp
..\..\_temp\cv$(DR)_icl\cvfloodfill.obj: ..\src\cvfloodfill.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvfloodfill.obj ..\src\cvfloodfill.cpp
..\..\_temp\cv$(DR)_icl\cvhistogram.obj: ..\src\cvhistogram.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvhistogram.obj ..\src\cvhistogram.cpp
..\..\_temp\cv$(DR)_icl\cvimage.obj: ..\src\cvimage.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvimage.obj ..\src\cvimage.cpp
..\..\_temp\cv$(DR)_icl\cvmineval.obj: ..\src\cvmineval.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmineval.obj ..\src\cvmineval.cpp
..\..\_temp\cv$(DR)_icl\cvmorph.obj: ..\src\cvmorph.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmorph.obj ..\src\cvmorph.cpp
..\..\_temp\cv$(DR)_icl\cvouttext.obj: ..\src\cvouttext.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvouttext.obj ..\src\cvouttext.cpp
..\..\_temp\cv$(DR)_icl\cvprecorner.obj: ..\src\cvprecorner.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvprecorner.obj ..\src\cvprecorner.cpp
..\..\_temp\cv$(DR)_icl\cvpyramids.obj: ..\src\cvpyramids.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvpyramids.obj ..\src\cvpyramids.cpp
..\..\_temp\cv$(DR)_icl\cvsamplers.obj: ..\src\cvsamplers.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsamplers.obj ..\src\cvsamplers.cpp
..\..\_temp\cv$(DR)_icl\cvsmooth.obj: ..\src\cvsmooth.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsmooth.obj ..\src\cvsmooth.cpp
..\..\_temp\cv$(DR)_icl\cvtemplmatch.obj: ..\src\cvtemplmatch.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvtemplmatch.obj ..\src\cvtemplmatch.cpp
..\..\_temp\cv$(DR)_icl\cvthresh.obj: ..\src\cvthresh.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvthresh.obj ..\src\cvthresh.cpp
..\..\_temp\cv$(DR)_icl\cvapprox.obj: ..\src\cvapprox.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvapprox.obj ..\src\cvapprox.cpp
..\..\_temp\cv$(DR)_icl\cvcontourtree.obj: ..\src\cvcontourtree.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcontourtree.obj ..\src\cvcontourtree.cpp
..\..\_temp\cv$(DR)_icl\cvconvhull.obj: ..\src\cvconvhull.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvconvhull.obj ..\src\cvconvhull.cpp
..\..\_temp\cv$(DR)_icl\cvdistransform.obj: ..\src\cvdistransform.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdistransform.obj ..\src\cvdistransform.cpp
..\..\_temp\cv$(DR)_icl\cvdominants.obj: ..\src\cvdominants.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvdominants.obj ..\src\cvdominants.cpp
..\..\_temp\cv$(DR)_icl\cvemd.obj: ..\src\cvemd.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvemd.obj ..\src\cvemd.cpp
..\..\_temp\cv$(DR)_icl\cvgeometry.obj: ..\src\cvgeometry.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvgeometry.obj ..\src\cvgeometry.cpp
..\..\_temp\cv$(DR)_icl\cvhough.obj: ..\src\cvhough.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvhough.obj ..\src\cvhough.cpp
..\..\_temp\cv$(DR)_icl\cvlinefit.obj: ..\src\cvlinefit.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvlinefit.obj ..\src\cvlinefit.cpp
..\..\_temp\cv$(DR)_icl\cvmatchcontours.obj: ..\src\cvmatchcontours.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmatchcontours.obj ..\src\cvmatchcontours.cpp
..\..\_temp\cv$(DR)_icl\cvpgh.obj: ..\src\cvpgh.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvpgh.obj ..\src\cvpgh.cpp
..\..\_temp\cv$(DR)_icl\cvproject.obj: ..\src\cvproject.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvproject.obj ..\src\cvproject.cpp
..\..\_temp\cv$(DR)_icl\cvpyrsegmentation.obj: ..\src\cvpyrsegmentation.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvpyrsegmentation.obj ..\src\cvpyrsegmentation.cpp
..\..\_temp\cv$(DR)_icl\cvrotcalipers.obj: ..\src\cvrotcalipers.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvrotcalipers.obj ..\src\cvrotcalipers.cpp
..\..\_temp\cv$(DR)_icl\cvshapedescr.obj: ..\src\cvshapedescr.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvshapedescr.obj ..\src\cvshapedescr.cpp
..\..\_temp\cv$(DR)_icl\cvsubdivision2d.obj: ..\src\cvsubdivision2d.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsubdivision2d.obj ..\src\cvsubdivision2d.cpp
..\..\_temp\cv$(DR)_icl\camshift.obj: ..\src\camshift.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\camshift.obj ..\src\camshift.cpp
..\..\_temp\cv$(DR)_icl\cvcamshift.obj: ..\src\cvcamshift.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcamshift.obj ..\src\cvcamshift.cpp
..\..\_temp\cv$(DR)_icl\cvcondens.obj: ..\src\cvcondens.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcondens.obj ..\src\cvcondens.cpp
..\..\_temp\cv$(DR)_icl\cvkalman.obj: ..\src\cvkalman.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvkalman.obj ..\src\cvkalman.cpp
..\..\_temp\cv$(DR)_icl\cvlkpyramid.obj: ..\src\cvlkpyramid.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvlkpyramid.obj ..\src\cvlkpyramid.cpp
..\..\_temp\cv$(DR)_icl\cvmotempl.obj: ..\src\cvmotempl.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmotempl.obj ..\src\cvmotempl.cpp
..\..\_temp\cv$(DR)_icl\cvoptflowbm.obj: ..\src\cvoptflowbm.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvoptflowbm.obj ..\src\cvoptflowbm.cpp
..\..\_temp\cv$(DR)_icl\cvoptflowhs.obj: ..\src\cvoptflowhs.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvoptflowhs.obj ..\src\cvoptflowhs.cpp
..\..\_temp\cv$(DR)_icl\cvoptflowlk.obj: ..\src\cvoptflowlk.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvoptflowlk.obj ..\src\cvoptflowlk.cpp
..\..\_temp\cv$(DR)_icl\cvsnakes.obj: ..\src\cvsnakes.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvsnakes.obj ..\src\cvsnakes.cpp
..\..\_temp\cv$(DR)_icl\cvcalcimagehomography.obj: ..\src\cvcalcimagehomography.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalcimagehomography.obj ..\src\cvcalcimagehomography.cpp
..\..\_temp\cv$(DR)_icl\cvcalibinit.obj: ..\src\cvcalibinit.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalibinit.obj ..\src\cvcalibinit.cpp
..\..\_temp\cv$(DR)_icl\cvcalibration.obj: ..\src\cvcalibration.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalibration.obj ..\src\cvcalibration.cpp
..\..\_temp\cv$(DR)_icl\cvcorrespond.obj: ..\src\cvcorrespond.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcorrespond.obj ..\src\cvcorrespond.cpp
..\..\_temp\cv$(DR)_icl\cvcreatehandmask.obj: ..\src\cvcreatehandmask.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcreatehandmask.obj ..\src\cvcreatehandmask.cpp
..\..\_temp\cv$(DR)_icl\cvfindhandregion.obj: ..\src\cvfindhandregion.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvfindhandregion.obj ..\src\cvfindhandregion.cpp
..\..\_temp\cv$(DR)_icl\cvfundam.obj: ..\src\cvfundam.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvfundam.obj ..\src\cvfundam.cpp
..\..\_temp\cv$(DR)_icl\cvlines.obj: ..\src\cvlines.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvlines.obj ..\src\cvlines.cpp
..\..\_temp\cv$(DR)_icl\cvlmeds.obj: ..\src\cvlmeds.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvlmeds.obj ..\src\cvlmeds.cpp
..\..\_temp\cv$(DR)_icl\cvmorphing.obj: ..\src\cvmorphing.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvmorphing.obj ..\src\cvmorphing.cpp
..\..\_temp\cv$(DR)_icl\cvposit.obj: ..\src\cvposit.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvposit.obj ..\src\cvposit.cpp
..\..\_temp\cv$(DR)_icl\cvprewarp.obj: ..\src\cvprewarp.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvprewarp.obj ..\src\cvprewarp.cpp
..\..\_temp\cv$(DR)_icl\cvscanlines.obj: ..\src\cvscanlines.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvscanlines.obj ..\src\cvscanlines.cpp
..\..\_temp\cv$(DR)_icl\cvundistort.obj: ..\src\cvundistort.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvundistort.obj ..\src\cvundistort.cpp
..\..\_temp\cv$(DR)_icl\cvcalceigenobjects.obj: ..\src\cvcalceigenobjects.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvcalceigenobjects.obj ..\src\cvcalceigenobjects.cpp
..\..\_temp\cv$(DR)_icl\cveigendecomposite.obj: ..\src\cveigendecomposite.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cveigendecomposite.obj ..\src\cveigendecomposite.cpp
..\..\_temp\cv$(DR)_icl\cveigenobjectswrap.obj: ..\src\cveigenobjectswrap.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cveigenobjectswrap.obj ..\src\cveigenobjectswrap.cpp
..\..\_temp\cv$(DR)_icl\cveigenprojection.obj: ..\src\cveigenprojection.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cveigenprojection.obj ..\src\cveigenprojection.cpp
..\..\_temp\cv$(DR)_icl\cvhmm.obj: ..\src\cvhmm.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvhmm.obj ..\src\cvhmm.cpp
..\..\_temp\cv$(DR)_icl\cvhmmobs.obj: ..\src\cvhmmobs.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvhmmobs.obj ..\src\cvhmmobs.cpp
..\..\_temp\cv$(DR)_icl\cvhmmwrap.obj: ..\src\cvhmmwrap.cpp ../include/cv.h ../include/cv.hpp ../include/cvcompat.h ../include/cverror.h ../include/cvtypes.h ../src/_cv.h ../src/_cvarr.h ../src/_cvdatastructs.h ../src/_cverror.h ../src/_cvfuncn.h ../src/_cvgeom.h ../src/_cvlist.h ../src/_cvmatrix.h ../src/_cvoptions.h ../src/_cvtables.h ../src/_cvutils.h ../src/_cvvm.h ../src/_ipcv.h ../src/_optcv.h
	-$(CXX) $(CXXFLAGS)..\..\_temp\cv$(DR)_icl\cvhmmwrap.obj ..\src\cvhmmwrap.cpp
