#include "sip.h"

static struct net_device ifdevice;
struct net_device *get_netif()
{
	return &ifdevice;
}
void DISPLAY_MAC(struct ethhdr* eth)
{
	printf("From:%02x-%02x-%02x-%02x-%02x-%02x          ",
		eth->h_source[0],		eth->h_source[1],		eth->h_source[2],
		eth->h_source[3],		eth->h_source[4],		eth->h_source[5]);
	printf("to:%02x-%02x-%02x-%02x-%02x-%02x\n",
		eth->h_dest[0],		eth->h_dest[1],		eth->h_dest[2],
		eth->h_dest[3],		eth->h_dest[4],		eth->h_dest[5]);

}
static __u8 input(struct skbuff *pskb, struct net_device *dev)
{
	DBGPRINT(DBG_LEVEL_TRACE,"==>input\n");
	char ef[ETH_FRAME_LEN];  							/*��̫֡������,1514�ֽ�*/
	int n,i;			
	int retval = 0;

	/*��ȡ��̫�����ݣ�nΪ���ص�ʵ�ʲ������̫֡��֡��*/
	n = read(dev->s, ef, ETH_FRAME_LEN);   
	if(n <=0)											/*û�ж�������*/
	{
		DBGPRINT(DBG_LEVEL_ERROR,"Not datum\n");
		retval = -1;
		goto EXITinput;								/*�˳�*/
	}
	else												/*��������*/
	{
		DBGPRINT(DBG_LEVEL_NOTES,"%d bytes datum\n", n);
	};	

	struct skbuff *skb = skb_alloc(n);					/*�����ŸղŶ�ȡ�����ݵĿռ�*/
	if(!skb)											/*����ʧ��*/
	{
		retval = -1;
		goto EXITinput;								/*�˳�*/
	}
	
	memcpy(skb->head, ef, n);							/*�����յ����������ݿ�����skb�ṹ*/
	skb->tot_len =skb->len= n;							/*���ó���ֵ*/
	skb->phy.ethh= (struct sip_ethhdr*)skb_put(skb, sizeof(struct sip_ethhdr));/*�����̫��ͷ��ָ��*/
	if(samemac(skb->phy.ethh->h_dest, dev->hwaddr) 	/*���ݷ�������?*/
		|| samemac(skb->phy.ethh->h_dest, dev->hwbroadcast))/*�㲥����?*/
	{
		switch(htons(skb->phy.ethh->h_proto))			/*�鿴��̫��Э������*/
		{
			case ETH_P_IP:							/*IP����*/
				skb->nh.iph = (struct sip_iphdr*)skb_put(skb, sizeof(struct sip_iphdr));/*���IPͷ��ָ��*/
				/*���ղŽ��յ�������������������ARP���е�ӳ���ϵ*/
				arp_add_entry(skb->nh.iph->saddr, skb->phy.ethh->h_source, ARP_ESTABLISHED);
				
				ip_input(dev, skb);					/*����IP�㴦������*/
				break;
			case ETH_P_ARP:							/*ARP����*/
			{
				skb->nh.arph = (struct sip_arphdr*)skb_put(skb, sizeof(struct sip_arphdr));/*���ARPͷ��ָ��*/								
				if(*((__be32*)skb->nh.arph->ar_tip) == dev->ip_host.s_addr)	/*Ŀ��IP��ַΪ����?*/
				{					
					arp_input(&skb, dev);				/*ARPģ�鴦����յ���ARP����*/
				}
				skb_free(skb);						/*�ͷ��ڴ�*/
			}
				break;
			default:									/*Ĭ�ϲ���*/
				DBGPRINT(DBG_LEVEL_ERROR,"ETHER:UNKNOWN\n");
				skb_free(skb);						/*�ͷ��ڴ�*/
				break;
		}
	}
	else
	{
		skb_free(skb);								/*�ͷ��ڴ�*/
	}

EXITinput:
	DBGPRINT(DBG_LEVEL_TRACE,"<==input\n");
	return 0;
}

static __u8 output(struct skbuff *skb, struct net_device *dev)
{
	DBGPRINT(DBG_LEVEL_TRACE,"==>output\n");
	int retval = 0;
	
	struct arpt_arp  *arp = NULL;
	int times = 0,found = 0;

	/*�����������ݵ�Ŀ��IP��ַΪskb��ָ��Ŀ�ĵ�ַ*/
	__be32 destip = skb->nh.iph->daddr;						
	/*�ж�Ŀ�������ͱ����Ƿ���ͬһ��������*/
	if((skb->nh.iph->daddr & dev->ip_netmask.s_addr )			
		!= (dev->ip_host.s_addr & dev->ip_netmask.s_addr))
	{
		destip = dev->ip_gw.s_addr;		/*����ͬһ��������,�����ݷ��͸�����*/
	}
	/*��5�β���Ŀ��������MAC��ַ*/
	while((arp = 	arp_find_entry(destip)) == NULL && times < 5)	/*����MAC��ַ*/
	{
		arp_request(dev,destip);			/*û���ҵ�,����ARP����*/
		sleep(1);							/*��һ��*/
		times ++;						/*��������*/
	}

	if(!arp)								/*û���ҵ���Ӧ��MAC��ַ*/
	{
		retval = 1;
		goto EXIToutput;
	}	
	else									/*�ҵ�һ����Ӧ��*/
	{
		struct sip_ethhdr *eh = skb->phy.ethh;
		memcpy(eh->h_dest, arp->ethaddr, ETH_ALEN);	/*����Ŀ��MAC��ַΪ����ֵ*/
		memcpy(eh->h_source, dev->hwaddr, ETH_ALEN);	/*����ԴMAC��ַΪ����MACֵ*/
		eh->h_proto = htons(ETH_P_IP);					/*��̫����Э����������ΪIP*/
		dev->linkoutput(skb,dev);						/*��������*/
	}
EXIToutput:
	DBGPRINT(DBG_LEVEL_TRACE,"<==output\n");
	return retval;
}

