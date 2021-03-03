#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_LEN 1024	/*����������*/
#define MYPORT 8888		/*�����������˿�*/
#define BACKLOG 10		/*��������������*/
int main(int argc, char *argv[])
{
	int s_c;	/*�ͻ����׽����ļ�������*/
	socklen_t len;
	int err = -1;
	
	struct sockaddr_in6 server_addr;	/*���ص�ַ�ṹ*/
	struct sockaddr_in6 client_addr;/*�ͻ��˵�ַ�ṹ*/
	char buf[BUF_LEN + 1];	/*�շ�������*/

    
    s_c = socket(PF_INET6, SOCK_STREAM, 0);/*����IPv6�׽���*/
    if (s_c == -1) { /*�жϴ���*/
        perror("socket error");
        return(1);
    } else{
        printf("socket() success\n");
	}
	
    bzero(&server_addr, sizeof(server_addr));/*���ַ�ṹ*/
    server_addr.sin6_family = PF_INET6;    /*Э����*/
    server_addr.sin6_port = htons(MYPORT); /*Э��˿�*/
    server_addr.sin6_addr = in6addr_loopback;   /*IPv6�����ַ*/

	/*���ӷ�����*/
	err = connect(s_c, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (err == -1) {/*�жϴ���*/
        perror("connect error");
        return (1);
    } else{
        printf("connect() success\n");
	}
	
	/*���㻺����*/
	bzero(buf, BUF_LEN + 1);
	len = recv(s_c, buf, BUF_LEN, 0);/*���շ���������*/
	/*��ӡ��Ϣ*/
	printf("RECVED %dbytes:%s\n",len,buf);
	bzero(buf, BUF_LEN + 1);/*���㻺����*/
    strcpy(buf,"From Client");/*�����ͻ��˵���Ϣ*/
    len = send(s_c, buf, strlen(buf), 0);/*���������������*/
    close(s_c);/*�ر��׽���*/
    
	
    return 0;
}
