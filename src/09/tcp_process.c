#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define RECVMSGSENDMSG

#ifdef RECVSEND


/* �������Կͻ��˵Ĵ��� */
void process_conn_server(int s)
{
	ssize_t size = 0;
	char buffer[1024];	/* ���ݵĻ����� */
	
	for(;;){/* ѭ��������� */
		/* ���׽����ж�ȡ���ݷŵ�������buffer�� */
		size = recv(s, buffer, 1024,0);	
		if(size == 0){/* û������ */
			return;	
		}
		
		/* ������Ӧ�ַ���Ϊ���յ��ͻ����ֽڵ����� */
		sprintf(buffer, "%d bytes altogether\n", size);
		send(s, buffer, strlen(buffer)+1,0);/* �����ͻ��� */
	}	
}

/* �ͻ��˵Ĵ������ */
void process_conn_client(int s)
{
	ssize_t size = 0;
	char buffer[1024];	/* ���ݵĻ����� */
	
	for(;;){/* ѭ��������� */
		/* �ӱ�׼�����ж�ȡ���ݷŵ�������buffer�� */
		size = read(0, buffer, 1024);
		if(size > 0){/* �������� */
			send(s, buffer, size,0);		/* ���͸������� */
			size = recv(s, buffer, 1024,0);/* �ӷ�������ȡ���� */
			write(1, buffer, size);		/* д����׼��� */
		}
	}	
}

void sig_proccess(int signo)
{
	printf("Catch a exit signal\n");
	_exit(0);	
}

void sig_pipe(int sign)
{
	printf("Catch a SIGPIPE signal\n");
	
	/* �ͷ���Դ */	
}
#endif

#ifdef READVWRITEV
#include <sys/uio.h>
#include <string.h>
#include <stdlib.h>

static struct iovec *vs=NULL,*vc=NULL;
/* �������Կͻ��˵Ĵ��� */
void process_conn_server(int s)
{
	char buffer[30];	/* �����Ļ����� */
	ssize_t size = 0;
	/* ����3������ */
	struct iovec *v = (struct iovec*)malloc(3*sizeof(struct iovec));
	if(!v){
		printf("Not enough memory\n");
		return;		
	}
	
	/**�ҽ�ȫ�ֱ����������ͷŹ���*/
	vs = v;
	
	/* ÿ������10���ֽڵĿռ� */
	v[0].iov_base = buffer;	/*0-9*/
	v[1].iov_base = buffer + 10;/*10-19*/
	v[2].iov_base = buffer + 20;/*20-29*/
	/*��ʼ������Ϊ10*/
	v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
	
	
	
	for(;;){/* ѭ��������� */
		/* ���׽����ж�ȡ���ݷŵ������������� */
		size = readv(s, v, 3);	
		if(size == 0){/* û������ */
			return;	
		}
		
		/* ������Ӧ�ַ���Ϊ���յ��ͻ����ֽڵ��������ֱ�ŵ������������� */
		sprintf(v[0].iov_base, "%d ", size); /*����*/
		sprintf(v[1].iov_base, "bytes alt"); /*��bytes alt���ַ���*/
		sprintf(v[2].iov_base, "ogether\n"); /*ogether\n���ַ���*/
		/*д���ַ�������*/
		v[0].iov_len = strlen(v[0].iov_base);
		v[1].iov_len = strlen(v[1].iov_base);
		v[2].iov_len = strlen(v[2].iov_base);
		writev(s, v, 3);/* �����ͻ��� */
	}	
}

/* �ͻ��˵Ĵ������ */
void process_conn_client(int s)
{
	char buffer[30];/* �����Ļ����� */
	ssize_t size = 0;
	/* ����3������ */
	struct iovec *v = (struct iovec*)malloc(3*sizeof(struct iovec));
	if(!v){
		printf("Not enough memory\n");
		return;		
	}
	/**�ҽ�ȫ�ֱ����������ͷŹ���*/
	vc = v;
	/* ÿ������10���ֽڵĿռ� */
	v[0].iov_base = buffer;/*0-9*/
	v[1].iov_base = buffer + 10;/*10-19*/
	v[2].iov_base = buffer + 20;/*20-29*/
	/*��ʼ������Ϊ10*/
	v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
	
	
	
	int i = 0;
	
	for(;;){/* ѭ��������� */
		/* �ӱ�׼�����ж�ȡ���ݷŵ�������buffer�� */
		size = read(0, v[0].iov_base, 10);
		if(size > 0){/* �������� */
			v[0].iov_len= size;
			writev(s, v,1);		/* ���͸������� */
			v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
			size = readv(s, v, 3);/* �ӷ�������ȡ���� */
			for(i = 0;i<3;i++){
				if(v[i].iov_len > 0){
					write(1, v[i].iov_base, v[i].iov_len);		/* д����׼��� */
				}
			}
		}
	}	
}

void sig_proccess(int signo)
{
	printf("Catch a exit signal\n");
	/* �ͷ���Դ */	
	free(vc);
	free(vs);
	_exit(0);	
}

