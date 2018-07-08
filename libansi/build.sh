#!/bin/bash
export CFLAGS="-I../_headers -c -Di8088 -ms -4 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj -zro -fpi87 -fp3 -op"
export LIBCNAME="../libansi.lib"

cd assert   
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd ctype  
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd float 
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd locale  
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd stdio   
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd string  
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd time
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd errno 
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd math
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..

cd stdlib
wcl $CFLAGS *.c
wlib -q -fo $LIBCNAME *.obj
cd ..
