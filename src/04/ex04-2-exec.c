#include<stdio.h>
#include<unistd.h>

int main(void)
{
	char *args[]={"/bin/ls",NULL};
	printf("ϵͳ����Ľ��̺��ǣ�%d\n",getpid());
	if(execve("/bin/ls",args,NULL)<0)
		printf("�������̳���\n");
		
	return 0;
}
