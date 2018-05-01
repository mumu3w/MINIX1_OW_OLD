
OBJS1 = alloc.obj break.obj exec.obj forkexit.obj getset.obj main.obj 
OBJS2 = putc.obj signal.obj table.obj utility.obj

CROSSCC = wcc
CROSSLD = wlink
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj


mm.exe : $(OBJS1) $(OBJS2)
	$(CROSSLD) @mm.ld
 
.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
  