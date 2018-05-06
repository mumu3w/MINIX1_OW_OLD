CROSSCC = wcc
CROSSLD = wlink
CROSSASM= nasm -f obj
CROSSCFLAGS = -I../libansi/_headers -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj -zro -fpi87
#CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj

OUT1 = tctype.exe  tfloat.exe   tlocale.exe  tmath2.exe  tsetjmp.exe  tstdarg.exe  tstdio1.exe  tstdlib.exe  ttime.exe
OUT2 = tassert.exe  terrno.exe  tlimits.exe  tmath1.exe   tmath3.exe  tsignal.exe  tstddef.exe  tstdio2.exe  tstring.exe
OUT3 = 
OUTS = $(OUT1) $(OUT2) $(OUT3)

all : $(OUTS)
	../tools/minixfsutils ../tools/c.img.old put /tstdio1 tstdio1.out
	../tools/minixfsutils ../tools/c.img.old put /tstdio2 tstdio2.out
	../tools/minixfsutils ../tools/c.img.old put /tctype tctype.out
	../tools/minixfsutils ../tools/c.img.old put /tfloat tfloat.out
	../tools/minixfsutils ../tools/c.img.old put /tlocale tlocale.out
	../tools/minixfsutils ../tools/c.img.old put /tmath2 tmath2.out
	../tools/minixfsutils ../tools/c.img.old put /tsetjmp tsetjmp.out
	../tools/minixfsutils ../tools/c.img.old put /tstdarg tstdarg.out
	../tools/minixfsutils ../tools/c.img.old put /tstdlib tstdlib.out
	../tools/minixfsutils ../tools/c.img.old put /ttime ttime.out
	../tools/minixfsutils ../tools/c.img.old put /tassert tassert.out
	../tools/minixfsutils ../tools/c.img.old put /terrno terrno.out
	../tools/minixfsutils ../tools/c.img.old put /tlimits tlimits.out
	../tools/minixfsutils ../tools/c.img.old put /tmath1 tmath1.out
	../tools/minixfsutils ../tools/c.img.old put /tmath3 tmath3.out
	../tools/minixfsutils ../tools/c.img.old put /tsignal tsignal.out
	../tools/minixfsutils ../tools/c.img.old put /tstddef tstddef.out
	../tools/minixfsutils ../tools/c.img.old put /tstring tstring.out
	
.obj.exe : 
	$(CROSSLD) system dos option map F { ../libsys/crtso.obj $< } L { ../libansi/libansi.lib ../libsys/libsys.lib } N $@
	../tools/dos2out $@ > /dev/null

.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
  
.asm.obj :
	$(CROSSASM) $<
	