extern void bios_putc(int c);
extern int bios_getc(void);

void putc(char c)
{
  if(c == '\n')
    bios_putc('\r');

  bios_putc(c);
}

void puts(const char *s)
{
  int i;
  for(i = 0; s[i]; i++) {
    putc(s[i]);
  }
}

int getc(void)
{
  char c;
  if ((c = bios_getc()) == '\r') {
    c = '\n';
  }
  putc(c);
	return c;
}

int cmain()
{
  char c;
  for (;;) {
    puts("\n\n\n\n");
    puts("\nHit key as follows:\n\n");
    puts("    =  start MINIX (root file system in /dev/fd1.)\n");
    puts("\n# ");
    
    c = getc();
    switch (c) {
      case '=' :
        return c;
      default:
        puts("Illegal command\n");
        continue;
    }
  }
}
