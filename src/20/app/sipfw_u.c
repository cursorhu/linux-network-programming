#include "sipfw.h"
#include "sipfw_para.h"
union response
{
	char info_str[128];
	struct sipfw_rules rule;
	unsigned int count;
};
struct packet_u
{
	struct nlmsghdr nlmsgh;
	union response  payload;
};
struct packet_u message;
struct sockaddr_nl nlsource, nldest;		/*Դ��ַ��Ŀ���ַ*/
int nls = -1;							/*�׽����ļ�������*/


/*SIGINT�źŽ�ȡ����*/
static void sig_int(int signo)
{
	DBGPRINT("==>sig_int\n");
	memset(&nldest, 0, sizeof(nldest));		/*Ŀ�ĵ�ַ*/
	nldest.nl_family = AF_NETLINK;			
	nldest.nl_pid    = 0;					/*�ں�*/
	nldest.nl_groups = 0;

	memset(&message, 0, sizeof(message));
	message.nlmsgh.nlmsg_len 	= NLMSG_LENGTH(0);	/*��Ϣ����*/
	message.nlmsgh.nlmsg_flags 	= 0;
	message.nlmsgh.nlmsg_type 	= SIPFW_MSG_CLOSE;	/*�ر�NL�׽���*/
	message.nlmsgh.nlmsg_pid 	= getpid();			/*��ǰ��PID*/

	/*��NL�׽��ֹرյ���Ϣ���͸��ں�*/
	sendto(nls, &message, message.nlmsgh.nlmsg_len, 0, (struct sockaddr*)&nldest, sizeof(nldest));

	close(nls);
	DBGPRINT("<==sig_int\n");
	_exit(0);
}





/*��ʾ��������*/
int SIPFW_DisplayOpts(struct sipfw_cmd_opts *opts)
{
	DBGPRINT("==>SIPFW_DisplayOpts\n");
	if(opts)
	{
		struct in_addr source, dest;
		source.s_addr = opts->source.v_uint;
		dest.s_addr = opts->dest.v_uint;
		DBGPRINT("SIPFW_CMD_LIST is %d\n",opts->command.v_uint);
		printf("command:%s\n", sipfw_command_name[opts->command.v_uint]);
		printf("source IP:%s\n",inet_ntoa(source));
		printf("Dest IP:%s\n",inet_ntoa(dest));
		printf("sport : %u\n",opts->sport.v_uint);
		printf("dport: %u\n",opts->dport.v_uint);
		printf("proto: %u\n",opts->protocol.v_uint);
		printf("ifname:%s\n",  opts->ifname.v_str);
	}
	DBGPRINT("<==SIPFW_DisplayOpts\n");
}

/*��������ѡ��*/
static int SIPFW_ParseOpt(int opt, char *str, union sipfw_variant *var)
{
	const struct vec *p = NULL;
	int chain = SIPFW_CHAIN_ALL;
	int action = SIPFW_ACTION_DROP;
	unsigned int port = 0,ip = 0;
	int protocol = 0, i = 0;

	DBGPRINT("==>SIPFW_ParseOpt\n");
	switch(opt)
	{
		case SIPFW_OPT_CHAIN:/*������*/
			if(str){				
				for(i = 0;i<SIPFW_CHAIN_NUM;i++){/*����������ƥ����*/
					if(!strncmp(str, sipfw_chain_name[i].ptr, sipfw_chain_name[i].len)){
						chain = i;
						break;
					}
				}
			}
			var->v_uint = chain;
			break;
		case SIPFW_OPT_ACTION:/*��������ƥ��*/
			if(str)	{				
				for(i = 0;i<SIPFW_ACTION_NUM;i++){/*���Ҷ������Ʋ�ƥ��*/				
					if(!strncmp(str, sipfw_action_name[i].ptr, sipfw_action_name[i].len))	{
						action = i;
						break;
					}
				}
			}
			var->v_uint = action;
			break;
		case SIPFW_OPT_IP:/*���ַ���תΪ�����ֽ���*/
			if(str)
				ip = inet_addr(str);

			var->v_uint = ip;
			break;
			
		case SIPFW_OPT_PORT:/*���ַ�������תΪ������*/
			if(str){
				port = htons(strtoul(str, NULL, 10));
			}
			var->v_uint = port;
			break;
			
		case SIPFW_OPT_PROTOCOL:/*��Э�������תΪֵ*/
			if(str){
				for(p=sipfw_protocol_name + 0; p->ptr != NULL; p++){
					if(!strncmp(p->ptr, str, p->len)){
						protocol = p->value;
						break;
					}
				}
			}
			var->v_uint = protocol;
			break;
			
		case SIPFW_OPT_STR:/*�ַ���ֱ�ӿ���*/
			if(str){
				int 	len = strlen(str);
				memset(var->v_str, 0, sizeof(var->v_str));
				if(len < 8){
					memcpy(var->v_str, str, len);

				}
			}
			break;

		default:
			break;
	}
	DBGPRINT("<==SIPFW_ParseOpt\n");
}

