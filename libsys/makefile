
OBJS01 = access.obj	chdir.obj	creat.obj	fork.obj	getpid.obj	lseek.obj    
OBJS02 = pause.obj	setgid.obj  sync.obj	umount.obj	alarm.obj	chmod.obj   
OBJS03 = dup2.obj	fstat.obj	getuid.obj  message.obj	perror.obj	setuid.obj  
OBJS04 = syslib.obj	unlink.obj	brk2.obj    chown.obj	dup.obj		getegid.obj  
OBJS05 = ioctl.obj	mknod.obj	pipe.obj    signal.obj  time.obj    utime.obj
OBJS06 = brk.obj	chroot.obj  exec.obj	geteuid.obj	kill.obj    mount.obj    
OBJS07 = printk.obj	stat.obj	times.obj   wait.obj	call.obj    close.obj   
OBJS08 = exit.obj	getgid.obj	link.obj    open.obj	read.obj    stime.obj   
OBJS09 = umask.obj	write.obj

OBJS10 = sendrec.obj catchsig.obj setjmp.obj getutil.obj

OBJS_A = $(OBJS01) $(OBJS02) $(OBJS03) $(OBJS04)
OBJS_B = $(OBJS05) $(OBJS06) $(OBJS07) $(OBJS08)
OBJS_C = $(OBJS09) $(OBJS10)
OBJS = $(OBJS_A) $(OBJS_B) $(OBJS_C)
CRTS = head.obj crtso.obj

CROSSASM = nasm -f obj
CROSSLIB = wlib
CROSSCC = wcc
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj

all : libsys.lib $(CRTS)

libsys.lib : $(OBJS)
  $(CROSSLIB) -q -fo libsys.lib $(OBJS) owclibs/i4d.obj owclibs/i4m.obj 
 
.c.obj : 
  $(CROSSCC) $(CROSSCFLAGS) $<
 
.asm.obj :
   $(CROSSASM) $<
   