/* _Fopen function -- UNIX version */
#include "xstdio.h"

		/* UNIX system call */
int open(const char *, int);
int creat(const char *, int);
int lseek(int, long, int);
int close(int);
#if 0
int _Fopen(const char *path, unsigned int smode,
	const char *mods)
	{	/* open from a file */
	unsigned int acc;

	acc = (smode & (_MOPENR|_MOPENW)) == (_MOPENR|_MOPENW) ? 2
		: smode & _MOPENW ? 1 : 0;
	if (smode & _MOPENA)
		acc |= 010;	/* O_APPEND */
	if (smode & _MTRUNC)
		acc |= 02000;	/* O_TRUNC */
	if (smode & _MCREAT)
		acc |= 01000;	/* O_CREAT */
	return (open(path, acc, 0666));
	}
#else
int _Fopen(const char *path, unsigned int smode,
	const char *mods)
  {
    int fd;
    unsigned int acc;
    
    acc = (smode & (_MOPENR|_MOPENW)) == (_MOPENR|_MOPENW) ? 2
      : smode & _MOPENW ? 1 : 0;
    if ((fd = open(path, acc)) < 0) {
      if (smode & _MCREAT) {
        fd = creat(path, 0666);
        close(fd);
        fd = open(path, acc);
      }
    } else {
      if (smode & _MTRUNC) {
        close(fd);
        fd = creat(path, 0666);
        close(fd);
        fd = open(path, acc);
      } else if (smode & _MOPENA) {
        lseek(fd, 0, 2);
      }
    }
    return fd;
  }
#endif
