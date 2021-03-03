#include "sip.h"


static struct arpt_arp arp_table[ARP_TABLE_SIZE];
void init_arp_entry()
{
	int i= 0;
	for(i = 0; i<ARP_TABLE_SIZE; i++)					/*��ʼ������ARPӳ���*/
	{
		arp_table[i].ctime = 0;						/*��ʼʱ��ֵΪ0*/
		memset(arp_table[i].ethaddr, 0, ETH_ALEN);		/*MAC��ַΪ0*/
		arp_table[i].ipaddr = 0;						/*IP��ַΪ0*/
		arp_table[i].status = ARP_EMPTY;				/*����״̬Ϊ��*/
	}
}
struct arpt_arp* arp_find_entry(__u32 ip)
{
	int i = -1;
	struct arpt_arp*found = NULL;
	for(i = 0; i<ARP_TABLE_SIZE; i++)					/*��ARP���в���IPƥ����*/
	{
		if(arp_table[i].ctime >  time(NULL) + ARP_LIVE_TIME)/*�鿴�Ƿ���ʱ*/
			arp_table[i].status = ARP_EMPTY;			/*��ʱ,�ÿձ���*/
		else if(arp_table[i].ipaddr == ip 				/*û�г�ʱ,�鿴�Ƿ�IP��ַƥ��*/
			&& arp_table[i].status == ARP_ESTABLISHED)	/*����״̬Ϊ�Ѿ�����*/
		{
			found = &arp_table[i];						/*�ҵ�һ�����ʵı���*/
			break;									/*�˳����ҹ���*/
		}
	}

	return found;
}

struct arpt_arp  * update_arp_entry(__u32 ip,  __u8 *ethaddr)
{
	struct arpt_arp  *found = NULL;
	found = arp_find_entry(ip);						/*����IP����ARP����*/
	if(found){										/*�ҵ���Ӧ����*/
		memcpy(found->ethaddr, ethaddr, ETH_ALEN);	/*��������Ӳ����ַ������������*/
		found->status = ARP_ESTABLISHED;				/*����ARP�ı���״̬*/
		found->ctime = time(NULL);						/*���±����������ʱ��*/
	}

	return found;
}

void arp_add_entry(__u32 ip, __u8 *ethaddr, int status)
{
	int i = 0;
	struct arpt_arp  *found = NULL;
	found = update_arp_entry(ip, ethaddr);			/*����ARP����*/
	if(!found)										/*���²��ɹ�*/
	{												/*����һ���հױ��ӳ���д��*/
		for(i = 0; i<ARP_TABLE_SIZE; i++)
		{
			if(arp_table[i].status == ARP_EMPTY)		/*ӳ����Ϊ��*/
			{
				found = &arp_table[i];					/*����found����*/
				break;								/*�˳�����*/
			}
		}
	}

	if(found){										/*�Դ�����и���*/
		found->ipaddr = ip;							/*IP��ַ����*/
		memcpy(found->ethaddr, ethaddr, ETH_ALEN);	/*MAC��ַ����*/
		found->status = status;						/*״̬����*/
		found->ctime = time(NULL);						/*������ʱ�����*/
	}
}


struct skbuff *arp_create(struct net_device *dev, 		/*�豸*/
							int type, 					/*ARPЭ�������*/
							__u32 src_ip,				/*Դ����IP*/
							__u32 dest_ip,			/*Ŀ������IP*/
							__u8* src_hw,			/*Դ����MAC*/							
							__u8* dest_hw, 			/*Ŀ������MAC*/							
							__u8* target_hw)			/*����������MAC*/
{
	struct skbuff *skb;
	struct sip_arphdr *arph;
	DBGPRINT(DBG_LEVEL_TRACE,"==>arp_create\n");

	/*����skbuff�ṹ�ڴ�,��СΪһ����С����̫��֡,60�ֽ�*/	
	skb = skb_alloc(ETH_ZLEN);
	if (skb == NULL)									/*����ʧ��*/
	{
		goto EXITarp_create;							/*�˳�*/
	}

	skb->phy.raw = skb_put(skb,sizeof(struct sip_ethhdr));	/*���������ͷ��ָ��λ��*/
	skb->nh.raw = skb_put(skb,sizeof(struct sip_arphdr));	/*���������ͷ��ָ��λ��*/
	arph = skb->nh.arph;								/*����ARPͷ��ָ��,���ڲ���*/
	skb->dev = dev;									/*���������豸ָ��*/
	if (src_hw == NULL)								/*��̫��Դ��ַΪ��*/
		src_hw = dev->hwaddr;						/*Դ��ַ����Ϊ�����豸��Ӳ����ַ*/
	if (dest_hw == NULL)								/*��̫��Ŀ�ĵ�ַΪ��*/
		dest_hw = dev->hwbroadcast;					/*Ŀ�ĵ�ַ����Ϊ��̫���㲥Ӳ����ַ*/

	skb->phy.ethh->h_proto = htons(ETH_P_ARP);			/*���������Э������ΪARPЭ��*/
	memcpy(skb->phy.ethh->h_dest, dest_hw, ETH_ALEN);	/*���ñ��ĵ�Ŀ��Ӳ����ַ*/
	memcpy(skb->phy.ethh->h_source, src_hw, ETH_ALEN);/*���ñ��ĵ�ԴӲ����ַ*/

	arph->ar_op = htons(type);							/*����ARP��������*/
	arph->ar_hrd = htons(ETH_P_802_3);					/*����ARP��Ӳ����ַ����Ϊ802.3*/
	arph->ar_pro =  htons(ETH_P_IP);					/*����ARP��Э���ַ����ΪIP*/
	arph->ar_hln = ETH_ALEN;							/*����ARPͷ����Ӳ����ַ����Ϊ6*/
	arph->ar_pln = 4;									/*����ARPͷ����Э���ַ����Ϊ4*/

	memcpy(arph->ar_sha, src_hw, ETH_ALEN);			/*ARP���ĵ�ԴӲ����ַ*/
	memcpy(arph->ar_sip,  (__u8*)&src_ip, 4);			/*ARP���ĵ�ԴIP��ַ*/
	memcpy(arph->ar_tip, (__u8*)&dest_ip, 4);			/*ARP���ĵ�Ŀ��IP��ַ*/
	if (target_hw != NULL)							/*���Ŀ��Ӳ����ַ��Ϊ��*/
		memcpy(arph->ar_tha, target_hw, dev->hwaddr_len);/*ARP���ĵ�Ŀ��Ӳ����ַ*/
	else												/*û�и���Ŀ��Ӳ����ַ*/
		memset(arph->ar_tha, 0, dev->hwaddr_len);		/*Ŀ��Ӳ����ַ����*/
EXITarp_create:	
	DBGPRINT(DBG_LEVEL_TRACE,"<==arp_create\n");
	return skb;
	
}



