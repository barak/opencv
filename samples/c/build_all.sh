#!/bin/sh
for i in *.c; do
    echo "compiling $i"
    g++ -ggdb -O0 `pkg-config --cflags opencv` -o `basename $i .c` $i `pkg-config --libs opencv`;
done

