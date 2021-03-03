#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
int main(int argc, char *argv[])
{
	int s_s, s_c;	/*�������׽����ļ�������*/
	struct sockaddr_in local, from;	/*���ص�ַ*/
	time_t now;		/*ʱ��*/
	char buff[BUFFLEN];/*�շ����ݻ�����*/
	int n = 0;
	int len = sizeof(from);
	
	/*����TCP�׽���*/
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	
	/*��ʼ����ַ��Ŷ��*/
	memset(&local, 0, sizeof(local));/*����*/
	local.sin_family = AF_INET;/*AF_INETЭ����*/
	local.sin_addr.s_addr = htonl(INADDR_ANY);/*���Ȿ�ص�ַ*/
	local.sin_port = htons(SERVER_PORT);/*�������˿�*/
	
	/*���׽����ļ��������󶨵����ص�ַ�Ͷ˿�*/
	int err = bind(s_s, (struct sockaddr*)&local, sizeof(local));
	err = listen(s_s, BACKLOG);/*����*/
	
	/*���������*/
	while(1)
	{
		/*���տͻ�������*/
		s_c = accept(s_s, (struct sockaddr*)&from, &len);
		memset(buff, 0, BUFFLEN);/*����*/
		n = recv(s_c, buff, BUFFLEN,0);/*���շ��ͷ�����*/
		if(n > 0 && !strncmp(buff, "TIME", 4))/*�ж��Ƿ�Ϸ���������*/
		{
			memset(buff, 0, BUFFLEN);/*����*/
			now = time(NULL);/*��ǰʱ��*/
			sprintf(buff, "%24s\r\n",ctime(&now));/*��ʱ�俽���뻺����*/
			send(s_c, buff, strlen(buff),0);/*��������*/
		}
		close(s_c);
	}
	close(s_s);
	
	return 0;		
}
