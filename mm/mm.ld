system dos
option map,stack=0
name mm.exe
file ../lib/head.obj
file alloc.obj, break.obj, exec.obj, forkexit.obj, getset.obj, main.obj
file putc.obj, signal.obj, table.obj, utility.obj
library ../lib/minix.lib, ../lib/libh.lib