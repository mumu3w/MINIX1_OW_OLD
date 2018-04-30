CROSSCC = wcc
CROSSLD = wlink
CROSSASM= nasm -f obj
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj


all : bootblok.bin init.exe fsck.exe
	
fsck.exe : fsck.obj fsck1.obj
	$(CROSSLD) @fsck.ld
	
init.exe : init.obj 
	$(CROSSLD) @init.ld
  
bootblok.bin : bootblok.asm
	nasm -f bin bootblok.asm -o bootblok.bin

.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
  
.asm.obj :
	$(CROSSASM) $<
	