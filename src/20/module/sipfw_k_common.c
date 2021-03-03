#ifndef __KERNEL__
#define __KERNEL__
#endif /*__KERNEL__*/
#ifndef MODULE
#define MODULE
#endif /*MODULE*/
#include "sipfw.h"


/* ����������ñ��� ���ڵļ򵥳���
*	���������ò��ķ���������
*	2007�굽2012���ʱ��
* ����:
*	r:ʱ��ṹ�����ڽ�����������
*/
void SIPFW_Localtime(struct tm *r, unsigned long time)
{
	unsigned int year, i, days, sec;
	__u16 *yday = NULL;
	/* 2007�굽2012����1970������� */
	__u16 days_since_epoch[] = 
	{
		/* 2007 - 2012 */
		13514,13879, 14245, 14610,14975, 15340, 
	};

	/*ĳ����һ���п�ʼ������*/
	__u16 days_since_year[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
	};
	/*ĳ����һ�������п�ʼ������*/
	__u16 days_since_leapyear[] = {
		0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
	};
	yday = days_since_year;				/*����Ϊһ����*/

	sec = time + 3600*8;					/*���϶�����������*/
	days = sec /(3600*24);					/*������*/
	sec =  sec % (3600*24);				/*һ��������*/
	r->hour = sec / 3600;					/*Сʱ��*/
	sec = sec % 3600;						/*һСʱ������*/
	r->min = sec / 60;					/*������*/
	r->sec = sec % 60;					/*һ����������*/

	/*������1970��ʼ������*/
	for(i= 0, year = 2007; days_since_epoch[i]<days; i++,year++)
		;
	year--;i--;							/*�ظ���ȷ����ֵ*/
	
	days -= days_since_epoch[i];			/*��������*/
	
	if (year% 4 == 0 && (year % 100 != 0 || year % 400 == 0)) 
	{
		yday = days_since_leapyear;		/*����*/
	}
	

	for (i=0; i < 12 && days > yday[i]; i++)	/*�����·�*/
		;
	r->year = year;						/*����*/
	r->mon    = i ;						/*����*/
	r->mday = days -yday[i-1];			/*��������*/
	
	return ;
}

/*�ж��������ݺ͹���ĸ������Ƿ�ƥ�䣬
*	�����˿ںš�TCP�ı�־λ��ICMP/IGMP���ʹ���
* ����:
*	iphΪIPͷ��ָ��
*	dataΪIP�ĸ���
*	rΪ����
*/
static int SIPFW_IsAddtionMatch(struct iphdr *iph,  void *data, struct sipfw_rules *r)
{
	int found = 0;

	DBGPRINT("==>SIPFW_IsAddtionMatch\n");

	switch(iph->protocol)
	{
		case IPPROTO_TCP:/*TCPЭ�����ж϶˿ںͱ�־λ*/
		{
			struct tcphdr *tcph = (struct tcphdr *)data;
			if( (tcph->source == r->sport || r->sport == 0)/*�˿�*/
				&&(tcph->dest == r->dport || r->dport == 0))
			{
				if(!r->addtion.valid)/*�����в����ڱ�־λ*/
				{
					found = 1;/*ƥ��*/
				}
				else	/*���ڱ�־λ*/
				{
					/*�ж�TCPͷ���ı�־λ*/
					struct tcp_flag *tcpf = &r->addtion.tcp;
					if(tcpf->ack == tcph->ack		/*ACK/SYN*/
						&&tcpf->fin == tcph->fin	/*FIN*/
						&&tcpf->syn == tcph->syn)	/*SYN*/
					{
						found = 1;/*ƥ��*/
					}
				}
			}
		}
		break;
		
		case IPPROTO_UDP:/*UDPЭ���ж϶˿�*/
		{
			struct tcphdr *udph = (struct tcphdr *)data;
			if( (udph->source == r->sport || r->sport == 0)
				&&(udph->dest == r->dport || r->dport == 0))
			{
				found = 1;
			}
		}
		break;
		
		case IPPROTO_ICMP:/*ICMP�ж����ͺʹ���*/		
		case IPPROTO_IGMP:/*IGMP�ж����ͺʹ���*/
		{
			struct igmphdr *igmph = (struct igmphdr*)data;
			if(!r->addtion.valid)/*����������*/
			{
				found = 1;
			}
			else/*��������*/
			{
				struct icgmp_flag *impf = &r->addtion.icgmp;
				if(impf->type == igmph->type && impf->code == igmph->code)
				{
					found = 1;/*����*/
				}
			}
		}
		break;
		
		default:/*����������*/
			found = 0;
			break;
	}

	DBGPRINT("==>SIPFW_IsAddtionMatch\n");

	return found;
}

/*ƥ���������ݺ͹����е�IP��ַ��Э���Ƿ�ƥ��*/
static int SIPFW_IsIPMatch(struct iphdr *iph, struct sipfw_rules *r)
{
	int found = 0;
	DBGPRINT("==>SIPFW_IsIPMatch\n");
	if((iph->daddr == r->dest|| r->dest == 0)/*Ŀ�ĵ�ַ*/
		&&(iph->saddr==r->source|| r->source == 0)/*Դ��ַ*/
		&&( iph->protocol== r->protocol  ||  r->protocol == 0))/*Э��*/
	{
		found = 1;/*ƥ��*/
	}

	DBGPRINT("<==SIPFW_IsIPMatch\n");
	return found;
}

/*�ж��������ݺ�һ�����ϵĹ����Ƿ�ƥ��*/
struct sipfw_rules * SIPFW_IsMatch(struct sk_buff *skb,struct sipfw_rules *l)
{
	struct sipfw_rules *r = NULL;	/*����*/
	struct iphdr *iph = NULL;		/*IPͷ��*/
	void *p = NULL;				/*�������ݸ���*/
	int found = 0;				/*�Ƿ�ƥ��*/

	

	iph = skb->nh.iph;			/*�ҵ�IPͷ��*/

	p = skb->data + skb->nh.iph->ihl*4;/*���ز���*/

	DBGPRINT("source IP:%x,dest:%x\n",iph->saddr,iph->daddr);
	if(l == NULL)					/*��Ϊ��ֱ���˳�*/
	{
		goto EXITSIPFW_IsMatch;
	}
	for(r = l; r != NULL; r = r->next)/*������ѭ��ƥ�����*/
	{
		if(SIPFW_IsIPMatch(iph, r))/*IP�Ƿ�ƥ��*/
		{
			if(SIPFW_IsAddtionMatch(iph,p,r))/*���������Ƿ�ƥ��*/
			{
				found = 1;/*ƥ��*/
				break;
			}
		}
	}
	
EXITSIPFW_IsMatch:	
	return found?r:NULL;
}

/*���ٹ��������ͷ���Դ*/
int SIPFW_ListDestroy(void)
{
	struct sipfw_list *l = NULL;
	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;
	int i ;
	DBGPRINT("==>SIPFW_ListDestroy\n");

	for(i = 0;i < 3; i++)/*����������*/
	{
		
		l = &sipfw_tables[i];
		for(cur = l->rule; 		/*����һ����*/
			cur != NULL; 
			prev = cur, cur = cur->next)
		{
			if(prev)	/*�ͷ��ڴ�*/
			{
				kfree(prev);
			}
		}
		l->rule = NULL;/*����ָ��*/
		l->number = 0;/*���������ڹ������*/
	}

	DBGPRINT("<==SIPFW_ListDestroy\n");
	return 0;
}



