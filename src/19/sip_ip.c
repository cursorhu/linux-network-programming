#include "sip.h"

inline int IP_IS_BROADCAST(struct net_device *dev, __be32 ip)
{
	int retval = 1;
	
	if((ip == IP_ADDR_ANY_VALUE) 						/*IP��ַΪ��������IP��ַ*/
		||(~ip == IP_ADDR_ANY_VALUE))				/*����Ϊ��λȡ��IP��ַ*/
	{
		DBGPRINT(DBG_LEVEL_NOTES, "IP is ANY ip\n");
		retval = 1;									/*�ǹ㲥��ַ*/
		goto EXITin_addr_isbroadcast;					/*�˳�*/
	}else if(ip == dev->ip_host.s_addr)	{				/*IP��ַΪ���ص�ַ*/
		DBGPRINT(DBG_LEVEL_NOTES, "IP is local ip\n");
		retval = 0;									/*���ǹ㲥��ַ*/
		goto EXITin_addr_isbroadcast;					/*�˳�*/
	}else if(((ip&dev->ip_netmask.s_addr) 				/*IP��ַΪ�������ڵ�ַ*/
					== (dev->ip_host.s_addr &dev->ip_netmask.s_addr))
		&& ((ip & ~dev->ip_netmask.s_addr) 			/*��㲥��ַͬ����*/
					==(IP_ADDR_BROADCAST_VALUE & ~dev->ip_netmask.s_addr))){
		DBGPRINT(DBG_LEVEL_NOTES, "IP is ANY ip\n");
		retval =1;									/*�ǹ㲥��ַ*/
		goto EXITin_addr_isbroadcast;					/*�˳�*/
	}else{											/*���ǹ㲥IP��ַ*/
		retval = 0;
	}
EXITin_addr_isbroadcast:
	return retval;
}

#define IP_FREE_REASS(ipr)				\
do{										\
	struct skbuff *skb=NULL,*skb_prev=NULL;	\
	for(skb_prev = skb = ipr->skb; 			\
		skb != NULL; 						\
		skb_prev = skb, 					\
		skb = skb->next,					\
		skb_free(skb_prev));				\
		free(ipr);						\
}while(0);


#define IPREASS_TIMEOUT 3							/*IP��������ĳ�ʱʱ��Ϊ3��*/
static struct sip_reass *ip_reass_list = NULL;				/*IP���������*/
struct skbuff *sip_reassemble(struct skbuff* skb)
{
	struct sip_iphdr *fraghdr = skb->nh.iph;
	int retval = 0;
	__u16 offset, len;
	int found = 0;

	offset = (fraghdr->frag_off & 0x1FFF)<<3;				/*ȡ��IP����ƫ�Ƶ�ַ,32λ��*/
	len = fraghdr->tot_len - fraghdr->ihl<<2;				/*IP��������ݳ���*/

	struct sip_reass *ipr = NULL,*ipr_prev = NULL;	
	for(ipr_prev = ipr= ip_reass_list; ipr != NULL;	)	
	{
		if(time(NULL) -ipr->timer > IPREASS_TIMEOUT)	/*�˷����ǳ�ʱ?*/
		{
			if(ipr_prev == NULL)						/*��һ����Ƭ?*/
			{
				ipr_prev = ipr;						/*�����ػ���ָ��Ϊ������*/
				ip_reass_list->next = ipr = ipr->next;	/*����ʱ�ķ�Ƭ������������ȡ����*/
				ipr = ipr->next;						/*���µ�ǰ�ķ���ָ��*/
				IP_FREE_REASS(ipr_prev);				/*�ͷ���Դ*/
				ipr_prev->next =NULL;					/*����ָ��Ϊ��*/

				continue;							/*�������Һ��ʵķ���*/
			}
			else										/*���ǵ�һ������*/
			{
				ipr_prev->next = ipr->next;				/*�ӷ�Ƭ������ժ����ǰ��*/
				IP_FREE_REASS(ipr);					/*�ͷŵ�ǰ������*/
				ipr = ipr_prev->next;					/*���µ�ǰ����ָ��*/
				continue;							/*��������*/
			}			
		}

		/*��Ƭ�Ƿ����������*/
		if(ipr->iphdr.daddr == fraghdr->daddr 			/*Ŀ��IP��ַƥ��*/
			&&ipr->iphdr.saddr == fraghdr->saddr		/*ԴIP��ַƥ��*/
			&&ipr->iphdr.id == fraghdr->id)				/*��Ƭ��IDƥ��*/
		{
			found = 1;								/*����������*/
			break;
		}
	}