static int 
SIPFW_ParseCommand(int argc, char *argv[], 	struct sipfw_cmd_opts *cmd_opt)
{
	DBGPRINT("==>SIPFW_ParseCommand\n");
	struct option longopts[] = 
	{						/*��ѡ��*/
		{"source",	required_argument, 	NULL,	's'},	/*Դ����IP��ַ*/
		{"dest",        	required_argument, 	NULL,	'd'},	/*Ŀ������IP��ַ*/
		{"sport",        	required_argument, 	NULL,	'm'},	/*Դ�˿ڵ�ַ*/
		{"dport",       	required_argument, 	NULL,	'n'},	/*Ŀ�Ķ˿ڵ�ַ*/
		{"protocol", 	required_argument, 	NULL,	'p'},	/*Э������*/
		{"list",    		optional_argument, 	NULL,	'L'},	/*�����б�*/
		{"flush",        	optional_argument, 	NULL,	'F'},	/*��չ���*/
		{"append", 	required_argument, 	NULL,	'A'},	/*���ӹ�����β��*/
		{"insert",     	required_argument, 	NULL,	'I'},	/*���������ӹ���*/
		{"delete",  	required_argument, 	NULL,	'D'},	/*ɾ������*/
		{"interface",  	required_argument, 	NULL,	'i'},	/*����ӿ�*/
		{"action",  	required_argument, 	NULL,	'j'},	/*����*/
		{"syn",  		no_argument, 		NULL,	'y'},	/*syn*/
		{"rst",  		no_argument, 		NULL,	'r'},	/*rst*/
		{"acksyn",  	no_argument, 		NULL,	'k'},	/*acksyn*/
		{"fin",  		no_argument, 		NULL,	'f'},	/*fin*/
		{"number",	required_argument,	NULL,	'u'},	/*ɾ�����߲����λ��*/
		{0, 0, 0, 0},
	};
	static char opts_short[] =  "s:d:m:n:p:LFA:I:D:i:j:yrkfu:";	/*��ѡ��*/

	static char *l_opt_arg = NULL;/*��ѡ��Ĳ���*/
	
	cmd_opt->command.v_int = -1;	/*����Ĭ��ֵΪ-1*/
	cmd_opt->source.v_int = 0;		/*Դ��ַĬ��ֵΪ0*/
	cmd_opt->sport.v_int = 0;			/*Դ�˿�Ĭ��ֵΪ0*/
	cmd_opt->dest.v_int =0;			/*Ŀ�ĵ�ַĬ��ֵΪ0*/
	cmd_opt->dport.v_int = 0;			/*Ŀ�Ķ˿�Ĭ��ֵΪ0*/
	cmd_opt->protocol.v_int = -1;		/*Э������Ĭ��ֵΪ-1*/
	cmd_opt->chain.v_int = -1;		/*��Ĭ��ֵΪ-1*/
	cmd_opt->action.v_int= -1;		/*����Ĭ��ֵΪ-1*/
	memset(cmd_opt->ifname.v_str, 0, 8);
	
		
	char c = 0;
	while ((c = getopt_long(argc, argv, opts_short,  longopts, NULL)) != -1) 
	{
		switch(c)
		{
			case 's':		/*Դ����IP��ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_IP, optarg, &cmd_opt->source);
				}
				
				break;
			case 'd':/*Ŀ������IP��ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_IP, optarg, &cmd_opt->dest);
				}
				
				break;
			case 'm':/*Դ�˿ڵ�ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_PORT, optarg, &cmd_opt->sport);
				}
				
				break;
			case 'n':/*Ŀ�Ķ˿ڵ�ַ*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_PORT, optarg, &cmd_opt->dport);
				}
				
				break;
			case 'p':/*Э������*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_PROTOCOL, optarg, &cmd_opt->protocol);
				}
				
				break;
			case 'L':/*�����б�*/
				cmd_opt->command.v_uint = SIPFW_CMD_LIST;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				break;
				
			case 'F':/*��չ���*/
				cmd_opt->command.v_uint = SIPFW_CMD_FLUSH;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'A':/*���ӹ�����β��*/
				cmd_opt->command.v_uint = SIPFW_CMD_APPEND;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'I':/*���������ӹ���*/
				cmd_opt->command.v_uint = SIPFW_CMD_INSERT;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'D':/*ɾ������*/
				cmd_opt->command.v_uint = SIPFW_CMD_DELETE;
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_CHAIN, optarg, &cmd_opt->chain);
				}
				
				break;
			case 'i':/*����ӿ�*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_STR, optarg, &cmd_opt->ifname);
				}
				break;
			case 'j':/*����*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_ACTION, optarg, &cmd_opt->action);
				}
				break;
			case 'y':/*syn*/
				cmd_opt->addtion.tcp.valid = 1;
				cmd_opt->addtion.tcp.syn = 1;
				break;
			case 'r':/*rst*/
				cmd_opt->addtion.tcp.valid = 1;
				cmd_opt->addtion.tcp.rst= 1;
				break;
			case 'k':/*acksyn*/
				cmd_opt->addtion.tcp.valid = 1;
				cmd_opt->addtion.tcp.ack= 1;
				cmd_opt->addtion.tcp.syn= 1;
				break;
			case 'f':/*fin*/
				cmd_opt->addtion.tcp.valid = 1;
				cmd_opt->addtion.tcp.fin= 1;
				break;
			case 'u':/*number*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':')
				{
					SIPFW_ParseOpt(SIPFW_OPT_PORT, optarg, &cmd_opt->number);
				}
				break;
			default:
				break;
		}
	}

	DBGPRINT("<==SIPFW_ParseCommand\n");
}

static int SIPFW_NLCreate(void)
{
	DBGPRINT("==>SIPFW_NLCreate\n");
	int err = -1;
	int retval = -1;
	nls = socket(PF_NETLINK, 				/*�����׽���*/
				SOCK_RAW, NL_SIPFW);
	if(nls < 0)/*ʧ��*/
	{
		DBGPRINT("can not create a netlink socket\n");
		retval = -1;
		goto EXITSIPFW_NLCreate;
	}

	/*����Դ��ַ*/
	memset(&nlsource, 0, sizeof(nlsource));		/*��ջ�����*/
	nlsource.nl_family 	= AF_NETLINK;		/*Э����*/
	nlsource.nl_pid 		= getpid();  			/*������ID*/
	nlsource.nl_groups 	= 0;  				/*����*/

	err = bind(nls, 							/*��*/
		(struct sockaddr*)&nlsource, sizeof(nlsource));
	if(err == -1)
	{
		retval = -1;
		goto EXITSIPFW_NLCreate;
	}

