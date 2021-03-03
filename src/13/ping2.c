/* ping.c -- ��13�� ���� */ 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h> /* bzero */
#include <netdb.h>
#include <pthread.h>

static pingm_pakcet *icmp_findpacket(int seq);
static unsigned short icmp_cksum(unsigned char *data,  int len);
static struct timeval icmp_tvsub(struct timeval end,struct timeval begin);
static void icmp_statistics(void);
static void icmp_pack(struct icmp *icmph, int seq, struct timeval *tv, int length );
static int icmp_unpack(char *buf,int len);
static void *icmp_recv(void *argv);
static void * icmp_send(void *argv);
static void icmp_sigint(int signo);
static void icmp_usage();


/* �����Ѿ����Ͱ���״ֵ̬ */
typedef struct pingm_pakcet{
	struct timeval tv_begin;	/*���͵�ʱ��*/
	struct timeval tv_end;		/*���յ���ʱ��*/
	short seq;				/*���к�*/
	int flag;					/*1����ʾ�Ѿ����͵�û�н��յ���Ӧ��
								0����ʾ���յ���Ӧ��*/
}pingm_pakcet;
static pingm_pakcet pingpacket[128];

#define K 1024
#define BUFFERSIZE 72			/*���ͻ�������С*/
static unsigned char send_buff[BUFFERSIZE];

static unsigned char recv_buff[2*K];	/*Ϊ��ֹ������������ջ��������ô�һЩ*/
static struct sockaddr_in dest;		/*Ŀ�ĵ�ַ*/
static int rawsock = 0;				/*���ͺͽ����߳���Ҫ��socket������*/
static pid_t pid=0;					/*����PID*/
static int alive = 0;				/*�Ƿ���յ��˳��ź�*/
static short packet_send = 0;		/*�Ѿ����͵����ݰ�����*/
static short packet_recv = 0;		/*�Ѿ����յ����ݰ�����*/
static char dest_str[80];			/*Ŀ�������ַ���*/
static struct timeval tv_begin, tv_end,tv_interval; /*������ʼ���͡�������ʱ����*/

static void icmp_usage()
{
	/* ping��IP��ַ�������� */
	printf("ping aaa.bbb.ccc.ddd\n");
}

/* �ն��źŴ�����SIGINT */
static void icmp_sigint(int signo)
{
	/* ���߽��պͷ����߳̽������� */
	alive = 0;
	/* ��ȡ�������ʱ�� */
	gettimeofday(&tv_end, NULL);
	/* ����һ���ܹ�����ʱ�� */
	tv_interval = icmp_tvsub(tv_end, tv_begin);
	
	return;
}


/*����һ�����ʵİ�λ��
��seqΪ-1ʱ����ʾ���ҿհ�
����ֵ��ʾ����seq��Ӧ�İ�*/
static pingm_pakcet *icmp_findpacket(int seq)
{
	int i=0;
	pingm_pakcet *found = NULL;

	/* ���Ұ���λ�� */
	if(seq == -1)/* ���ҿյ�λ�� */
	{
		for(i = 0;i<128;i++)
		{
			if(pingpacket[i].flag == 0)
			{
				found = &pingpacket[i];
				break;
			}
			
		}
	}
	else if(seq >= 0)/*���Ҷ�ӦSEQ�İ�*/
	{
		for(i = 0;i<128;i++)
		{
			if(pingpacket[i].seq == seq)
			{
				found = &pingpacket[i];
				break;
			}
			
		}
	}

	return found;
}

/* CRC16У��ͼ���icmp_cksum
������
	data:����
	len:���ݳ���
����ֵ��
	��������short����
*/
static unsigned short icmp_cksum(unsigned char *data,  int len)
{
       int sum=0;/* ������ */
	int odd = len & 0x01;/*�Ƿ�Ϊ����*/

	unsigned short *value = (unsigned short*)data;
	/*�����ݰ���2�ֽ�Ϊ��λ�ۼ�����*/
       while( len & 0xfffe)  {
              sum += *(unsigned short*)data;
		data += 2;
		len -=2;
       }
	/*�ж��Ƿ�Ϊ���������ݣ���ICMP��ͷΪ�������ֽڣ���ʣ�����һ�ֽڡ�*/
       if( odd) {
		unsigned short tmp = ((*data)<<8)&0xff00;
              sum += tmp;
       }
       sum = (sum >>16) + (sum & 0xffff);
       sum += (sum >>16) ;
       
       return ~sum;
}

/*����ʱ���time_sub
������
	end,���յ���ʱ��
	begin����ʼ���͵�ʱ��
����ֵ��
	ʹ�õ�ʱ��
*/
static struct timeval icmp_tvsub(struct timeval end,struct timeval begin)
{
	struct timeval tv;