	if(!found)										/*û���ҵ����ʵķ�����?*/
	{
		ipr_prev = NULL;								/*��ʼ��Ϊ��*/
		ipr = (struct sip_reass*)malloc(sizeof(struct sip_reass));/*����һ���������ݽṹ*/
		if(!ipr)										/*����ʧ��*/
		{
			retval = -1;								/*����ֵ-1*/
			goto freeskb;							/*�˳�*/
		}

		memset(ipr, 0, sizeof(struct sip_reass));			/*��ʼ������ṹ*/

		ipr->next = ip_reass_list;						/*����ǰ����ṹ�ҽӵ���������ͷ��*/
		ip_reass_list = ipr;	

		memcpy(&ipr->iphdr, skb->nh.raw, sizeof(IPHDR_LEN));/*����IP������ͷ��,����֮��ķ�Ƭƥ��*/
	}else{											/*�ҵ����ʵķ�����*/
		if(((fraghdr->frag_off & 0x1FFF) == 0)			/*��ǰ����λ�ڷ�Ƭ��һ��*/
			&&((ipr->iphdr.frag_off & 0x1FFF) != 0))		/*�������ϵ�ͷ�����ǵ�һ����Ƭ*/
		{
			memcpy(&ipr->iphdr, fraghdr, IPHDR_LEN);	/*���������е�IPͷ���ṹ*/
		}		
	}

	/* ����Ƿ�Ϊ���һ������*/
	if( (fraghdr->frag_off & htons(0x2000)) == 0) {		/*û�и������*/
		#define IP_REASS_FLAG_LASTFRAG 0x01		

	  	ipr->flags |= IP_REASS_FLAG_LASTFRAG;			/*�����������־*/
		ipr->datagram_len = offset + len;				/*����IP���ݱ��ĵ�ȫ��*/
  	}


	/*����ǰ�����ݷŵ���������,������״̬*/
	struct skbuff *skb_prev=NULL, *skb_cur=NULL;
	int finish =0;
	void *pos = NULL;
	__u32 length = 0;
#define FRAG_OFFSET(iph) (ntohs(iph->frag_off & 0x1FFF)<<3)
#define FRAG_LENGTH(iph) (ntohs(iph->tot_len) - IPHDR_LEN)
	for(skb_prev =NULL, skb_cur=ipr->skb,length = 0,found = 0;
		skb_cur != NULL && !found;
		skb_prev=skb_cur,skb_cur = skb_cur->next)
	{
		if(skb_prev !=NULL)									/*���ǵ�һ����Ƭ*/
		{
			if((offset  < FRAG_OFFSET(skb_cur->nh.iph))			/*�������ݵ�ƫ��ֵλ��ǰ������֮��*/
				&&(offset > FRAG_OFFSET(skb_prev->nh.iph)))
			{				
				skb->next = skb_cur;							/*�����յ������ݷŵ���λ��*/
				skb_prev->next = skb;

				if(offset + len > FRAG_OFFSET(skb_cur->nh.iph))	/*��ǰ���������ķ�Ƭ���ݸ���?*/
				{
					__u16 modify = FRAG_OFFSET(skb_cur->nh.iph) - offset + IPHDR_LEN;/*���㵱ǰ�������ݳ����޸�ֵ*/
					skb->nh.iph->tot_len = htons(modify);		/*���µ�ǰ������*/
				}

				if(FRAG_OFFSET(skb_prev->nh.iph) 				/*ǰ��ķ�Ƭ���ȸ��ǵ�ǰ����?*/
					+ FRAG_LENGTH(skb_prev->nh.iph) 
						> FRAG_OFFSET(skb_cur->nh.iph))
				{
					__u16 modify = FRAG_OFFSET(skb_prev->nh.iph) - offset + IPHDR_LEN;/*����ǰ�����ݳ��ȵĸ���֮*/
					skb_prev->nh.iph->tot_len = htons(modify);	/*�޸�ǰһƬ�����ݳ���*/
				}

				found = 1;									/*�ҵ����ʵķ�Ƭ����λ��*/
			}
		}
		else													/*Ϊ�������ϵ�ͷ��*/
		{
			if(offset  < FRAG_OFFSET(skb_cur->nh.iph)){			/*��ǰ����ƫ����С�ڵ�һ����Ƭ��ƫ�Ƴ���*/
				skb->next = ipr->skb;							/*�ҽӵ���������ͷ��*/
				ipr->skb = skb;								
				if(offset + len + IPHDR_LEN 						/*�鿴�Ƿ񸲸Ǻ����Ƭ������*/
					> FRAG_OFFSET(skb_cur->nh.iph))
					{
					__u16 modify = FRAG_OFFSET(skb_cur->nh.iph) - offset + IPHDR_LEN;/*�޸ķ�Ƭ�����ݳ���*/
					if(!offset)								/*ƫ����Ϊ0*/
						modify -= IPHDR_LEN;					/*����ͷ��,�������ݶγ�����Ҫ��ȥIPͷ������*/

					skb->nh.iph->tot_len = htons(modify);		/*���÷�Ƭ���޸ĺ�ĳ���*/
				}
			}
		}
		
		length += skb_cur->nh.iph->tot_len - IPHDR_LEN;			/*��ǰ�����е����ݳ���*/
	}