EXITSIPFW_NLCreate:
	DBGPRINT("<==SIPFW_NLCreate\n");
	return retval;
}

static ssize_t SIPFW_NLSend(char *buf, int len, int type)
{
	DBGPRINT("==>SIPFW_NLSend\n");
	ssize_t size = -1;
	memset(&nldest, 0, sizeof(nldest));			/*��ջ�����*/
	nldest.nl_family 	= AF_NETLINK;		/*Э����*/
	nldest.nl_pid 		= 0;   				/*���͸��ں�*/
	nldest.nl_groups 	= 0; 				/*����*/

	/* ���netlink��Ϣͷ*/
	message.nlmsgh.nlmsg_len 	= NLMSG_LENGTH(len);/*����*/
	message.nlmsgh.nlmsg_pid 	= getpid();  		/*�����̵�PID*/
	message.nlmsgh.nlmsg_flags 	= 0;				/*��־*/
	message.nlmsgh.nlmsg_type	= type;			/*����*/
	/* ���netlink��Ϣ�ĸ���*/
	memcpy(NLMSG_DATA(&message.nlmsgh), buf, len);
	/*���͸��ں�*/
	size = sendto(nls,&message, message.nlmsgh.nlmsg_len, 0, (struct sockaddr*)&nldest, sizeof(nldest));

	DBGPRINT("<==SIPFW_NLSend\n");
	return size;
}

