#include <sys/types.h>
#include <sys/socket.h>	/*socket()/bind()*/
#include <linux/in.h>	/*struct sockaddr_in*/
#include <string.h>		/*memset()*/
#include <stdio.h>

#define PORT_SERV 8888
int main(int argc, char *argv[])
{
	int s;		/*�׽����ļ�������*/
	struct sockaddr_in addr_serv;/*��������ַ*/
	struct sockaddr_in local;/*���ص�ַ*/
	int len = sizeof(local);/*��ַ����*/
		
	s = socket(AF_INET, SOCK_DGRAM, 0);/*�������ݱ��׽���*/
	
	/*����������ַ*/
	memset(&addr_serv, 0, sizeof(addr_serv));/*����*/
	addr_serv.sin_family = AF_INET;/*AF_INETЭ����*/
	addr_serv.sin_addr.s_addr = inet_addr("127.0.0.1");/*��ַΪ127.0.0.1*/
	addr_serv.sin_port = htons(PORT_SERV);/*�������˿�*/
	
	/*���ӷ�����*/
	connect(s, (struct sockaddr*)&addr_serv, sizeof(addr_serv));
	/*����׽����ļ��������ĵ�ַ*/	
	getsockname(s, &local, &len);	
	/*��ӡ��õĵ�ַ*/
	printf("UDP local addr:%s\n",inet_ntoa(local.sin_addr));
	
	close(s);
	return 0;	
}

