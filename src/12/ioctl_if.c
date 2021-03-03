#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <string.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int s;/* �׽��������� */
	int err = -1;/*����ֵ*/

	/* ����һ�����ݱ��׽��� */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)	{
		printf("socket() ����\n");
		return -1;
	}

	/* �������ӿڵ����� */
	{
		struct ifreq ifr;
		ifr.ifr_ifindex = 2; /* ��ȡ��2������ӿڵ����� */
		err = ioctl(s, SIOCGIFNAME, &ifr);
		if(err){
			printf("SIOCGIFNAME Error\n");
		}else{
			printf("the %dst interface is:%s\n",ifr.ifr_ifindex,ifr.ifr_name);
		}
	}

	/* �������ӿ����ò��� */
	{
		/* ��ѯ������eth0������� */
		struct ifreq ifr;
		memcpy(ifr.ifr_name, "eth0",5);

		/* ��ȡ��� */
		err = ioctl(s, SIOCGIFFLAGS, &ifr);
		if(!err){
			printf("SIOCGIFFLAGS:%d\n",ifr.ifr_flags);
		}

		/* ��ȡMETRIC */
		err = ioctl(s, SIOCGIFMETRIC, &ifr);
		if(!err){
			printf("SIOCGIFMETRIC:%d\n",ifr.ifr_metric);
		}
		
		/* ��ȡMTU */		
		err = ioctl(s, SIOCGIFMTU, &ifr);
		if(!err){
			printf("SIOCGIFMTU:%d\n",ifr.ifr_mtu);
		}	
		
		/* ��ȡMAC��ַ */
		err = ioctl(s, SIOCGIFHWADDR, &ifr);
		if(!err){
			unsigned char *hw = ifr.ifr_hwaddr.sa_data;
			printf("SIOCGIFHWADDR:%02x:%02x:%02x:%02x:%02x:%02x\n",hw[0],hw[1],hw[2],hw[3],hw[4],hw[5]);
		}	
		
		/* ��ȡ����ӳ����� */
		err = ioctl(s, SIOCGIFMAP, &ifr);
		if(!err){
			printf("SIOCGIFMAP,mem_start:%d,mem_end:%d, base_addr:%d, dma:%d, port:%d\n",
				ifr.ifr_map.mem_start, 	/*��ʼ��ַ*/
				ifr.ifr_map.mem_end,		/*������ַ*/
				ifr.ifr_map.base_addr,	/*����ַ*/
				ifr.ifr_map.irq ,				/*�ж�*/
				ifr.ifr_map.dma ,				/*DMA*/
				ifr.ifr_map.port );			/*�˿�*/
		}
		
		/* ��ȡ������� */
		err = ioctl(s, SIOCGIFINDEX, &ifr);
		if(!err){
			printf("SIOCGIFINDEX:%d\n",ifr.ifr_ifindex);
		}
		
		/* ��ȡ���Ͷ��г��� */		
		err = ioctl(s, SIOCGIFTXQLEN, &ifr);
		if(!err){
			printf("SIOCGIFTXQLEN:%d\n",ifr.ifr_qlen);
		}			
	}

	/* �������ӿ�IP��ַ */
	{
		struct ifreq ifr;
		/* �����������ָ��sockaddr_in��ָ�� */
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
		char ip[16];/* ����IP��ַ�ַ��� */
		memset(ip, 0, 16);
		memcpy(ifr.ifr_name, "eth0",5);/*��ѯeth0*/
		
		/* ��ѯ����IP��ַ */		
		err = ioctl(s, SIOCGIFADDR, &ifr);
		if(!err){
			/* ������ת��Ϊ����Ķε��ַ��� */
			inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16 );
			printf("SIOCGIFADDR:%s\n",ip);
		}
		
		/* ��ѯ�㲥IP��ַ */
		err = ioctl(s, SIOCGIFBRDADDR, &ifr);
		if(!err){
			/* ������ת��Ϊ����Ķε��ַ��� */
			inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16 );
			printf("SIOCGIFBRDADDR:%s\n",ip);
		}
		
		/* ��ѯĿ��IP��ַ */
		err = ioctl(s, SIOCGIFDSTADDR, &ifr);
		if(!err){
			/* ������ת��Ϊ����Ķε��ַ��� */
			inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16 );
			printf("SIOCGIFDSTADDR:%s\n",ip);
		}
		
		/* ��ѯ�������� */
		err = ioctl(s, SIOCGIFNETMASK, &ifr);
		if(!err){
			/* ������ת��Ϊ����Ķε��ַ��� */
			inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16 );
			printf("SIOCGIFNETMASK:%s\n",ip);
		}
	}

	/* ���Ը���IP��ַ */
	{
		struct ifreq ifr;
		/* �����������ָ��sockaddr_in��ָ�� */
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
		char ip[16];/* ����IP��ַ�ַ��� */
		int err = -1;
		
		/* ������IP��ַ����Ϊ192.169.1.175 */
		printf("Set IP to 192.168.1.175\n");
		memset(&ifr, 0, sizeof(ifr));/*��ʼ��*/
		memcpy(ifr.ifr_name, "eth0",5);/*��eth0��������IP��ַ*/
		inet_pton(AF_INET, "192.168.1.175", &sin->sin_addr.s_addr);/*���ַ���ת��Ϊ�����ֽ��������*/
		sin->sin_family = AF_INET;/*Э����*/
		err = ioctl(s, SIOCSIFADDR, &ifr);/*�������ñ���IP��ַ��������*/
		if(err){/*ʧ��*/
			printf("SIOCSIFADDR error\n");
		}else{/*�ɹ����ٶ�ȡһ�½���ȷ��*/
			printf("check IP --");
			memset(&ifr, 0, sizeof(ifr));/*������0*/
			memcpy(ifr.ifr_name, "eth0",5);/*����eth0*/
			ioctl(s, SIOCGIFADDR, &ifr);/*��ȡ*/
			inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);/*��IP��ַת��Ϊ�ַ���*/
			printf("%s\n",ip);/*��ӡ*/
		}		
	}

	close(s);
	return 0;
}
