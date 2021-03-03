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
	char buff[BUFF_LEN] = "UDP TEST";	/*��������˷��͵�����*/
	connect(s, to, sizeof(*to));/*����*/
	
	n = write(s, buff, BUFF_LEN);/*��������*/
	
	read(s, buff, n);/*��������*/
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

