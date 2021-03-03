#include <netdb.h>
#include <stdio.h>
 
 /* ��ʾЭ�����Ŀ */
void display_protocol(struct protoent *pt)
{
	int i = 0;
	if(pt){/*�Ϸ���ָ��*/
		printf("protocol name:%s,",pt->p_name);/*Э��Ĺٷ�����*/
		if(pt->p_aliases){/*������Ϊ��*/
			printf("alias name:");/*��ʾ����*/
			while(pt->p_aliases[i]){/*�б�û����β*/
				printf("%s ",pt->p_aliases[i]);	/*��ʾ��ǰ����*/
				i++;/*��һ������*/
			}
		}
		printf(",value:%d\n",pt->p_proto);	/*Э��ֵ*/
	}
}

int main(int argc, char *argv[])
{
	int i = 0;
	/* Ҫ��ѯ��Э������ */
	const char  *const protocol_name[]={
 	"ip",
 	"icmp",
 	"igmp",	
 	"ggp",
 	"ipencap",
 	"st",
 	"tcp",
 	"egp",
 	"igp",
 	"pup",
 	"udp",
 	"hmp",
 	"xns-idp",
 	"rdp",
 	"iso-tp4",
 	"xtp",
 	"ddp",
 	"idpr-cmtp",
 	"ipv6",
 	"ipv6-route",
 	"ipv6-frag",
 	"idrp",
 	"rsvp",
 	"gre",
 	"esp",
 	"ah",
 	"skip",
 	"ipv6-icmp",
 	"ipv6-nonxt",
 	"ipv6-opts",
 	"rspf",
 	"vmtp",
 	"eigrp",
 	"ospf",
 	"ax.25",
 	"ipip",
 	"etherip",
 	"encap",
 	"pim",
 	"ipcomp",
 	"vrrp",
 	"l2tp",
 	"isis",
 	"sctp",
 	"fc",
 	NULL}; 
	
	setprotoent(1);/*��ʹ�ú���getprotobynameʱ���ر��ļ�/etc/protocols*/
	while(protocol_name[i]!=NULL){/*û�е�����protocol_name�Ľ�β*/
		struct protoent *pt = getprotobyname((const char*)&protocol_name[i][0]);/*��ѯЭ��*/
		if(pt){/*�ɹ�*/
			display_protocol(pt);	/*��ʾЭ����Ŀ*/
		}
		i++;/*�Ƶ�����protocol_name����һ��*/
	};
	endprotoent();/*�ر��ļ�/etc/protocols*/
	return 0;	
}