void arp_send(struct net_device *dev, 				/*�豸*/
				int type, 								/*ARPЭ�������*/
				__u32 src_ip,							/*Դ����IP*/
				__u32 dest_ip,						/*Ŀ������IP*/
				__u8* src_hw,						/*Դ����MAC*/							
				__u8* dest_hw, 						/*Ŀ������MAC*/							
				__u8* target_hw)						/*����������MAC*/
{
	struct skbuff *skb;
	DBGPRINT(DBG_LEVEL_TRACE,"==>arp_send\n");
	/*����һ��ARP���籨��*/
	skb = arp_create(dev,type,src_ip,dest_ip,src_hw,dest_hw,target_hw);
	if(skb)											/*�����ɹ�*/
	{
		dev->linkoutput(skb, dev);						/*���õײ�����緢�ͺ���*/
	}
	DBGPRINT(DBG_LEVEL_TRACE,"<==arp_send\n");
}

int arp_request(struct net_device *dev, __u32 ip)
{
	struct skbuff *skb;
	DBGPRINT(DBG_LEVEL_TRACE,"==>arp_request\n");
	__u32 tip = 0;
	/*�鿴�����IP��ַ�ͱ���IP��ַ�Ƿ���ͬһ��������*/
	if( (ip & dev->ip_netmask.s_addr) 					/*�����IP��ַ*/
		== 											/*ͬһ����*/
		(dev->ip_host.s_addr & dev->ip_netmask.s_addr ) )/*������IP��ַ*/
	{
		tip = ip;										/*ͬһ����,��IPΪĿ��IP*/
	}
	else												/*��ͬ����*/
	{
		tip = dev->ip_gw.s_addr;						/*Ŀ��IPΪ���ص�ַ*/
	}
	/*����һ��ARP������,���е�Ŀ��IPΪ������ַ*/
	skb = arp_create(dev,
					ARPOP_REQUEST,
					dev->ip_host.s_addr,
					tip,
					dev->hwaddr,
					NULL,
					NULL);
	if(skb)											/*����skbuff�ɹ�*/
	{
		dev->linkoutput(skb, dev);						/*ͨ���ײ����纯������*/
	}
	DBGPRINT(DBG_LEVEL_TRACE,"<==arp_request\n");
}
int arp_input(struct skbuff **pskb, struct net_device *dev)
{
	struct skbuff *skb = *pskb;

	__be32 ip = 0;
	DBGPRINT(DBG_LEVEL_TRACE,"==>arp_input\n");
	if(skb->tot_len < sizeof(struct sip_arphdr))			/*���յ������������ܳ���С��ARPͷ������*/
	{
		goto EXITarp_input;							/*����,����*/
	}
	
	ip = *(__be32*)(skb->nh.arph->ar_tip) ;				/*ARP�����Ŀ�ĵ�ַ*/
	if(ip == dev->ip_host.s_addr)						/*Ϊ����IP?*/
	{
		update_arp_entry(ip, dev->hwaddr);			/*����ARP��*/
	}

	switch(ntohs(skb->nh.arph->ar_op))					/*�鿴ARPͷ��Э������*/
	{
		case ARPOP_REQUEST:							/*ARP��������*/
		{
			struct in_addr t_addr;
			t_addr.s_addr = *(unsigned int*)skb->nh.arph->ar_sip;/*ARP����ԴIP��ַ*/
			DBGPRINT(DBG_LEVEL_ERROR,"ARPOP_REQUEST, FROM:%s\n",inet_ntoa(t_addr));

			/*��ARP�����IP��ַ����Ӧ��*/
			arp_send(dev, 
					ARPOP_REPLY, 
					dev->ip_host.s_addr,
					*(__u32*)skb->nh.arph->ar_sip, 
					dev->hwaddr,
					skb->phy.ethh->h_source, 
					skb->nh.arph->ar_sha);
			/*������ARPӳ�����ݼ���ӳ���*/
			arp_add_entry(*(__u32*)skb->nh.arph->ar_sip, skb->phy.ethh->h_source, ARP_ESTABLISHED);
		}
			break;
		case ARPOP_REPLY:							/*ARPӦ������*/
			/*������ARPӳ�����ݼ���ӳ���*/
			arp_add_entry(*(__u32*)skb->nh.arph->ar_sip, skb->phy.ethh->h_source, ARP_ESTABLISHED);
			break;
	}
	DBGPRINT(DBG_LEVEL_TRACE,"<==arp_input\n");
EXITarp_input:
	return;
}


