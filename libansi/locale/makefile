OBJS01 = localeco.obj  xdefloc.obj  xfreeloc.obj  xloctab.obj   xmakeloc.obj  xsetloc.obj
OBJS02 = setlocal.obj  xfmtval.obj  xgetloc.obj   xlocterm.obj  xreadloc.obj  xstate.obj



OBJS_A = $(OBJS01) $(OBJS02) $(OBJS03) $(OBJS04)
OBJS_B = $(OBJS05) $(OBJS06) $(OBJS07) $(OBJS08)
OBJS_C = $(OBJS09) $(OBJS10) 
OBJS = $(OBJS_A) $(OBJS_B) $(OBJS_C)


LIBCNAME = ../libansi.lib
CROSSLIB = wlib
CROSSCC = wcc
#CROSSCFLAGS = -Di8088 -ms -3 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj
CROSSCFLAGS = -I../_headers -Di8088 -ms -3 -s -zls -ecc -zp1 -bt=myos -od -zq -j -zl -fo=.obj -zro -fpi87 -op

all : $(LIBCNAME) 

$(LIBCNAME) : $(OBJS)
  $(CROSSLIB) -q -fo $(LIBCNAME) $(OBJS) 
 
.c.obj : 
  $(CROSSCC) $(CROSSCFLAGS) $< 
 