#ifndef __SIP_UDP_H__
#define __SIP_UDP_H__
#define UDP_FLAGS_NOCHKSUM 0x01U
#define UDP_FLAGS_UDPLITE  0x02U
#define UDP_FLAGS_CONNECTED  0x04U
struct sip_udphdr 
{
	__be16	source;		/*Դ�˿�*/
	__be16	dest;		/*Ŀ�Ķ˿�*/
	__u16	len;			/*���ݳ���*/
	__be16	check;		/*UDPУ���*/
};

struct ip_pcb 
{
	/* Common members of all PCB types */
	/* ip addresses in network byte order */
	struct in_addr ip_local; 
	struct in_addr ip_remote;
	/* Socket options */  
	__u16 so_options;     
	/* Type Of Service */ 
	__u8 tos;              
	/* Time To Live */     
	__u8 ttl  ;            
	/* link layer address resolution hint */ 
	__u8 addr_hint;
};
struct udp_pcb 
{
	struct in_addr 	ip_local; 		/*����IP��ַ*/
	__u16 			port_local;		/*���ض˿ڵ�ַ*/
	struct in_addr 	ip_remote;		/*����Ŀ��IP��ַ*/
	__u16 			port_remote;	/*����Ŀ�Ķ˿ڵ�ַ*/
	__u8 			tos;              		/*��������*/
	__u8 			ttl ;             		/*����ʱ��*/
	__u8 			flags;			/*���*/

	struct sock 		*sock;			/*�����޹ؽṹ*/

	struct udp_pcb 	*next;			/*��һ��UDP���Ƶ�Ԫ*/
	struct udp_pcb 	*prev;			/*ǰһ��UDP���Ƶ�Ԫ*/
};

#endif /*__SIP_UDP_H__*/
