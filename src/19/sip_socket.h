#ifndef __SIP_SOCKET_H__

#define __SIP_SOCKET_H__
/** Contains all internal pointers and states used for a socket */
struct sip_socket 
{
	/** Э���޹ز�Ľṹָ��,һ��socket��Ӧһ��sock */
	struct sock *sock;
	/** �����յ��������� */
	struct skbuff *lastdata;
	/** ���յ���������ƫ����,������
	�ڲ���һ�ν��������ݿ������û���ɵ� */
	__u16 lastoffset;

	/** ����ֵ */
	int err;
};

#endif /*__SIP_SOCKET_H__*/
