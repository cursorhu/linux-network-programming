#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

/* �������Կͻ��˵Ĵ��� */
void process_conn_server(int s)
{
	ssize_t size = 0;
	char buffer[1024];	/* ���ݵĻ����� */
	
	for(;;){/* ѭ��������� */
		/* ���׽����ж�ȡ���ݷŵ�������buffer�� */
		size = read(s, buffer, 1024);	
		if(size == 0){/* û������ */
			return;	
		}
		
		/* ������Ӧ�ַ���Ϊ���յ��ͻ����ֽڵ����� */
		sprintf(buffer, "%d bytes altogether\n", size);
		write(s, buffer, strlen(buffer)+1);/* �����ͻ��� */
	}	
}

/* �ͻ��˵Ĵ������ */
void process_conn_client(int s)
{
	ssize_t size = 0;
	char buffer[1024];	/* ���ݵĻ����� */
	
	for(;;){/* ѭ��������� */
		/* �ӱ�׼�����ж�ȡ���ݷŵ�������buffer�� */
		size = read(0, buffer, 1024);
		if(size > 0){/* �������� */
			write(s, buffer, size);		/* ���͸������� */
			size = read(s, buffer, 1024);/* �ӷ�������ȡ���� */
			write(1, buffer, size);		/* д����׼��� */
		}
	}	
}

void sig_proccess(int signo)
{
	printf("Catch a exit signal\n");
	exit(0);	
}

void sig_pipe(int sign)
{
	printf("Catch a SIGPIPE signal\n");
	
	/* �ͷ���Դ */	
}