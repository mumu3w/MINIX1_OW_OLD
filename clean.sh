#!/bin/bash
cd tools
rm build dos2out
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
rm -fr *.exe *.map *.out *.obj *.err bootblok.bin
cd ..

rm -fr log.txt tools/*.img