	/*�����ֵ*/
	tv.tv_sec = end.tv_sec - begin.tv_sec;
	tv.tv_usec = end.tv_usec - begin.tv_usec;

	/* �������ʱ���usecֵС�ڷ���ʱ��usecֵ����usec���λ */
	if(tv.tv_usec < 0)
	{
		tv.tv_sec --;
		tv.tv_usec += 1000000; 
	}
	
	return tv;
}


/* ��ӡȫ��ICMP���ͽ���ͳ�ƽ�� */
static void icmp_statistics(void)
{       
	long time = (tv_interval.tv_sec * 1000 )+ (tv_interval.tv_usec/1000);
	printf("--- %s ping statistics ---\n",dest_str);/* Ŀ����IP��ַ */
	printf("%d packets transmitted, %d received, %d%c packet loss, time %d ms\n",
		packet_send,/* ���� */
		packet_recv,  /* ���� */
		(packet_send-packet_recv)*100/packet_send, /* ��ʧ�ٷֱ� */
		'%',
		time); /* ʱ�� */
}


/* ��ѹ���յ��İ�������ӡ��Ϣ */
static int icmp_unpack(char *buf,int len)
{       
	int i,iphdrlen;
	struct ip *ip = NULL;
	struct icmp *icmp = NULL;
	int rtt;
	
	ip=(struct ip *)buf; 				/* IPͷ�� */
	iphdrlen=ip->ip_hl*4;				/* IPͷ������ */
	icmp=(struct icmp *)(buf+iphdrlen);	/* ICMP�εĵ�ַ */
	len-=iphdrlen;

	/* �жϳ����Ƿ�ΪICMP�� */
	if( len<8) 
	{       
		printf("ICMP packets\'s length is less than 8\n");
		return -1;
	}

	/* ICMP����ΪICMP_ECHOREPLY����Ϊ�����̵�PID */
	if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id== pid) )	
	{
		struct timeval tv_internel,tv_recv,tv_send;
		/* �ڷ��ͱ���в����Ѿ����͵İ�������seq */
		pingm_pakcet* packet = icmp_findpacket(icmp->icmp_seq);
		if(packet == NULL)
			return -1;

		/* ȡ����־ */
		packet->flag = 0;
		/* ��ȡ�����ķ���ʱ�� */
		tv_send = packet->tv_begin;
		
		/* ��ȡ��ʱʱ�䣬����ʱ��� */
		gettimeofday(&tv_recv, NULL);
		tv_internel = icmp_tvsub(tv_recv,tv_send);  
		
		rtt = tv_internel.tv_sec*1000+tv_internel.tv_usec/1000; 

		/* ��ӡ���������
		*  ICMP�γ���
		*  ԴIP��ַ
		*  �������к�
		*  TTL
		*  ʱ���
		*/
		printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%d ms\n",
			len,
			inet_ntoa(ip->ip_src),
			icmp->icmp_seq,
			ip->ip_ttl,
			rtt);

		/* ���հ�������1 */
		packet_recv ++;
	}
	else
	{
		return -1;
	}
}



/*����ICMP��ͷ*/
static void icmp_pack(struct icmp *icmph, int seq, struct timeval *tv, int length )
{       
	unsigned char i = 0;

	/* ���ñ�ͷ */
	icmph->icmp_type = ICMP_ECHO;	/*ICMP��������*/
	icmph->icmp_code = 0;			/*codeֵΪ0*/
	icmph->icmp_cksum = 0;			/*�Ƚ�cksumֵ��д0������֮���cksum����*/
	icmph->icmp_seq = seq;			/*���������к�*/
	icmph->icmp_id = pid &0xffff;		/*��дPID*/

	for(i = 0; i< length; i++)
		icmph->icmp_data[i] = i;
	/* ����У��� */
	icmph->icmp_cksum = icmp_cksum((unsigned char*)icmph, length);
}


/* ����ICMP��������� */
static void * icmp_send(void *argv)
{
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = 1;

	/* �������ʼ�������ݵ�ʱ�� */
	gettimeofday(&tv_begin, NULL);

	while(alive)
	{
		int size = 0;
		struct timeval tv;
		gettimeofday(&tv, NULL);/*��ǰ���ķ���ʱ��*/
		
		icmp_pack((struct icmp *)send_buff, packet_send, &tv, 64 );/*�������*/
		size = sendto (rawsock,  send_buff, 64,  0,  /*���͸�Ŀ�ĵ�ַ*/
			(struct sockaddr *)&dest, sizeof(dest) );
		if(size <0)
		{
			perror("sendto error");
			continue;
		}
		else
		{
			/* �ڷ��Ͱ�״̬��������һ������λ�� */
			pingm_pakcet *packet = icmp_findpacket(-1);
			if(packet)
			{
				packet->seq = packet_send;			/* ����seq */
				packet->flag = 1;						/* �Ѿ�ʹ�� */
				gettimeofday( &packet->tv_begin, NULL);/* ����ʱ�� */
				packet_send ++;						/* �������� */
			}
		}

		/* ÿ��һ�룬����һ��ICMP��������� */
		sleep(1);		
	}
}


