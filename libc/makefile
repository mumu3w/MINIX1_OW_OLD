OBJS01 = abort.obj	crypt.obj	fopen.obj	ftell.obj	getpwent.obj	mktemp.obj    
OBJS02 = regsub.obj	stb.obj		strncat.obj	abs.obj		ctype.obj	fprintf.obj  
OBJS03 = fwrite.obj	gets.obj	printdat.obj rindex.obj	stderr.obj	strncmp.obj
OBJS04 = atoi.obj	doprintf.obj fputs.obj	getc.obj	index.obj	prints.obj    
OBJS05 = strcat.obj	strncpy.obj	atol.obj	fclose.obj	fread.obj    
OBJS06 = getenv.obj	isatty.obj	putc.obj	setbuf.obj	strcmp.obj	ungetc.obj
OBJS07 = bcopy.obj	fflush.obj	freopen.obj	getgrent.obj itoa.obj	rand.obj     
OBJS08 = sleep.obj	strcpy.obj	cleanup.obj	fgets.obj	fseek.obj	getpass.obj   
OBJS09 = malloc.obj	regexp.obj	sprintf.obj	strlen.obj

OBJS_A = $(OBJS01) $(OBJS02) $(OBJS03) $(OBJS04)
OBJS_B = $(OBJS05) $(OBJS06) $(OBJS07) $(OBJS08)
OBJS_C = $(OBJS09) 
OBJS = $(OBJS_A) $(OBJS_B) $(OBJS_C)

CROSSASM = nasm -f obj
CROSSLIB = wlib
CROSSCC = wcc
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=dos -oh -zq -j -zl -fo=.obj

all : libc.lib 

libc.lib : $(OBJS)
  $(CROSSLIB) -q -fo libc.lib $(OBJS) 
 
.c.obj : 
  $(CROSSCC) $(CROSSCFLAGS) $<
 