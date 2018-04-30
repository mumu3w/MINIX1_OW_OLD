system dos
option map,stack=0
name kernel.exe
file mpx88.obj, main.obj, proc.obj, system.obj, clock.obj, memory.obj
file floppy.obj, wini.obj, tty.obj, printer.obj, table.obj, klib88.obj
file dmp.obj
library ../libsys/libsys.lib