/* ����pingĿ�������Ļظ� */
static void *icmp_recv(void *argv)
{
	/* ��ѵ�ȴ�ʱ�� */
	struct timeval tv;
	tv.tv_usec = 200;
	tv.tv_sec = 0;
	fd_set  readfd;

	/* ��û���źŷ���һֱ�������� */
	while(alive)
	{
		int ret = 0;
		FD_ZERO(&readfd);
		FD_SET(rawsock, &readfd);
		ret = select(rawsock+1,&readfd, NULL, NULL, &tv);
		switch(ret)
		{
			case -1:
				/* ������ */
				break;
			case 0:
				/* ��ʱ */
				break;
			default:
				{
					/* �յ�һ���� */
					int fromlen = 0;
					struct sockaddr from;

					/* �������� */
					int size = recv(rawsock, recv_buff,sizeof(recv_buff), 0) ;
					if(errno == EINTR)
					{
						perror("recvfrom error");
						continue;
					}

					/* �������������ر��� */
					ret = icmp_unpack(recv_buff, size);
					if(ret == -1)
					{
						continue;
					}
				}
				break;
		}
		
	}
}



/* ������ */
int main(int argc, char *argv[])
{
	struct hostent * host = NULL;
	struct protoent *protocol = NULL;
	char protoname[]= "icmp";

	unsigned long inaddr = 1;
	int size = 128*K;

	/* �����Ƿ�������ȷ */
	if(argc < 2)
	{
		icmp_usage();
		return -1;
	}

	/* ��ȡЭ������ICMP */
	protocol = getprotobyname(protoname);
	if (protocol == NULL)
	{
		perror("getprotobyname()");
		return -1;
	}

	/* ��Ŀ�ĵ�ַ�ַ������� */
	memcpy(dest_str,  argv[1], strlen(argv[1])+1);
	memset(pingpacket, 0, sizeof(pingm_pakcet) * 128);

	/* socket��ʼ�� */
	rawsock = socket(AF_INET, SOCK_RAW,  protocol->p_proto);
	if(rawsock < 0)
	{
		perror("socket");
		return -1;
	}

	/* Ϊ�����������̵�ping�������𣬼���pid */

	pid = getuid();

	/* ������ջ���������ֹ���յİ������� */
	setsockopt(rawsock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	bzero(&dest, sizeof(dest));

	/* ��ȡĿ�ĵ�ַ��IP��ַ */
	dest.sin_family = AF_INET;
	
	/* �����Ŀ�ĵ�ַΪ�ַ���IP��ַ */
	inaddr = inet_addr(argv[1]);
	if(inaddr == INADDR_NONE)
	{
		/* ΪDNS��ַ */
		host = gethostbyname(argv[1]);
		if(host == NULL)
		{
			perror("gethostbyname");
			return -1;
		}

		/* ����ַ������dest�� */
		memcpy((char *)&dest.sin_addr, host->h_addr, host->h_length);
	}
	else/* ΪIP��ַ�ַ��� */
	{
		memcpy((char*)&dest.sin_addr, &inaddr, sizeof(inaddr));
	}

	/* ��ӡ��ʾ */
	inaddr = dest.sin_addr.s_addr;
	printf("PING %s (%d.%d.%d.%d) 56(84) bytes of data.\n", 
		dest_str, 
		(inaddr&0x000000FF)>>0,
		(inaddr&0x0000FF00)>>8,
		(inaddr&0x00FF0000)>>16,
		(inaddr&0xFF000000)>>24);

	/* ��ȡ�ź�SIGINT,��icmp_sigint�ҽ��� */
	signal(SIGINT, icmp_sigint);
	
	/* ��ʼ��Ϊ������ */
	alive = 1;
	/* ���������̣߳����ڷ��ͺͽ��� */
	pthread_t send_id, recv_id;
	int err = 0;
	err = pthread_create(&send_id, NULL, icmp_send, NULL);/* ���� */
	if(err < 0)
	{
		return -1;
	}

	err = pthread_create(&recv_id, NULL, icmp_recv, NULL);/* ���� */
	if(err < 0)
	{
		return -1;
	}
	
	/* �ȴ��߳̽��� */
	pthread_join(send_id, NULL);
	pthread_join(recv_id, NULL);

	/* ������ӡͳ�ƽ�� */
	close(rawsock);
	icmp_statistics();

	return 0;	
}


