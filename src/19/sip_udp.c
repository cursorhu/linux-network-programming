#include "sip.h"

#define UDP_HTABLE_SIZE 128						/*UDP���Ƶ�Ԫ�Ĵ�С*/
static struct udp_pcb *udp_pcbs[UDP_HTABLE_SIZE];	/*UDP���Ƶ�Ԫ����*/
static __u16 found_a_port()
{
	static __u32 index = 0x0;				/*��̬����,���ڱ��浱ǰ�Ѿ�����Ķ˿�*/
	index ++;							/*���Ӷ˿�ֵ*/
	return (__u16)(index&0xFFFF);			/*����16Ϊ�Ķ˿�ֵ*/
}


struct udp_pcb *SIP_UDPNew(void)					
{
	struct udp_pcb *pcb = NULL;				/*pcb����*/
	pcb = (struct udp_pcb *)malloc(sizeof(struct udp_pcb));/*�������*/
	if (pcb != NULL) 							/*����ɹ�*/
	{
		memset(pcb, 0, sizeof(struct udp_pcb));	/*��ʼ��Ϊ0*/
		pcb->ttl = 255;						/*��������ռ�Ϊ255*/
	}
	
	return pcb;								/*����pcbָ��*/
}

void SIP_UDPRemove(struct udp_pcb *pcb)
{
	struct udp_pcb *pcb_t;
	int i = 0;

	if(!pcb){								/*pcbΪ��*/
		return;
	}

	pcb_t = udp_pcbs[pcb->port_local%UDP_HTABLE_SIZE];/*���ض˿�ֵ��hash��λ�ÿ��ƽṹ*/
	if(!pcb_t){								/*Ϊ��*/
		;
	}else if(pcb_t == pcb)	{					/*Ϊ��ǰ���ƽṹ*/
		udp_pcbs[pcb->port_local%UDP_HTABLE_SIZE] = pcb_t->next;/*�ӿ���������ժ���ṹ*/
	}else{									/*ͷ�����ǿ��ƽṹ*/
		for (; pcb_t->next != NULL; pcb_t = pcb_t->next) /*����ƥ����*/
		{
			if (pcb_t->next == pcb) 			/*�ҵ�*/
			{
				pcb_t->next = pcb->next;		/*�ӿ���������ժ���ṹ*/
			}
		}
	}
	
	free(pcb);								/*�ͷ���Դ*/
}


int SIP_UDPBind(struct udp_pcb *pcb, 
			struct in_addr *ipaddr,
			__u16 port)
{
	struct udp_pcb *ipcb;
	__u8 rebind;

	rebind = 0;
	/* ����udp_pcbs���Ƿ��Ѿ�����������Ƶ�Ԫ */
	for (ipcb = udp_pcbs[port&(UDP_HTABLE_SIZE-1)]; ipcb != NULL; ipcb = ipcb->next) 
	{
		if (pcb == ipcb) 						/*�Ѿ�����*/
		{
			rebind = 1;						/*�Ѿ���*/
		}
	}

	pcb->ip_local.s_addr= ipaddr->s_addr;

	if (port == 0) 							/*��û��ָ���˿ڵ�ַ*/
	{
#define UDP_PORT_RANGE_START 4096
#define UDP_PORT_RANGE_END   0x7fff
		port = found_a_port();				/*���ɶ˿�*/
		ipcb = udp_pcbs[port];
		/*�������������еĵ�Ԫ�Ƿ��Ѿ�ʹ������˿ڵ�ַ*/
		while ((ipcb!=NULL)&&(port != UDP_PORT_RANGE_END) )
		{
			if (ipcb->port_local == port) 		/*�Ѿ�ʹ�ô˶˿�*/
			{
				port = found_a_port();		/*�������ɶ˿ڵ�ַ*/
				ipcb = udp_pcbs[port];		/*����ɨ��*/
			}else{						
				ipcb = ipcb->next;				/*��һ��*/
			}
		}

		if (ipcb != NULL) 						/*û�к��ʵĶ˿�*/
		{
			return -1;						/*���ش���ֵ*/
		}
	}

	pcb->port_local = port;					/*�󶨺��ʵĶ˿�ֵ*/
	if (rebind == 0) 							/*��û�н��˿��Ƶ�Ԫ��������*/
	{
		pcb->next = udp_pcbs[port];			/*�ŵ����Ƶ�Ԫ�����hashλ��ͷ��*/
		udp_pcbs[port] = pcb;				/*����ͷָ��*/
	}

	return 0;
}


