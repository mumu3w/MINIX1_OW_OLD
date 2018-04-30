#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

#ifndef __linux__
// 文件类型
#define S_IFMT  (0170000)
#define S_IFLNK (0120000)
#define S_IFREG (0100000)
#define S_IFBLK (0060000)
#define S_IFDIR (0040000)
#define S_IFCHR (0020000)
#define S_IFIFO (0010000)

#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
// 用户权限
#define S_IRWXU (000700)
#define S_IRUSR (000400)
#define S_IWUSR (000200)
#define S_IXUSR (000100)
#endif

// 
#define SUPER_MAGIC (0x137F)
// root i-number
#define ROOTINO (1) 
// block size 
#define BSIZE   (1024)
// sector size
#define SSIZE   (512)
// Inodes per block.
#define IPB     (BSIZE / sizeof(struct dinode))
// Bitmap bits per block
#define BPB     (BSIZE * 8)
// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ  (14)

typedef uchar (*img_t)[BSIZE];

// super block
#define SBLK(img) ((struct superblock *)(img)[1])

struct partition {
  uchar   boot_ind;
  uchar   head;
  ushort  sect_cyl;  // uint8 sector; uint8 cyl;
  uchar   sys_ind;
  uchar   end_head;
  ushort  end_sect_cyl; // uint8 end_sector; uint8 end_cyl;
  uint    start_sect;
  uint    nr_sects;
};

struct superblock {
  ushort  s_ninodes;
  ushort  s_nzones;
  ushort  s_imap_block;
  ushort  s_zmap_block;
  ushort  s_firstdatazone;
  ushort  s_log_zine_size;
  uint    s_max_size;
  ushort  s_magic;
};

struct dinode {
  ushort  i_mode;
  ushort  i_uid;
  uint    i_size;
  uint    i_mtime;
  uchar   i_gid;
  uchar   i_nlinks;
  ushort  i_zone[9];
};

struct dirent {
  ushort  inum;
  char    name[DIRSIZ];
};

struct cmd_table_ent {
  char  *name;
  char  *args;
  int   (*fun)(img_t, int, char **);
};

#define ddebug(...) debug_message("DEBUG", __VA_ARGS__)
#define derror(...) debug_message("ERROR", __VA_ARGS__)
#define dwarn(...) debug_message("WARNING", __VA_ARGS__)

// inode
typedef struct dinode *inode_t;

int exec_cmd(img_t img, char *cmd, int argc, char *argv[]);
void rpart(char *hd_img, int id, struct partition *part);
void debug_message(const char *tag, const char *fmt, ...);
void error(const char *fmt, ...);
void fatal(const char *fmt, ...);
char *typename(ushort mode);
inode_t iget(img_t img, uint inum);
uint geti(img_t img, inode_t ip);
inode_t ialloc(img_t img, uint mode);
int ifree(img_t img, uint inum);
int valid_data_block(img_t img, uint b);
uint balloc(img_t img);
int bfree(img_t img, uint b);
uint bmap(img_t img, inode_t ip, uint n);
int iread(img_t img, inode_t ip, uchar *buf, uint n, uint off);
int iwrite(img_t img, inode_t ip, uchar *buf, uint n, uint off);
int itruncate(img_t img, inode_t ip, uint size);
int is_empty(char *s);
int is_sep(char c);
char *skipelem(char *path, char *name);
char *splitpath(char *path, char *dirbuf, uint size);
inode_t dlookup(img_t img, inode_t dp, char *name, uint *offp);
int daddent(img_t img, inode_t dp, char *name, inode_t ip);
int dmkparlink(img_t img, inode_t pip, inode_t cip);
inode_t ilookup(img_t img, inode_t rp, char *path);
inode_t icreat(img_t img, inode_t rp, char *path, uint mode, inode_t *dpp);
int emptydir(img_t img, inode_t dp);
int iunlink(img_t img, inode_t rp, char *path);

int do_diskinfo(img_t img, int argc, char *argv[]);
int do_info(img_t img, int argc, char *argv[]);
int do_ls(img_t img, int argc, char *argv[]);
int do_get(img_t img, int argc, char *argv[]);
int do_put(img_t img, int argc, char *argv[]);
int do_rm(img_t img, int argc, char *argv[]);
int do_cp(img_t img, int argc, char *argv[]);
int do_mv(img_t img, int argc, char *argv[]);
int do_ln(img_t img, int argc, char *argv[]);
int do_mkdir(img_t img, int argc, char *argv[]);
int do_rmdir(img_t img, int argc, char *argv[]);

struct cmd_table_ent cmd_table[] = {
  { "diskinfo", "", do_diskinfo },
  { "info", "path", do_info },
  { "ls", "path", do_ls },
  { "get", "spath dpath", do_get },
  { "put", "dpath spath", do_put },
  { "rm", "path", do_rm },
  { "cp", "spath dpath", do_cp },
  { "mv", "spath dpath", do_mv },
  { "ln", "spath dpath", do_ln },
  { "mkdir", "path", do_mkdir },
  { "rmdir", "path", do_rmdir },
  { NULL, NULL}
};

// inode of the root directory
const uint root_inode_number = 1;
inode_t root_inode;

// program name
char *progname;
jmp_buf fatal_exception_buf;


int exec_cmd(img_t img, char *cmd, int argc, char *argv[])
{
  for(int i = 0; cmd_table[i].name != NULL; i++) {
    if(strcmp(cmd, cmd_table[i].name) == 0)
      return cmd_table[i].fun(img, argc, argv);
  }
  error("unknown command: %s\n", cmd);
  return EXIT_FAILURE;
}

void rpart(char *hd_img, int id, struct partition *part) 
{
  struct partition *p;
  
  p = (struct partition *)&hd_img[0x1be];
  *part = *(p + id);
}

#undef min
static inline int min(int x, int y)
{
  return x < y ? x : y;
}

