#ifdef __WATCOMC__
#pragma aux (cdecl) main;
#endif

#include <stdio.h>

int main(void)
{
  int i;
  double f1, f2;
  scanf("%lf %lf", &f1, &f2);
  i = (int) (f1 + f2);
  printf("%.2f\n", (f1 + f2));
  printf("%d\n", i);
  printf("hello, world!\n");
  return 0;
}
