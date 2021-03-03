#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
 #include <sys/select.h>
#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
#define CLIENTNUM 1024/*���֧�ֿͻ�������*/

/*�����ӿͻ��˵��ļ�����������*/
int connect_host[CLIENTNUM];
int connect_number =  0;
static void *handle_request(void *argv)
{	
	time_t now;		/*ʱ��*/
	char buff[BUFFLEN];/*�շ����ݻ�����*/
	int n = 0;
	
	int maxfd = -1;/*��������ļ�������*/
	fd_set scanfd;	/*��������������*/
	struct   timeval   timeout; 	/*��ʱ*/
	timeout.tv_sec     =   1; /*   ����1���ʱ����   */     
	timeout.tv_usec   =   0;     
    
	int i = 0;
	int err  = -1;
	for(;;)
	{   
		/*����ļ�������ֵ��ʼ��Ϊ-1*/		
		maxfd = -1;
		FD_ZERO(&scanfd);/*�����ļ�����������*/
		for(i=0;i<CLIENTNUM;i++)/*���ļ����������뼯��*/
		{
			if(connect_host[i] != -1)/*�Ϸ����ļ�������*/
			{
				FD_SET(connect_host[i], &scanfd);/*���뼯��*/
				if(maxfd < 	connect_host[i])/*��������ļ�������ֵ*/
				{
					maxfd = connect_host[i];
				}
			}
		}
		/*select�ȴ�*/
		err = select(maxfd + 1, &scanfd, NULL, NULL, &timeout) ;		
		switch(err)
		{
			case 0:/*��ʱ*/
				break;
			case -1:/*������*/
				break;
			default:/*�пɶ��׽����ļ�������*/
				if(connect_number<=0)
					break;
				for(i = 0;i<CLIENTNUM;i++)
				{
					/*���Ҽ�����ļ�������*/
					if(connect_host[i] != -1)
					if(FD_ISSET(connect_host[i],&scanfd))   
					{  
						memset(buff, 0, BUFFLEN);/*����*/
						n = recv(connect_host[i], buff, BUFFLEN,0);/*���շ��ͷ�����*/
						if(n > 0 && !strncmp(buff, "TIME", 4))/*�ж��Ƿ�Ϸ���������*/
						{
							memset(buff, 0, BUFFLEN);/*����*/
							now = time(NULL);/*��ǰʱ��*/
							sprintf(buff, "%24s\r\n",ctime(&now));/*��ʱ�俽���뻺����*/
							send(connect_host[i], buff, strlen(buff),0);/*��������*/
						}
						/*�����ļ��������������е�ֵ*/
						connect_host[i] = -1;
						connect_number --;	/*�ͻ��˼�������1*/	
						/*�رտͻ���*/
						close(connect_host[i]);							
					} 
				}
				break; 	
		}		  
	} 
	
	return NULL;
}

static void *handle_connect(void *argv)
{	
	int s_s = *((int*)argv) ;/*��÷����������׽����ļ�������*/
	int s_c = -1;/*���ӿͻ����ļ�������*/
	struct sockaddr_in from;
	int len = sizeof(from);
	/*���տͻ�������*/
	for(;;)
	{
		int i = 0;
		int s_c = accept(s_s, (struct sockaddr*)&from, &len);/*���տͻ��˵�����*/
		printf("a client connect, from:%s\n",inet_ntoa(from.sin_addr));
		/*���Һ���λ�ã����ͻ��˵��ļ�����������*/				
		for(i=0;i<CLIENTNUM;i++)
		{
			if(connect_host[i] == -1)/*�ҵ�*/
			{
				/*����*/
				connect_host[i]= s_c;
				
				/*�ͻ��˼�������1*/
				connect_number ++;
				/*������ѯ�ȴ��ͻ�������*/
				break;						
			}	
		}		
	}	
	return NULL;
}

int main(int argc, char *argv[])
{
	int s_s;	/*�������׽����ļ�������*/
	struct sockaddr_in local;	/*���ص�ַ*/	
	int i = 0;
	memset(connect_host, -1, CLIENTNUM);
	
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
	
	pthread_t  thread_do[2];/*�߳�ID*/
	/*�����̴߳���ͻ�������*/
	pthread_create(&thread_do[0],/*�߳�ID*/
					NULL,/*����*/
					handle_connect,/*�̻߳ص�����*/
					(void*)&s_s);		/*�̲߳���*/
	/*�����̴߳���ͻ�������*/					
	pthread_create(&thread_do[1],/*�߳�ID*/
					NULL,/*����*/
					handle_request,/*�̻߳ص�����*/
					NULL);		/*�̲߳���*/
	/*�ȴ��߳̽���*/
	for(i=0;i<2;i++)
		pthread_join(thread_do[i], NULL);
	
	close(s_s);
	
	return 0;		
}
