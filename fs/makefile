OBJS1 = cache.obj filedes.obj link.obj misc.obj open.obj pipe.obj 
OBJS2 = putc.obj stadir.obj table.obj utility.obj device.obj inode.obj
OBJS3 = main.obj mount.obj path.obj protect.obj read.obj super.obj
OBJS4 = time.obj write.obj

CROSSCC = wcc
CROSSLD = wlink
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj


fs.exe : $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)
	$(CROSSLD) @fs.ld
	
.c.obj : 
	$(CROSSCC) $(CROSSCFLAGS) $<
  