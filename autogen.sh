#!/bin/sh
# Primary configuration script
# Serguei Boldyrev, s_boldyrev@mail.ru, 10.04.2002
# 
echo "Generating build information using aclocal, automake and autoconf"
echo "This may take a while ..."

# Touch the timestamps on all the files since CVS messes them up
# directory=`dirname $0`
touch ./configure.in

# Regenerate configuration files
aclocal
automake  -a -c 
autoconf

#(cd tests; aclocal; automake --include-deps  --generate-deps --add-missing; autoconf)
#(cd cv; aclocal; automake --include-deps  --generate-deps --add-missing; autoconf)
#(cd cvaux; aclocal; automake --include-deps  --generate-deps --add-missing; autoconf)
#(cd optcv; aclocal; automake --include-deps  --generate-deps --add-missing; autoconf)
#(cd ippcv; aclocal; automake --include-deps  --generate-deps --add-missing; autoconf)

# Run configure for this platform
#./configure $*
echo "Now you are ready to run ./configure"
