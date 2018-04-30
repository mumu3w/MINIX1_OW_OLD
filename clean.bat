@echo off
rem cd tools
rem rm build dos2out
rem cd ..

@cd lib
@del minix.lib *.obj *.err
@cd ..

@cd kernel
@del kernel.exe kernel.map kernel.out *.obj *.err
@cd ..

@cd fs
@del fs.exe fs.map fs.out *.obj *.err
@cd ..

@cd mm
@del mm.exe mm.map mm.out *.obj *.err
@cd ..

@cd tools
@del *.exe *.map *.out *.obj *.err bootblok.bin
@cd ..

@del log.txt
@echo on