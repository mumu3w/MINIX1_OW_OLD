OBJS01 = clearerr.obj  fputc.obj    getchar.obj  scanf.obj     vsprintf.obj  xgetfloa.obj
OBJS02 = fclose.obj    fputs.obj    gets.obj     setbuf.obj    xfgpos.obj    xgetint.obj
OBJS03 = feof.obj      fread.obj    perror.obj   setvbuf.obj   xfiles.obj    xldtob.obj
OBJS04 = ferror.obj    freopen.obj  printf.obj   sprintf.obj   xfopen.obj    xlitob.obj
OBJS05 = fflush.obj    fscanf.obj   putc.obj     sscanf.obj    xfoprep.obj   xprintf.obj
OBJS06 = fgetc.obj     fseek.obj    putchar.obj  tmpfile.obj   xfrprep.obj   xputfld.obj
OBJS07 = fgetpos.obj   fsetpos.obj  puts.obj     tmpnam.obj    xfspos.obj    xscanf.obj
OBJS08 = fgets.obj     ftell.obj    remove.obj   ungetc.obj    xfwprep.obj
OBJS09 = fopen.obj     fwrite.obj   rename.obj   vfprintf.obj  xgenld.obj
OBJS10 = fprintf.obj   getc.obj     rewind.obj   vprintf.obj   xgetfld.obj


OBJS_A = $(OBJS01) $(OBJS02) $(OBJS03) $(OBJS04)
OBJS_B = $(OBJS05) $(OBJS06) $(OBJS07) $(OBJS08)
OBJS_C = $(OBJS09) $(OBJS10) 
OBJS = $(OBJS_A) $(OBJS_B) $(OBJS_C)


LIBCNAME = ../libansi.lib
CROSSLIB = wlib
CROSSCC = wcc
#CROSSCFLAGS = -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -I../_headers -Di8088 -ms -0 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj -zro -fpi87

all : $(LIBCNAME) 

$(LIBCNAME) : $(OBJS)
  $(CROSSLIB) -q -fo $(LIBCNAME) $(OBJS) 
 
.c.obj : 
  $(CROSSCC) $(CROSSCFLAGS) $< 
 