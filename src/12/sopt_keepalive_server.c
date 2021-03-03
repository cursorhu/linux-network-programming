#include<stdio.h> 
#include<stdlib.h> 
#include<sys/socket.h> 
#include<error.h> 
#include<string.h> 
#include<sys/types.h> 
#include<netinet/in.h> 
#include<sys/wait.h> 
#include<arpa/inet.h> 
#include<unistd.h> 

/* ���ڴ���SIGPIP��SIGINT�źŵĺ��� */
static int sigpipe(int signo)
{
	alive = 0;
}

#define PORT 8888 		/* �����������˿�Ϊ8888 */
#define BACKLOG 8 		/* ��������Ŷ�����Ϊ8 */
static int alive = 1;	/* �Ƿ��˳� */
int main(int argc, char *argv[])
{
	/* sΪ�������������׽�����������scΪ�ͻ������ӳɹ����ص������� */
	int s, sc; 
	/* local_addr���ص�ַ��client_addr�ͻ��˵ĵ�ַ */
	struct sockaddr_in local_addr,client_addr; 
	int err = -1; 	/* ���󷵻�ֵ */
	socket_t optlen = -1;	/* ���͵�ѡ������ֵ */
	int optval = -1;		/* ѡ������ֵ���� */
	
	/* ��ȡSIGPIPE��SIGINT�ɺ���signo���� */
	signal(SIGPIPE, signo);
	signal(SIGINT,signo);
	
	/* �������ؼ����׽��� */
	s = socket(AF_INET,SOCK_STREAM,0);
	if( s == -1){ 
		printf("�׽��ִ���ʧ��!\n"); 
		return -1;
	} 
	
	/* ���õ�ַ�Ͷ˿����� */
	optval = 1; /* ������Ч */
	optlen = sizeof(optval);	
	err=setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(char *)&optval, optlen); 
	if(err!= -1){ /* ����ʧ�� */
		printf("�׽��ֿ���������ʧ��!\n"); 
		return -1; 
	}
	
	/* ��ʼ������Э���壬�˿ں�IP��ַ */
	bzero(&local_addr, 0, sizeof(local_addr));	/* ���� */
	local_addr.sin_family=AF_INET; 				/* Э���� */
	local_addr.sin_port=htons(PORT); 			/* �˿� */
	local_addr.sin_addr.s_addr=INADDR_ANY; 		/* ���Ȿ�ص�ַ */
	
	/* ���׽��� */
	err = bind(s, (struct sockaddr *)&local_addr, sizeof(struct sockaddr);
	if(err == -1){ /* ��ʧ�� */
		printf("��ʧ��!\n"); 
		return -1;
	} 
	
	/* ���������ջ�����������ͻ����� */
	optval = 128*1024; /* ��������СΪ128K */
	optlen = sizeof(optval);
	err = setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, optlen);
	if(err == -1){/* ���ý��ջ�������Сʧ�� */
		printf("���ý��ջ�����ʧ��\n");			
	}
	err = setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, optlen);
	if(err == -1){/* ���÷��ͻ�������Сʧ�� */
		printf("���÷��ͻ�����ʧ��\n");			
	}
	
	/* ���÷��ͺͽ��ճ�ʱʱ�� */
	struct timeval tv;
	tv.tv_sec = 1;	/* 1�� */
	tv.tv_usec = 200000;/* 200ms */
	optlen = sizeof(tv);
	err = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, optlen); /* ���ý��ճ�ʱʱ�� */
	if(err == -1){/* ���ý��ճ�ʱʱ��ʧ�� */
		printf("���ý��ճ�ʱʱ��ʧ��\n");			
	}
	err = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen);/* ���÷��ͳ�ʱʱ�� */
	if(err == -1){
		printf("���÷��ͳ�ʱʱ��ʧ��\n");			
	}
	
		
	/* ���ü��� */
	err = listen(s,BACKLOG);
	if( err ==-1){ /* ���ü���ʧ�� */
		printf("���ü���ʧ��!\n"); 
		return -1; 
	} 
	
	printf("�ȴ�����...\n"); 	
	fd_set fd_r;			/* ���ļ��������� */
	struct timeval tv;		
	tv.tv_usec = 200000;	/* ��ʱʱ��Ϊ200ms */
	tv.tv_sec = 0;	
	while(alive){ 
		//����������ʱ�������� 
		socklen_t sin_size=sizeof(struct sockaddr_in); 
		
		/* �˴�ÿ�λ���ѯ�Ƿ��пͻ������ӵ��������ʱ��Ϊ200ms */
		FD_ZERO(&fd_r);		/* ����ļ��������� */
		FD_SET(s, &fd_r);	/* ���������������� */
		switch (select(s + 1, &fd_r, NULL, &tv)) {  /* �����ļ���������fd_r */
                case -1:  	/* ������ */                        
                case 0: 	/* ��ʱ */
                	continue;  
                	break;
                default: 	/* �����ӵ��� */
                	break;                         
        }  
		/* �����ӵ���������... */
		sc = accept(s, (struct sockaddr *)&client_addr,&sin_size);
		if( sc ==-1){ /* ʧ�� */
			perror("��������ʧ��!\n"); 
			continue; 
		} 
	
		/* ��������̽�ⳬʱʱ�� */
		optval = 10;			/* 10�� */
		optlen = sizeof(optval);/**/
		err = setsockopt(sc, IPPROTO_TCP, SO_KEEPALIVE, (char*)&optval, optlen);/* ����... */
		if( err == -1){/* ʧ�� */
			printf("��������̽����ʱ��ʧ��\n");	
		}
		
		/* ���ý�ֹNagle�㷨 */
		optval = 1;					/* ��ֹ */
		optlen = sizeof(optval);
		err = setsockopt(sc, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, optlen);/* ����... */
		if( err == -1){/* ʧ�� */
			printf("��ֹNagle�㷨ʧ��\n");	
		}
		
		/* ���������ӳٹر�Ϊ�����ر� */
		struct linger;
		linger.l_onoff = 1;		/* �ӳٹر���Ч */
		linger.l_linger = 0;	/* �����ر� */
		optlen = sizeof(linger);
		err = setsockopt(sc, SOL_SOCKET, SO_LINGER, (char*)&linger, optlen);/* ����... */
		if( err == -1){/* ʧ�� */
			printf("���������ر�ʧ��\n");	
		}	
		
		/* ��ӡ�ͻ���IP��ַ��Ϣ */
		printf("�ӵ�һ������%s������\n",inet_ntoa(client_addr.sin_addr)); 	
		err = send(sc,"���ӳɹ�!\n",10,0);
		if(err == -1){
			printf("����֪ͨ��Ϣʧ��!\n"); 			 
		} 
		
		/*  �رտͻ������� */
		close(sc);
		
	} 
	/* �رշ������� */
	close(s); 
	
	return 0; 
} 
