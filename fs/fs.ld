system dos
option map,stack=0
name fs.exe
file ../lib/head.obj
file cache.obj, filedes.obj, link.obj, misc.obj, open.obj, pipe.obj
file putc.obj, stadir.obj, table.obj, utility.obj, device.obj, inode.obj
file main.obj, mount.obj, path.obj, protect.obj, read.obj, super.obj
file time.obj, write.obj
library ../lib/minix.lib, ../lib/libh.lib