	/*���¼����������ϵ������ݳ���*/
	for(skb_cur=ipr->skb,length = 0;
		skb_cur != NULL;
		skb_cur = skb_cur->next)
	{
		length += skb_cur->nh.iph->tot_len - IPHDR_LEN;
	}
	length += IPHDR_LEN;
	
	/*ȫ����IP��Ƭ���Ѿ����յ���������ݱ��ĵ��������
	���ݿ�����һ���µ����ݽṹ�У�ԭ�������ݽ��ն��ͷŵ�
	���ӷ�������ȡ���������������ݽṹָ�뷵��*/
	if(length == ipr->datagram_len )							/*����ȫ�����յ�?*/
	{
		ipr->datagram_len += IPHDR_LEN;					/*�������ݱ��ĵ�ʵ�ʳ��ȳ���*/
		skb = skb_alloc(ipr->datagram_len + ETH_HLEN);		/*����ռ�*/

		skb->phy.raw = skb_put(skb, ETH_HLEN);			/*�����*/
		skb->nh.raw = skb_put(skb, IPHDR_LEN);			/*�����*/
		memcpy(skb->nh.raw, & ipr->iphdr, sizeof(ipr->iphdr));	/*�������ݽṹ�п���IPͷ*/
		skb->nh.iph->tot_len = htons(ipr->datagram_len);		/*�½ṹ�е�tot_len*/
		
		for(skb_prev=skb_cur=ipr->skb;skb_cur != NULL;)		/*��������������*/
		{
			int size = skb_cur->end - skb_cur->tail;			/*���㿽������Դ�ĳ���*/
			pos = skb_put(skb, size);						/*���㿽��Ŀ�ĵ�ַλ��*/
			memcpy(pos, 									/*��һ����Ƭ�������½ṹ��*/
				skb_cur->tail, 
				skb_cur->nh.iph->tot_len - skb_cur->nh.iph->ihl<<2);
		}

		/*һ�´���������ժ�����ݲ��ͷ�,Ȼ�������½ṹ�еļ���IPͷ������*/
		ipr_prev->next = ipr->next;							/*�������ݱ��Ĵ���������ժ��*/
		IP_FREE_REASS(ipr);								/*�ͷŴ˱��ĵ�������*/
		skb->nh.iph->check = 0;							/*����У��ֵΪ0*/
		skb->nh.iph->frag_off = 0;							/*ƫ��ֵΪ0*/
		skb->nh.iph->check = SIP_Chksum(skb->nh.raw, skb->nh.iph->tot_len);/*����IPͷ��У���*/
	}
	
normal:
	return skb;
freeskb:
	skb_free(skb);
	return NULL;
}

struct skbuff * ip_frag(struct net_device *dev, struct skbuff *skb)
{
	__u8 frag_num = 0;
	__u16 tot_len = ntohs(skb->nh.iph->tot_len);
	__u8 mtu = dev->mtu;
	__u8 half_mtu = (mtu+1)/2;
	frag_num = (tot_len - IPHDR_LEN + half_mtu)/(mtu - IPHDR_LEN - ETH_HLEN);/*�����Ƭ�ĸ���*/

