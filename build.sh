#!/bin/bash

function tools {
	cd tools
	echo "Start compiling dos2out & build"
	gcc -o build build.c 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile build"
		exit
	fi	
	gcc -o dos2out dos2out.c 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile dos2out"
		exit
	fi
	cd ..
}

function libsys {
	cd libsys
	rm -fr libsys.lib
	echo "Start compiling library"
	wmake all 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile libsys"
		exit
	fi
	cd ..
}

function kernel {
	cd kernel
	echo "Start compiling kernel"
	wmake kernel.exe 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile kernel"
		exit
	fi
	../tools/dos2out kernel 1>>../log.txt 2>&1
	cd ..
}

function fs {
	cd fs
	echo "Start compiling fs"
	wmake fs.exe 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile fs"
		exit
	fi
	../tools/dos2out fs 1>>../log.txt 2>&1
	cd ..
}

function mm {
	cd mm
	echo "Start compiling mm"
	wmake mm.exe 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile mm"
		exit
	fi
	../tools/dos2out mm 1>>../log.txt 2>&1
	cd ..
}

function init_fsck {
	cd tools
	echo "Start compiling init & fsck"
	wmake all 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile init_fsck"
		exit
	fi
	../tools/dos2out init 1>>../log.txt 2>&1
	../tools/dos2out fsck 1>>../log.txt 2>&1
	cd ..
}

function image {
	$1/build $1/$2 ./kernel/$3.out ./mm/$4.out ./fs/$5.out $1/$6.out $1/$7.out $1/$8
	if [ $? -ne 0 ]; then
		echo "Failed to Image"
		exit
	fi
}

echo "The MINIX documentation is contained in the appendices of the following book:"
echo "	Title:     Operating Systems: Design and Implementation"
echo "	Author:    Andrew S. Tanenbaum"
echo "	Publisher: Prentice-Hall (1987)"
echo " "
rm -fr log.txt
tools
libsys
kernel
fs
mm
init_fsck
image ./tools bootblok.bin kernel mm fs init fsck image
cd tools
tar xf img.tar
cd ..