void sig_pipe(int sign)
{
	printf("Catch a SIGPIPE signal\n");
	
	/* �ͷ���Դ */	
	free(vc);
	free(vs);
	_exit(0);
}
#endif
#ifdef RECVMSGSENDMSG
#include <sys/uio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
static struct iovec *vs=NULL,*vc=NULL;
/* �������Կͻ��˵Ĵ��� */
void process_conn_server(int s)
{
	char buffer[30];	/* �����Ļ����� */
	ssize_t size = 0;
	struct msghdr msg;	/* ��Ϣ�ṹ */
	
	
	/* ����3������ */
	struct iovec *v = (struct iovec*)malloc(3*sizeof(struct iovec));
	if(!v){
		printf("Not enough memory\n");
		return;		
	}
	
	/**�ҽ�ȫ�ֱ����������ͷŹ���*/
	vs = v;
	
	/* ��ʼ����Ϣ */
	msg.msg_name = NULL;	/*û��������*/
	msg.msg_namelen = 0;	/*�����򳤶�Ϊ0*/
	msg.msg_control = NULL;	/*û�п�����*/
	msg.msg_controllen = 0;	/*�����򳤶�Ϊ0*/
	msg.msg_iov = v;		/*�ҽ�����ָ��*/
	msg.msg_iovlen = 30;	/*���ջ���������Ϊ30*/
	msg.msg_flags = 0;		/*���������*/
	
	/* ÿ������10���ֽڵĿռ� */
	v[0].iov_base = buffer;	/*0-9*/
	v[1].iov_base = buffer + 10;/*10-19*/
	v[2].iov_base = buffer + 20;/*20-29*/
	/*��ʼ������Ϊ10*/
	v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
	
	
	
	for(;;){/* ѭ��������� */
		/* ���׽����ж�ȡ���ݷŵ������������� */
		size = recvmsg(s, &msg, 0);	
		if(size == 0){/* û������ */
			return;	
		}
		
		/* ������Ӧ�ַ���Ϊ���յ��ͻ����ֽڵ��������ֱ�ŵ������������� */
		sprintf(v[0].iov_base, "%d ", size); /*����*/
		sprintf(v[1].iov_base, "bytes alt"); /*��bytes alt���ַ���*/
		sprintf(v[2].iov_base, "ogether\n"); /*ogether\n���ַ���*/
		/*д���ַ�������*/
		v[0].iov_len = strlen(v[0].iov_base);
		v[1].iov_len = strlen(v[1].iov_base);
		v[2].iov_len = strlen(v[2].iov_base);
		sendmsg(s, &msg, 0);/* �����ͻ��� */
	}	
}

/* �ͻ��˵Ĵ������ */
void process_conn_client(int s)
{
	char buffer[30];	/* �����Ļ����� */
	ssize_t size = 0;
	struct msghdr msg;	/* ��Ϣ�ṹ */
	
	/* ����3������ */
	struct iovec *v = (struct iovec*)malloc(3*sizeof(struct iovec));
	if(!v){
		printf("Not enough memory\n");
		return;		
	}
	
	/**�ҽ�ȫ�ֱ����������ͷŹ���*/
	vc = v;
	/* ��ʼ����Ϣ */
	msg.msg_name = NULL;	/*û��������*/
	msg.msg_namelen = 0;	/*�����򳤶�Ϊ0*/
	msg.msg_control = NULL;	/*û�п�����*/
	msg.msg_controllen = 0;	/*�����򳤶�Ϊ0*/
	msg.msg_iov = v;		/*�ҽ�����ָ��*/
	msg.msg_iovlen = 30;	/*���ջ���������Ϊ30*/
	msg.msg_flags = 0;		/*���������*/
	
	/* ÿ������10���ֽڵĿռ� */
	v[0].iov_base = buffer;/*0-9*/
	v[1].iov_base = buffer + 10;/*10-19*/
	v[2].iov_base = buffer + 20;/*20-29*/
	/*��ʼ������Ϊ10*/
	v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
	
	
	
	int i = 0;
	
	for(;;){/* ѭ��������� */
		/* �ӱ�׼�����ж�ȡ���ݷŵ�������buffer�� */
		size = read(0, v[0].iov_base, 10);
		if(size > 0){/* �������� */
			v[0].iov_len= size;
			sendmsg(s, &msg,0);		/* ���͸������� */
			v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
			size = recvmsg(s, &msg,0);/* �ӷ�������ȡ���� */
			for(i = 0;i<3;i++){
				if(v[i].iov_len > 0){
					write(1, v[i].iov_base, v[i].iov_len);		/* д����׼��� */
				}
			}
		}
	}	
}

void sig_proccess(int signo)
{
	printf("Catch a exit signal\n");
	/* �ͷ���Դ */	
	free(vc);
	free(vs);
	_exit(0);	
}

void sig_pipe(int sign)
{
	printf("Catch a SIGPIPE signal\n");
	
	/* �ͷ���Դ */	
	free(vc);
	free(vs);
	_exit(0);
}
#endif