	__u16 i = 0;
	struct skbuff *skb_h = NULL,*skb_t = NULL,*skb_c = NULL;
	for(i = 0,skb->tail = skb->head; i<frag_num;i++)
	{
		if(i ==0){													/*��һ����Ƭ*/
			skb_t = skb_alloc(mtu);									/*�����ڴ�*/
			skb_t->phy.raw = skb_put(skb_t, ETH_HLEN);					/*�����*/
			skb_t->nh.raw = skb_put(skb_t, IPHDR_LEN);					/*�����*/
			
			memcpy(skb_t->head, skb->head, mtu);						/*��������*/
			skb_put(skb,mtu);										/*�������ݳ���lenֵ*/
			skb_t->nh.iph->frag_off = htons(0x2000);						/*����ƫ�Ʊ��ֵ*/
			skb_t->nh.iph->tot_len = htons(mtu-ETH_HLEN);				/*����IPͷ���ܳ���*/
			skb_t->nh.iph->check = 0;									/*����У���Ϊ0*/
			skb_t->nh.iph->check = SIP_Chksum(skb_t->nh.raw, IPHDR_LEN);/*����У���*/

			skb_h = skb_c =skb_t;										/*ͷ����Ƭָ������*/
		}else if(i==frag_num -1){										/*���һ����Ƭ*/
			skb_t = skb_alloc(mtu);									/*�����ڴ�*/
			skb_t->phy.raw = skb_put(skb_t, ETH_HLEN);					/*�����*/
			skb_t->nh.raw = skb_put(skb_t, IPHDR_LEN);					/*�����*/
			
			memcpy(skb_t->head, skb->head, ETH_HLEN + IPHDR_LEN);		/*��������*/
			memcpy(skb_t->head + ETH_HLEN + IPHDR_LEN, skb->tail, skb->end - skb->tail);/*�������ݳ���lenֵ*/
			skb_t->nh.iph->frag_off = htons(i*(mtu - ETH_HLEN - IPHDR_LEN) + IPHDR_LEN);/*����ƫ�Ʊ��ֵ*/
			skb_t->nh.iph->tot_len = htons(skb->end - skb->tail + IPHDR_LEN);/*����IPͷ���ܳ���*/
			skb_t->nh.iph->check = 0;									/*����У���Ϊ0*/
			skb_t->nh.iph->check = SIP_Chksum(skb_t->nh.raw, IPHDR_LEN);/*����У���*/

			skb_c->next=skb_t;										/*�ҽӴ˷�Ƭ*/
		}else{
			skb_t = skb_alloc(mtu);
			skb_t->phy.raw = skb_put(skb_t, ETH_HLEN);
			skb_t->nh.raw = skb_put(skb_t, IPHDR_LEN);
			
			memcpy(skb_t->head, skb->head, ETH_HLEN + IPHDR_LEN);
			memcpy(skb_t->head + ETH_HLEN + IPHDR_LEN, skb->tail, mtu - ETH_HLEN - IPHDR_LEN);
			skb_put(skb_t, mtu - ETH_HLEN - IPHDR_LEN);
			skb_t->nh.iph->frag_off = htons((i*(mtu - ETH_HLEN - IPHDR_LEN) + IPHDR_LEN)|0x2000);
			skb_t->nh.iph->tot_len = htons(mtu - ETH_HLEN);
			skb_t->nh.iph->check = 0;
			skb_t->nh.iph->check = SIP_Chksum(skb_t->nh.raw, IPHDR_LEN);

			skb_c->next=skb_t;
			skb_c = skb_t;
		}
		skb_t->ip_summed = 1;										/*�Ѿ�������IPУ��ͼ���*/
	}

	skb_free(skb);													/*�ͷ�ԭ������������*/
	return skb_h;													/*���ط�Ƭ��ͷ��ָ��*/
}

