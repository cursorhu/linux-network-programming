/* main.c */
#include <stdio.h>
#include "add.h"
#include "sub.h"

int main(void)
{
	int a = 1, b = 1;
	
	printf("int a+b is:%d\n", add_int(a,b));
	printf("int a-b is:%d\n", sub_int(a,b));
	
	return 0;	
}

