#ifndef __SIP_ETHER_H__
#define __SIP_ETHER_H__

#define ETH_P_IP		0x0800		/* IP���ͱ���*/
#define ETH_P_ARP	0x0806		/* ARP����*/
#define ETH_ALEN	6			/* ��̫����ַ���� */
#define ETH_HLEN	14			/* ��̫��ͷ������ */
#define ETH_ZLEN	60			/* ��̫����С���� */
#define ETH_DATA_LEN	1500	/* ��̫��������س��� */
#define ETH_FRAME_LEN	1514	/* ��̫����󳤶� */
#define ETH_P_ALL	0x0003		/* ʹ��SOCK_PACKET��ȡÿһ���� */

/*
 *	This is an Ethernet frame header.
 */ 
struct sip_ethhdr {
	__u8	h_dest[ETH_ALEN];	/* Ŀ����̫����ַ*/
	__u8	h_source[ETH_ALEN];	/* Դ��̫����ַ*/
	__be16	h_proto;			/* ���ݰ�������	*/
} ;
struct skbuff;
struct net_device {
	char				name[IFNAMSIZ];	

	/** IP address configuration in network byte order */
	struct in_addr	ip_host;			/* ����IP��ַ*/
	struct in_addr 	ip_netmask;		/*������������*/
	struct in_addr 	ip_broadcast;	/*�����Ĺ㲥��ַ*/
	struct in_addr 	ip_gw;			/*����������*/
	struct in_addr 	ip_dest;			/*���͵�Ŀ��IP��ַ*/
	__u16			type;			/*��������*/	
	

	/** ����������ڴ������豸�л�ȡ����
	*  ��������Э��ջ���д��� */
	__u8 (* input)(struct skbuff *skb, struct net_device *dev);
	/** �����������IPģ�鷢������ʱ�����
	*  �˺������ȵ���ARPģ���IP��ַ���в��ң�Ȼ��������*/
	__u8 (* output)(struct skbuff *skb, struct net_device *dev);
	/** ���������ARPģ�����,ֱ�ӷ�����������*/
	__u8 (* linkoutput)(struct skbuff *skb, struct net_device *dev);
	
	__u8 hwaddr_len;				/** Ӳ����ַ�ĳ��� */
	__u8 hwaddr[ETH_ALEN];			/*Ӳ����ַ��ֵ,MAC*/
	__u8 hwbroadcast[ETH_ALEN];		/** Ӳ���Ĺ㲥��ַ */	
	__u8 mtu;						/** ����������䳤�� */

	int s;							/*�������׽���������*/
	struct sockaddr to;				/*����Ŀ�ĵ�ַ�ṹ*/
};

#endif /*__SIP_ETHER_H__*/

