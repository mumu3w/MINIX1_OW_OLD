OBJS01 = memchr.obj  memmove.obj  strchr.obj   strcpy.obj    strlen.obj   strncpy.obj  strspn.obj  strxfrm.obj
OBJS02 = memcmp.obj  memset.obj   strcmp.obj   strcspn.obj   strncat.obj  strpbrk.obj  strstr.obj  xstrxfrm.obj
OBJS03 = memcpy.obj  strcat.obj   strcoll.obj  strerror.obj  strncmp.obj  strrchr.obj  strtok.obj






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
 