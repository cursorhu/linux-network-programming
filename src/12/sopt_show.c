#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <linux/in.h>

/* �ṹ�����ȡ��� */
typedef union optval {
	int 			val;		/*����ֵ*/
	struct linger 	linger;		/*linger�ṹ*/
	struct timeval 	tv;			/*ʱ��ṹ*/
	unsigned char 	str[16];	/*�ַ���*/
}val;

/*��ֵ����*/
typedef enum valtype{
	VALINT,		/*int����*/
	VALLINGER,	/*struct linger����*/
	VALTIMEVAL,	/*struct timeval����*/
	VALUCHAR,	/*�ַ���*/
	VALMAX		/*��������*/
}valtype;

/* ���ڱ����׽���ѡ��Ľṹ */
typedef struct sopts{
	int level;		/*�׽���ѡ���*/
	int optname;	/*�׽���ѡ������*/
	char *name;		/*�׽�������*/
	valtype valtype;/*�׽��ַ��ز�������*/
}sopts;

static val optval;/*���ڱ�����ֵ*/
sopts sockopts[] = {
	{SOL_SOCKET, 	SO_BROADCAST,		"SO_BROADCAST",			VALINT},
	{SOL_SOCKET, 	SO_DEBUG,			"SO_DEBUG",				VALINT},
	{SOL_SOCKET, 	SO_DONTROUTE,		"SO_DONTROUTE",			VALINT},
	{SOL_SOCKET, 	SO_ERROR,			"SO_ERROR",				VALINT},
	{SOL_SOCKET, 	SO_KEEPALIVE,		"SO_KEEPALIVE",			VALINT},
	{SOL_SOCKET, 	SO_LINGER,			"SO_LINGER",			VALINT},
	{SOL_SOCKET, 	SO_OOBINLINE,		"SO_OOBINLINE",			VALINT},
	{SOL_SOCKET, 	SO_RCVBUF,			"SO_RCVBUF",			VALINT},
	{SOL_SOCKET, 	SO_RCVLOWAT,		"SO_RCVLOWAT",			VALINT},
	{SOL_SOCKET, 	SO_RCVTIMEO,		"SO_RCVTIMEO",			VALTIMEVAL},
	{SOL_SOCKET, 	SO_SNDTIMEO,		"SO_SNDTIMEO",			VALTIMEVAL},
	{SOL_SOCKET, 	SO_TYPE,			"SO_TYPE",				VALINT},
	{IPPROTO_IP, 	IP_HDRINCL,			"IP_HDRINCL",			VALINT},
	{IPPROTO_IP, 	IP_OPTIONS,			"IP_OPTIONS",			VALINT},
	{IPPROTO_IP, 	IP_TOS,				"IP_TOS",				VALINT},
	{IPPROTO_IP, 	IP_TTL,				"IP_TTL",				VALINT},
	{IPPROTO_IP, 	IP_MULTICAST_TTL,	"IP_MULTICAST_TTL",		VALUCHAR},
	{IPPROTO_IP, 	IP_MULTICAST_LOOP,	"IP_MULTICAST_LOOP",	VALUCHAR},
	{IPPROTO_TCP, 	TCP_KEEPCNT,		"TCP_KEEPCNT",		VALINT},
	{IPPROTO_TCP, 	TCP_MAXSEG,			"TCP_MAXSEG",			VALINT},
	{IPPROTO_TCP, 	TCP_NODELAY,		"TCP_NODELAY",			VALINT},
	{0,				0,					NULL,					VALMAX}/*��β�����������ж�VALMAX*/
};

/* ��ʾ��ѯ��� */
static void disp_outcome(sopts *sockopt, int len, int err)
{
	if(err == -1){/* ���� */
		printf("optname %s NOT support\n",sockopt->name);
		return;	
	}
	
	switch(sockopt->valtype){/*���ݲ�ͬ�����ͽ�����Ϣ��ӡ*/
		case VALINT:/*����*/
			printf("optname %s: default is %d\n",sockopt->name,optval.val);
			break;
		case VALLINGER:/*struct linger*/
			printf("optname %s: default is  %d(ON/OFF), %d to linger\n",
				sockopt->name,			/*����*/
				optval.linger.l_onoff,/*linger��*/
				optval.linger.l_linger);/*��ʱʱ��*/
			break;
		case VALTIMEVAL:/*struct timeval�ṹ*/
			printf("optname %s: default is %.06f\n",
				sockopt->name,/*����*/
				((((double)optval.tv.tv_sec*100000+(double)optval.tv.tv_usec))/(double)1000000));/*�����ͽṹ*/
			break;
		case VALUCHAR:/*�ַ������ͣ�ѭ����ӡ*/
			{
				int i = 0;
				printf("optname %s: default is ",sockopt->name);/*ѡ������*/
				for(i = 0; i < len; i++){
					printf("%02x ", optval.str[i]);			
				}
				printf("\n");					
			}
			break;
		default:
			break;
	}	
}

int main(int argc, char *argv[])
{
	int err = -1;
	int len = 0;
	int i = 0;
	int s = socket(AF_INET, SOCK_STREAM, 0);/*����һ����ʽ�׽���*/
	while(sockopts[i].valtype != VALMAX){/*�ж��Ƿ��β��������ѯִ��*/
		len = sizeof(sopts);/*����ṹ����*/
		err = getsockopt(s, sockopts->level, sockopts->optname, &optval, &len);/*��ȡѡ��״̬*/
		
		disp_outcome(&sockopts[i], len, err);/*��ʾ���*/
		i++;/*����*/
	}

	close(s);
	return 0;	
}
