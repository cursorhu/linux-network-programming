#ifndef __SIP_SKBUFF_H__
#define __SIP_SKBUFF_H__

#include "sip_ether.h"
#include "sip_skbuff.h"
#include "sip_arp.h"
#include "sip_ip.h"
#include "sip_icmp.h"
#include "sip_tcp.h"
#include "sip_udp.h"
#define CHECKSUM_NONE 0
#define CHECKSUM_HW 1
#define CHECKSUM_UNNECESSARY 2
struct sip_tcphdr;
struct sip_udphdr;
struct sip_icmphdr;
struct sip_igmphdr;
struct sip_iphdr;
struct sip_arphdr;
struct sip_ethhdr;
struct net_device;
struct skbuff {
	struct skbuff *next;				/*��һ��skbuff�ṹ*/

	union 							/*�����ö�ٱ���*/
	{
		struct sip_tcphdr		*tcph;	/*tcpЭ���ͷ��ָ��*/
		struct sip_udphdr		*udph;	/*udpЭ���ͷ��ָ��*/
		struct sip_icmphdr		*icmph;	/*icmpЭ���ͷ��ָ��*/
		struct sip_igmphdr	*igmph;	/*igmpЭ���ͷ��ָ��*/
		__u8				*raw;	/*�����ԭʼ����ָ��*/
	} th;							/*��������*/

	union 							/*�����ö�ٱ���*/
	{
		struct sip_iphdr		*iph;	/*ipЭ���ͷ��ָ��*/
		struct sip_arphdr		*arph;	/*arpЭ���ͷ��ָ��*/
		__u8				*raw;	/*�����ԭʼ����ָ��*/
	} nh;							/*��������*/

	union 							/*�����ö�ٱ���*/
	{
		struct sip_ethhdr		*ethh;	/*��������̫��ͷ��*/
	  	__u8 				*raw;	/*������ԭʼ����ָ��*/
	} phy;							/*��������*/

	struct net_device  		*dev;	/*�����豸*/
	__be16		protocol;	/*Э������*/
	__u32 		tot_len;		/*skbuff���������ݵ��ܳ���*/
	__u32 		len;  		/*skbuff�е�ǰЭ�������ݳ���*/
	
	__u8 		csum;		/*У���*/
	__u8		ip_summed;	/*ip��ͷ���Ƿ������У��*/
	__u8		*head,		/*ʵ���������ݵ�ͷ��ָ��*/
				*data,		/*��ǰ���������ݵ�ͷ��ָ��*/
				*tail,		/*��ǰ�����ݵ�β��ָ��*/
				*end;		/*ʵ���������ݵ�β����ָ��*/  
};

struct sip_sk_buff_head {
	struct skbuff	*next;
	struct skbuff	*prev;

	__u32		qlen;
};

#include <semaphore.h>
/** A sock descriptor */
struct sock {
	int type;					/*Э������*/
	int state;				/*Э���״̬*/
	union 
	{
		struct ip_pcb  *ip;		/*IP��Ŀ��ƽṹ*/
		struct tcp_pcb *tcp;	/*TCP��Ŀ��ƽṹ*/
		struct udp_pcb *udp;	/*UDP�Ŀ��ƽṹ*/
	} pcb;
	int err;					/*����ֵ*/
	struct skbuff *skb_recv;	/*���ջ�����*/

	sem_t sem_recv;			/*���ջ����������ź���*/
	int socket;				/*���sock��Ӧ���ļ�������ֵ*/
	int recv_timeout;			/*�������ݳ�ʱʱ��*/
	__u16 recv_avail;		/*���Խ�������*/
};

#endif /*__SIP_SKBUFF_H__*/
