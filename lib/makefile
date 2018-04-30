
OBJS1 = abort.obj chmod.obj exec.obj fseek.obj getpid.obj malloc.obj 
OBJS2 = prints.obj signal.obj strncat.obj utime.obj abs.obj chown.obj 
OBJS3 = exit.obj fstat.obj getpwent.obj message.obj putc.obj sleep.obj 
OBJS4 = strncmp.obj wait.obj access.obj chroot.obj fclose.obj ftell.obj
OBJS5 = gets.obj mknod.obj rand.obj sprintf.obj strncpy.obj write.obj
OBJS6 = alarm.obj cleanup.obj fflush.obj fwrite.obj getuid.obj mktemp.obj 
OBJS7 = read.obj stat.obj sync.obj atoi.obj close.obj fgets.obj 
OBJS8 = getc.obj index.obj mount.obj regexp.obj stb.obj syslib.obj
OBJS9 = atol.obj creat.obj fopen.obj getegid.obj ioctl.obj open.obj
OBJS10 = regsub.obj stderr.obj time.obj bcopy.obj crypt.obj fork.obj
OBJS11 = getenv.obj isatty.obj pause.obj rindex.obj stime.obj times.obj
OBJS12 = brk2.obj ctype.obj fprintf.obj geteuid.obj itoa.obj perror.obj 
OBJS13 = strcat.obj umask.obj brk.obj doprintf.obj fputs.obj getgid.obj
OBJS14 = kill.obj pipe.obj setbuf.obj strcmp.obj umount.obj call.obj
OBJS15 = dup2.obj fread.obj getgrent.obj link.obj printdat.obj setgid.obj
OBJS16 = strcpy.obj ungetc.obj chdir.obj dup.obj freopen.obj getpass.obj
OBJS17 = lseek.obj printk.obj setuid.obj strlen.obj unlink.obj getutil.obj
OBJS18 = sendrec.obj catchsig.obj setjmp.obj

OBJS_A = $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) 
OBJS_B = $(OBJS7) $(OBJS8) $(OBJS9) $(OBJS10) $(OBJS11) $(OBJS12) 
OBJS_C = $(OBJS13) $(OBJS14) $(OBJS15) $(OBJS16) $(OBJS17) $(OBJS18) 
OBJS = $(OBJS_A) $(OBJS_B) $(OBJS_C)
CRTS = head.obj crtso.obj

CROSSASM = nasm -f obj
CROSSLIB = wlib
CROSSCC = wcc
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -2 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj

all : minix.lib $(CRTS)

minix.lib : $(OBJS)
  $(CROSSLIB) -q -fo minix.lib $(OBJS)
 
.c.obj : 
  $(CROSSCC) $(CROSSCFLAGS) $<
 
.asm.obj :
   $(CROSSASM) $<
   