#undef max
static inline int max(int x, int y)
{
  return x > y ? x : y;
}

// ceiling(x / y) where x >=0, y >= 0
static inline int divceil(int x, int y)
{
  return x == 0 ? 0 : (x - 1) / y + 1;
}

uint bitcount(uint x)
{
  int n = 0;
  uchar m = 0x1;
  
  for(int i = 0; i < 8; i++) {
    if(!(x & m)) {
      n++;
    }
    m = m << 1;
  }
  
  return n;
}

void debug_message(const char *tag, const char *fmt, ...)
{
#ifndef NDEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s: ", tag);
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

void error(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void fatal(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "FATAL: ");
  vfprintf(stderr, fmt, args);
  va_end(args);
  longjmp(fatal_exception_buf, 1);
}

char *typename(ushort mode)
{
  if(S_ISREG(mode))  
    return "file";
  else if(S_ISLNK(mode))
    return "link";
  else if(S_ISDIR(mode))
    return "directory";
  else if(S_ISCHR(mode))
    return "char device";
  else if(S_ISBLK(mode))
    return "block device";
  else if(S_ISFIFO(mode))
    return "fifo";
  else 
    return "unknown";
}

inode_t iget(img_t img, uint inum)
{
  uint firstinode = SBLK(img)->s_imap_block + SBLK(img)->s_zmap_block + 2;
  uint ioff = inum - 1;
  
  if(0 < inum && inum < SBLK(img)->s_ninodes)
    return (inode_t)img[firstinode + ioff / IPB] + ioff % IPB;
  derror("iget: %u: invalid inode number\n", inum);
  return NULL;
}

// retrieves the inode number of a dinode structure
uint geti(img_t img, inode_t ip)
{
  uint firstinode = SBLK(img)->s_imap_block + SBLK(img)->s_zmap_block + 2;
  uint Ni = SBLK(img)->s_ninodes / IPB + 1;       // # of inode blocks
  
  for(int i = 0; i < Ni; i++) {
    inode_t bp = (inode_t)img[i + firstinode];
    if(bp <= ip && ip < bp + IPB)
      return ip - bp + i * IPB + 1;
  }
  derror("geti: %p: not in the inode blocks\n", ip);
  return 0;
}

inode_t ialloc(img_t img, uint mode)
{
  uint firstimap = 2;
  
  for(int b = 0; b <= SBLK(img)->s_ninodes; b += BPB) {
    uchar *bp = img[firstimap + b / BPB];
    for(int bi = 0; bi < BPB && b + bi <= SBLK(img)->s_ninodes; bi++) {
      uchar m = 0x1 << (bi % 8);
      if((bp[bi / 8] & m) == 0) {
        bp[bi / 8] |= m;
        inode_t ip = iget(img, b + bi);
        memset(ip, 0, sizeof(struct dinode));
        ip->i_mode = mode;
        return ip;
      }
    }
  }
  fatal("ialloc: cannot allocate\n");
  return NULL;
}

int ifree(img_t img, uint inum)
{
  uint firstimap = 2;
  
  inode_t ip =iget(img, inum);
  if(ip == NULL)
    return -1;
  if(ip->i_mode == 0)
    dwarn("ifree: inode #%d is already freed\n", inum);
  if(ip->i_nlinks > 0)
    dwarn("ifree: nlink of inode #%d is not zero\n", inum);
  ip->i_mode = 0;
  
  uchar *bp = (uchar *)img[firstimap + inum / BPB];
  int bi = inum % BPB;
  uchar m = 0x1 << (bi % 8);
  if((bp[bi / 8] & m) == 0)
    dwarn("ifree: inode #%d is already freed\n", inum);
  bp[bi / 8] &= ~m;
  
  return 0;
}

int valid_data_block(img_t img, uint b)
{
  return b >= SBLK(img)->s_firstdatazone && b <= SBLK(img)->s_nzones - 1;
}

uint balloc(img_t img)
{
  uint firstzmap = SBLK(img)->s_imap_block + 2;
  uint datasize = SBLK(img)->s_nzones - SBLK(img)->s_firstdatazone;
  uint blkno;
  
  for(int b = 0; b <= datasize; b += BPB) {
    uchar *bp = img[firstzmap + b / BPB];
    for(int bi = 0; bi < BPB && b + bi <= datasize; bi++) {
      uchar m = 0x1 << (bi % 8);
      if((bp[bi / 8] & m) == 0) {
        bp[bi / 8] |= m;
        blkno = b + bi + SBLK(img)->s_firstdatazone - 1;
        if(!valid_data_block(img, blkno)) {
          fatal("balloc: %u: invalid data block number\n", blkno);
          return 0; // dummy
        }
        memset(img[blkno], 0, BSIZE);
        return blkno;
      }
    }
  }
  fatal("balloc: no free blocks\n");
  return 0; // dummy
}

int bfree(img_t img, uint b)
{
  uint firstzmap = SBLK(img)->s_imap_block + 2;
  
  if(!valid_data_block(img, b)) {
    fatal("bfree: %u: invalid data block number\n", b);
    return -1; // dummy
  }
  uint nr = b - SBLK(img)->s_firstdatazone + 1;
  uchar *bp = (uchar *)img[firstzmap + nr / BPB];
  int bi = nr % BPB;
  uchar m = 0x1 << (bi % 8);
  if((bp[bi / 8] & m) == 0)
    dwarn("bfree: %u: already freed block\n", b);
  bp[bi / 8] &= ~m;
  return 0;
}

uint bmap(img_t img, inode_t ip, uint n)
{
  uint k = n;
  
  if(n < 7) {
    uint addr = ip->i_zone[n];
    if(addr == 0) {
      addr = balloc(img);
      ip->i_zone[n] = addr;
    }
    return addr;
  }
  
  n -= 7;
  if(n < 512) {
    uint iaddr = ip->i_zone[7];
    if(iaddr == 0) {
      iaddr = balloc(img);
      ip->i_zone[7] = iaddr;
    }
    ushort *iblock = (ushort *)img[iaddr];
    if(iblock[n] == 0) {
      iblock[n] = balloc(img);
    }
    return iblock[n];
  }
  
  n -= 512;
  if(n < 512 * 512) {
    uint iaddr = ip->i_zone[8];
    if(iaddr == 0) {
      iaddr = balloc(img);
      ip->i_zone[8] = iaddr;
    }
    ushort *iblock = (ushort *)img[iaddr];
    uint iiaddr = iblock[n >> 9];
    if(iiaddr == 0) {
      iiaddr = balloc(img);
      iblock[n >> 9] = iiaddr;
    }
    ushort *iiblock = (ushort *)img[iiaddr];
    if(iiblock[n & 511] == 0) {
      iiblock[n & 511] = balloc(img);
    }
    return iiblock[n & 511];
  }
  
  derror("bmap: %u: invalid index number\n", k);
  return 0;
}

int iread(img_t img, inode_t ip, uchar *buf, uint n, uint off)
{
  if(S_ISLNK(ip->i_mode))
    return -1;
  if(S_ISCHR(ip->i_mode))
    return -1;
  if(S_ISBLK(ip->i_mode))
    return -1;
  if(S_ISFIFO(ip->i_mode))
    return -1;
  if(off > ip->i_size || off + n < off)
    return -1;
  if(off + n > ip->i_size)
    n = ip->i_size - off;
  
  // t : total bytes that have been read
  // m : last bytes that were read
  uint t = 0;
  for(uint m = 0; t < n; t += m, off += m, buf += m) {
    uint b = bmap(img, ip, off / BSIZE);
    if(!valid_data_block(img, b)) {
      derror("iread: %u: invalid data block\n", b);
      break;
    }
    m = min(n - t, BSIZE - off % BSIZE);
    memmove(buf, img[b] + off % BSIZE, m);
  }
  return t;
}

int iwrite(img_t img, inode_t ip, uchar *buf, uint n, uint off)
{
  if(S_ISLNK(ip->i_mode))
    return -1;
  if(S_ISCHR(ip->i_mode))
    return -1;
  if(S_ISBLK(ip->i_mode))
    return -1;
  if(S_ISFIFO(ip->i_mode))
    return -1;
  if(off > ip->i_size || off + n < off || off + n > SBLK(img)->s_max_size)
    return -1;
  
  // t : total bytes that have been read
  // m : last bytes that were read
  uint t = 0;
  for(uint m = 0; t < n; t += m, off += m, buf += m) {
    uint b = bmap(img, ip, off / BSIZE);
    if(!valid_data_block(img, b)) {
      derror("iwrite: %u: invalid data block\n", b);
      break;
    }
    m = min(n - t, BSIZE - off % BSIZE);
    memmove(img[b] + off % BSIZE, buf, m);
  }
  if(t > 0 && off > ip->i_size)
    ip->i_size = off;
  return t;
}

#define NDIRECT 7
void free_ind(img_t img, uint b)
{
  ushort *bp = (ushort *)img[b];
  for(int i = 0; i < 512; i++) {
    if(bp[i])
      bfree(img, bp[i]);
  }
  bfree(img, b);
}

void free_dind(img_t img, uint b)
{
  ushort *bp = (ushort *)img[b];
  for(int i = 0; i < 512; i++) {
    if(bp[i])
      free_ind(img, bp[i]);
  }
  bfree(img, b);
}

int itruncate(img_t img, inode_t ip, uint size)
{
  if(S_ISLNK(ip->i_mode))
    return -1;
  if(S_ISCHR(ip->i_mode))
    return -1;
  if(S_ISBLK(ip->i_mode))
    return -1;
  if(S_ISFIFO(ip->i_mode))
    return -1;
  if(size != 0)
    return -1;
  
  for(int i = 0; i < NDIRECT; i++) {
    if(ip->i_zone[i]) {
      bfree(img, ip->i_zone[i]);
      ip->i_zone[i] = 0;
    }
  }

  if(ip->i_zone[7]) {
    free_ind(img, ip->i_zone[7]);
    ip->i_zone[7] = 0;
  }
  if(ip->i_zone[8]) {
    free_dind(img, ip->i_zone[8]);
    ip->i_zone[8] = 0;
  }
  
  ip->i_size = size;
  return 0;
}

// check if s is an empty string
int is_empty(char *s)
{
  return *s == 0;
}

// check if c is a path separator
int is_sep(char c)
{
  return c == '/';
}

// adapted from skipelem in xv6/fs.c
/*
	str = "/root/test/bin"		   skipelem return = "/test/bin"     name = "root"
	str = "root/test/bin"        skipelem return = "/test/bin"     name = "root"
	str = "///root//test//bin"	 skipelem return = "//test//bin"   name = "root"
	str = ""                     skipelem return = ""              name = ""
	str = "///root//test//bin/"  skipelem return = "//test//bin/"  name = "root"
	str = "bin/"                 skipelem return = "/"             name = "bin"
	str = "/"                    skipelem return = ""              name = ""
*/
char *skipelem(char *path, char *name)
{
  while(is_sep(*path))
    path++;
  char *s = path;
  while(!is_empty(path) && !is_sep(*path))
    path++;
  int len = min(path - s, DIRSIZ);
  memmove(name, s, len);
  if(len < DIRSIZ)
    name[len] = 0;
  return path;
}

// split the path into directory name and base name
/*
 str = "/root/test/bin"       splitpath return = "bin"  name = "/root/test/"
 str = "root/test/bin"        splitpath return = "bin"  name = "root/test/"
 str = "///root//test//bin"   splitpath return = "bin"  name = "///root//test//"
 str = ""                     splitpath return = ""     name = ""
 str = "///root//test//bin/"  splitpath return = ""     name = "///root//test//bin/"
 str = "bin/"                 splitpath return = ""     name = "bin/"
 str = "/"                    splitpath return = ""     name = "/"
*/
char *splitpath(char *path, char *dirbuf, uint size)
{
  char *s = path, *t = path;
  while(!is_empty(path)) {
    while(is_sep(*path))
      path++;
     s = path;
    while(!is_empty(path) && !is_sep(*path))
      path++;
  }
  if(dirbuf != NULL) {
    int n = min(s - t, size - 1);
    memmove(dirbuf, t, n);
    dirbuf[n] = 0;
  }
  return s;
}

inode_t dlookup(img_t img, inode_t dp, char *name, uint *offp)
{
  assert(S_ISDIR(dp->i_mode));
  struct dirent de;
  
  for(uint off = 0; off < dp->i_size; off += sizeof(de)) {
    if(iread(img, dp, (uchar *)&de, sizeof(de), off) != sizeof(de)) {
      derror("dlookup: %s: read error\n", name);
      return NULL;
    }
    if(strncmp(name, de.name, DIRSIZ) == 0) {
      if (offp != NULL)
      *offp = off;
      return iget(img, de.inum);
    }
  }
  return NULL;
}

int daddent(img_t img, inode_t dp, char *name, inode_t ip)
{
  struct dirent de;
  uint off;
  
  for(off = 0; off < dp->i_size; off += sizeof(de)) {
    if(iread(img, dp, (uchar *)&de, sizeof(de), off) != sizeof(de)) {
      derror("daddent: %u: read error\n", geti(img, dp));
      return -1;
    }
    if(de.inum == 0)
      break;
    if(strncmp(de.name, name, DIRSIZ) == 0) {
      derror("daddent: %s: exists\n", name);
      return -1;
    }
  }
  strncpy(de.name, name, DIRSIZ);
  de.inum = geti(img, ip);
  if(iwrite(img, dp, (uchar *)&de, sizeof(de), off) != sizeof(de)) {
    derror("daddent: %u: write error\n", geti(img, dp));
    return -1;
  }
  if(strncmp(name, ".", DIRSIZ) != 0)
    ip->i_nlinks++;
  return 0;
}

// create a link to the parent directory
int dmkparlink(img_t img, inode_t pip, inode_t cip)
{
  if(!S_ISDIR(pip->i_mode)) {
    derror("dmkparlink: %d: not a directory\n", geti(img, pip));
    return -1;
  }
  if(!S_ISDIR(cip->i_mode)) {
    derror("dmkparlink: %d: not a directory\n", geti(img, cip));
    return -1;
  }
  uint off;
  dlookup(img, cip, "..", &off);
  struct dirent de;
  de.inum = geti(img, pip);
  strncpy(de.name, "..", DIRSIZ);
  if(iwrite(img, cip, (uchar *)&de, sizeof(de), off) != sizeof(de)) {
    derror("dmkparlink: write error\n");
    return -1;
  }
  pip->i_nlinks++;
  return 0;
}

inode_t ilookup(img_t img, inode_t rp, char *path)
{
  char name[DIRSIZ + 1];
  name[DIRSIZ] = 0;
  while(1) {
    assert(path != NULL && rp != NULL && S_ISDIR(rp->i_mode));
    path = skipelem(path, name);
    // if path is empty (or a sequence of path separators),
    // it should specify the root direcotry (rp) itself
    if(is_empty(name))
      return rp;
    
    inode_t ip = dlookup(img, rp, name, NULL);
    if(ip == NULL)
      return NULL;
    if(is_empty(path))
      return ip;
    if(!(S_ISDIR(rp->i_mode))) {
      derror("ilookup: %s: not a directory\n", name);
      return NULL;
    }
    rp = ip;
  }
}

inode_t icreat(img_t img, inode_t rp, char *path, uint mode, inode_t *dpp)
{
  char name[DIRSIZ + 1];
  name[DIRSIZ] = 0;
  while(1) {
    assert(path != NULL && rp != NULL && S_ISDIR(rp->i_mode));
    path = skipelem(path, name);
    if(is_empty(name)) {
      derror("icreat: %s: empty file name\n", path);
      return NULL;
    }
    
    inode_t ip = dlookup(img, rp, name, NULL);
    if(is_empty(path)) {
      if(ip != NULL) {
        derror("icreat: %s: file exists\n", name);
        return NULL;
      }
      ip = ialloc(img, mode);
      daddent(img, rp, name, ip);
      if(S_ISDIR(rp->i_mode)) {
        daddent(img, ip, ".", ip);
        daddent(img, ip, "..", rp);
      }
      if(dpp != NULL)
        *dpp = rp;
      return ip;
    }
    if(ip == NULL || !(S_ISDIR(ip->i_mode))) {
      derror("icreat: %s: no such directory\n", name);
      return NULL;
    }
    rp = ip;
  }
}

// checks if dp is an empty directory
int emptydir(img_t img, inode_t dp)
{
  int nent = 0;
  struct dirent de;
  for(uint off = 0; off < dp->i_size; off += sizeof(de)) {
    iread(img, dp, (uchar *)&de, sizeof(de), off);
    if(de.inum != 0)
      nent++;
  }
  return nent == 2;
}

// unlinks a file (dp/path)
int iunlink(img_t img, inode_t rp, char *path)
{
  char name[DIRSIZ + 1];
  name[DIRSIZ] = 0;
  while(1) {
    assert(path != NULL && rp != NULL && S_ISDIR(rp->i_mode));
    path = skipelem(path, name);
    if(is_empty(name)) {
      derror("iunlink: empty file name\n");
      return -1;
    }
    uint off;
    inode_t ip = dlookup(img, rp, name, &off);
    if(ip != NULL && is_empty(path)) {
      if(strncmp(name, ".", DIRSIZ) == 0 ||
        strncmp(name, "..", DIRSIZ) == 0) {
        derror("iunlink: cannot unlink \".\" or \"..\"\n");
        return -1;
      }
      // erase the directory entry
      uchar zero[sizeof(struct dirent)];
      memset(zero, 0, sizeof(zero));
      if(iwrite(img, rp, zero, sizeof(zero), off) != sizeof(zero)) {
        derror("iunlink: write error\n");
        return -1;
      }
      if(S_ISDIR(ip->i_mode) && dlookup(img, ip, "..", NULL) == rp)
        rp->i_nlinks--;
      ip->i_nlinks--;
//printf("i_nlinks = %d\n", ip->i_nlinks);
      if(ip->i_nlinks == 0) {
        if(S_ISREG(ip->i_mode) || S_ISDIR(ip->i_mode))
          itruncate(img, ip, 0);
        ifree(img, geti(img, ip));
      }
      return 0;
    }
    if(ip == NULL || !S_ISDIR(ip->i_mode)) {
      derror("iunlink: %s: no such directory\n", name);
      return -1;
    }
    rp = ip;
  }
}

// diskinfo
int do_diskinfo(img_t img, int argc, char *argv[]) 
{
  if(argc != 0) {
    error("usage: %s img_file diskinfo\n", progname);
    return EXIT_FAILURE;
  }

  uint N = SBLK(img)->s_nzones;
  uint Nm = SBLK(img)->s_imap_block + SBLK(img)->s_zmap_block;
  uint Ni = SBLK(img)->s_firstdatazone - (2 + Nm);
  uint Nd = N - SBLK(img)->s_firstdatazone;

  printf(" total blocks: %d (%d bytes)\n", N, N * BSIZE);
  printf(" bitmap blocks: #%d-#%d (%d blocks)\n", 2, 2 + Nm - 1, Nm);
  printf(" inode blocks: #%d-#%d (%d blocks, %d inodes)\n", 
    2 + Nm, SBLK(img)->s_firstdatazone - 1 ,Ni ,SBLK(img)->s_ninodes);
  printf(" data blocks: #%d-#%d (%d blocks)\n", 
    SBLK(img)->s_firstdatazone, SBLK(img)->s_nzones - 1, Nd);
  printf(" maximum file size (bytes): %d\n", SBLK(img)->s_max_size);

  int nblocks = 0;
  for(uint b = SBLK(img)->s_imap_block + 2; b < 2 + Nm; b++)
    for (int i = 0; i < BSIZE; i++)
      nblocks += bitcount(img[b][i]);
  printf(" free blocks: %d\n", nblocks);
  printf(" # of used blocks: %d\n", Nd - nblocks);

  int n_dirs = 0, n_files = 0, n_devs = 0, n_fifos = 0;
  for (uint b = 2 + Nm; b < SBLK(img)->s_firstdatazone; b++)
    for (int i = 0; i < IPB; i++) {
      if(S_ISREG(((inode_t)img[b])[i].i_mode))  
        n_files++;
//      else if(S_ISLNK(((inode_t)img[b])[i].i_mode))
//        n_files++;
      else if(S_ISDIR(((inode_t)img[b])[i].i_mode))
        n_dirs++;
      else if(S_ISCHR(((inode_t)img[b])[i].i_mode))
        n_devs++;
      else if(S_ISBLK(((inode_t)img[b])[i].i_mode))
        n_dirs++;
      else if(S_ISFIFO(((inode_t)img[b])[i].i_mode))
        n_fifos++;
      else 
        ;
    }
  printf(" # of used inodes: %d", n_dirs + n_files + n_devs + n_fifos);
  printf(" (dirs: %d, files: %d, devs: %d, fifos: %d)\n",
       n_dirs, n_files, n_devs, n_fifos);

  return EXIT_SUCCESS;
}

static inline void print_block(int a, int b, int c) 
{
  if(a % b == 0 && a)
    printf("\n");
  printf(" %6d", c);
}

int do_info(img_t img, int argc, char *argv[])
{
  if(argc != 1) {
    error("usage: %s img_file info path\n", progname);
    return EXIT_FAILURE;
  }
  char *path = argv[0];
  
  inode_t ip = ilookup(img, root_inode, path);
  if(ip == NULL) {
    error("info: no such file or directory: %s\n", path);
    return EXIT_FAILURE;
  }
  printf("inode: %d\n", geti(img, ip));
  printf("type: %o (%s)\n", (ip->i_mode & S_IFMT) >> 12, 
                          typename(ip->i_mode & S_IFMT));
  printf("nlink: %d\n", ip->i_nlinks);
  printf("size: %d\n", ip->i_size);
  if(ip->i_size > 0) {
    printf("data blocks:\n");
    int bcount = 0;
    for(uint i = 0; i < 7; i++)
      if(ip->i_zone[i] != 0) {
        print_block(bcount, 10, ip->i_zone[i]);
        bcount++;
      }
      
    uint iaddr = ip->i_zone[7];
    if(iaddr != 0) {
      print_block(bcount, 10, iaddr);
      bcount++;
      ushort *iblock = (ushort *)img[iaddr];
      for(int i = 0; i < BSIZE / sizeof(ushort); i++)
        if(iblock[i] != 0) {
          print_block(bcount, 10, iblock[i]);
          bcount++;
        }
    }
    
    iaddr = ip->i_zone[8];
    if(iaddr != 0) {
      print_block(bcount, 10, iaddr);
      bcount++;
      ushort *iblock = (ushort *)img[iaddr];
      for(int i = 0; i < BSIZE / sizeof(ushort); i++)
        if(iblock[i] != 0) {
          print_block(bcount, 10, iblock[i]);
          bcount++;
          ushort *iiblock = (ushort *)img[iblock[i]];
          for(int j = 0; j < BSIZE / sizeof(ushort); j++)
            if(iiblock[j] != 0) {
              print_block(bcount, 10, iiblock[j]);
              bcount++;
            }
        }
    }
    
    printf("\n");
    printf("# of data blocks: %d\n", bcount);
  }  
  return EXIT_SUCCESS;
}  

int do_ls(img_t img, int argc, char *argv[])
{
  if(argc != 1) {
    error("usage: %s img_file ls path\n", progname);
    return EXIT_FAILURE;
  }
  char *path = argv[0];
  inode_t ip = ilookup(img, root_inode, path);
  if(ip == NULL) {
    error("ls: %s: no such file or directory\n", path);
    return EXIT_FAILURE;
  }
  printf("Name           Type Inode Size(Byte)\n");
  if(S_ISDIR(ip->i_mode)) {
    struct dirent de;
    for(uint off = 0; off < ip->i_size; off += sizeof(de)) {
      if(iread(img, ip, (uchar *)&de, sizeof(de), off) != sizeof(de)) {
        error("ls: %s: read error\n", path);
        return EXIT_FAILURE;
      }
      if(de.inum == 0)
        continue;
      char name[DIRSIZ + 1];
      name[DIRSIZ] = 0;
      strncpy(name, de.name, DIRSIZ);
      inode_t p = iget(img, de.inum);
      printf("%-14s %4o %5d %10d\n", name, (p->i_mode & S_IFMT) >> 12, 
                                          de.inum, p->i_size);
    }
  }else {
    printf("%-14s %4o %5d %10d\n", path, (ip->i_mode & S_IFMT) >> 12, 
                                     geti(img, ip), ip->i_size);
  }
  
  return EXIT_SUCCESS;
}

int do_get(img_t img, int argc, char *argv[]) {
  FILE *dfp;
  
  if(argc != 2) {
    error("usage: %s img_file get spath dpath\n", progname);
    return EXIT_FAILURE;
  }
  char *spath = argv[0];
  char *dpath = argv[1];

  dfp = fopen(dpath, "wb");
  if(dfp == NULL) {
    error("get: failed to create the file: %s\n", dpath);
    return EXIT_FAILURE;
  }
  
  // source
  inode_t ip = ilookup(img, root_inode, spath);
  if(ip == NULL) {
    error("get: no such file or directory: %s\n", spath);
    return EXIT_FAILURE;
  }
  
  uchar buf[BSIZE];
  for(uint off = 0; off < ip->i_size; off += BSIZE) {
    int n = iread(img, ip, buf, BSIZE, off);
    if (n < 0) {
      error("get: %s: read error\n", spath);
      return EXIT_FAILURE;
    }
    fwrite(buf, 1, n, dfp);
  }
  fclose(dfp);

  return EXIT_SUCCESS;
}

int do_put(img_t img, int argc, char *argv[]) {
  FILE *sfp;
  
  if(argc != 2) {
    error("usage: %s img_file put dpath spath\n", progname);
    return EXIT_FAILURE;
  }
  char *dpath = argv[0];
  char *spath = argv[1];
  
  sfp = fopen(spath, "rb");
  if(sfp == NULL) {
    error("put: failed to open the file: %s\n", spath);
    return EXIT_FAILURE;
  }
  
  // destination
  inode_t ip = ilookup(img, root_inode, dpath);
  if(ip == NULL) {
    ip = icreat(img, root_inode, dpath, S_IFREG | S_IRWXU, NULL);
    if(ip == NULL) {
      error("put: %s: cannot create\n", dpath);
      return EXIT_FAILURE;
    }
  }else {
    if(!S_ISREG(ip->i_mode)) {
      error("put: %s: directory or device\n", dpath);
      return EXIT_FAILURE;
    }
    itruncate(img, ip, 0);
  }
  
  uchar buf[BSIZE];
  for(uint off = 0; off < SBLK(img)->s_max_size; off += BSIZE) {
    int n = fread(buf, 1, BSIZE, sfp);
    if(n < 0) {
      perror(NULL);
      return EXIT_FAILURE;
    }
    if(iwrite(img, ip, buf, n, off) != n) {
      error("put: %s: write error\n", dpath);
      return EXIT_FAILURE;
    }
    if(n < BSIZE)
      break;
  }
  return EXIT_SUCCESS;
}

int do_rm(img_t img, int argc, char *argv[]) 
{
  if(argc != 1) {
    error("usage: %s img_file rm path\n", progname);
    return EXIT_FAILURE;
  }
  char *path = argv[0];
    
  inode_t ip = ilookup(img, root_inode, path);
  if(ip == NULL) {
    error("rm: %s: no such file or directory\n", path);
    return EXIT_FAILURE;
  }
  if(S_ISDIR(ip->i_mode)) {
    error("rm: %s: a directory\n", path);
    return EXIT_FAILURE;
  }
  if(iunlink(img, root_inode, path) < 0) {
    error("rm: %s: cannot unlink\n", path);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// cp src_path dest_path
int do_cp(img_t img, int argc, char *argv[])
{
  if(argc != 2) {
    error("usage: %s img_file cp spath dpath\n", progname);
    return EXIT_FAILURE;
  }
  char *spath = argv[0];
  char *dpath = argv[1];
  
  // source
  inode_t sip = ilookup(img, root_inode, spath);
  if(sip == NULL) {
    error("cp: %s: no such file or directory\n", spath);
    return EXIT_FAILURE;
  }
  if(!S_ISREG(sip->i_mode)) {
    error("cp: %s: directory or device file\n", spath);
    return EXIT_FAILURE;
  }
  
  // destination
  inode_t dip = ilookup(img, root_inode, dpath);
  char ddir[BSIZE];
  char *dname = splitpath(dpath, ddir, BSIZE);
  if(dip == NULL) {
    if(is_empty(dname)) {
      error("cp: %s: no such directory\n", dpath);
      return EXIT_FAILURE;
    }
    inode_t ddip = ilookup(img, root_inode, ddir);
    if(ddip == NULL) {
      error("cp: %s: no such directory\n", ddir);
      return EXIT_FAILURE;
    }
    if(!S_ISDIR(ddip->i_mode)) {
      error("cp: %s: not a directory\n", ddir);
      return EXIT_FAILURE;
    }
    dip = icreat(img, ddip, dname, S_IFREG | S_IRWXU, NULL);
    if(dip == NULL) {
      error("cp: %s/%s: cannot create\n", ddir, dname);
      return EXIT_FAILURE;
    }
  }else {
    if(S_ISDIR(dip->i_mode)) {
      char *sname = splitpath(spath, NULL, 0);
      inode_t fp = icreat(img, dip, sname, S_IFREG | S_IRWXU, NULL);
      if(fp == NULL) {
        error("cp: %s/%s: cannot create\n", dpath, sname);
        return EXIT_FAILURE;
      }
      dip = fp;
    }else if(S_ISREG(dip->i_mode)) {
      itruncate(img, dip, 0);
    }else {
      error("cp: %s: not an ordinary file\n", dpath);
      return EXIT_FAILURE;
    }
  }
  
  // sip : source file inode, dip : destination file inode
  uchar buf[BSIZE];
  for(uint off = 0; off < sip->i_size; off += BSIZE) {
    int n = iread(img, sip, buf, BSIZE, off);
    if(n < 0) {
      error("cp: %s: read error\n", spath);
      return EXIT_FAILURE;
    }
    if(iwrite(img, dip, buf, n, off) != n) {
      error("cp: %s: write error\n", dpath);
      return EXIT_FAILURE;
    }
  }
    
  return EXIT_SUCCESS;
}

// mv src_path dest_path
int do_mv(img_t img, int argc, char *argv[]) 
{
  if (argc != 2) {
    error("usage: %s img_file mv spath dpath\n", progname);
    return EXIT_FAILURE;
  }
  char *spath = argv[0];
  char *dpath = argv[1];

    // source
  inode_t sip = ilookup(img, root_inode, spath);
  if(sip == NULL) {
    error("mv: %s: no such file or directory\n", spath);
    return EXIT_FAILURE;
  }
  if(sip == root_inode) {
    error("mv: %s: root directory\n", spath);
    return EXIT_FAILURE;
  }

  inode_t dip = ilookup(img, root_inode, dpath);
  char ddir[BSIZE];
  char *dname = splitpath(dpath, ddir, BSIZE);
  if(dip != NULL) {
    if(S_ISDIR(dip->i_mode)) {
      char *sname = splitpath(spath, NULL, 0);
      inode_t ip = dlookup(img, dip, sname, NULL);
      // ip : inode of dpath/sname
      if(ip != NULL) {
        if(S_ISDIR(ip->i_mode)) {
          // override existing empty directory
          if(!S_ISDIR(sip->i_mode)) {
            error("mv: %s: not a directory\n", spath);
            return EXIT_FAILURE;
          }
          if(!emptydir(img, ip)) {
            error("mv: %s/%s: not empty\n", ddir, sname);
            return EXIT_FAILURE;
          }
          iunlink(img, dip, sname);
          daddent(img, dip, sname, sip);
          iunlink(img, root_inode, spath);
          dmkparlink(img, dip, sip);
          return EXIT_SUCCESS;
        }else if(S_ISREG(ip->i_mode)) {
          // override existing file
          if(!S_ISREG(sip->i_mode)) {
            error("mv: %s: directory or device\n", spath);
            return EXIT_FAILURE;
          }
          iunlink(img, dip, sname);
          daddent(img, dip, sname, sip);
          iunlink(img, root_inode, spath);
          return EXIT_SUCCESS;
        }else {
          error("mv: %s: device\n", dpath);
          return EXIT_FAILURE;
        }
      }else { // ip == NULL
        daddent(img, dip, sname, sip);
        iunlink(img, root_inode, spath);
        if(S_ISDIR(sip->i_mode))
          dmkparlink(img, dip, sip);
      }
    }else if(S_ISREG(dip->i_mode)) {
      // override existing file
      if(!S_ISREG(sip->i_mode)) {
        error("mv: %s: not a file\n", spath);
        return EXIT_FAILURE;
      }
      iunlink(img, root_inode, dpath);
      inode_t ip = ilookup(img, root_inode, ddir);
      assert(ip != NULL && S_ISDIR(ip->i_mode));
      daddent(img, ip, dname, sip);
      iunlink(img, root_inode, spath);
    }else { // dip->type == T_DEV
      error("mv: %s: device\n", dpath);
      return EXIT_FAILURE;
    }
  }else { // dip == NULL
    if(is_empty(dname)) {
      error("mv: %s: no such directory\n", dpath);
      return EXIT_FAILURE;
    }
    inode_t ip = ilookup(img, root_inode, ddir);
    if(ip == NULL) {
      error("mv: %s: no such directory\n", ddir);
      return EXIT_FAILURE;
    }
    if(!S_ISDIR(ip->i_mode)) {
      error("mv: %s: not a directory\n", ddir);
      return EXIT_FAILURE;
    }
    daddent(img, ip, dname, sip);
    iunlink(img, root_inode, spath);
    if(S_ISDIR(sip->i_mode))
      dmkparlink(img, ip, sip);
  }
  return EXIT_SUCCESS;
}

// ln src_path dest_path
int do_ln(img_t img, int argc, char *argv[]) 
{
  if(argc != 2) {
    error("usage: %s img_file ln spath dpath\n", progname);
    return EXIT_FAILURE;
  }
  char *spath = argv[0];
  char *dpath = argv[1];

  // source
  inode_t sip = ilookup(img, root_inode, spath);
  if(sip == NULL) {
    error("ln: %s: no such file or directory\n", spath);
    return EXIT_FAILURE;
  }
  if(!S_ISREG(sip->i_mode)) {
    error("ln: %s: is a directory or a device\n", spath);
    return EXIT_FAILURE;
  }

  // destination
  char ddir[BSIZE];
  char *dname = splitpath(dpath, ddir, BSIZE);
  inode_t dip = ilookup(img, root_inode, ddir);
  if(dip == NULL) {
    error("ln: %s: no such directory\n", ddir);
    return EXIT_FAILURE;
  }
  if(!S_ISDIR(dip->i_mode)) {
    error("ln: %s: not a directory\n", ddir);
    return EXIT_FAILURE;
  }
  if(is_empty(dname)) {
    dname = splitpath(spath, NULL, 0);
    if(dlookup(img, dip, dname, NULL) != NULL) {
      error("ln: %s/%s: file exists\n", ddir, dname);
      return EXIT_FAILURE;
    }
  }else {
    inode_t ip = dlookup(img, dip, dname, NULL);
    if(ip != NULL) {
      if(!S_ISDIR(ip->i_mode)) {
        error("ln: %s/%s: file exists\n", ddir, dname);
        return EXIT_FAILURE;
      }
      dname = splitpath(spath, NULL, 0);
      dip = ip;
    }
  }
  if(daddent(img, dip, dname, sip) < 0) {
    error("ln: %s/%s: cannot create a link\n", ddir, dname);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// mkdir path
int do_mkdir(img_t img, int argc, char *argv[]) 
{
  if(argc != 1) {
    error("usage: %s img_file mkdir path\n", progname);
    return EXIT_FAILURE;
  }
  char *path = argv[0];
    
  if(ilookup(img, root_inode, path) != NULL) {
    error("mkdir: %s: file exists\n", path);
    return EXIT_FAILURE;
  }
  inode_t dip = icreat(img, root_inode, path, S_IFDIR | S_IRWXU, NULL);
  if(dip == NULL) {
    error("mkdir: %s: cannot create\n", path);
    return EXIT_FAILURE;
  }
  // 将目录大小对齐到BSIZE的倍数上
  uint off = dip->i_size;
	off = ((off / BSIZE) + 1) * BSIZE;
	dip->i_size = off;
  return EXIT_SUCCESS;
}

// rmdir path
int do_rmdir(img_t img, int argc, char *argv[]) 
{
  if (argc != 1) {
    error("usage: %s img_file rmdir path\n", progname);
    return EXIT_FAILURE;
  }
  char *path = argv[0];
    
  inode_t ip = ilookup(img, root_inode, path);
  if(ip == NULL) {
    error("rmdir: %s: no such file or directory\n", path);
    return EXIT_FAILURE;
  }
  if(!S_ISDIR(ip->i_mode)) {
    error("rmdir: %s: not a directory\n", path);
    return EXIT_FAILURE;
  }
  if(!emptydir(img, ip)) {
    error("rmdir: %s: non-empty directory\n", path);
    return EXIT_FAILURE;
  }
  ip->i_nlinks--;//iunlink只对i_nlinks减一次,导致目录块无法释放
  if(iunlink(img, root_inode, path) < 0) {
    error("rmdir: %s: cannot unlink\n", path);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  progname = argv[0];
  if (argc < 3) {
    error("Minixfsutils 0.8 beta\n");
    error("Copyright (c) %s Mumu3w@outlook.com\n\n", __DATE__);
    error("Usage: %s img_file command [arg...]\n", progname);
    error("Commands are:\n");
    for (int i = 0; cmd_table[i].name != NULL; i++)
      error("    %s %s\n", cmd_table[i].name, cmd_table[i].args);
    return EXIT_FAILURE;
  }
  char *img_file = argv[1];
  char *cmd = argv[2];
#ifdef _WIN32  
  HANDLE img_fd = CreateFile(img_file, 
                    GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ, 
                    NULL, 
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);  
  if(img_fd == INVALID_HANDLE_VALUE) {
    perror(img_file);
    return EXIT_FAILURE;
  }
  
  HANDLE img_map = CreateFileMapping(img_fd, 
                    NULL, 
                    PAGE_READWRITE, 
                    0, 
                    0, 
                    NULL);
  if(img_map == NULL) {
    perror(img_file);
    CloseHandle(img_fd);
    return EXIT_FAILURE;
  }
  
  char *hd_img = MapViewOfFile(img_map, FILE_MAP_WRITE, 0, 0, 0);
  if(hd_img == NULL) {
    perror(img_file);
    CloseHandle(img_map);
    CloseHandle(img_fd);
    return EXIT_FAILURE;
  }
#endif  
#ifdef __linux__
  int img_fd = open(img_file, O_RDWR);
  if(img_fd < 0) {
    perror(img_file);
    return EXIT_FAILURE;
  }

  struct stat img_sbuf;
  if(fstat(img_fd, &img_sbuf) < 0) {
    perror(img_file);
    close(img_fd);
    return EXIT_FAILURE;
  }
  size_t img_size = (size_t)img_sbuf.st_size;

  char *hd_img = mmap(NULL, img_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, img_fd, 0);
  if(hd_img == MAP_FAILED) {
    perror(img_file);
    close(img_fd);
    return EXIT_FAILURE;
  }
#endif
  struct partition part;
  rpart(hd_img, 0, &part);
  // printf("start_sect = %d\n", part.start_sect);
  img_t img = (img_t)(hd_img + (part.start_sect * SSIZE));
  if(SBLK(img)->s_magic != SUPER_MAGIC) {
    error("Unable to identify the file system type.\n");
    return 1;
  }
  
  root_inode = iget(img, root_inode_number);
  
  int status = EXIT_FAILURE;
  if(setjmp(fatal_exception_buf) == 0)
    status = exec_cmd(img, cmd, argc - 3, argv + 3);
  
#ifdef _WIN32  
  UnmapViewOfFile(hd_img);
  CloseHandle(img_map);
  CloseHandle(img_fd);
#endif
#ifdef __linux__
  munmap(hd_img, img_size);
  close(img_fd);
#endif

  return status;
}
