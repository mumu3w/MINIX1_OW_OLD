#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

// 文件类型
#define S_IFMT	(0170000)
#define S_IFLNK	(0120000)
#define S_IFREG	(0100000)
#define S_IFBLK	(0060000)
#define S_IFDIR	(0040000)
#define S_IFCHR	(0020000)
#define S_IFIFO	(0010000)

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
// 用户权限
#define S_IRWXU	(000700)
#define S_IRUSR	(000400)
#define S_IWUSR	(000200)
#define S_IXUSR	(000100)

// root i-number
#define ROOTINO (1)	
// block size	
#define BSIZE	(1024)
// sector size
#define SSIZE	(512)
// Inodes per block.
#define IPB		(BSIZE / sizeof(struct dinode))
// Bitmap bits per block
#define BPB		(BSIZE * 8)
// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 	(14)

struct dirent {
	ushort	inum;
	char	name[DIRSIZ];
};

struct partition {
    uchar   boot_ind;
    uchar   head;
    ushort  sect_cyl;
    uchar   sys_ind;
    uchar   end_head;
    ushort  end_sect_cyl;
    uint  start_sect;
    uint  nr_sects;
};

struct superblock {
	ushort	s_ninodes;
	ushort	s_nzones;
	ushort	s_imap_block;
	ushort	s_zmap_block;
	ushort	s_firstdatazone;
	ushort	s_log_zine_size;
	uint	s_max_size;
	ushort	s_magic;
};

struct dinode {
	ushort	i_mode;
	ushort	i_uid;
	uint	i_size;
	uint	i_mtime;
	uchar	i_gid;
	uchar	i_nlinks;
	ushort	i_zone[9];
};


FILE *fsfd;
uint start_sec;// 分区的开始扇区
uint freeinode = 1;
uint freeblock;
struct superblock sb;
ushort firstimapzone;
ushort firstzmapzone;
ushort firstinodezone;


void rpart(int id, struct partition *part);
void cls_bit(char *map, int x);
void make_sb(uint nr_sects);
void wblock(uint blkno, void *buf);
void winode(uint inum, struct dinode *ip);
void rinode(uint inum, struct dinode *ip);
void rblock(uint blkno, void *buf);
uint ialloc(ushort type, int major, int minor);
void balloc(int used);
void iappend(uint inum, void *xp, int n);
uint creat_dir(uint inum, char *name);
int creat_file(uint inum, char *name, char ch, int major, int minor);
void fixsize(uint inum);


#define UPPER(size, n)  	((size+((n)-1))/(n))

#define INODE_SIZE			(sizeof(struct dinode))
#define INODE_BLOCKS		UPPER(INODES, IPB)
#define INODE_BUFFER_SIZE 	(INODE_BLOCKS * BSIZE)

#define INODES			(Super->s_ninodes)
#define ZONES			(Super->s_nzones)
#define IMAPS			(Super->s_imap_block)
#define ZMAPS			(Super->s_zmap_block)
#define FIRSTZONE		(Super->s_firstdatazone)
#define ZONESIZE		(Super->s_log_zine_size)
#define MAXSIZE			(Super->s_max_size)
#define MAGIC			(Super->s_magic)

#define NORM_FIRSTZONE	(2+IMAPS+ZMAPS+INODE_BLOCKS)
char sb_buf[BSIZE];
char inode_map[BSIZE * 8];
char zone_map[BSIZE * 8];


int 
main(int argc, char *argv[])
{
	int i, status = 0;
  uint rootino, devino, binino;
	struct partition part;
	
  if(argc < 2){
    fprintf(stderr, "Usage: mkfs rootfs.img files...\n");
    exit(1);
  }
  
	fsfd = fopen(argv[1], "rb+");
  if(fsfd == NULL) {
    perror(argv[1]);
    exit(1);
  }
	
	rpart(0, &part);
	start_sec = part.start_sect;
	make_sb(18000);

	rootino = creat_dir(0, NULL);
  binino = creat_dir(rootino, "bin");
  devino = creat_dir(rootino, "dev");
  
  creat_file(devino, "tty", 'c', 5, 0);
  creat_file(devino, "console", 'c', 5, 1);

	for(i = 2; i < argc; i++) {
    if(!strcmp(argv[i], "-")) {
      status = 1;
      continue;
    }

    if(status == 1)
      creat_file(binino, argv[i], '-', 0, 0);
    if(status == 0)
      creat_file(rootino, argv[i], '-', 0, 0);
	}
  
  fixsize(rootino);
  fixsize(binino);
  fixsize(devino);

	balloc(freeblock - sb.s_firstdatazone + 1);
	
	fclose(fsfd);
	exit(0);
}

void
rpart(int id, struct partition *part) 
{
	char buf[BSIZE];
	struct partition *p;
	
	fseek(fsfd, 0, SEEK_SET);
	fread(buf, 1, BSIZE, fsfd);
	p = (struct partition *)&buf[0x1be];
	*part = *(p + id);
}

