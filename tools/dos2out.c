/*      dos2out  -  convert ms-dos exe-format to a.out format
 *                  december '85
 *                  April '18 (mumu3w@outlook.com)
 *
 *
 *
 *
 *

Description of file-structures:

         a.out                                   .exe
-------------------------               -------------------------
|        header         |               |        header         |
|                       |               |          +            |
+-----------------------+               |      relocation       |
|        text           |               |         info          |
|          +            |               +-----------------------+
|        data           |               |                       |
+-----------------------+               |                       |
|   relocation info     |               |        text           |
+-----------------------+               |          +            |
|    symbol table       |               |        data           |
+-----------------------+               |                       |
|    string table       |               |                       |
+-----------------------+               +-----------------------+



For more information see MS-DOS programmers reference manual

*/

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;


/* define the a.out header format (short form) */


/* define the formatted part of the header of an MS-DOS exe-file */


#define D_FMT_SIZE       28             /* size of formatted header part */     
#define IS_EXE       0x5A4D             /* valid linker signature; note that
                                           in a dump it shows as 4D 5A */


typedef struct d_reloctab {             /* DOS relocation table entry */
        WORD    r_offset;               /* offset to symbol in load-module */
        WORD    r_segment;              /* segment relative to zero;
                                           real segment must now be added */
}d_reloctab;

/* dos formatted part of the header */
typedef struct d_fmt_hdr {
        WORD    ldsign;                 /* linker signature, 5A4Dh */
        WORD    last_page_size;         /* size of last page */
        WORD    nr_of_pages;            /* a page is 512 b */
        WORD    nr_of_relocs;           /* nr of relocation items */
        WORD    hdr_size;               /* header-size in *16b */
        WORD    min_free;               /* min free mem required after prog */
        WORD    max_free;               /* max free mem ever needed (FFFFh) */
        WORD    ss_seg;                 /* stack-segment; must be relocated */
        WORD    sp_reg;                 /* sp value to be loaded */
        WORD    chksum;                 /* neg. sum of all words in file */
        WORD    ip_reg;                 /* ip value (program entry-point) */
        WORD    cs_seg;                 /* offset of code-seg in load-module */
        WORD    reloc_table;            /* start of reloc-table in hdr, 1Bh */
        WORD    overlay_num;            /* overlay number - not used */
}d_fmt_hdr;

#define MAGIC0         0x01             /* magic number, byte 1 */
#define MAGIC1         0x03             /* and second byte */
#define A_SEP          0x20             /* seperate I&D flag */
#define A_EXEC         0x10             /* executable */
#define A_I8086        0x04             /* Intel 8088/86 cpu */
#define A_HDR_LEN        32             /* short form of header */
#define DOS          0xFFFF             /* version for a.out is -1 */


typedef struct a_out_hdr {
        BYTE            a_magic[2];     /* magic number */
        BYTE            a_flags;        /* flags for sep I&D etc */
        BYTE            a_cpu;          /* cpu-type */
        BYTE            a_hdrlen;       /* length of header */
        BYTE            a_unused;       /* sic */
        WORD            a_version;      /* version stamp */
        DWORD           a_text;         /* size of text segment in bytes */
        DWORD           a_data;         /* size of data-segment in bytes */
        DWORD           a_bss;          /* size of bss (stack) segment   */
        DWORD           a_entry;        /* program entry-point */
        DWORD           a_totb;         /* other, eg initial stack-ptr */
        DWORD           a_syms;         /* symbol-table size */
        /* END OF SHORT FORM */
}a_out_hdr;

/* bytes & words left to right? */
#define A_BLR(cputype)  ((cputype&0x01)!=0)  /* TRUE if bytes left-to-right */
#define A_WLR(cputype)  ((cputype&0x02)!=0)  /* TRUE if words left-to-right */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define PH_SECTSIZE     512             /* size of a disk-block */
#define NOT_A_HDR_LEN   (PH_SECTSIZE-A_HDR_LEN) /* complement */
#define FNAME            32             /* length of filename/path +1 */


unsigned char inbuf[PH_SECTSIZE];
unsigned char inbuf2[PH_SECTSIZE];
unsigned char inbuf3[PH_SECTSIZE];
unsigned char outbuf[PH_SECTSIZE];

struct a_out_hdr *a_ptr= (a_out_hdr *)outbuf;
struct d_fmt_hdr *d_ptr= (d_fmt_hdr *)inbuf;


