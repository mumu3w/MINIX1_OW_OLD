@echo off
@cd lib
nmake all 2>&1 >>../log.txt
@cd ..

@cd kernel
nmake kernel.exe 2>&1 >>../log.txt
dos2out kernel.exe 2>&1 >>../log.txt
@cd ..

@cd fs
nmake fs.exe 2>&1 >>../log.txt
dos2out fs.exe 2>&1 >>../log.txt
@cd ..

@cd mm
nmake mm.exe 2>&1 >>../log.txt
dos2out mm.exe 2>&1 >>../log.txt
@cd ..

@cd tools
nmake all 2>&1 >>../log.txt
dos2out init.exe 2>&1 >>../log.txt
dos2out fsck.exe 2>&1 >>../log.txt
build bootblok.bin ..\kernel\kernel.out ..\mm\mm.out ..\fs\fs.out init.out fsck.out image
@cd ..

@echo on