#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/in.h>
#include <signal.h>

extern void sig_proccess(int signo);

#define PORT 8888		/* �����˿ڵ�ַ */
#define BACKLOG 2		/* �������г��� */
int main(int argc, char *argv[])
{
	int ss,sc;		/* ssΪ��������socket������,scΪ�ͻ��˵�socket������ */
	struct sockaddr_in server_addr; /* ��������ַ�ṹ */
	struct sockaddr_in client_addr;	/* �ͻ��˵�ַ�ṹ */
	int err;	/* ����ֵ */
	pid_t pid;	/* �ֲ�Ľ���id */
	
	signal(SIGINT, sig_proccess);
	signal(SIGPIPE, sig_proccess);
	
	
	/* ����һ����ʽ�׽��� */
	ss = socket(AF_INET, SOCK_STREAM, 0);
	if(ss < 0){/* ���� */
		printf("socket error\n");
		return -1;	
	}	
	
	/* ���÷�������ַ */
	bzero(&server_addr, sizeof(server_addr));	/* ��0 */
	server_addr.sin_family = AF_INET;			/* Э���� */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/* ���ص�ַ */
	server_addr.sin_port = htons(PORT);			/* �������˿� */
	
	/* �󶨵�ַ�ṹ���׽��������� */
	err = bind(ss, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(err < 0){/* ���� */
		printf("bind error\n");
		return -1;	
	}
	
	/* �������� */
	err = listen(ss, BACKLOG);
	if(err < 0){/* ���� */
		printf("listen error\n");
		return -1;	
	}
	
	/* ��ѭ������ */
	for(;;)	{
		int addrlen = sizeof(struct sockaddr);
		/* ���տͻ������� */
		sc = accept(ss, (struct sockaddr*)&client_addr, &addrlen);
		if(sc < 0){		/* ���� */
			continue;	/* ��������ѭ�� */
		}	
		
		/* ����һ���µĽ��̴����������� */
		pid = fork();		/* �ֲ���� */
		if( pid == 0 ){		/* �ӽ����� */
			close(ss);		/* ���ӽ����йرշ����������� */
			process_conn_server(sc);/* �������� */
		}else{
			close(sc);		/* �ڸ������йرտͻ��˵����� */
		}
	}
}
