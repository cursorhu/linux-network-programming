#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#define BUFFLEN 1024
#define SERVER_PORT 8888
int main(int argc, char *argv[])
{
	int s;	/*�������׽����ļ�������*/
	struct sockaddr_in local, to;	/*���ص�ַ*/
	time_t now;		/*ʱ��*/
	char buff[BUFFLEN];/*�շ����ݻ�����*/
	int n = 0;
	int len = sizeof(to);
	
	/*����UDP�׽���*/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	
	/*��ʼ����ַ��Ŷ��*/
	memset(&local, 0, sizeof(local));/*����*/
	local.sin_family = AF_INET;/*AF_INETЭ����*/
	local.sin_addr.s_addr = htonl(INADDR_ANY);/*���Ȿ�ص�ַ*/
	local.sin_port = htons(SERVER_PORT);/*�������˿�*/
	
	/*���׽����ļ��������󶨵����ص�ַ�Ͷ˿�*/
	int err = bind(s, (struct sockaddr*)&local, sizeof(local));
	/*���������*/
	while(1)
	{
		memset(buff, 0, BUFFLEN);/*����*/
		n = recvfrom(s, buff, BUFFLEN,0,(struct sockaddr*)&to, &len);/*���շ��ͷ�����*/
		if(n > 0 && !strncmp(buff, "TIME", 4))/*�ж��Ƿ�Ϸ���������*/
		{
			memset(buff, 0, BUFFLEN);/*����*/
			now = time(NULL);/*��ǰʱ��*/
			sprintf(buff, "%24s\r\n",ctime(&now));/*��ʱ�俽���뻺����*/
			sendto(s, buff, strlen(buff),0, (struct sockaddr*)&to, len);/*��������*/
		}
	}
	close(s);
	
	return 0;		
}
