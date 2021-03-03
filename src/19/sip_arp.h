#ifndef __SIP_ARP_H__
#define __SIP_ARP_H__

#define	ARPOP_REQUEST	1		/* ARP ����*/
#define	ARPOP_REPLY	2		/* ARP Ӧ��*/
#define 	ETH_P_802_3	0x0001	


#define 	ARP_TABLE_SIZE 10		/*ӳ����С*/
#define 	ARP_LIVE_TIME	20		/*ARPӳ�������ʱ��*/
enum arp_status
{
	ARP_EMPTY,					/*ARP״̬Ϊ��*/
	ARP_ESTABLISHED			/*ARP�Ѿ�ӳ������*/
};

struct arpt_arp					/*ARP����ṹ*/
{
	__u32	ipaddr;				/*IP��ַ*/
	__u8	ethaddr[ETH_ALEN];	/*MAC��ַ*/
  	time_t 	ctime;				/*������ʱ��*/
	enum 	arp_status status;		/*ARP״ֵ̬*/
};


struct sip_arphdr
{
	 /*	 ����ΪARPͷ���ṹ*/
	__be16	ar_hrd;				/* Ӳ����ַ����*/
	__be16	ar_pro;				/* Э���ַ����*/
	__u8	ar_hln;				/* Ӳ����ַ����*/
	__u8	ar_pln;				/* Э���ַ����*/
	__be16	ar_op;				/* ARP������*/
	
	 /*	 ����Ϊ��̫���е�ARP����*/
	__u8 	ar_sha[ETH_ALEN];	/* ���ͷ���Ӳ����ַ*/
	__u8 	ar_sip[4];			/* ���ͷ���IP��ַ*/
	__u8 	ar_tha[ETH_ALEN];	/* Ŀ��Ӳ����ַ*/
	__u8 	ar_tip[4];			/* Ŀ��IP��ַ*/
};




#endif /*__SIP_ARP_H__*/
