#!/bin/bash
cd tools
rm -fr build dos2out minixfsutils
cd ..

cd libsys
rm -fr *.lib *.obj *.err
cd ..

cd kernel
rm -fr kernel.exe kernel.map kernel.out *.obj *.err
cd ..

cd fs
rm -fr fs.exe fs.map fs.out *.obj *.err
cd ..

cd mm
rm -fr mm.exe mm.map mm.out *.obj *.err
cd ..

cd tools
rm -fr *.exe *.map *.out *.obj *.err bootblok.bin *.img c.img.old image
cd ..

cd libc
rm -fr *.lib *.obj *.err
cd ..

cd test
rm -fr *.obj *.err *.map *.exe *.out
cd ..

cd test2
rm -fr *.obj *.err *.map *.exe *.out
cd ..

rm -fr log.txt
