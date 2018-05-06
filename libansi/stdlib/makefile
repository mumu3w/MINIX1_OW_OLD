OBJS01 = abort.obj atoi.obj     div.obj     labs.obj    mbstowcs.obj  realloc.obj  strtoul.obj   xgetmem.obj  xwctomb.obj
OBJS02 = abs.obj     atol.obj     exit.obj    ldiv.obj    mbtowc.obj    srand.obj    system.obj    xmbtowc.obj
OBJS03 = atexit.obj  bsearch.obj  free.obj    malloc.obj  qsort.obj     strtod.obj   wcstombs.obj  xstod.obj
OBJS04 = atof.obj    calloc.obj   getenv.obj  mblen.obj   rand.obj      strtol.obj   wctomb.obj    xstoul.obj raise.obj





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
 