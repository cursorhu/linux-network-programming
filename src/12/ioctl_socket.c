#include 

int main(void)
{
	int s  =-1;		/* socket������ */
	int err = -1;	/* ����ֵ */
	
	......
	......
	
	
	int request = -1;/* �������� */
	int para = -1;	/* ioctl���������� */
	struct timeval tv;/* ioctl���������� */
	
	/* ���޴��������ж� */	
	request = SIOCATMARK;
	err = ioctl(s, request, &para);
	if(err){/* ioctl�������� */
		/* ������ */	
	}
	if(para){ /* �д������� */
		/* ���մ������ݣ�����... */
		...
	}else{/*�޴�������*/
		...
	}
	
	
	
	/* ���SIGIO��SIGURG�źŴ������ID */
	request = SIOCGPGRP;/*����FIOGETOWN*/
	err = ioctl(s, request, &para);
	if(err){/* ioctl�������� */
		/* ������ */
		...
	}else{
		/* ����˴����źŵĽ���ID�� */
		...
	}
	
	
	
	/* ����SIGIO��SIGURG�źŴ������ID */
	request = SIOCSPGRP;/*FIOSETOWN*/
	err = ioctl(s, request, &para);
	if(err){/* ioctl�������� */
		/* ������ */
		...
	}else{
		/* �ɹ������˴����źŵĽ���ID�� */
		...
	}
	
	
	
	/* ������ݱ��ĵ����ʱ�� */	
	request = SIOCGSTAMP;/*FIOSETOWN*/
	err = ioctl(s, request, &tv);
	if(err){/* ioctl�������� */
		/* ������ */
		...	
	}else{
		/* �����������ݱ��ĵ���ʱ�䣬�ڲ���tv�� */
		...
	}
	
	...
	...	
}