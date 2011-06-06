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

OUTBIN = ..\..\..\bin\cvtest$(SUFFIX).exe
OUTLIB = ..\..\..\lib\cvtest$(SUFFIX).lib

OBJS = ..\..\..\_temp\cvtest$(DR)_icl\aabsdiff.obj \
..\..\..\_temp\cvtest$(DR)_icl\aaccum.obj \
..\..\..\_temp\cvtest$(DR)_icl\aadaptthresh.obj \
..\..\..\_temp\cvtest$(DR)_icl\aapproxpoly.obj \
..\..\..\_temp\cvtest$(DR)_icl\aarithm.obj \
..\..\..\_temp\cvtest$(DR)_icl\aarrayiterator.obj \
..\..\..\_temp\cvtest$(DR)_icl\abackproject.obj \
..\..\..\_temp\cvtest$(DR)_icl\acalccontrasthist.obj \
..\..\..\_temp\cvtest$(DR)_icl\acalculate.obj \
..\..\..\_temp\cvtest$(DR)_icl\acameracalibration.obj \
..\..\..\_temp\cvtest$(DR)_icl\acamshift.obj \
..\..\..\_temp\cvtest$(DR)_icl\acanny.obj \
..\..\..\_temp\cvtest$(DR)_icl\achesscorners.obj \
..\..\..\_temp\cvtest$(DR)_icl\acondens.obj \
..\..\..\_temp\cvtest$(DR)_icl\acontourcollection.obj \
..\..\..\_temp\cvtest$(DR)_icl\acontourmoments.obj \
..\..\..\_temp\cvtest$(DR)_icl\acontours.obj \
..\..\..\_temp\cvtest$(DR)_icl\acontoursmatch.obj \
..\..\..\_temp\cvtest$(DR)_icl\aconvert.obj \
..\..\..\_temp\cvtest$(DR)_icl\aconvhull.obj \
..\..\..\_temp\cvtest$(DR)_icl\acorner.obj \
..\..\..\_temp\cvtest$(DR)_icl\acreatecontourtree.obj \
..\..\..\_temp\cvtest$(DR)_icl\adistancetransform.obj \
..\..\..\_temp\cvtest$(DR)_icl\adrawing.obj \
..\..\..\_temp\cvtest$(DR)_icl\adrawing_regress.obj \
..\..\..\_temp\cvtest$(DR)_icl\aeigenobjects.obj \
..\..\..\_temp\cvtest$(DR)_icl\aemd.obj \
..\..\..\_temp\cvtest$(DR)_icl\afitellipse.obj \
..\..\..\_temp\cvtest$(DR)_icl\afitlines.obj \
..\..\..\_temp\cvtest$(DR)_icl\afloodfill.obj \
..\..\..\_temp\cvtest$(DR)_icl\afloodfill8.obj \
..\..\..\_temp\cvtest$(DR)_icl\ahaar.obj \
..\..\..\_temp\cvtest$(DR)_icl\ahistogram.obj \
..\..\..\_temp\cvtest$(DR)_icl\ahistograms.obj \
..\..\..\_temp\cvtest$(DR)_icl\ahmmobs.obj \
..\..\..\_temp\cvtest$(DR)_icl\ahoughtransform.obj \
..\..\..\_temp\cvtest$(DR)_icl\aimage.obj \
..\..\..\_temp\cvtest$(DR)_icl\aimagestatistics.obj \
..\..\..\_temp\cvtest$(DR)_icl\akalman.obj ..\..\..\_temp\cvtest$(DR)_icl\akmeans.obj \
..\..\..\_temp\cvtest$(DR)_icl\alogic.obj \
..\..\..\_temp\cvtest$(DR)_icl\amaskaccum.obj \
..\..\..\_temp\cvtest$(DR)_icl\amatchcontourtrees.obj \
..\..\..\_temp\cvtest$(DR)_icl\amathutils.obj \
..\..\..\_temp\cvtest$(DR)_icl\amatrix.obj \
..\..\..\_temp\cvtest$(DR)_icl\amatrix2.obj \
..\..\..\_temp\cvtest$(DR)_icl\ameanshift.obj \
..\..\..\_temp\cvtest$(DR)_icl\aminarearect.obj \
..\..\..\_temp\cvtest$(DR)_icl\amineval.obj \
..\..\..\_temp\cvtest$(DR)_icl\amoments.obj \
..\..\..\_temp\cvtest$(DR)_icl\amorphology.obj \
..\..\..\_temp\cvtest$(DR)_icl\amotiontemplates.obj \
..\..\..\_temp\cvtest$(DR)_icl\amotseg.obj \
..\..\..\_temp\cvtest$(DR)_icl\anodeiterator.obj \
..\..\..\_temp\cvtest$(DR)_icl\anorm.obj ..\..\..\_temp\cvtest$(DR)_icl\anormmask.obj \
..\..\..\_temp\cvtest$(DR)_icl\aoperations.obj \
..\..\..\_temp\cvtest$(DR)_icl\aoptflowhs.obj \
..\..\..\_temp\cvtest$(DR)_icl\aoptflowlk.obj \
..\..\..\_temp\cvtest$(DR)_icl\aoptflowpyrlk.obj \
..\..\..\_temp\cvtest$(DR)_icl\apixelaccess.obj \
..\..\..\_temp\cvtest$(DR)_icl\aposit.obj \
..\..\..\_temp\cvtest$(DR)_icl\aprecorner.obj \
..\..\..\_temp\cvtest$(DR)_icl\apyramids.obj \
..\..\..\_temp\cvtest$(DR)_icl\apyrsegmentation.obj \
..\..\..\_temp\cvtest$(DR)_icl\asamplers.obj \
..\..\..\_temp\cvtest$(DR)_icl\asequence.obj \
..\..\..\_temp\cvtest$(DR)_icl\asnakes.obj ..\..\..\_temp\cvtest$(DR)_icl\asobel.obj \
..\..\..\_temp\cvtest$(DR)_icl\astoragearray.obj \
..\..\..\_temp\cvtest$(DR)_icl\asubdivisions.obj \
..\..\..\_temp\cvtest$(DR)_icl\asvd.obj \
..\..\..\_temp\cvtest$(DR)_icl\atemplmatch.obj \
..\..\..\_temp\cvtest$(DR)_icl\athresh.obj ..\..\..\_temp\cvtest$(DR)_icl\atree.obj \
..\..\..\_temp\cvtest$(DR)_icl\atreeiterator.obj \
..\..\..\_temp\cvtest$(DR)_icl\ats.obj ..\..\..\_temp\cvtest$(DR)_icl\aundistort.obj \
..\..\..\_temp\cvtest$(DR)_icl\canny.obj ..\..\..\_temp\cvtest$(DR)_icl\cvtest.obj \
..\..\..\_temp\cvtest$(DR)_icl\tsysa.obj ..\..\..\_temp\cvtest$(DR)_icl\ucontours.obj \
..\..\..\_temp\cvtest$(DR)_icl\ugraphics.obj \
..\..\..\_temp\cvtest$(DR)_icl\uiplutils.obj \
..\..\..\_temp\cvtest$(DR)_icl\ureadfile.obj 



