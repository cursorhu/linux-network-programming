#if 0
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <signal.h>
#include "../module/sipfw.h"

struct msg_to_kernel
{
	struct nlmsghdr hdr;
};

struct u_packet_info
{
	struct nlmsghdr hdr;
	struct packet_info icmp_info;
};

static int skfd;

static void sig_int(int signo)
{
	struct sockaddr_nl kpeer;
	struct msg_to_kernel message;

	memset(&kpeer, 0, sizeof(kpeer));
	kpeer.nl_family = AF_NETLINK;
	kpeer.nl_pid    = 0;
	kpeer.nl_groups = 0;

	memset(&message, 0, sizeof(message));
	message.hdr.nlmsg_len = NLMSG_LENGTH(0);
	message.hdr.nlmsg_flags = 0;
	message.hdr.nlmsg_type = SIPFW_CLOSE;
	message.hdr.nlmsg_pid = getpid();

	sendto(skfd, &message, message.hdr.nlmsg_len, 0, (struct sockaddr *)(&kpeer),sizeof(kpeer));

	close(skfd);
	exit(0);
}
enum{
	CMD_NONE 			= 0x0000U,
	CMD_INSERT 		= 0x0001U,
	CMD_DELETE 		= 0x0002U,
	CMD_APPEND 		= 0x0004U,
	CMD_LIST 			= 0x0008U,
	CMD_FLUSH 			= 0x0001U,
	NUMBER_OF_CMD 	= 5
}

struct opt_value{
	unsigned int flag;
	unsigned int from;
	unsigned int to;
};

union variant {
	char			v_str[8];
	int			v_int;
	unsigned int	v_uint;
	time_t		v_time;
	void			(*v_func)(void);
	void			*v_void;
	struct vec	v_vec;
	struct opt_value v_opt;
};

struct vec {
	void *ptr;
	unsigned long len;
};


/*
 * This guy holds parsed HTTP headers
 */
struct opts_value {
	union variant	command;
	union variant	source;
	union variant	dest;
	union variant	sport;
	union variant	dport;
	union variant	protocol;
	union variant	chain;
	union variant	ifname;
};


static struct option opts_long[] = {						/*��ѡ��*/
	{.name = "source",    	.has_arg = 1, 		.val = 's'},	/*Դ����IP��ַ*/
	{.name = "dest",        	.has_arg = 1, 		.val = 'd'},	/*Ŀ������IP��ַ*/
	{.name = "sport",        	.has_arg = 1, 		.val = 'm'},	/*Դ�˿ڵ�ַ*/
	{.name = "dport",       	.has_arg = 1, 		.val = 'n'},	/*Ŀ�Ķ˿ڵ�ַ*/
	{.name = "protocol", 	.has_arg = 2, 		.val = 'p'},	/*Э������*/
	{.name = "list",    		.has_arg = 2, 		.val = 'L'},	/*�����б�*/
	{.name = "flush",         	.has_arg = 2, 		.val = 'F'},	/*��չ���*/
	{.name = "append", 	.has_arg = 2, 		.val = 'A'},	/*���ӹ�����β��*/
	{.name = "insert",     	.has_arg = 1, 		.val = 'I'},	/*���������ӹ���*/
	{.name = "delete",  	.has_arg = 2, 		.val = 'D'},	/*ɾ������*/
	{.name = "interface",  	.has_arg = 1, 		.val = 'i'},	/*����ӿ�*/
	{NULL},
};
enum{
	OPT_IP,
	OPT_PROTOCOL,
	OPT_STR,
	OPT_INT
};

static const char opts_short[] =  "s:d:m:n:p:L:F:A:I:D:i:",;	/*��ѡ��*/
static const vec chain_name[] = {	
	{"INPUT",	5},
	{"OUTPUT",	6},
	{"FORWARD",	7},
	{NULL,		0}  };

static const vec action_name[] = {	
	{"ACCEPT",	6},
	{"DROP",	4},
	{NULL,		0}  };
enum{
	OPT_CHAIN,
	OPT_IP,
	OPT_PORT,
	OPT_PROTOCOL,
	OPT_STR,
	OPT_VEC
};



int parse_opt(int opt, char *str, union variant *var)
{
	switch(opt)
	{
		case OPT_CHAIN:
			int chain = SIPFW_CHAIN_ALL;
			if(str){				
				int i = 0;
				
				for(i = 0;i<SIPFW_CHAIN_NUM;i++)
				{
					if(!strncmp(str, chain_name[i].ptr, chain_name[i].len))
					{
						chain = i;
						break;
					}
				}
			}
			var->v_big_int = chain;
			break;
			
		case OPT_IP:
			unsigned int ip = 0;
			if(str)
				ip = inet_addr(str);

			var->v_big_int = ip;
			break;
			
		case OPT_PORT:
			unsigned int port = 0;
			if(str)
				port = strtonl(str, NULL, strlen(str));

			var->v_big_int = port;
			
			break;
			
		case OPT_PROTOCOL:
			unsigned short proto = 0;
			if(str)
				proto = strtonl(str, NULL, strlen(str));

			proto = htons(proto);

			var->v_big_int = proto;
			break;
			
		case OPT_STR:
			break;
			
		case OPT_STR:
			if(str)
			{
				int 	len = strlen(str);
				memset(var->v_str, 0, sizeof(var->v_str));
				if(len < 8)
				{
					memcpy(var->v_str, str, len);

				}
			}
			break;

		default:
			break;
	}
}

