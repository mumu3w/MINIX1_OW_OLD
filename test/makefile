CROSSCC = wcc
CROSSLD = wlink
CROSSASM= nasm -f obj
CROSSCFLAGS = -I../include -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
#CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj

OUT1 = test0.exe test1.exe test2.exe test3.exe test4.exe
OUT2 = test5.exe test6.exe test7.exe test8.exe test9.exe
OUT3 = test10.exe test11.exe t11a.exe t10a.exe t11b.exe 
OUTS = $(OUT1) $(OUT2) $(OUT3)

all : $(OUTS)
	
.obj.exe : 
	$(CROSSLD) system dos option map F { ../libsys/crtso.obj $< } L { ../libsys/libsys.lib ../libc/libc.lib } N $@
	../tools/dos2out $@ > /dev/null

.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
  
.asm.obj :
	$(CROSSASM) $<
	