#include "sip.h"

#define NUM_SOCKETS 4
static struct sip_socket sockets[NUM_SOCKETS];
/**
 * Map a externally used socket index to the internal socket representation.
 *
 * @param s externally used socket index
 * @return struct sip_socket for the socket or NULL if not found
 */
static struct sip_socket *get_socket(int s)
{
	struct sip_socket *socket;
	if ((s < 0) || (s >= NUM_SOCKETS)) 
	{
		return NULL;
	}

	socket = &sockets[s%NUM_SOCKETS];
	if (!socket->sock) 
	{
		return NULL;
	}
	else if(socket->sock->socket != s)
	{
		return NULL;
	}
		

	return socket;
}

/**
 * Allocate a new socket for a given sock.
 *
 * @param newsock the sock for which to allocate a socket
 * @return the index of the new socket; -1 on error
 */
static int alloc_socket(struct sock *newsock)
{
	int i;

	/* allocate a new socket identifier */
	for (i = 0; i < NUM_SOCKETS; ++i) 
	{
		if (!sockets[i].sock) 
		{
			sockets[i].sock       = newsock;
			sockets[i].lastdata   = NULL;
			sockets[i].lastoffset = 0;
			sockets[i].err        = 0;
			return i;
		}
	}

	return -1;
}


int sip_socket(int domain, int type, int protocol)
{
	struct sock *sock;
	int i = 0;

	if(domain != AF_INET || protocol != 0)		/*Э�����Ͳ���*/
		return -1;

	switch (type) 							/*�������ͽ�����ͬ���׽���*/
	{  
		case SOCK_DGRAM:					/*���ݱ�����*/
			sock = (struct sock *)SIP_SockNew( SOCK_DGRAM);/*�����׽���*/
			break;
		case SOCK_STREAM:					/*��ʽ����*/
			break;
		default:
			return -1;
	}

	if (!sock) {								/*�����׽���ʧ��*/
		return -1;
	}

	i = alloc_socket(sock);					/*��ʼ��socket����,�������ļ�������*/
	if (i == -1) {								/*��������ʧ��*/
		SIP_SockDelete(sock);				/*�ͷ�sock���ͱ���*/
		return -1;
	}

	sock->socket = i;							/*����sock�ṹ�е�socketֵ*/
	return i;
}


int sip_close(int s)
{
	struct sip_socket *socket;

	socket = get_socket(s);					/*���socket����ӳ��*/
	if (!socket) 								/*ʧ��*/
	{
		return -1;
	}

	SIP_SockDelete(socket->sock);			/*�ͷ�sock�ṹ*/

	if (socket->lastdata) 
	{
		skb_free(socket->lastdata);			/*�ͷ�socket�Ϲҽӵ���������*/
	}
	socket->lastdata   = NULL;					/*���socket�ṹ����������*/
	socket->sock       = NULL;					/*���sockָ��*/

	return 0;
}

int   sip_bind(int   sockfd, 
			const  struct  sockaddr  *my_addr,  
			socklen_t    addrlen)
{
	struct sip_socket *socket;
	struct in_addr local_addr;
	__u16 port_local;
	int err;

	socket = get_socket(sockfd);				/*���socket����ӳ��*/
	if (!socket)
		return -1;

	local_addr.s_addr = ((struct sockaddr_in *)my_addr)->sin_addr.s_addr;
	port_local = ((struct sockaddr_in *)my_addr)->sin_port;

	err = SIP_SockBind(socket->sock, &local_addr, ntohs(port_local));/*Э���޹ز�İ󶨺���*/
	if (err != 0) 
	{
		return -1;
	}

	return 0;
}


