#include "sip.h"


struct skbuff *skb_alloc(__u32 size)
{
	DBGPRINT(DBG_LEVEL_MOMO,"==>skb_alloc\n");
	struct skbuff *skb = (struct skbuff*)malloc(sizeof(struct skbuff));	/*����skbuff�ṹ�ڴ�ռ�*/
	if(!skb)														/*ʧ��*/
	{
		DBGPRINT(DBG_LEVEL_ERROR,"Malloc skb header error\n");
		goto EXITskb_alloc;										/*�˳�*/
	}
	memset(skb, 0, sizeof(struct skbuff));							/*��ʼ��skbuff�ڴ�ṹ*/

	size = SKB_DATA_ALIGN(size);								/*����ϵͳ���ö�����ռ�Ĵ�С���й�����*/
	skb->head = (__u8*)malloc(size);								/*�������������ڴ�,��������headָ����*/
	if(!skb->head)												/*�����ڴ�ʧ��*/
	{
		DBGPRINT(DBG_LEVEL_ERROR,"Malloc skb data error\n");
		free(skb);												/*�ͷ�֮ǰ����ɹ���skbuff�ṹ�ڴ�*/
		goto EXITskb_alloc;										/*�˳�*/
	}
	memset(skb->head, 0, size);									/*��ʼ���û��ڴ���*/

	skb->end = skb->head + size;									/*endָ��λ�ó�ʼ��*/
	skb->data = skb->head;										/*dataָ���ʼ��Ϊ��headһ��*/
	skb->tail = skb->data;											/*tail�����dataһ��*/
	skb->next = NULL;												/*next��ʼ��Ϊ��*/
	skb->tot_len = 0;												/*���������ܳ���Ϊ0*/
	skb->len = 0;													/*��ǰ�ṹ�е����ݳ���Ϊ0*/
	DBGPRINT(DBG_LEVEL_MOMO,"<==skb_alloc\n");
	return skb;													/*���سɹ���ָ��*/
EXITskb_alloc:
	return NULL;													/*����,���ؿ�*/
}

void skb_free(struct skbuff *skb)
{
	if(skb)														/*�жϽṹ�Ƿ�Ϊ��*/
	{
		if(skb->head)											/*�ж��Ƿ����û��ռ�*/
			free(skb->head);										/*�ͷ��û��ռ��ڴ�*/
		free(skb);												/*�ͷ�skb�ṹ�ڴ�ռ�*/
	}
}

void skb_clone(struct skbuff *from, struct skbuff *to)
{
	memcpy(to->head, from->head, from->end - from->head);		/*�����û�����*/
	to->phy.ethh = (struct sip_ethhdr*)skb_put(to, ETH_HLEN);			/*����Ŀ�Ľṹ��̫����ָ��λ��*/
	to->nh.iph = (struct sip_iphdr*)skb_put(to, IPHDR_LEN);			/*����IPͷ����ָ��λ��*/
}
__u8 *skb_put(struct skbuff *skb, __u32 len)
{
	DBGPRINT(DBG_LEVEL_MOMO,"==>skb_put\n");
	__u8 *tmp = skb->tail;										/*����β��ָ��λ��*/
	skb->tail += len;												/*�ƶ�β��ָ��*/
	skb->len  -= len;												/*���ȵ�ǰ�������ݳ��ȼ���*/
	//skb->tot_len += len;

	DBGPRINT(DBG_LEVEL_MOMO,"<==skb_put\n");
	return tmp;													/*����β��ָ��λ��*/
}
#if 0
/* CRC16У��ͼ���icmp_cksum
������
	data:����
	len:���ݳ���
����ֵ��
	��������short����
*/
unsigned short cksum(__u8 *data,  int len)
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
       sum = (sum >>16) + (sum & 0xffff);/* �ߵ�λ��� */
       sum += (sum >>16) ;		/* �����λ���� */
       
       return ~sum; /* ����ȡ��ֵ */
}
#else
static __u16
SIP_ChksumStandard(void *dataptr, __u16 len)
{
  __u32 acc;
  __u16 src;
  __u8 *octetptr;

  acc = 0;
  /* dataptr may be at odd or even addresses */
  octetptr = (__u8*)dataptr;
  while (len > 1)
  {
    /* declare first octet as most significant
       thus assume network order, ignoring host order */
    src = (*octetptr) << 8;
    octetptr++;
    /* declare second octet as least significant */
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }
  if (len > 0)
  {
    /* accumulate remaining octet */
    src = (*octetptr) << 8;
    acc += src;
  }
  /* add deferred carry bits */
  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  if ((acc & 0xffff0000) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }
  /* This maybe a little confusing: reorder sum using htons()
     instead of ntohs() since it has a little less call overhead.
     The caller must invert bits for Internet sum ! */
  return htons((__u16)acc);
}

__u16 SIP_Chksum(void *dataptr, __u16 len)
{
	__u32 acc;

	acc = SIP_ChksumStandard(dataptr, len);
	while ((acc >> 16) != 0) 
	{
		acc = (acc & 0xffff) + (acc >> 16);
	}

	return (__u16)~(acc & 0xffff);
}
#endif



/* inet_chksum_pseudo:
 *
 * Calculates the pseudo Internet checksum used by TCP and UDP for a pbuf chain.
 * IP addresses are expected to be in network byte order.
 *
 * @param p chain of pbufs over that a checksum should be calculated (ip data part)
 * @param src source ip address (used for checksum of pseudo header)
 * @param dst destination ip address (used for checksum of pseudo header)
 * @param proto ip protocol (used for checksum of pseudo header)
 * @param proto_len length of the ip data part (used for checksum of pseudo header)
 * @return checksum (as u16_t) to be saved directly in the protocol header
 */
__u16
SIP_ChksumPseudo(struct skbuff *skb,
       struct in_addr *src, struct in_addr *dest,
       __u8 proto, __u16 proto_len)
{
	__u32 acc;
	__u8 swapped;

	acc = 0;
	swapped = 0;
	{
		acc += SIP_Chksum(skb->data, skb->end - skb->data);
		while ((acc >> 16) != 0) 
		{
			acc = (acc & 0xffffUL) + (acc >> 16);
		}
		if (skb->len % 2 != 0) 
		{
			swapped = 1 - swapped;
			acc = ((acc & 0xff) << 8) | ((acc & 0xff00UL) >> 8);
		}
	}

	if (swapped) 
	{
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00UL) >> 8);
	}

	/*Ϊͷ��У���*/
	acc += (src->s_addr & 0xffffUL);
	acc += ((src->s_addr >> 16) & 0xffffUL);
	acc += (dest->s_addr & 0xffffUL);
	acc += ((dest->s_addr >> 16) & 0xffffUL);
	acc += (__u32)htons((__u16)proto);
	acc += (__u32)htons(proto_len);

	while ((acc >> 16) != 0) 
	{
		acc = (acc & 0xffffUL) + (acc >> 16);
	}

	return (__u16)~(acc & 0xffffUL);
}

