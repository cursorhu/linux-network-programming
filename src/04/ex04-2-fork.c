#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
	pid_t pid;
	
	/* �ֲ���� */
	pid = fork();
	
	/* �ж��Ƿ�ִ�гɹ� */
	if(-1 == pid){
		printf("���̴���ʧ�ܣ�\n");
		return -1;
	} else if(pid == 0){
		/* �ӽ�����ִ�д˶δ��� */
		printf("�ӽ��̣�fork����ֵ��%d, ID:%d, ������ID:%d\n",pid, getpid(), getppid());
	}	else{
		/* ��������ִ�д˶δ��� */
		printf("�����̣�fork����ֵ��%d, ID:%d, ������ID:%d\n",pid, getpid(), getppid());
	}
	
	return 0;
}

