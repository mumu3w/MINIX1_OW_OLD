system dos
option map,stack=0
name mm.exe
file ../libsys/head.obj
file alloc.obj, break.obj, exec.obj, forkexit.obj, getset.obj, main.obj
file putc.obj, signal.obj, table.obj, utility.obj
library ../libsys/libsys.lib