static ssize_t SIPFW_NLRecv(void)
{
	DBGPRINT("==>SIPFW_NLRecv\n");
	/* ���ں˽�����Ϣ */
	int len = sizeof(nldest);
	char *info = NULL;
	ssize_t size = -1;
	
	memset(&nldest, 0, sizeof(nldest));
	nldest.nl_family = AF_NETLINK;
	nldest.nl_pid    = 0;				/*���ں˽���*/
	nldest.nl_groups = 0;
	
	size = recvfrom(nls, /*������Ϣ*/
			&message, 
			sizeof(message), 
			0, 
			(struct sockaddr*)&nldest, 
			&len);					
	DBGPRINT("<==SIPFW_NLRecv\n");
	return size;
}

static void SIPFW_NLClose(void)
{
	DBGPRINT("==>SIPFW_NLClose\n");
	close(nls);
	DBGPRINT("<==SIPFW_NLClose\n");
}

/*���ղ���ʾ����,�����б�ÿ�ν�������һ��
*	countΪ����ĸ���
*/
static ssize_t SIPFW_NLRecvRuleList(unsigned int count)
{
	DBGPRINT("==>SIPFW_NLRecvRuleList\n");
	int i = -1;
	int size = -1;
	unsigned int sip = 0, dip = 0;		/*ԭIP��ַ��Ŀ��IP��ַ*/
	unsigned short sport = 0, dport = 0;	/*Դ�˿ں�Ŀ�Ķ˿�*/
	unsigned char proto = 0;			/*Э������*/
	int action = 0;			/*��������*/
	unsigned char chain_org = SIPFW_CHAIN_NUM, chain;
	struct sipfw_rules *rules = NULL;
	struct in_addr source, dest;
	

	for(i=0; i< count; i++)
	{
		size = SIPFW_NLRecv();		/*�����ں˷��͵�����*/
		if(size < 0)
		{
			continue;
		}
		
		rules = &message.payload.rule;
		action = rules->action;		/*����*/
		source.s_addr = rules->source;/*ԴIP*/
		sport = ntohs(rules->sport);		/*Դ�˿�*/
		dest.s_addr = rules->dest;	/*Ŀ��IP*/
		dport = ntohs(rules->dport);		/*Ŀ�Ķ˿�*/
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
				"\t\tDPORT"
				"\tPROTO"
				"\n",
				sipfw_chain_name[chain_org]);
		}

		if((action>-1 && action <3))/*��������*/
			printf("%s", sipfw_action_name[action]);
		else
			printf("%s", "NOTSET");
		printf("\t%s", inet_ntoa(source));/*ԴIP��ַ*/
		printf("\t%d", sport);/*Դ�˿�*/
		printf("\t%s", inet_ntoa(dest));/*Ŀ��IP*/
		printf("\t%d", dport);/*Ŀ�Ķ˿�*/
		printf("\t%d\n", proto);/*Э������*/
		

	}
	DBGPRINT("<==SIPFW_NLRecvRuleList\n");
}

