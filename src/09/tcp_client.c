#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/in.h>
#include <signal.h>

extern void sig_proccess(int signo);
extern void sig_pipe(int signo);
static int s;
void sig_proccess_client(int signo)
{
	printf("Catch a exit signal\n");
	close(s);
	exit(0);	
}

#define PORT 8888	/* �����˿ڵ�ַ */
int main(int argc, char *argv[])
{

	struct sockaddr_in server_addr;	/* ��������ַ�ṹ */
	int err;/* ����ֵ */
		
	if(argc == 1){
		printf("PLS input server addr\n");
		return 0;	
	}
	signal(SIGINT, sig_proccess);
	signal(SIGPIPE, sig_pipe);
	
	/* ����һ����ʽ�׽��� */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0){/* ���� */
		printf("socket error\n");
		return -1;	
	}	
	
	/* ���÷�������ַ */
	bzero(&server_addr, sizeof(server_addr));		/* ��0 */
	server_addr.sin_family = AF_INET;				/* Э���� */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);/* ���ص�ַ */
	server_addr.sin_port = htons(PORT);				/* �������˿� */
	
	/* ���û�������ַ������͵�IP��ַתΪ���� */
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);	
	/* ���ӷ����� */
	connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	process_conn_client(s);	/* �ͻ��˴������ */
	close(s);	/* �ر����� */
}

