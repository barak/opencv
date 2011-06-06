# Intel Proton Compiler >=5.0

!ifdef debug
OPT= debug=1
!else
OPT= 
!endif

all:
	cd cv\make
	nmake -f makefile.icl $(OPT)
	cd ..\..
	cd cvaux\make
	nmake -f makefile.icl $(OPT)
	cd ..\..
	cd otherlibs\highgui
	nmake -f makefile.icl $(OPT)
	cd ..\..
	cd tests\trs\make
	nmake -f makefile.icl $(OPT)
	cd ..\..\..
	cd tests\cv\src
	nmake -f makefile.icl $(OPT)
	cd ..\..\..
	cd samples\c
	nmake -f makefile.icl $(OPT)
	cd ..\..