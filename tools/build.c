#include <stdio.h>
#include <stdlib.h>


typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned int UDWORD;


#define PROGRAMS 5            
#define PROG_ORG 1536         
#define DS_OFFSET 4L          
#define SECTOR_SIZE 512       
#define READ_UNIT 512         
#define KERNEL_D_MAGIC 0x526F 
#define FS_D_MAGIC 0xDADA	
#define CLICK_SHIFT 4
#define KERN 0
#define MM   1
#define FS   2
#define INIT 3
#define FSCK 4


#define HEADER1 32
#define HEADER2 48
#define SEP_POS 1
#define HDR_LEN 2
#define TEXT_POS 0 
#define DATA_POS 1 
#define BSS_POS 2
#define SEP_ID_BIT 0x20


#define IMAGE_SIZE 360


void patch3(void);
void patch2(void);
UWORD get_byte(long offset);
void put_byte(long offset, int byte_value);
void write_block(int blocknr, char *buf);
void read_block(int blocknr, char *buf);
void patch1(long all_size);
void copy2(int num, char *file_name);
void copy1(char *filename);
void create_image(char *s);
void pexit(char *s1, char *s2);


FILE *image;
char zero[SECTOR_SIZE];


long cum_size;
long all_size;


struct sizes {
  unsigned text_size;  
  unsigned data_size;
  unsigned bss_size; 
  int sep_id;
} sizes[PROGRAMS];


char *name[] = {"\nkernel", "mm    ", "fs    ", "init  ", "fsck  "};


int main(int argc, char *argv[])
{
  int i;
  
  if (argc != PROGRAMS + 3)
    pexit("seven file names expected. ", "");
  
  create_image(argv[7]);
  
  copy1(argv[1]);
  
  for (i = 0; i < PROGRAMS; i++)
    copy2(i, argv[i+2]);
  
  printf("                                               -----     -----\n");
  printf("Operating system size  %29ld     %5lx\n", cum_size, cum_size);
  printf("\nTotal size including fsck is %ld.\n", all_size);
  
  patch1(all_size);
  patch2();
  patch3();
  
  fclose(image);
  exit(0);
  return 0;
}


int wr_out(char *buffer, int bytes)
{
  return fwrite(buffer, 1, bytes, image);
}


void patch3(void)
{
  UWORD init_text_size, init_data_size, init_buf[SECTOR_SIZE/2], i;
  UWORD w0, w1, w2;
  int b0, b1, b2, b3, b4, b5, mag;
  UDWORD init_org, fs_org, fbase, mm_data;

  init_org  = PROG_ORG;
  init_org += sizes[KERN].text_size+sizes[KERN].data_size+sizes[KERN].bss_size;
  mm_data = init_org - PROG_ORG + 512L; /* mm在文件中的偏移 */
  mm_data += (UDWORD)sizes[MM].text_size; /* mm数据段在文件中的偏移 */
  init_org += sizes[MM].text_size + sizes[MM].data_size + sizes[MM].bss_size;
  fs_org = init_org - PROG_ORG + 512L; /* fs在文件中的偏移 */
  fs_org += (UDWORD)sizes[FS].text_size; /* fs数据段在文件中的偏移 */
  init_org += sizes[FS].text_size + sizes[FS].data_size + sizes[FS].bss_size;
  init_text_size = sizes[INIT].text_size;
  init_data_size = sizes[INIT].data_size + sizes[INIT].bss_size;
  init_org  >>= CLICK_SHIFT; /* INIT在内存中的位置(CS) */
  if (sizes[INIT].sep_id == 0) {
    init_data_size += init_text_size;
    init_text_size = 0;
  }
  init_text_size >>= CLICK_SHIFT;
  init_data_size >>= CLICK_SHIFT;

  w0 = (UWORD)init_org;
  w1 = init_text_size;
  w2 = init_data_size;
  b0 =  w0 & 0377;
  b1 = (w0 >> 8) & 0377;
  b2 = w1 & 0377;
  b3 = (w1 >> 8) & 0377;
  b4 = w2 & 0377;
  b5 = (w2 >> 8) & 0377;

  fbase = fs_org;
  mag = (get_byte(mm_data+1L) << 8) | get_byte(mm_data+0L);
  if (mag != FS_D_MAGIC) 
    pexit("mm data space: no magic #", "");
  mag = (get_byte(fbase+1L) << 8) | get_byte(fbase+0L);
  if (mag != FS_D_MAGIC) 
    pexit("fs data space: no magic #", "");

  put_byte(fbase+4L, b0);
  put_byte(fbase+5L, b1);
  put_byte(fbase+6L, b2);
  put_byte(fbase+7L, b3);
  put_byte(fbase+8L ,b4);
  put_byte(fbase+9L, b5);
}


