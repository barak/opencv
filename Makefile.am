# The top-level input Makefile for OpenCV

# require automake 1.4
# AUTOMAKE_OPTIONS = 1.4

## Any directories that you want built and installed should go here.
SUBDIRS = cv cvaux otherlibs tests @ADDONS@

## Any directories you want a part of the distribution should be listed
## here, as well as have a Makefile generated at the end of configure.in
##
## This only works for subdirectories one level deep.
DIST_SUBDIRS = $(SUBDIRS) apps tests utils docs samples _dsw

# OpenCV runtime configuration script
bin_SCRIPTS = opencv-config

# All the rest of the distributed files
EXTRA_DIST = Makefile.gcc Makefile.bcc Makefile.icl Makefile.vc THANKS COPYING INSTALL README OpenCV.spec OpenCV.spec.in autogen.sh

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	$(srcdir)/utils/unix2dos.pl  $(distdir)/_dsw/OpenCVlight.dsw
	$(srcdir)/utils/unix2dos.pl  $(distdir)/cv/make/cv.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/cvaux/make/cvaux.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/otherlibs/highgui/highgui.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/otherlibs/cvcam/make_win/cvcam.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/tests/trs/make/trs.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/tests/cv/src/cvtest.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/apps/cvlkdemo/cvlkdemo.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/apps/cvcsdemo/cvcsdemo.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/apps/vmdemotk/VMDemoTk.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/apps/HaarFaceDetect/HaarFaceDetect.dsp
	$(srcdir)/utils/unix2dos.pl  $(distdir)/utils/cvinfo/cvinfo.dsp
	

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule to force automake to rebuild the library
changed:
	@echo "This build target is no longer necessary"

# Rule to install the libraries only - prevent rebuilding apps
install-lib:
	cd src && $(MAKE) install-libLTLIBRARIES

# Run ldconfig after installing the library:
install-hook:
	-ldconfig