INC = ./cvtest.h 


CXXFLAGS2 = /I"../../../cvaux/include" /I"../../trs/include" /I"../../../cv/include" /I"../../../otherlibs/highgui" /I"." /nologo /GX /G6 /W4 "/Qwd68,171,424,444,869,981,522,9" /Qaxi /Qxi /c /Fo 
LINKFLAGS2 = /libpath:"../../../lib" /nologo /subsystem:console /pdb:none /machine:I386 /out:$(OUTBIN) 

!ifdef debug

CXXFLAGS = /D"WIN32" /D"_MBCS" /D"_CONSOLE" /D"_DEBUG" /MDd /Gm /Zi /Od /FD /GZ $(CXXFLAGS2)
LIBS = cvd.lib cvauxd.lib highguid.lib trsd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib 
LINKFLAGS = $(LINKFLAGS2) /debug

!else

CXXFLAGS = /D"WIN32" /D"NDEBUG" /D"_CONSOLE" /D"_MBCS" /MD /O3 /Ob2 $(CXXFLAGS2)
LIBS = cv.lib cvaux.lib highgui.lib trs.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib 
LINKFLAGS = $(LINKFLAGS2) 

!endif


$(OUTBIN): $(OBJS)
	-mkdir ..\..\..\bin 2> nul
	-mkdir ..\..\..\lib 2> nul
	$(LINK) $(LINKFLAGS) $** $(LIBS) 
	

all: $(OUTBIN)

