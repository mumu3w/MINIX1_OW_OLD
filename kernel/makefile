
OBJS1 = clock.obj dmp.obj floppy.obj wini.obj memory.obj printer.obj 
OBJS2 = proc.obj system.obj table.obj tty.obj main.obj
ASMOBJ1 = mpx88.obj klib88.obj

CROSSCC = wcc
CROSSLD = wlink
CROSSASM= nasm -f obj 
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj


kernel.exe : $(OBJS1) $(OBJS2) $(ASMOBJ1)
	$(CROSSLD) @kernel.ld
 
.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
	
.asm.obj :
	$(CROSSASM) $<
	