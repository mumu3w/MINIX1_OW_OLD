#!/bin/bash

function tools {
	cd tools
	echo "Start compiling dos2out & build & minixfsutils"
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
	gcc -o minixfsutils minixfsutils.c 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile minixfsutils"
		exit
	fi
	cd ..
}

function libsys {
	cd libsys
	rm -fr libsys.lib
	echo "Start compiling libsys"
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

function libc {
	cd libc
	rm -fr libc.lib
	echo "Start compiling libc"
	wmake all 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile libc"
		exit
	fi
	cd ..
}

function test {
	cd test
	echo "Start compiling test"
	wmake all 1>>../log.txt 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to compile test"
		exit
	fi
	../tools/minixfsutils ../tools/c.img.old put /user/test/t10a t10a.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/t11b t11b.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test10 test10.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test1 test1.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test3 test3.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test5 test5.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test7 test7.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test9 test9.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/t11a t11a.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test0 test0.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test11 test11.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test2 test2.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test4 test4.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test6 test6.out
	../tools/minixfsutils ../tools/c.img.old put /user/test/test8 test8.out
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
libc
image ./tools bootblok.bin kernel mm fs init fsck image
cd tools
tar xf img.tar.xz
cd ..
test
