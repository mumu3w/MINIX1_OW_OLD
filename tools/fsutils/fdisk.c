#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;

struct partition
{
    uint8   boot_ind;
    uint8   head;
 // uint8   sector;
 // uint8   cyl;
    uint16  sect_cyl;
    uint8   sys_ind;
    uint8   end_head;
 // uint8   end_sector;
 // uint8   end_cyl;
    uint16  end_sect_cyl;
    uint32  start_sect;
    uint32  nr_sects;
};

// 76543210 fedcba98
//   543210 -> sect
// 76       fedcba98 -> 76fedcba98 -> cyl
static inline uint8 read_sect(uint16 sect_cyl)
{
    return sect_cyl & 0x3f;
}

static inline uint16 read_cyl(uint16 sect_cyl)
{
    return ((sect_cyl << 2) & 0x0300) | ((sect_cyl >> 8) & 0xff);
}

static inline uint16 set_sect(uint16 sect_cyl, uint8 sector)
{
    uint8 *p = (uint8 *)&sect_cyl;
    p[0] = (p[0] & 0xc0) | (sector & 0x3f);
    return sect_cyl;
}

static inline uint16 set_cyl(uint16 sect_cyl, uint16 cyl)
{
    uint8 *p1 = (uint8 *)&sect_cyl;
    uint8 *p2 = (uint8 *)&cyl;
    
    p1[0] &= 0x3f;
    p1[1] &= 0x00;
    p2[1] <<= 6;
    
    p1[0] |= p2[1];
    p1[1] |= p2[0];
    
    return sect_cyl;
}

#define UPPER(size, n)  ((size+((n)-1))/(n))
#define BLOCK_SIZE      (1024)
#define SECTOR_SIZE     (512)

FILE *img_fp;

void write_part(int id, uint32 disk_sign, struct partition part, uint16 sign)
{
	char buf[SECTOR_SIZE];
	struct partition *p;
	
	fseek(img_fp, 0, SEEK_SET);
	fread(buf, 1, SECTOR_SIZE, img_fp);
	
	*(uint32 *)&buf[440] = disk_sign;
	p = (struct partition *)(buf + 0x1be + id * sizeof(struct partition));
	memcpy(p, &part, sizeof(struct partition));
	*(uint16 *)&buf[510] = sign; // 0xaa55
	
	fseek(img_fp, 0, SEEK_SET);
    fwrite(buf, 1, SECTOR_SIZE, img_fp);
}

void fdisk(const int hd_size, const int HPC, const int SPT)
{
    int C;
    int img_size;
    char buf[BLOCK_SIZE];
    int i;
    struct partition *p, part;
    int nr_sects;
    int start_sect = 2048;
    
    C = UPPER(hd_size * 1000000 / SECTOR_SIZE / HPC, SPT);
    img_size = C * HPC * SPT * SECTOR_SIZE;
//	printf("%d\n", img_size);
    
//    img_fp = fopen("rootfs.img", "wb+");
    memset(buf, 0, BLOCK_SIZE);
    for(i = 0; i < img_size / BLOCK_SIZE; i++)
    {
        fwrite(buf, 1, BLOCK_SIZE, img_fp);
    }
    
    nr_sects = C * HPC * SPT - start_sect;
    part.boot_ind = 0x0;
    part.head = start_sect / SPT % HPC;
    part.sect_cyl = set_sect(part.sect_cyl, start_sect % SPT + 1);
    part.sect_cyl = set_cyl(part.sect_cyl, start_sect / (SPT * HPC));
    
    part.sys_ind = 0x80;
    part.end_head = (start_sect + nr_sects - 1) / SPT % HPC;
    part.end_sect_cyl \
        = set_sect(part.end_sect_cyl, (start_sect + nr_sects - 1) % SPT + 1);
    part.end_sect_cyl \
      = set_cyl(part.end_sect_cyl, (start_sect + nr_sects - 1) / (SPT * HPC));

    part.start_sect = start_sect;
    part.nr_sects = nr_sects;
	
	write_part(0, 0xefcdab89, part, 0xaa55);
	printf("part size = %d(KB)\n", part.nr_sects / 2);
    
//   p = (struct partition *)(buf + 0x1be);
//   memcpy(p, &part, sizeof(struct partition));
//	*(uint32 *)&buf[440] = 0xfedcba98;
//   buf[510] = 0x55;
//   buf[511] = 0xaa;
//   
//   fseek(img_fp, 0, SEEK_SET);
//   fwrite(buf, 1, SECTOR_SIZE, img_fp);
//    fclose(img_fp);
    
//  printf("0x%X\n", read_sect(part.sect_cyl));
//  printf("0x%X\n", read_cyl(part.sect_cyl));
//  printf("0x%X\n", read_sect(part.end_sect_cyl));
//  printf("0x%X\n", read_cyl(part.end_sect_cyl));
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf(" Usage: fdisk name size(M)\n");
		return 0;
	}
	
	img_fp = fopen(argv[1], "wb+");
    fdisk(atoi(argv[2]), 16, 63);
	fclose(img_fp);
    return 0;
}