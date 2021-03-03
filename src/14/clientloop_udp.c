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
	struct sockaddr_in server;	/*���ص�ַ*/
	time_t now;		/*ʱ��*/
	char buff[BUFFLEN];/*�շ����ݻ�����*/
	int n = 0;	/*�����ַ�������*/
	int len = 0;	/*��ַ����*/
	
	/*����UDP�׽���*/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	
	/*��ʼ����ַ��Ŷ��*/
	memset(&server, 0, sizeof(server));/*����*/
	server.sin_family = AF_INET;/*AF_INETЭ����*/
	server.sin_addr.s_addr = htonl(INADDR_ANY);/*���Ȿ�ص�ַ*/
	server.sin_port = htons(SERVER_PORT);/*�������˿�*/	
	
	memset(buff, 0, BUFFLEN);/*����*/
	strcpy(buff, "TIME");/*���������ַ���*/
	/*��������*/
	sendto(s, buff, strlen(buff), 0, (struct sockaddr*)&server, sizeof(server));
	memset(buff, 0, BUFFLEN);/*����*/
	/*��������*/
	len = sizeof(server);
	n = recvfrom(s, buff, BUFFLEN, 0, (struct sockaddr*)&server, &len);
	/*��ӡ��Ϣ*/
	if(n >0){
		printf("TIME:%s",buff);	
	}
	close(s);
	
	return 0;		
}