void patch2(void)
{
  int i, j;
  UWORD t, d, b, text_clicks, data_clicks, ds;
  UDWORD data_offset;
  
  data_offset = 512L + (UDWORD)sizes[KERN].text_size;
  i = (get_byte(data_offset + 1L) << 8) + get_byte(data_offset);
  if (i != KERNEL_D_MAGIC) {
	  printf("Magic number in kernel is %x: should be %x\n", i, KERNEL_D_MAGIC);
    pexit("kernel data space: no magic #", "");
  }
  
  for (i = 0; i < PROGRAMS - 1; i++) {
    t = sizes[i].text_size;
    d = sizes[i].data_size;
    b = sizes[i].bss_size;
    if (sizes[i].sep_id) {
      text_clicks = t >> CLICK_SHIFT;
      data_clicks = (d + b) >> CLICK_SHIFT;
    } else {
      text_clicks = 0;
      data_clicks = (t + d + b) >> CLICK_SHIFT;
    }
    put_byte(data_offset + 4*i + 0L, (text_clicks>>0) & 0377);
    put_byte(data_offset + 4*i + 1L, (text_clicks>>8) & 0377);
    put_byte(data_offset + 4*i + 2L, (data_clicks>>0) & 0377);
    put_byte(data_offset + 4*i + 3L, (data_clicks>>8) & 0377);
  }
  
  if (sizes[KERN].sep_id == 0)
    ds = PROG_ORG >> CLICK_SHIFT;
  else
    ds = (PROG_ORG + sizes[KERN].text_size) >> CLICK_SHIFT;
	printf("Kernel DS = %x\n", ds);
  put_byte(512L + DS_OFFSET, ds & 0377);
  put_byte(512L + DS_OFFSET + 1L, (ds>>8) & 0377);
}


void patch1(long all_size)
{
  long fsck_org;
  UWORD ip, cs, ds, ubuf[SECTOR_SIZE/2], sectors;
  
  if (cum_size % 16 != 0)
    pexit("MINIX is not multiple of 16 bytes", "");
  fsck_org = PROG_ORG + cum_size;
  ip = 0;
  cs = fsck_org >> CLICK_SHIFT;
  if (sizes[FSCK].sep_id)
    ds = cs + (sizes[FSCK].text_size >> CLICK_SHIFT);
  else
    ds = cs;
  
  sectors = (UWORD)(all_size / 512L);
  
  /*printf("%d %d %d %d", sectors + 1, ds, ip, cs);*/
  read_block(0, (char *)ubuf);
  ubuf[(SECTOR_SIZE/2) - 5] = sectors + 1;
  ubuf[(SECTOR_SIZE/2) - 4] = ds;
  ubuf[(SECTOR_SIZE/2) - 3] = ip;
  ubuf[(SECTOR_SIZE/2) - 2] = cs;
  ubuf[(SECTOR_SIZE/2) - 1] = 0xaa55;
  write_block(0, (char *)ubuf);
}


void read_header(FILE *fd,
                int *sepid,
                UWORD *text_bytes,
                UWORD *data_bytes,
                UWORD *bss_bytes,
                char *file_name)
{
  UDWORD head[12];
  UWORD hd[4];
  int n, header_len;

  n = fread(hd, 1, 8, fd);
  if (n != 8)
    pexit("file header too short: ", file_name); 
  header_len = hd[HDR_LEN];  
  if (header_len != HEADER1 && header_len != HEADER2)
    pexit("bad header length. File: ", file_name);
  
  *sepid = hd[SEP_POS] & SEP_ID_BIT;
  
  n = fread(head, 1, header_len - 8, fd);
  if (n != header_len - 8)
    pexit("header too short: ", file_name);
  
  *text_bytes = (UWORD)head[TEXT_POS];
  *data_bytes = (UWORD)head[DATA_POS];
  *bss_bytes  = (UWORD)head[BSS_POS];
}


