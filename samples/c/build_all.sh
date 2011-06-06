#!/bin/sh
for i in *.c; do g++ `opencv-config --cxxflags` -o `basename $i .c` $i `opencv-config --libs`; done