int SIP_UDPConnect(struct udp_pcb *pcb, 
					struct in_addr *ipaddr, 
					__u16 port)
{
	struct udp_pcb *ipcb;

	if (pcb->port_local == 0) 					/*��û�а󶨶˿ڵ�ַ*/
	{
		int err = SIP_UDPBind(pcb, &pcb->ip_local, 0);/*�󶨶˿�*/
		if (err != 0)
			return err;
	}

	pcb->ip_remote.s_addr = ipaddr->s_addr;	/*Ŀ��IP��ַ*/
	pcb->port_remote = port;					/*Ŀ�Ķ˿�*/

	/* ��UDP��PCB����PCB������*/
	for (ipcb = udp_pcbs[pcb->port_local]; ipcb != NULL; ipcb = ipcb->next) 
	{
		if (pcb == ipcb) 						/*�Ѿ�������������*/
		{
			return 0;					
		}
	}

	/*���PCB���Ƶ�Ԫ��û�м��������У����˵�Ԫ���뵽�����ͷ��*/
	pcb->next = udp_pcbs[pcb->port_local];
	udp_pcbs[pcb->port_local] = pcb;

	return 0;
}


void SIP_UDPDisconnect(struct udp_pcb *pcb)
{
	/* reset remote address association */
	pcb->ip_remote.s_addr = INADDR_ANY;
	pcb->port_remote = 0;
	/* mark PCB as unconnected */
	pcb->flags &= ~UDP_FLAGS_CONNECTED;
}



int SIP_UDPSendTo(struct net_device *dev,
	struct udp_pcb *pcb,
	struct skbuff *skb,
	struct in_addr *dst_ip, __u16 dst_port)
{
	struct sip_udphdr *udphdr;
	struct in_addr *src_ip;
	int err;

	/*�����PCB��û�а󶨶˿�,���ж˿ڰ�*/
	if (pcb->port_local == 0) 								/*��û�а󶨶˿�*/
	{
		err = SIP_UDPBind(pcb, &pcb->ip_local, pcb->port_local);/*�󶨶˿�*/
		if (err != 0) 
		{
			return err;
		}
	}

	udphdr = skb->th.udph;					/*UDPͷ��ָ��*/
	udphdr->source = htons(pcb->port_local);	/*UDPԴ�˿�*/
	udphdr->dest = htons(dst_port);			/*UDPĿ�Ķ˿�*/
	udphdr->check= 0x0000; 					/*�Ƚ�UDP��У�������Ϊ0*/

	/* PCB���ص�ַΪ IP_ANY_ADDR? */
	if (pcb->ip_local.s_addr == 0) 
	{
		src_ip = &dev->ip_host;				/*��Դ��ַ����Ϊ����IP��ַ*/
	} 	else 	{
		src_ip = &(pcb->ip_local);				/*��PCB�е�IP��ַ��ΪԴIP��ַ*/
	}

	udphdr->len = htons(skb->len);				/*UDP��ͷ������*/
	/* ����У��� */
	if ((pcb->flags & UDP_FLAGS_NOCHKSUM) == 0) 
	{
		udphdr->check= SIP_ChksumPseudo(skb, src_ip, dst_ip, IPPROTO_UDP, skb->len);
		if (udphdr->check == 0x0000) 
			udphdr->check = 0xffff;
	}

	/*����UDP�ķ��ͺ��������ݷ��ͳ�ȥ*/
	err = SIP_UDPSendOutput(skb, src_ip, dst_ip, pcb->ttl, pcb->tos, IPPROTO_UDP);    

	return err;
}

int  SIP_UDPSend(struct net_device *dev,struct udp_pcb *pcb, struct skbuff *skb)
{
 	/* send to the packet using remote ip and port stored in the pcb */
	return SIP_UDPSendTo(dev, pcb,skb, &pcb->ip_remote, pcb->port_remote);
}
#include <semaphore.h>
#include <pthread.h>
int SIP_UDPInput(struct net_device *dev, struct skbuff *skb)
{
	__u16 port = ntohs(skb->th.udph->dest);
	
	struct udp_pcb *upcb = NULL;
	/*���ݶ˿ڵ�ַ���ҿ�������ṹ�еĿ��Ƶ�Ԫ*/
	for(upcb = udp_pcbs[port%UDP_HTABLE_SIZE]; upcb != NULL; upcb = upcb->next)
	{
		if(upcb->port_local== port)				/*�ҵ�*/
			break;
	}

	if(!upcb)
		return 0;

	struct sock *sock = upcb->sock;			/*Э���޹ز�Ľṹ*/
	if(!sock)
		return 1;

	struct skbuff *recvl = sock->skb_recv;		/*���ջ���������ͷָ��*/
	if(!recvl)									/*Ϊ��?*/
	{
		sock->skb_recv = skb;					/*�ҽӵ�ͷ��*/
		skb->next = NULL;
	}
	else
	{
		for(; recvl->next != NULL; upcb = upcb->next)/*��β��*/
			;
		recvl->next = skb;					/*��β���ҽ�*/
		skb->next = NULL;
	}

	sem_post(&sock->sem_recv);				/*���ջ���������ֵ����*/	
}


int SIP_UDPSendOutput(struct net_device *dev, struct skbuff *skb,struct udp_pcb *pcb,
	struct in_addr *src, struct in_addr *dest)
{
	ip_output(dev,skb, src, dest, pcb->ttl, pcb->tos, IPPROTO_UDP);    
}