do_command(int argc, char *argv[], 	struct opts_value *cmd_opt)
{
	unsigned int cmd = CMD_NONE;
	static char *l_opt_arg = NULL;
	cmd_opt->chain.v_uint = SIPFW_CHAIN_ALL;
	cmd_opt->command.v_uint = SIPFW_CMD_ACCEPT;
	cmd_opt->dest.v_uint = 0;
	cmd_opt->dport.v_uint = 0;
	cmd_opt->source.v_uint = 0;
	cmd_opt->sport.v_uint = 0;
	cmd_opt->protocol.v_uint = 0;
	memset(cmd_opt->ifname.v_str, 0, 8);
	
		
	char c = 0;
	while ((c = getopt_long(argc, argv, opts_short,  opts_long, NULL)) != -1) 
	{
		switch(c)
		{
			case 's':		/*Դ����IP��ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_IP, optarg, &cmd_opt->source);
				}
				
				break;
			case 'd':/*Ŀ������IP��ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_IP, optarg, &cmd_opt->dest);
				}
				
				break;
			case 'm':/*Դ�˿ڵ�ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_PORT, optarg, &cmd_opt->sport);
				}
				
				break;
			case 'n':/*Ŀ�Ķ˿ڵ�ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_PORT, optarg, &cmd_opt->dport);
				}
				
				break;
			case 'p':/*Э������*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_PROTOCOL, optarg, &cmd_opt->protocol);
				}
				
				break;
			case 'L':/*�����б�*/
				cmd |= CMD_LIST;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
			case 'F':/*��չ���*/
				cmd |= CMD_FLUSH;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'A':/*���ӹ�����β��*/
				cmd |= CMD_APPEND;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'I':/*���������ӹ���*/
				cmd |= CMD_INSERT;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'D':/*ɾ������*/
				cmd |= CMD_DELETE;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'i':/*����ӿ�*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					parse_opt(OPT_STR, optarg, &cmd_opt->ifname);
				}
				break;
			default:
				break;
		}

		
	}
}
struct iovec
{
	void  *iov_base;	/* BSD uses caddr_t (1003.1g requires void *) */
	ssize_t iov_len; /* Must be size_t (1003.1g) */
};

struct msghdr {
	void	*	msg_name;	/* Socket name			*/
	int		msg_namelen;	/* Length of name		*/
	struct iovec *	msg_iov;	/* Data blocks			*/
	ssize_t	msg_iovlen;	/* Number of blocks		*/
	void 	*	msg_control;	/* Per protocol magic (eg BSD file descriptor passing) */
	ssize_t	msg_controllen;	/* Length of cmsg list */
	unsigned	msg_flags;
};
struct nlmsghdr
{
	__u32		nlmsg_len;	/* Length of message including header */
	__u16		nlmsg_type;	/* Message content */
	__u16		nlmsg_flags;	/* Additional flags */
	__u32		nlmsg_seq;	/* Sequence number */
	__u32		nlmsg_pid;	/* Sending process PID */
};
struct sockaddr_nl
{
	sa_family_t	nl_family;	/* AF_NETLINK	*/
	unsigned short	nl_pad;		/* zero		*/
	__u32		nl_pid;		/* process pid	*/
       __u32		nl_groups;	/* multicast groups mask */
};

#include <sys/socket.h>
#include <linux/netlink.h>


void main(int argc, char *argv[]) 
{
#define MAX_PAYLOAD 1024  				/* ����س���*/

	struct sockaddr_nl source, dest;		/*Դ��ַ��Ŀ���ַ*/
	int s = -1;							/*�׽����ļ�������*/
	struct msghdr msg;					/*���ں�ͨ����Ϣ*/
	struct iovec iov;						/*��Ϣ�е�����*/
	char buffer[MAX_PAYLOAD];			/*nlmsghdrʹ�û�����*/
	struct nlmsghdr *nlmsgh = NULL;
	int err = -1;
	struct opts_value cmd_opt;

	do_command(argc, argv[], &cmd_opt);


	signal(SIGINT, sig_int);				/*�ҽ��ж��ź�*/
	
	s = socket(PF_NETLINK, 				/*�����׽���*/
		SOCK_RAW,NETLINK_TEST);
	if(s < 0)
	{
		printf("can not create a netlink socket\n");
		return -1;
	}

	/*����Դ��ַ*/
	memset(&source, 0, sizeof(source));		/*��ջ�����*/
	source.nl_family 	= AF_NETLINK;		/*Э����*/
	source.nl_pid 		= getpid();  			/*������ID*/
	source.nl_groups 	= 0;  				/*����*/

	err = bind(s, 							/*��*/
		(struct sockaddr*)&source, sizeof(source));
	{
		printf("bind() error\n");
		return -1;
	}

	memset(&dest, 0, sizeof(dest));			/*��ջ�����*/
	dest.nl_family 	= AF_NETLINK;		/*Э����*/
	dest.nl_pid 		= 0;   				/*���͸��ں�*/
	dest.nl_groups 	= 0; 				/*����*/

	nlmsgh=(struct nlmsghdr *)buffer;		/*��nlmsg�ṹָ�򻺳���*/
	/* ���netlink��Ϣͷ*/
	nlmsgh->nlmsg_len 	= NLMSG_SPACE(MAX_PAYLOAD);/*����*/
	nlmsgh->nlmsg_pid 	= getpid();  		/*�����̵�PID*/
	nlmsgh->nlmsg_flags 	= 0;				/*��־*/
	/* ���netlink��Ϣ�ĸ���*/
	memcpy(NLMSG_DATA(nlmsgh), &cmd_opt, sizeof(cmd_opt));

	iov.iov_base 		= (void *)nlmsgh;		/*��netlink˽����Ϣд������*/
	iov.iov_len 		= nlmsgh->nlmsg_len;	/*��������*/
	msg.msg_name 	= (void *)&dest;		/*��Ϣ����Ϊ��ַ*/
	msg.msg_namelen = sizeof(dest);		/*���Ƴ���*/
	msg.msg_iov 		= &iov;				/*��Ϣ����*/
	msg.msg_iovlen 	= 1;					/*��������*/

	sendmsg(s, &msg, 0);					/*������Ϣ*/

	/* ���ں˽�����Ϣ */
	memset(nlmsgh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	recvmsg(s, &msg, 0);					/*������Ϣ*/
	unsigned int retval = (unsigned int *)NLMSG_DATA(nlmsgh);
	
	if(cmd_opt.command.v_uint == CMD_LIST)/*��ù����б�*/
	{		
		unsigned int sip = 0, dip = 0;		/*ԭIP��ַ��Ŀ��IP��ַ*/
		unsigned short sport = 0, dport = 0;	/*Դ�˿ں�Ŀ�Ķ˿�*/
		unsigned char proto = 0;			/*Э������*/
		unsigned char action = 0;			/*��������*/
		unsigned char chain_org = SIPFW_CHAIN_NUM, chain;
		int i = 0;
		for(i = retval; i > 0; i -= 10)		/*�ֶ�ζ�ȡ�ں��еĹ���*/
		{
			int j = 0, len = 0;
			memset(nlmsgh, 0, NLMSG_SPACE(MAX_PAYLOAD));
			recvmsg(s, &msg, 0);			/*������Ϣ*/
			struct sipfw_rules *rules = NLMSG_DATA(nlmsgh);

			len = msg.msg_iov.iov_len/sizeof(*rules);/*�������*/
			for(j = 0; j < len; j++)			/*��ȡ����*/
			{
				action = rules->policy;		/*����*/
				source = rules->saddr;		/*ԴIP*/
				sport = rules->sport;		/*Դ�˿�*/
				dest = rules->daddr;		/*Ŀ��IP*/
				dport = rules->dport;		/*Ŀ�Ķ˿�*/
				proto = rules->protocol;	/*Э������*/
				chain = rules->chain;		/*��*/
				if(chain != chain_org)		/*�������仯*/
				{
					chain_org = chain;		/*�޸���*/
				
					printf("CHAIN %s Rules\n"	/*��ӡ������*/
						"ACTION"
						"\tSOURCE"
						"\tSPORT"
						"\tDEST"
						"\tDPORT"
						"\tPROTO"
						"\n",
						chain_name[chain_org]);
				}
				
				printf("%s"				/*��ӡ��Ϣ*/
					"\t%s"
					"\t%ud"
					"\t%s"
					"\t%ud",
					"\t%d"
					"\n",
					action_name[i],		/*��������*/
					inet_ntoa(sip),			/*ԴIP��ַ*/
					sport,				/*Դ�˿�*/
					inet_ntoa(dip),		/*Ŀ��IP*/
					dport,				/*Ŀ�Ķ˿�*/
					proto);				/*Э������*/
			}
		}
	}
	else									/*������������*/
	{
		printf(" %s\n", retval?"Failure":"Success");
	}
	
	//printf(" Received message payload: %s\n",		NLMSG_DATA(nlmsgh));

	/* �ر� Netlink�׽��� */
	close(s);

	return 0;
}
#endif