void copy2(int num, char *file_name)
{
  FILE *fd;
  int sepid, bytes_read, count;
  UWORD text_bytes, data_bytes, bss_bytes, tot_bytes, rest, filler;
  UWORD left_to_read;
  char inbuf[READ_UNIT];
  
  fd = fopen(file_name, "rb");
  if (fd == NULL)
    pexit("can`t open ", file_name);
  
  read_header(fd, &sepid, &text_bytes, &data_bytes, &bss_bytes, file_name);
  
  if (sepid && ((text_bytes % 16) != 0)) {
    pexit("separate I & D but text size not multiple of 16 bytes.  File: ", 
                                                                file_name);
  }
  
  tot_bytes = text_bytes + data_bytes + bss_bytes;
  rest = tot_bytes % 16;
  filler = (rest > 0 ? 16 - rest : 0);
  bss_bytes += filler;
  tot_bytes += filler;
  if (num < FSCK) 
    cum_size += tot_bytes;
  all_size += tot_bytes;
  
  sizes[num].text_size = text_bytes;
  sizes[num].data_size = data_bytes;
  sizes[num].bss_size = bss_bytes;
  sizes[num].sep_id = sepid;
  
  if (num < FSCK) { 
    printf("%s  text=%5u  data=%5u  bss=%5u  tot=%5u  hex=%4x  %s\n",
                name[num], text_bytes, data_bytes, bss_bytes, tot_bytes,
                tot_bytes, (sizes[num].sep_id ? "Separate I & D" : ""));
  }
  
  left_to_read = text_bytes + data_bytes;
  while (left_to_read > 0) {
    count = (left_to_read < READ_UNIT ? left_to_read : READ_UNIT);
    bytes_read = fread(inbuf, 1, count, fd);
    if (bytes_read != count) 
      pexit("read error on file ", file_name);
    if (bytes_read > 0) 
      wr_out(inbuf, bytes_read);
      /*fwrite(inbuf, 1, bytes_read, image);*/
    left_to_read -= count;
  }

  while (bss_bytes > 0) {
    count = (bss_bytes < SECTOR_SIZE ? bss_bytes : SECTOR_SIZE);
    wr_out(zero, count);
    /*fwrite(zero, 1, count, image);*/
    bss_bytes -= count;
  }
  
  fclose(fd);
}


void copy1(char *file_name)
{
  FILE *fd;
  int bytes_read;
  char inbuf[READ_UNIT];
  
  fd = fopen(file_name, "rb");
  if (fd == NULL)
    pexit("can`t open ", file_name);
  
  bytes_read = fread(inbuf, 1, READ_UNIT, fd);
  if (bytes_read != READ_UNIT)
    pexit("read error on file ", file_name);
  bytes_read = wr_out(inbuf, bytes_read);
  /*bytes_read = fwrite(inbuf, 1, bytes_read, image);*/
  if (bytes_read != READ_UNIT)
    pexit("write error on file ", file_name);
  
  fclose(fd);
}


void create_image(char *s)
{
  int i, bytes;
  
  
  remove(s);
  image = fopen(s, "wb+");
  if (image == NULL)
    pexit("unable to create file: ", s);
  
  for (i = 0; i < IMAGE_SIZE; i++) {
    bytes = wr_out(zero, SECTOR_SIZE);
    /*bytes = fwrite(zero, 1, SECTOR_SIZE, image);*/
    if (bytes != SECTOR_SIZE)
      pexit("write error on file ", s);
    bytes = wr_out(zero, SECTOR_SIZE);
    /*bytes = fwrite(zero, 1, SECTOR_SIZE, image);*/
    if (bytes != SECTOR_SIZE)
      pexit("write error on file ", s);
  }
  
  if (fseek(image, 0, SEEK_SET) != 0)
    pexit("fseek error on file ", s);
}


UWORD get_byte(long offset)
{
  char buff[SECTOR_SIZE];
  
  read_block((UWORD)(offset / SECTOR_SIZE), buff);
  return (buff[(UWORD)(offset % SECTOR_SIZE)] & 0377);
}


void put_byte(long offset, int byte_value)
{
  char buff[SECTOR_SIZE];

  read_block((UWORD)(offset/SECTOR_SIZE), buff);
  buff[(UWORD)(offset % SECTOR_SIZE)] = byte_value;
  write_block((UWORD)(offset/SECTOR_SIZE), buff);
}


void read_block(int blocknr, char *buf)
{
  long currpos;
  
  currpos = fseek(image, 0, SEEK_CUR);
  fseek(image, (UWORD)blocknr * SECTOR_SIZE, SEEK_SET);
  fread(buf, 1, SECTOR_SIZE, image);
  fseek(image, currpos, SEEK_SET);
}


void write_block(int blocknr, char *buf)
{
  long currpos;
  
  currpos = fseek(image, 0, SEEK_CUR);
  fseek(image, (UWORD)blocknr * SECTOR_SIZE, SEEK_SET);
  fwrite(buf, 1, SECTOR_SIZE, image);
  fseek(image, currpos, SEEK_SET);
}


void pexit(char *s1, char *s2)
{
  printf("Build: %s%s\n", s1, s2);
  exit(1);
}