void printinfo (void)
{
  printf ("\n\nDOS-header:\n");
  printf ("          nr of pages:  %4xh    (%4d dec)\n",
        d_ptr->nr_of_pages, d_ptr->nr_of_pages);
  printf ("   bytes in last page:  %4xh    (%4d dec)\n",
        d_ptr->last_page_size, d_ptr->last_page_size);
  printf ("    min. free mem *16:  %4xh    (%4d dec)\n",
        d_ptr->min_free, d_ptr->min_free);
  printf ("           stack-size:  %4xh    (%4d dec)\n",
        d_ptr->ss_seg, d_ptr->ss_seg);
  printf ("    stack-pointer val:  %4xh    (%4d dec)\n",
        d_ptr->sp_reg, d_ptr->sp_reg);
  printf ("  program entry-point:  %4xh    (%4d dec)\n",
        d_ptr->ip_reg, d_ptr->ip_reg);
  printf ("     code-segment val:  %4xh    (%4d dec)\n",
        d_ptr->cs_seg, d_ptr->cs_seg);

  printf ("\n\nOUT-header:\n");
  printf ("            text-size: %6xh   (%6d dec)\n",
        a_ptr->a_text, a_ptr->a_text);
  printf ("            data-size: %6xh   (%6d dec)\n",
        a_ptr->a_data, a_ptr->a_data);
  printf ("             bss-size: %6xh   (%6d dec)\n",
        a_ptr->a_bss, a_ptr->a_bss);
  printf ("          entry-point: %6xh   (%6d dec)\n",
        a_ptr->a_entry, a_ptr->a_entry);
  printf ("           totalbytes: %6xh   (%6d dec)\n\n",
        a_ptr->a_totb, a_ptr->a_totb);
}


