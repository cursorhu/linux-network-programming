#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <string.h>
#include <linux/sockios.h>

int main(int argc, char *argv[])
{
	int s;
	struct arpreq arpreq;
	struct sockaddr_in *addr = (struct sockaddr_in*)&arpreq.arp_pa;
	unsigned char *hw;

	int err = -1;

	if(argc < 2){
		printf("�����ʹ�÷�ʽ,��ʽΪ:\nmyarp ip(myarp 127.0.0.1)\n");
		return -1;
	}

	/* ����һ�����ݱ��׽��� */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)	{
		printf("socket() ����\n");
		return -1;
	}

	/* ���arpreq�ĳ�Աarp_pa */
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(argv[1]);
	if(addr->sin_addr.s_addr == INADDR_NONE){
		printf("IP ��ַ��ʽ����\n");
	}
	/* ����ӿ�Ϊeth0 */
	strcpy(arpreq.arp_dev, "eth0");

	err = ioctl(s, SIOCGARP, &arpreq);
	if(err < 0){/*ʧ��*/
		printf("IOCTL ����\n");
		return -1;
	}else{/*�ɹ�*/
		hw = (unsigned char*)&arpreq.arp_ha.sa_data;/*Ӳ����ַ*/
		printf("%s==>",argv[1]);/*��ӡIP*/
		printf("%02x:%02x:%02x:%02x:%02x:%02x\n",/*��ӡӲ����ַ*/
				hw[0],hw[1],hw[2],hw[3],hw[4],hw[5]);
	};

	close(s);
	return 0;
}
