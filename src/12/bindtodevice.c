

int s,err;
char ifname[] = "eth1";	/* �󶨵��������� */

/* һЩ������� */
...
...

err = setsockopt(s, SOL_SOCKET, SO_BANDTODEVICE, ifname, 5);	/* ��s�󶨵�����eth1�� */
if(err){/* ʧ�� */
	printf("setsockopt SO_BANDTODEVICE failure\n");	
}
