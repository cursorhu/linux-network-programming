#include <sys/types.h>
#include <unistd.h>
#include<stdio.h>

int main()
{
	pid_t pid,ppid;
	
	/* ��õ�ǰ���̺��丸���̵�ID�� */
	pid = getpid();
	ppid = getppid();
	
	printf("��ǰ���̵�ID��Ϊ��%d\n",pid);
	printf("��ǰ���̵ĵĸ����̺�ID��Ϊ��%d\n",ppid);
	
	return 0;
}
