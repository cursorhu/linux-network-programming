#include <sys/types.h>
#include <sys/socket.h>	/*socket()/bind()*/
#include <linux/in.h>	/*struct sockaddr_in*/
#include <string.h>		/*memset()*/

#define PORT_SERV 8888		/*�������˿�*/
#define NUM_DATA 100		/*���ջ���������*/
#define LENGTH 1024			/*�������ջ�������С*/
static char buff[NUM_DATA][LENGTH]; /*���ջ�����*/
static udpserv_echo(int s, struct sockaddr *client)
{
	int n;	/*��������*/
	char tmp_buff[LENGTH];	/*��ʱ������*/
	int len;	/*��ַ����*/
	while(1)/*���չ���*/
	{
		len = sizeof(*client);/*��ַ����*/
		n = recvfrom(s, tmp_buff, LENGTH, 0, client, &len);/*�������ݷŵ���ʱ��������*/
		/*���ݽ��յ����ݵ�ͷ����־��ѡ����ʵĻ�����λ�ÿ�������*/
		memcpy(&buff[ntohl(*((int*)&buff[i][0]))][0], tmp_buff+4, n-4); 
	}	
}

int main(int argc, char *argv[])
{
	int s;
	struct sockaddr_in addr_serv,addr_clie;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_serv.sin_port = htons(PORT_SERV);
	
	bind(s, (struct sockaddr*)&addr_serv, sizeof(addr_serv));
	udpserv_echo(s, (struct sockaddr*)&addr_clie);
	
	return 0;	
}
