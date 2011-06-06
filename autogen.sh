#!/bin/sh
# Primary configuration script
# Serguei Boldyrev, s_boldyrev@mail.ru, 10.04.2002
# Modified by VP: a part of autogen.sh from DIA has been borrowed
#
echo "Generating build information using aclocal, automake and autoconf"
echo "This may take a while ..."

# Touch the timestamps on all the files since CVS messes them up
touch ./configure.in

echo "Running intltoolize"
intltoolize --copy --force --automake

echo "Running libtoolize"
libtoolize --copy --force

aclocal $ACLOCAL_FLAGS
autoheader
automake --add-missing $am_opt
autoconf

chmod +x utils/unix2dos.py
chmod +x utils/dos2unix.py

echo "Now you are ready to run ./configure"