static __u8 lowoutput(struct skbuff *skb, struct net_device *dev)
{
	DBGPRINT(DBG_LEVEL_TRACE,"==>lowoutput\n");

	int n = 0;
	int len = sizeof(struct sockaddr);
	struct skbuff *p =NULL;
	/*��skbuff���ṹ�е��������ݷ��ͳ�ȥ*/
	for(p=skb;												/*��skbuff�ĵ�һ���ṹ��ʼ*/
		p!= NULL;											/*��ĩβһ������*/
		skb= p, p=p->next, skb_free(skb),skb=NULL)			/*������һ�����ݱ��ĺ��ƶ�ָ�벢�ͷŽṹ�ڴ�*/
	{
		n = sendto(dev->s, skb->head, skb->len,0, &dev->to, len);/*������������*/
		DBGPRINT(DBG_LEVEL_NOTES,"Send Number, n:%d\n",n);
	}

	DBGPRINT(DBG_LEVEL_TRACE,"<==lowoutput\n");

	return 0;
}

static void sip_init_ethnet(struct net_device *dev)
{
	DBGPRINT(DBG_LEVEL_TRACE, "==>sip_init_ethnet\n");

	memset(dev, 0, sizeof(struct net_device));					/*��ʼ�������豸*/

	dev->s = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ALL));	/*����һ��SOCK_PACKET�׽���*/
	if(dev->s > 0)											/*�ɹ�*/
	{
		DBGPRINT(DBG_LEVEL_NOTES,"create SOCK_PACKET fd success\n");
	}
	else														/*ʧ��*/
	{
		DBGPRINT(DBG_LEVEL_ERROR,"create SOCK_PACKET fd falure\n");
		exit(-1);
	}
	/*�����׽��ְ󶨵�����eth1��*/
	strcpy(dev->name, "eth1");									/*����eth1��name*/
	memset(&dev->to, '\0', sizeof(struct sockaddr));				/*����to��ַ�ṹ*/
	dev->to.sa_family = AF_INET;								/*Э����*/
	strcpy(dev->to.sa_data, dev->name);  						/*to����������*/
	int r = bind(dev->s, &dev->to, sizeof(struct sockaddr));		/*���׽���s��eth1��*/
	
	memset(dev->hwbroadcast, 0xFF, ETH_ALEN);					/*������̫���Ĺ㲥��ַ*/
#if 0	
	dev->hwaddr[0] = 0x00;
	dev->hwaddr[1] = 0x12;
	dev->hwaddr[2] = 0x34;
	dev->hwaddr[3] = 0x56;
	dev->hwaddr[4] = 0x78;
	dev->hwaddr[5] = 0x90;
#else
	/*����MAC��ַ*/
	dev->hwaddr[0] = 0x00;			
	dev->hwaddr[1] = 0x0c;
	dev->hwaddr[2] = 0x29;
	dev->hwaddr[3] = 0x73;
	dev->hwaddr[4] = 0x9D;
	dev->hwaddr[5] = 0x1F;
#endif
	dev->hwaddr_len = ETH_ALEN;							/*����Ӳ����ַ����*/
#if 0
	dev->ip_host.s_addr = inet_addr("192.168.1.250");
	dev->ip_gw.s_addr = inet_addr("192.168.1.1");
	dev->ip_netmask.s_addr = inet_addr("255.255.255.0");
	dev->ip_broadcast.s_addr = inet_addr("192.168.1.255");
	
	dev->ip_host.s_addr = inet_addr("10.10.10.250");
	dev->ip_gw.s_addr = inet_addr("10.10.10..1");
	dev->ip_netmask.s_addr = inet_addr("255.255.255.0");
	dev->ip_broadcast.s_addr = inet_addr("10.10.10.255");
#else
	dev->ip_host.s_addr = inet_addr("172.16.12.250");		/*���ñ���IP��ַ*/
	dev->ip_gw.s_addr = inet_addr("172.16.12.1");		/*���ñ���������IP��ַ*/
	dev->ip_netmask.s_addr = inet_addr("255.255.255.0");	/*���ñ��������������ַ*/
	dev->ip_broadcast.s_addr = inet_addr("172.16.12.255");/*���ñ����Ĺ㲥IP��ַ*/
#endif
	dev->input = input;								/*�һ���̫�����뺯��*/
	dev->output = output;							/*�ҽ���̫���������*/
	dev->linkoutput = lowoutput;						/*�ҽӵײ��������*/
	dev->type = ETH_P_802_3;							/*�豸������*/
	DBGPRINT(DBG_LEVEL_TRACE,"<==sip_init_ethnet\n");
}

struct net_device * sip_init(void)
{
	sip_init_ethnet(&ifdevice);						/*��ʼ�������豸ifdevice*/
	
	return &ifdevice;									/*����ifdevice*/
}

