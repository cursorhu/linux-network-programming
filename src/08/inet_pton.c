#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#define ADDRLEN 16
int main(int argc, char *argv[])
{
	struct in_addr ip;
	char IPSTR[]="192.168.1.1";		/* �����ַ�ַ��� */
	char addr[ADDRLEN];				/* ����ת������ַ���IP��ַ��16���ֽڴ�С */
	const char*str=NULL;	
	int err = 0;					/* ����ֵ */
	
	/* ���Ժ���inet_ptonת��192.168.1.1Ϊ��������ʽ */	
	err = inet_pton(AF_INET, IPSTR, &ip);	/* ���ַ���ת��Ϊ������ */
	if(err > 0){
		printf("inet_pton:ip,%s value is:0x%x\n",IPSTR,ip.s_addr);
	}
	
	/* ���Ժ���inet_ntopת��192.168.1.1Ϊ�ַ��� */
	ip.s_addr = htonl(192<<24|168<<16|12<<8|255);/*192.168.12.255*/
	/*�������������ֽ���192.168.12.255ת��Ϊ�ַ���*/
	str = (const char*)inet_ntop(AF_INET, (void*)&ip, (char*)&addr[0], ADDRLEN);
	if(str){
		printf("inet_ntop:ip,0x%x is %s\n",ip.s_addr,str);	
	}
	
	return 0;	
}
