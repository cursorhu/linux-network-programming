#include<stdio.h>
#include<stdlib.h>

int main()
{
	int ret;
	
	printf("ϵͳ����Ľ��̺��ǣ�%d\n",getpid());
	ret = system("ping www.hisense.com -c 2");
	printf("����ֵΪ��%d\n",ret);
	return 0;
}