void 
cls_bit(char *map, int x)
{
	uchar m = 0;
	
	m = 0x01 << (x%8);
	map[x/8] = map[x/8] & ~m;
}

void
make_sb(uint nr_sects)
{
	int i;
	struct superblock *Super = (struct superblock *)sb_buf;
	
	memset(inode_map, 0xff, 8 * BSIZE);
	memset(zone_map, 0xff, 8 * BSIZE);
	memset(sb_buf, 0, BSIZE);
	
	MAGIC = 0x137f;
	ZONESIZE = 0;
	MAXSIZE = (7 + 512 + 512 * 512) * 1024;
	ZONES = nr_sects / 2;
	INODES = ZONES / 3;
	if((INODES & 8191) > 8188)
	{
		INODES -= 5;
	}
	if((INODES & 8191) < 10)
	{
		INODES -= 20;
	}
	IMAPS = UPPER(INODES, BPB);
	ZMAPS = 0;
	while(ZMAPS != UPPER(ZONES - NORM_FIRSTZONE, BPB))
	{
		ZMAPS = UPPER(ZONES - NORM_FIRSTZONE, BPB);
	}
	FIRSTZONE = NORM_FIRSTZONE;
	
	for(i = FIRSTZONE; i < ZONES; i++)
	{
		cls_bit(zone_map, i - FIRSTZONE + 1);
	}
	for(i = ROOTINO; i < INODES; i++)
	{
		cls_bit(inode_map, i);
	}
	
	sb = *Super;
	freeblock = FIRSTZONE;
	firstimapzone = 2;
	firstzmapzone = firstimapzone + IMAPS;
	firstinodezone = firstzmapzone + ZMAPS;
	
	printf("%d inodes\n", INODES);
	printf("%d blocks\n", ZONES);
	printf("Firstdatazone=%d (%d)\n", FIRSTZONE, NORM_FIRSTZONE);
	printf("Zonesize=%d\n", BSIZE << ZONESIZE);
	printf("Maxsize=%d \n\n", MAXSIZE);
	
	wblock(1, sb_buf);
	for(i = 0; i < IMAPS; i++) {
		wblock(firstimapzone + i, &inode_map[i * BSIZE]);
	}
	for(i = 0; i < ZMAPS; i++) {
		wblock(firstzmapzone + i, &zone_map[i * BSIZE]);
	}
}

void 
wblock(uint blkno, void *buf)
{
	if(fseek(fsfd, start_sec * SSIZE + blkno * BSIZE, SEEK_SET) != 0) {
		perror("fseek");
		exit(1);
	}
	if(fwrite(buf, 1, BSIZE, fsfd) != BSIZE) {
		perror("fwrite");
		exit(1);
	}
}

void
winode(uint inum, struct dinode *ip)
{
	char buf[BSIZE];
	uint bn;
	struct dinode *dip;
	
	bn = (inum - 1) / IPB;
	rblock(firstinodezone + bn, buf);
	dip = ((struct dinode*)buf) + ((inum - 1) % IPB);
	*dip = *ip;
	wblock(firstinodezone + bn, buf);
}

void
rinode(uint inum, struct dinode *ip)
{
	char buf[BSIZE];
	uint bn;
	struct dinode *dip;
	
	bn = (inum - 1) / IPB;
	rblock(firstinodezone + bn, buf);
	dip = ((struct dinode*)buf) + ((inum - 1) % IPB);
	*ip = *dip;
}

void
rblock(uint blkno, void *buf)
{
	if(fseek(fsfd, start_sec * SSIZE + blkno * BSIZE, SEEK_SET) != 0) {
		perror("fseek");
		exit(1);
	}
	if(fread(buf, 1, BSIZE, fsfd) != BSIZE) {
		perror("fwrite");
		exit(1);
	}
}

uint
ialloc(ushort type, int major, int minor)
{
	char buf[BSIZE];
	uint bi, b, inum;
	uchar m;
	struct dinode din;

	inum = freeinode++;
	rblock(firstimapzone + inum / BPB, buf);
	buf[inum / 8] |= (0x1 << (inum % 8));
	wblock(firstimapzone + inum / BPB, buf);
	
	memset(&din, 0, sizeof(struct dinode));
	din.i_mode = type | S_IRWXU;
	din.i_uid = 0;
	din.i_size = 0;
	din.i_mtime = 0;
	din.i_gid = 0;
	din.i_nlinks = 1;
	if(S_ISBLK(type) || S_ISCHR(type)) {
		din.i_zone[0] = major << 8 | minor;
	}
	winode(inum, &din);
	
	return inum;// 返回找到的空闲i节点
}

void
balloc(int used)
{
	char buf[BSIZE];
	uint bi, b;
	uchar m;

	for(b = 0; b < used; b += BPB) {
		rblock(firstzmapzone + b / BPB, buf);
		for(bi = 0; bi < BPB && b + bi < used; bi++) {
			buf[bi / 8] |= (0x1 << (bi % 8));// 置位
		}
		wblock(firstzmapzone + b / BPB, buf);
	}
}