..\..\..\_temp\cvtest$(DR)_icl\aabsdiff.obj: .\aabsdiff.cpp ./cvtest.h
	@-mkdir ..\..\..\_temp\cvtest$(DR)_icl 2>nul
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aabsdiff.obj .\aabsdiff.cpp
..\..\..\_temp\cvtest$(DR)_icl\aaccum.obj: .\aaccum.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aaccum.obj .\aaccum.cpp
..\..\..\_temp\cvtest$(DR)_icl\aadaptthresh.obj: .\aadaptthresh.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aadaptthresh.obj .\aadaptthresh.cpp
..\..\..\_temp\cvtest$(DR)_icl\aapproxpoly.obj: .\aapproxpoly.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aapproxpoly.obj .\aapproxpoly.cpp
..\..\..\_temp\cvtest$(DR)_icl\aarithm.obj: .\aarithm.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aarithm.obj .\aarithm.cpp
..\..\..\_temp\cvtest$(DR)_icl\aarrayiterator.obj: .\aarrayiterator.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aarrayiterator.obj .\aarrayiterator.cpp
..\..\..\_temp\cvtest$(DR)_icl\abackproject.obj: .\abackproject.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\abackproject.obj .\abackproject.cpp
..\..\..\_temp\cvtest$(DR)_icl\acalccontrasthist.obj: .\acalccontrasthist.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acalccontrasthist.obj .\acalccontrasthist.cpp
..\..\..\_temp\cvtest$(DR)_icl\acalculate.obj: .\acalculate.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acalculate.obj .\acalculate.cpp
..\..\..\_temp\cvtest$(DR)_icl\acameracalibration.obj: .\acameracalibration.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acameracalibration.obj .\acameracalibration.cpp
..\..\..\_temp\cvtest$(DR)_icl\acamshift.obj: .\acamshift.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acamshift.obj .\acamshift.cpp
..\..\..\_temp\cvtest$(DR)_icl\acanny.obj: .\acanny.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acanny.obj .\acanny.cpp
..\..\..\_temp\cvtest$(DR)_icl\achesscorners.obj: .\achesscorners.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\achesscorners.obj .\achesscorners.cpp
..\..\..\_temp\cvtest$(DR)_icl\acondens.obj: .\acondens.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acondens.obj .\acondens.cpp
..\..\..\_temp\cvtest$(DR)_icl\acontourcollection.obj: .\acontourcollection.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acontourcollection.obj .\acontourcollection.cpp
..\..\..\_temp\cvtest$(DR)_icl\acontourmoments.obj: .\acontourmoments.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acontourmoments.obj .\acontourmoments.cpp
..\..\..\_temp\cvtest$(DR)_icl\acontours.obj: .\acontours.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acontours.obj .\acontours.cpp
..\..\..\_temp\cvtest$(DR)_icl\acontoursmatch.obj: .\acontoursmatch.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acontoursmatch.obj .\acontoursmatch.cpp
..\..\..\_temp\cvtest$(DR)_icl\aconvert.obj: .\aconvert.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aconvert.obj .\aconvert.cpp
..\..\..\_temp\cvtest$(DR)_icl\aconvhull.obj: .\aconvhull.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aconvhull.obj .\aconvhull.cpp
..\..\..\_temp\cvtest$(DR)_icl\acorner.obj: .\acorner.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acorner.obj .\acorner.cpp
..\..\..\_temp\cvtest$(DR)_icl\acreatecontourtree.obj: .\acreatecontourtree.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\acreatecontourtree.obj .\acreatecontourtree.cpp
..\..\..\_temp\cvtest$(DR)_icl\adistancetransform.obj: .\adistancetransform.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\adistancetransform.obj .\adistancetransform.cpp
..\..\..\_temp\cvtest$(DR)_icl\adrawing.obj: .\adrawing.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\adrawing.obj .\adrawing.cpp
..\..\..\_temp\cvtest$(DR)_icl\adrawing_regress.obj: .\adrawing_regress.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\adrawing_regress.obj .\adrawing_regress.cpp
..\..\..\_temp\cvtest$(DR)_icl\aeigenobjects.obj: .\aeigenobjects.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aeigenobjects.obj .\aeigenobjects.cpp
..\..\..\_temp\cvtest$(DR)_icl\aemd.obj: .\aemd.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aemd.obj .\aemd.cpp
..\..\..\_temp\cvtest$(DR)_icl\afitellipse.obj: .\afitellipse.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\afitellipse.obj .\afitellipse.cpp
..\..\..\_temp\cvtest$(DR)_icl\afitlines.obj: .\afitlines.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\afitlines.obj .\afitlines.cpp
..\..\..\_temp\cvtest$(DR)_icl\afloodfill.obj: .\afloodfill.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\afloodfill.obj .\afloodfill.cpp
..\..\..\_temp\cvtest$(DR)_icl\afloodfill8.obj: .\afloodfill8.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\afloodfill8.obj .\afloodfill8.cpp
..\..\..\_temp\cvtest$(DR)_icl\ahaar.obj: .\ahaar.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ahaar.obj .\ahaar.cpp
..\..\..\_temp\cvtest$(DR)_icl\ahistogram.obj: .\ahistogram.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ahistogram.obj .\ahistogram.cpp
..\..\..\_temp\cvtest$(DR)_icl\ahistograms.obj: .\ahistograms.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ahistograms.obj .\ahistograms.cpp
..\..\..\_temp\cvtest$(DR)_icl\ahmmobs.obj: .\ahmmobs.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ahmmobs.obj .\ahmmobs.cpp
..\..\..\_temp\cvtest$(DR)_icl\ahoughtransform.obj: .\ahoughtransform.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ahoughtransform.obj .\ahoughtransform.cpp
..\..\..\_temp\cvtest$(DR)_icl\aimage.obj: .\aimage.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aimage.obj .\aimage.cpp
..\..\..\_temp\cvtest$(DR)_icl\aimagestatistics.obj: .\aimagestatistics.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aimagestatistics.obj .\aimagestatistics.cpp
..\..\..\_temp\cvtest$(DR)_icl\akalman.obj: .\akalman.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\akalman.obj .\akalman.cpp
..\..\..\_temp\cvtest$(DR)_icl\akmeans.obj: .\akmeans.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\akmeans.obj .\akmeans.cpp
..\..\..\_temp\cvtest$(DR)_icl\alogic.obj: .\alogic.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\alogic.obj .\alogic.cpp
..\..\..\_temp\cvtest$(DR)_icl\amaskaccum.obj: .\amaskaccum.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amaskaccum.obj .\amaskaccum.cpp
..\..\..\_temp\cvtest$(DR)_icl\amatchcontourtrees.obj: .\amatchcontourtrees.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amatchcontourtrees.obj .\amatchcontourtrees.cpp
..\..\..\_temp\cvtest$(DR)_icl\amathutils.obj: .\amathutils.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amathutils.obj .\amathutils.cpp
..\..\..\_temp\cvtest$(DR)_icl\amatrix.obj: .\amatrix.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amatrix.obj .\amatrix.cpp
..\..\..\_temp\cvtest$(DR)_icl\amatrix2.obj: .\amatrix2.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amatrix2.obj .\amatrix2.cpp
..\..\..\_temp\cvtest$(DR)_icl\ameanshift.obj: .\ameanshift.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ameanshift.obj .\ameanshift.cpp
..\..\..\_temp\cvtest$(DR)_icl\aminarearect.obj: .\aminarearect.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aminarearect.obj .\aminarearect.cpp
..\..\..\_temp\cvtest$(DR)_icl\amineval.obj: .\amineval.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amineval.obj .\amineval.cpp
..\..\..\_temp\cvtest$(DR)_icl\amoments.obj: .\amoments.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amoments.obj .\amoments.cpp
..\..\..\_temp\cvtest$(DR)_icl\amorphology.obj: .\amorphology.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amorphology.obj .\amorphology.cpp
..\..\..\_temp\cvtest$(DR)_icl\amotiontemplates.obj: .\amotiontemplates.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amotiontemplates.obj .\amotiontemplates.cpp
..\..\..\_temp\cvtest$(DR)_icl\amotseg.obj: .\amotseg.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\amotseg.obj .\amotseg.cpp
..\..\..\_temp\cvtest$(DR)_icl\anodeiterator.obj: .\anodeiterator.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\anodeiterator.obj .\anodeiterator.cpp
..\..\..\_temp\cvtest$(DR)_icl\anorm.obj: .\anorm.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\anorm.obj .\anorm.cpp
..\..\..\_temp\cvtest$(DR)_icl\anormmask.obj: .\anormmask.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\anormmask.obj .\anormmask.cpp
..\..\..\_temp\cvtest$(DR)_icl\aoperations.obj: .\aoperations.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aoperations.obj .\aoperations.cpp
..\..\..\_temp\cvtest$(DR)_icl\aoptflowhs.obj: .\aoptflowhs.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aoptflowhs.obj .\aoptflowhs.cpp
..\..\..\_temp\cvtest$(DR)_icl\aoptflowlk.obj: .\aoptflowlk.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aoptflowlk.obj .\aoptflowlk.cpp
..\..\..\_temp\cvtest$(DR)_icl\aoptflowpyrlk.obj: .\aoptflowpyrlk.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aoptflowpyrlk.obj .\aoptflowpyrlk.cpp
..\..\..\_temp\cvtest$(DR)_icl\apixelaccess.obj: .\apixelaccess.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\apixelaccess.obj .\apixelaccess.cpp
..\..\..\_temp\cvtest$(DR)_icl\aposit.obj: .\aposit.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aposit.obj .\aposit.cpp
..\..\..\_temp\cvtest$(DR)_icl\aprecorner.obj: .\aprecorner.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aprecorner.obj .\aprecorner.cpp
..\..\..\_temp\cvtest$(DR)_icl\apyramids.obj: .\apyramids.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\apyramids.obj .\apyramids.cpp
..\..\..\_temp\cvtest$(DR)_icl\apyrsegmentation.obj: .\apyrsegmentation.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\apyrsegmentation.obj .\apyrsegmentation.cpp
..\..\..\_temp\cvtest$(DR)_icl\asamplers.obj: .\asamplers.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asamplers.obj .\asamplers.cpp
..\..\..\_temp\cvtest$(DR)_icl\asequence.obj: .\asequence.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asequence.obj .\asequence.cpp
..\..\..\_temp\cvtest$(DR)_icl\asnakes.obj: .\asnakes.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asnakes.obj .\asnakes.cpp
..\..\..\_temp\cvtest$(DR)_icl\asobel.obj: .\asobel.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asobel.obj .\asobel.cpp
..\..\..\_temp\cvtest$(DR)_icl\astoragearray.obj: .\astoragearray.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\astoragearray.obj .\astoragearray.cpp
..\..\..\_temp\cvtest$(DR)_icl\asubdivisions.obj: .\asubdivisions.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asubdivisions.obj .\asubdivisions.cpp
..\..\..\_temp\cvtest$(DR)_icl\asvd.obj: .\asvd.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\asvd.obj .\asvd.cpp
..\..\..\_temp\cvtest$(DR)_icl\atemplmatch.obj: .\atemplmatch.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\atemplmatch.obj .\atemplmatch.cpp
..\..\..\_temp\cvtest$(DR)_icl\athresh.obj: .\athresh.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\athresh.obj .\athresh.cpp
..\..\..\_temp\cvtest$(DR)_icl\atree.obj: .\atree.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\atree.obj .\atree.cpp
..\..\..\_temp\cvtest$(DR)_icl\atreeiterator.obj: .\atreeiterator.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\atreeiterator.obj .\atreeiterator.cpp
..\..\..\_temp\cvtest$(DR)_icl\ats.obj: .\ats.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ats.obj .\ats.cpp
..\..\..\_temp\cvtest$(DR)_icl\aundistort.obj: .\aundistort.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\aundistort.obj .\aundistort.cpp
..\..\..\_temp\cvtest$(DR)_icl\canny.obj: .\canny.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\canny.obj .\canny.cpp
..\..\..\_temp\cvtest$(DR)_icl\cvtest.obj: .\cvtest.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\cvtest.obj .\cvtest.cpp
..\..\..\_temp\cvtest$(DR)_icl\tsysa.obj: .\tsysa.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\tsysa.obj .\tsysa.cpp
..\..\..\_temp\cvtest$(DR)_icl\ucontours.obj: .\ucontours.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ucontours.obj .\ucontours.cpp
..\..\..\_temp\cvtest$(DR)_icl\ugraphics.obj: .\ugraphics.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ugraphics.obj .\ugraphics.cpp
..\..\..\_temp\cvtest$(DR)_icl\uiplutils.obj: .\uiplutils.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\uiplutils.obj .\uiplutils.cpp
..\..\..\_temp\cvtest$(DR)_icl\ureadfile.obj: .\ureadfile.cpp ./cvtest.h
	-$(CXX) $(CXXFLAGS)..\..\..\_temp\cvtest$(DR)_icl\ureadfile.obj .\ureadfile.cpp
