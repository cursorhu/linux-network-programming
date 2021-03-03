#include <sys/types.h>
#include <sys/socket.h>	/*socket()/bind()*/
#include <linux/in.h>	/*struct sockaddr_in*/
#include <string.h>		/*memset()*/
#include <stdio.h>

#define PORT_SERV 8888
#define BUFF_LEN 256


#define PORT_SERV 8888		/*�������˿�*/
#define NUM_DATA 100		/*���ջ���������*/
#define LENGTH 1024			/*�������ջ�������С*/
static char buff_send[LENGTH]; /*���ջ�����*/
static void udpclie_echo(int s, struct sockaddr *to)
{
	char buff_init[BUFF_LEN] = "UDP TEST";	/*��������˷��͵�����*/
	struct sockaddr_in from;/*�������ݵ�������ַ*/
	int len = sizeof(*to);/*��ַ����*/
	int i = 0;/*����*/
	for(i = 0; i< NUM_DATA; i++)/*ѭ������*/
	{
		*((int*)&buff_send[0]) = htonl(i);/*�����ݱ�Ǵ��*/
		memcpy(&buff_send[4],buff_init, sizeof(buff_init));/*���ݿ��뷢�ͻ�����*/
		sendto(s, &buff_send[0], NUM_DATA, 0, to, len);/*��������*/
	}
}
int main(int argc, char *argv[])
{
	int s;
	struct sockaddr_in addr_serv;	
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_serv.sin_port = htons(PORT_SERV);
	
	udpclie_echo(s, (struct sockaddr*)&addr_serv);
	
	close(s);
	return 0;	
}