#define min(a, b)	((a) < (b) ? (a) : (b))

void
iappend(uint inum, void *xp, int n)
{
	char *p = (char *)xp;
	uint fbn, fbnt, off, nl;
	struct dinode din;
	char buf[BSIZE];
	ushort buf_ind[BSIZE / sizeof(ushort)];
	ushort buf_dind[BSIZE / sizeof(ushort)];
	uint x;
	
	rinode(inum, &din);
	
	off = din.i_size;
	while(n > 0) {
		fbn = off / BSIZE;
		fbnt = fbn;
		if(fbn < 7) {
			if(din.i_zone[fbn] == 0) {
				din.i_zone[fbn] = freeblock++;
			}
			x = din.i_zone[fbn];
		}else if((fbn - 7) < 512) {
			fbn -= 7;
			if(din.i_zone[7] == 0) {
				din.i_zone[7] = freeblock++;
			}
			rblock(din.i_zone[7], (char *)buf_ind);
			if(buf_ind[fbn] == 0) {
				buf_ind[fbn] = freeblock++;
				wblock(din.i_zone[7], (char *)buf_ind);
			}
			x = buf_ind[fbn];
		}else {
			fbn -= (7 + 512);
			if(din.i_zone[8] == 0) {
				din.i_zone[8] = freeblock++;
			}
			rblock(din.i_zone[8], (char *)buf_ind);
			if(buf_ind[fbn >> 9] == 0) {
				buf_ind[fbn >> 9] = freeblock++;
				wblock(din.i_zone[8], (char *)buf_ind);
			}
			rblock(buf_ind[fbn >> 9], (char *)buf_dind);
			if(buf_dind[fbn & 511] == 0) {
				buf_dind[fbn & 511] = freeblock++;
				wblock(buf_ind[fbn >> 9], (char *)buf_dind);
			}
			x = buf_dind[fbn & 511];
		}
		nl = min(n, (fbnt + 1) * BSIZE - off);
		rblock(x, buf);
		memcpy(buf + off - (fbnt * BSIZE), p, nl);
		wblock(x, buf);
		n -= nl;
		off += nl;
		p += nl;
	}
	din.i_size = off;
	winode(inum, &din);
}	

uint 
creat_dir(uint inum, char *name)
{
  struct dirent de;
  struct dinode din;
  uint i;
  
  if(inum == 0) {
    i = ialloc(S_IFDIR, 0, 0);
    
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strcpy(de.name, ".");
    iappend(i, &de, sizeof(struct dirent));
	
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strcpy(de.name, "..");
    iappend(i, &de, sizeof(struct dirent));
    
    rinode(i, &din);
    din.i_nlinks = 1;
    winode(i, &din);
  }else {
    i = ialloc(S_IFDIR, 0, 0);
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strncpy(de.name, name, 14);
    iappend(inum, &de, sizeof(struct dirent));
    
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strcpy(de.name, ".");
    iappend(i, &de, sizeof(struct dirent));
    
    memset(&de, 0, sizeof(struct dirent));
    de.inum = inum;
    strcpy(de.name, "..");
    iappend(i, &de, sizeof(struct dirent));
    
    rinode(i, &din);
    din.i_nlinks = 2;
    winode(i, &din);
  }
  
  return i;
}

int 
creat_file(uint inum, char *name, char ch, int major, int minor)
{
  uint i;
  struct dirent de;
  struct dinode din;
  FILE *fd;
  int cc;
	uint off;
	char buf[BSIZE];
  
  if(ch == '-') {
    if((fd = fopen(name, "rb+")) == NULL) {
			perror(name);
			exit(1);
		}
    
    i = ialloc(S_IFREG, 0, 0);
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strncpy(de.name, name, 14);
    iappend(inum, &de, sizeof(struct dirent));
		
		while((cc = fread(buf, 1, BSIZE, fd)) > 0)
			iappend(i, buf, cc);
    
    fclose(fd);
  }
  
  if(ch == 'c' || ch == 'b') {
    if(ch == 'c')
      i = ialloc(S_IFCHR, major, minor);
    if(ch == 'b')
      i = ialloc(S_IFBLK, major, minor);
    memset(&de, 0, sizeof(struct dirent));
    de.inum = i;
    strncpy(de.name, name, 14);
    iappend(inum, &de, sizeof(struct dirent));
  }

	rinode(inum, &din);
  din.i_nlinks++;
	winode(inum, &din);
  
  return 0;
}

void 
fixsize(uint inum)
{
  struct dinode din;
  uint off;
  
  rinode(inum, &din);
	off = din.i_size;
	off = ((off / BSIZE) + 1) * BSIZE;
	din.i_size = off;
	winode(inum, &din);
}
