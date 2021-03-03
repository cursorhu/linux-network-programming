#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5

static void handle_request(int s, struct sockaddr_in* from, char *buff)
{
	time_t now;		/*ʱ��*/
	int len = sizeof(*from);
	memset(buff, 0, BUFFLEN);/*����*/
	now = time(NULL);/*��ǰʱ��*/
	sprintf(buff, "%24s\r\n",ctime(&now));/*��ʱ�俽���뻺����*/
	sendto(s, buff, strlen(buff),0, (struct sockaddr*)from, len);/*��������*/
}

static void handle_connect(int s_s)
{	
	struct sockaddr_in from;	/*�ͻ��˵�ַ*/
	int len = sizeof(from);
	int n = 0;
	char buff[BUFFLEN];
	/*���������*/
	while(1)
	{
		memset(buff, 0, BUFFLEN);/*����*/
		/*���տͻ�������*/
		n = recvfrom(s_s, buff, BUFFLEN,0,(struct sockaddr*)&from, &len);/*���շ��ͷ�����*/
		if(n > 0 && !strncmp(buff, "TIME", 4))/*�ж��Ƿ�Ϸ���������*/
		{
			/*�������̽������ݴ���*/
			if(!fork() ){
				handle_request(s_s, &from, buff);/*������������*/
				return ;	
			}
		}
	}		
}


int main(int argc, char *argv[])
{
	int s_s;	/*�������׽����ļ�������*/
	struct sockaddr_in local;	/*���ص�ַ*/	
	
	/*����TCP�׽���*/
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	
	/*��ʼ����ַ��Ŷ��*/
	memset(&local, 0, sizeof(local));/*����*/
	local.sin_family = AF_INET;/*AF_INETЭ����*/
	local.sin_addr.s_addr = htonl(INADDR_ANY);/*���Ȿ�ص�ַ*/
	local.sin_port = htons(SERVER_PORT);/*�������˿�*/
	
	/*���׽����ļ��������󶨵����ص�ַ�Ͷ˿�*/
	int err = bind(s_s, (struct sockaddr*)&local, sizeof(local));
		
	/*����ͻ�������*/
	handle_connect(s_s);
	
	close(s_s);
	
	return 0;		
}
