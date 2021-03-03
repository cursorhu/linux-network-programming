#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_LEN 1024	/*����������*/
#define MYPORT 8888		/*�����������˿�*/
#define BACKLOG 10		/*��������������*/
int main(int argc, char **argv)
{
	int s_s;	/*���������׽����ļ�������*/
	int s_c;	/*�ͻ����׽����ļ�������*/
	socklen_t len;
	int err = -1;
	
	struct sockaddr_in6 local_addr;	/*���ص�ַ�ṹ*/
	struct sockaddr_in6 client_addr;/*�ͻ��˵�ַ�ṹ*/
	char buf[BUF_LEN + 1];

    
    s_s = socket(PF_INET6, SOCK_STREAM, 0);/*����IPv6�׽���*/
    if (s_s == -1) { /*�жϴ���*/
        perror("socket error");
        return(1);
    } else{
        printf("socket() success\n");
	}
	
    bzero(&local_addr, sizeof(local_addr));/*���ַ�ṹ*/
    local_addr.sin6_family = PF_INET6;    /*Э����*/
    local_addr.sin6_port = htons(MYPORT); /*Э��˿�*/
    local_addr.sin6_addr = in6addr_any;   /*IPv6�����ַ*/

	err = bind(s_s, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in6));
	if (err == -1) {/*�жϴ���*/
        perror("bind error");
        return (1);
    } else{
        printf("bind() success\n");
	}
	
	err = listen(s_s, BACKLOG);/*������������*/
    if (err == -1) {/*�жϴ���*/
        perror("listen error");
        exit(1);
    } else{
        printf("listen() success\n");
	}
	
    while (1) 
    {
        len = sizeof(struct sockaddr);/*��ַ����*/
        /*�ȴ��ͻ�������*/
        s_c = accept(s_s, (struct sockaddr *)&client_addr, &len);
        if (s_c == -1) {/*�жϴ���*/
            perror("accept error");
            return (errno);
        } else{
        	/*���ͻ��˵ĵ�ַת�����ַ���*/
        	inet_ntop(AF_INET6, &client_addr.sin6_addr, buf, sizeof(buf));
            printf("a client from ip: %s, port %d, socket %d\n",
                   buf, /*�ͻ��˵�ַ*/
                   client_addr.sin6_port,/*�ͻ��˶˿�*/ 
                   s_c); /*�ͻ����׽����ļ�������*/
		}
		
        /* ��ʼ����ÿ���������ϵ������շ� */
        bzero(buf, BUF_LEN + 1);/*���㻺����*/
        strcpy(buf,"From Server");/*��������*/
        /* ����Ϣ���ͻ��� */
        len = send(s_c, buf, strlen(buf), 0);
        if (len < 0) {/*������Ϣ*/
            printf("message '%s' send error,errno:%d,'%s'\n",
                 buf, errno, strerror(errno));
        } else{/*�ɹ���Ϣ*/
            printf("message '%s' send success, %dbytes\n",buf, len);
		}
		/*���㻺����*/
        bzero(buf, BUF_LEN + 1);
        /* ���տͻ��˵���Ϣ */
        len = recv(s_c, buf, BUF_LEN, 0);
        if (len > 0)
            printf("recv message success:'%s',%dbytes\n",
                   buf, len);
        else
            printf
                ("recv message failure, errno: %d��'%s'\n",
                 errno, strerror(errno));
        /* ����ÿ���������ϵ������շ����� */
        close(s_c);
    }

    close(s_s);
    return 0;
}