int ip_input(struct net_device *dev, struct skbuff *skb)
{
	DBGPRINT(DBG_LEVEL_TRACE,"==>ip_input\n");
	struct sip_iphdr *iph = skb->nh.iph;
	int retval = 0;

	if(skb->len < 0)									/*�������ݳ��Ȳ��Ϸ�*/
	{	
		skb_free(skb);								/*�ͷŽṹ*/
		retval = -1;									/*���÷���ֵ*/
		goto EXITip_input;							/*�˳�*/
	}

	if(iph->version != 4)								/*IP�汾������,����IPv4*/
	{
		skb_free(skb);								
		retval = -1;									
		goto EXITip_input;							
	}

	__u16 hlen = iph->ihl<<2;							/*����IPͷ������*/
	if(hlen < IPHDR_LEN)								/*���ȹ�С*/
	{
		skb_free(skb);								
		retval = -1;								
		goto EXITip_input;							
	}

	if(skb->tot_len - ETH_HLEN < ntohs(iph->tot_len))		/*�����ܳ����Ƿ�Ϸ�*/
	{
		skb_free(skb);
		retval = -1;
		goto EXITip_input;
	}


	if(hlen < ntohs(iph->tot_len))						/*ͷ�������Ƿ�Ϸ�*/
	{
		skb_free(skb);
		retval = -1;
		goto EXITip_input;
	}

	if(SIP_Chksum(skb->nh.raw, IPHDR_LEN))			/*����IPͷ����У���,�Ƿ���ȷ,Ϊ0*/
	{
		DBGPRINT(DBG_LEVEL_ERROR, "IP check sum error\n");
		skb_free(skb);
		retval= -1;
		goto EXITip_input;
	}
	else												/*У��ͺϷ�*/
	{
		skb->ip_summed = CHECKSUM_HW;				/*����IPУ����*/
		DBGPRINT(DBG_LEVEL_NOTES, "IP check sum success\n");
	}

	if((iph->daddr != dev->ip_host.s_addr 				/*���Ƿ�������*/
		&& !IP_IS_BROADCAST(dev, iph->daddr)		/*Ŀ�ĵ�ַ���ǹ㲥��ַ*/
		||IP_IS_BROADCAST(dev, iph->saddr)))		/*Դ��ַ���ǹ㲥��ַ*/
	{
		DBGPRINT(DBG_LEVEL_NOTES, "IP address INVALID\n");
		skb_free( skb);
		retval= -1;
		goto EXITip_input;
	}

	if((ntohs(iph->frag_off) & 0x3FFF) !=0)				/*��ƫ��,��һ����Ƭ*/
	{
		skb = sip_reassemble(skb);					/*���з�Ƭ����*/
		if(!skb){									/*���鲻�ɹ�*/
			retval = 0;
			goto EXITip_input;
		}
	}
	
	switch(iph->protocol)								/*IPЭ������*/
	{
		case IPPROTO_ICMP:							/*ICMPЭ��*/
			skb->th.icmph = 							/*ICMPͷ��ָ���ȡ*/
				(struct sip_icmphdr*)skb_put(skb, sizeof(struct sip_icmphdr));
			icmp_input(dev, skb);					/*ת��ICMPģ�鴦��*/
			break;
		case IPPROTO_UDP:							/*UDPЭ��*/
			skb->th.udph = 							/*UDPͷ��ָ���ȡ*/
				(struct sip_udphdr*)skb_put(skb, sizeof(struct sip_udphdr));
			SIP_UDPInput(dev, skb);					/*ת��UDPģ�鴦��*/
			break;
		default:
			break;
	}
EXITip_input:	
	DBGPRINT(DBG_LEVEL_TRACE,"<==ip_input\n");
	return retval;
}

int ip_output(struct net_device *dev, struct skbuff *skb,
	struct in_addr *src, struct in_addr *dest,
          __u8 ttl, __u8 tos, __u8 proto)
{
	struct sip_iphdr *iph = skb->nh.iph;					/*IPͷ��ָ����*/	
	iph->protocol = proto;								/*Э����������*/
	iph->tos = tos;									/*������������*/
	iph->ttl = ttl;										/*����ʱ������*/

	iph->daddr = dest->s_addr;						/*����Ŀ��IP��ַ*/
	iph->saddr = src->s_addr;							/*����ԴIP��ַ*/

	iph->check = 0;									/*У��ͳ�ʼ��Ϊ0*/
	iph->check = (SIP_Chksum(skb->nh.raw, sizeof(struct sip_iphdr)));/*IPͷ��У��ͼ���*/
	if(SIP_Chksum(skb->nh.raw, sizeof(struct sip_iphdr)))/**/
	{
		DBGPRINT(DBG_LEVEL_ERROR, "ICMP check IP sum error\n");		
	}
	else
	{
		DBGPRINT(DBG_LEVEL_NOTES, "ICMP check IP sum success\n");
	}
	skb->len =skb->tot_len;							/*�����������ݵ��ܳ���*/

	if(skb->len > dev->mtu){							/*����������ݳ�����̫����MTU*/
		skb= ip_frag(dev, skb);						/*���з�Ƭ*/
	}
	
	dev->output( skb,dev);							/*ͨ����̫�������������������*/
}