int  sip_connect(int  sockfd,  
				const  struct sockaddr *serv_addr, 
				socklen_t       addrlen)
{
	struct sip_socket *socket;
	int err;

	socket = get_socket(sockfd);				/*���socket����ӳ��*/
	if (!socket)
		return -1;

	struct in_addr remote_addr;
	__u16 remote_port;

	remote_addr.s_addr = ((struct sockaddr_in *)serv_addr)->sin_addr.s_addr;
	remote_port = ((struct sockaddr_in *)serv_addr)->sin_port;

	err = SIP_SockConnect(socket->sock, &remote_addr, ntohs(remote_port));

	return 0;
}


ssize_t sip_recvfrom(int s, void *buf, size_t len, int flags,
                        struct sockaddr *from, socklen_t *fromlen)   
{
	struct sip_socket *socket;
	struct skbuff      *skb;
	struct sockaddr_in *f = (struct sockaddr_in *)from;
	int len_copy = 0;

	socket = get_socket(s);					/*���socket����ӳ��*/
	if (!socket)
		return -1;

	if(!socket->lastdata){						/*lastdata��û����ʣ������*/
		socket->lastdata =(struct skbuff*) SIP_SockRecv(socket->sock);/*��������*/
		socket->lastoffset = 0;					/*ƫ����Ϊ0*/
	}

	skb = socket->lastdata;					/*skbuffָ��*/

	/*����û��������*/
	*fromlen = sizeof(struct sockaddr_in);		/*��ַ�ṹ����*/
	f->sin_family = AF_INET;					/*��ַ����*/
	f->sin_addr.s_addr = skb->nh.iph->saddr;	/*��ԴIP��ַ*/
	f->sin_port = skb->th.udph->source;			/*��Դ�˿�*/

	len_copy = skb->len - socket->lastoffset;		/*����lastdata��ʣ�������*/
	if(len > len_copy)	{						/*�û����������Է�����������*/
		memcpy(buf, 							/*ȫ���������û�������*/
			skb->data+socket->lastoffset, 
			len_copy);
		skb_free(skb);						/*�ͷŴ˽ṹ*/
		socket->lastdata = NULL;				/*����������ݽṹָ��*/
		socket->lastoffset = 0;					/*ƫ������������Ϊ0*/
	}else{									/*�û��������Ų�����������*/
		len_copy = len;						/*��������������С������*/
		memcpy(buf, 							/*����*/
			skb+socket->lastoffset, 
			len_copy);
		socket->lastoffset += len_copy;			/*ƫ��������*/
	}	

	return len_copy;							/*���ؿ�����ֵ*/
}

ssize_t sip_recv(int s, void *buf, size_t len, int flags)	
{
  	return sip_recvfrom(s, buf, len, flags, NULL, NULL);
}


ssize_t sip_send(int s, const void *buf, size_t len, int flags)	
{
	struct sip_socket *socket;
	int err;

	socket = get_socket(s);
	if (!socket)
		return -1;
	return sip_sendto(s, buf, len, flags, NULL, 0);


	return -1;
}


ssize_t  sip_sendto(int  s,  
				const  void *buf, 
				size_t len, 
				int flags, 
				const struct sockaddr *to, 
				socklen_t tolen)       
{
	struct sip_socket *socket;
	struct in_addr remote_addr;
	struct sockaddr_in* to_in = (struct sockaddr_in*)to;
	/*��������ͷ���ĳ���*/
	int l_head = sizeof(struct sip_ethhdr) + sizeof(struct sip_iphdr) + sizeof(struct sip_udphdr);
	int size = l_head + len;					/*�����ܳ���*/
	struct skbuff *skb = skb_alloc( size);		/*����ռ�*/

	char* data = skb_put(skb, l_head);			/*����dataָ��*/
	memcpy(data, buf, len);					/*���û����ݿ�����������*/

	remote_addr =to_in->sin_addr;				/*����Ŀ��IP��ַ*/

	socket = get_socket(s);
	if (!socket)
		return -1;
	SIP_SockSendTo(socket->sock, skb, &remote_addr, to_in->sin_port);/*��������*/

	return len;
}





