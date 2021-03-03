#include <stdio.h>
extern int StrLen(char* str);
int main(void)
{
  char src[]="Hello Dymatic";
  printf("string length is:%d\n",StrLen(src));
  return 0;
}