int main (int argc, char *argv[])
{
  char *dptr;                   /* pointer to DOS-header */
  char *sptr;                   /* pointer to OUT-header */
  char in_name[FNAME];          /* input filename */
  char out_name[FNAME];         /* output filename */
  WORD  i;
  WORD  in_cnt;                  /* # bytes read from input */
  WORD  out_cnt;                 /* # bytes written to output */
  WORD  block_cnt;               /* # nr of bloks of load-module */
  WORD  lastp_cnt;               /* # bytes in last block */
  FILE *inf, *outf;              /* filedescriptors */
  WORD  print=0;                 /* switch: print information */
  WORD  delete=0;                /* delete exe file after processing */
  WORD  sym;                     /* data-relocation symbol offset */
  int  sym_blk, sym_off;         /* load-module block & offset in blk */
  DWORD d_size;                  /* size of data-segment */
  DWORD t_size;                  /* size of text-segment */
  DWORD load_size;               /* size of load-module in bytes */


/*================================================================
 *                      process parameters
 *===============================================================*/


  if (argc<2 || argc>3) {
     printf ("usage:  dos2out [-pd] fname[.ext]\n");
     exit (2);
  }

  while (--argc)
    switch (argv[argc][0]) {
      case '-': while (*++argv[argc])
                  switch (*argv[argc] & ~32) {
                    case 'P' : print=1; break;
                    case 'D' : delete =1; break;
                  default :
                  printf ("Bad switch %c, ignored\n",*argv[argc]);
                  }
                break;

      default : /* make filenames */
        dptr=in_name; sptr=out_name;
        while (*argv[argc] && *argv[argc]!='.') {
           *dptr++ = *argv[argc]; 
           *sptr++ = *argv[argc]++;
        }
        /* is there an extension to the filename? */
        if (*argv[argc]=='.') {
           while (*argv[argc]) *dptr++ = *argv[argc]++;
           *dptr=0;
        }
        else  {
           *dptr=0;
           strcat(dptr,".exe");
        }
        *sptr=0;
        strcat(sptr,".out");
    } /* end switch */

    /* open & create files */
    if ((inf=fopen(in_name, "rb+")) == NULL) {
       printf ("input file %s not found\n",in_name);
       exit(2);
    }


    /* get first block and check conditions for conversion */
    if ((in_cnt=fread(inbuf,1,A_HDR_LEN,inf))!=A_HDR_LEN) {
        printf ("read %d bytes, should be %d - abort\n",in_cnt,A_HDR_LEN);
        exit(2);
    }
    
    if (d_ptr->ldsign!=IS_EXE) {
        printf ("not a valid .exe file - stop\n");
        exit(2);
    }
    if (d_ptr->nr_of_relocs>1) {
        printf ("Too many relocations - stop\n");
        exit(2);
    }
    /* see documentation for explanation of this condition */
    if (d_ptr->nr_of_relocs<1) {
        printf ("Exactly one relocation item required - can't process\n");
        exit(2);
    }
    if (d_ptr->ip_reg) {
        printf ("Warning - program entry point not at zero\n");
    }


    /* input file is ok, open output (possibly destroy existing file) */
    if ((outf=fopen(out_name, "wb+")) == NULL) {
        printf ("cannot open output %s\n",out_name);
        exit(2);
    }


/*========================= PROCESS FILE =============================*/

    /* get reloc symbol's position in load-module */
    //sym  = (WORD) inbuf[d_ptr->reloc_table+1] << 8;
    //sym += (WORD) inbuf[d_ptr->reloc_table];
    //sym_blk = sym / PH_SECTSIZE;
    //sym_off = sym % PH_SECTSIZE;
    //sym_off = sym - A_HDR_LEN;
    DWORD sym_size = (d_ptr->nr_of_relocs / 4 + (d_ptr->nr_of_relocs % 4 ? 1 : 0)) * 16;
    fseek(inf, d_ptr->reloc_table, SEEK_SET);
    fread(inbuf2, 1, sym_size, inf);
    sym  = (WORD) inbuf2[1] << 8;
    sym += (WORD) inbuf2[0];
    sym_blk = sym / PH_SECTSIZE;
    sym_off = sym % PH_SECTSIZE;
    
    /* get block with relocation symbol */
    fseek(inf, d_ptr->hdr_size << 4, SEEK_SET);
    while (sym_blk>=0) {
        fread(inbuf3,1,PH_SECTSIZE,inf);
        sym_blk--;
    }

    /* get symbol and calculate sizes */
    t_size = inbuf3[sym_off+1] << 8;
    t_size+= inbuf3[sym_off];
    t_size <<= 4;
    load_size  = (d_ptr->nr_of_pages-(d_ptr->last_page_size ? 1 : 0)) * 
                  PH_SECTSIZE + d_ptr->last_page_size - (d_ptr->hdr_size << 4);
    d_size = load_size - t_size;

    /* reposition file */
    fclose (inf);
    inf=fopen(in_name, "rb+");
    fseek(inf, d_ptr->hdr_size << 4, SEEK_SET);
    //in_cnt=fread(inbuf3,1,A_HDR_LEN+sym_size,inf);
    //inf=open(in_name,O_RDONLY|O_BINARY);
    //in_cnt=read(inf,inbuf2,PH_SECTSIZE);

    /* make a.out header */
    a_ptr->a_magic[0] = MAGIC0;
    a_ptr->a_magic[1] = MAGIC1;
    a_ptr->a_flags    = A_SEP;
    a_ptr->a_cpu      = A_I8086;
    a_ptr->a_hdrlen   = A_HDR_LEN;
    a_ptr->a_syms     = 0;
    a_ptr->a_unused   = 0;
    a_ptr->a_version  = DOS;
    a_ptr->a_data     = d_size;
    a_ptr->a_text     = t_size;
    a_ptr->a_bss      = (d_ptr->min_free<<4) - d_ptr->sp_reg;
    //a_ptr->a_bss      = (d_ptr->min_free<<4);
    a_ptr->a_entry    = d_ptr->ip_reg;
    a_ptr->a_totb     = a_ptr->a_text + a_ptr->a_bss + 
                        a_ptr->a_data + d_ptr->sp_reg;
    //a_ptr->a_totb     = a_ptr->a_text + a_ptr->a_bss + 
    //                    a_ptr->a_data;

    if (print) printinfo();
    
    fwrite(a_ptr,1,A_HDR_LEN,outf);
    while(in_cnt=fread(inbuf,1,PH_SECTSIZE,inf))
      fwrite(inbuf,1,in_cnt,outf);
#if 0
    /* void remainder of first block; it holds nothing */
    /* determine nr of blocks to copy and copy them */
    block_cnt = d_ptr->nr_of_pages-((d_ptr->last_page_size)?1:0); /* exclude header-block */
    lastp_cnt = d_ptr->last_page_size;
    while (block_cnt--) {

        //if ((in_cnt=read(inf,inbuf,PH_SECTSIZE))!=PH_SECTSIZE)
        if ((in_cnt=fread(inbuf,1,PH_SECTSIZE,inf))!=PH_SECTSIZE)
           if (block_cnt || (!block_cnt && in_cnt<lastp_cnt)) {
                printf ("read %d bytes, should be %d - abort\n",
                        in_cnt,(block_cnt ? PH_SECTSIZE : lastp_cnt));
                exit(2);
        };      
        i = (!block_cnt && lastp_cnt<NOT_A_HDR_LEN ?
             lastp_cnt : NOT_A_HDR_LEN);
        dptr = &outbuf[A_HDR_LEN];
        sptr = inbuf;
        while (i--) *dptr++ = *sptr++;

        i = (!block_cnt && lastp_cnt<NOT_A_HDR_LEN ?
             lastp_cnt+A_HDR_LEN : PH_SECTSIZE);
        //if ((out_cnt=write(outf,outbuf,i)) != i) {
        if ((out_cnt=fwrite(outbuf,1,i,outf)) != i) {
                printf ("wrote %d bytes, should be %d - abort\n",out_cnt,i);
                exit(2);
        }

        i = (i<PH_SECTSIZE ? 0 :
            (!block_cnt ? lastp_cnt-NOT_A_HDR_LEN : A_HDR_LEN));
        dptr = outbuf;
        while (i--) *dptr++ = *sptr++;
    }

    /* write last block */
    if (out_cnt==PH_SECTSIZE) {
        i = lastp_cnt-NOT_A_HDR_LEN;
        //if ((out_cnt=write(outf,outbuf,i)) != i) {
        if ((out_cnt=fwrite(outbuf,1,i,outf)) != i) {
          printf("write error last block: %d, should be %d\n",out_cnt,i);
          exit(2);
        }
    }
#endif
    fclose (outf);
    fclose (inf);
    if (delete) remove (in_name);
    printf("   -done-\n");

    exit(0);
}