static int 
SIPFW_JudgeCommand(struct sipfw_cmd_opts *opts)
{
	int retval = 0;
	switch(opts->command.v_int)
	{
		case SIPFW_CMD_APPEND:
			if(opts->chain.v_int>SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0
				|| opts->action.v_int == -1)
				retval = -1;
			break;
		case SIPFW_CMD_DELETE:
			if(opts->chain.v_int>SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0)
				retval = -1;
			break;
		case SIPFW_CMD_FLUSH:
			if(opts->chain.v_int == -1)
				opts->chain.v_int = SIPFW_CHAIN_ALL;
			
			if(opts->chain.v_int>SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0)
				retval = -1;
			break;
		case SIPFW_CMD_INSERT:
			if(opts->chain.v_int>=SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0
				|| opts->number.v_int == -1)
			break;
		case SIPFW_CMD_LIST:
			if(opts->chain.v_int == -1)
				opts->chain.v_int = SIPFW_CHAIN_ALL;
			if(opts->chain.v_int>SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0)
				retval = -1;
			break;
		case SIPFW_CMD_REPLACE:
			if(opts->chain.v_int>=SIPFW_CHAIN_NUM 
				|| opts->chain.v_int< 0
				|| opts->number.v_int == -1)
				retval = -1;
			break;
		default:
			retval = -1;
			break;		
	}

	return retval;
}
int main(int argc, char *argv[]) 
{

	int err = -1;
	char *msg = "test";
	struct sipfw_cmd_opts cmd_opt;
	ssize_t size ;

	signal(SIGINT, sig_int);				/*�ҽ��ж��ź�*/
	cmd_opt.action.v_int = -1;
	cmd_opt.addtion.valid = 0;
	cmd_opt.chain.v_int  = -1;
	cmd_opt.command.v_int = -1;
	cmd_opt.dest.v_uint = 0;
	cmd_opt.dport.v_int = -1;
	cmd_opt.protocol.v_int = -1;
	cmd_opt.number.v_int = -1;
	cmd_opt.source.v_uint = 0;
	cmd_opt.sport.v_int = -1;

	SIPFW_ParseCommand(argc, argv, &cmd_opt);	/*���������ʽ*/
	if(SIPFW_JudgeCommand(&cmd_opt))
		return -1;
	SIPFW_DisplayOpts(&cmd_opt);				/*��ʾ�������*/
	SIPFW_NLCreate();							/*����NetLink�׽���*/
	
	size = SIPFW_NLSend((char*)&cmd_opt, sizeof(cmd_opt), SIPFW_MSG_PID);/*��������*/
	if(size < 0){									/*ʧ��*/	
		return -1;
	}

	size = SIPFW_NLRecv();						/*�����ں���Ӧ*/
	if(size < 0){									/*ʧ��*/	
		return -1;
	}
	
	if(cmd_opt.command.v_uint == SIPFW_CMD_LIST){	/*��ù����б�*/		
		unsigned int count  = 0;					/*�����б��������*/
		
		if(size > 0){
			count = message.payload.count;			/*�������*/
		}	else		{
			return -1;
		}
		SIPFW_NLRecvRuleList(count);				/*���ղ���ʾ����*/
	}else{
		DBGPRINT("information:%s\n",message.payload.info_str);
	}	
	
	SIPFW_NLClose();							/*�ر�NetLink�׽���*/

	return 0;
}


