#ifndef __KERNEL__
#define __KERNEL__
#endif /*__KERNEL__*/
#ifndef MODULE
#define MODULE
#endif /*MODULE*/
#include "sipfw.h"

static struct sock *nlfd = NULL;

/*���û���������*/
static int SIPFW_NLSendToUser(struct nlmsghdr *to, void *data, int len, int type)
{
	int size = 0;
	struct nlmsghdr *nlmsgh = NULL;
	char *pos = NULL;
	struct sk_buff *skb = alloc_skb(len, GFP_ATOMIC);	/*������Դ����û�����*/
	unsigned char *oldtail = skb->tail;			/*�������ݵĽ�β*/
	
	DBGPRINT("==>SIPFW_NLSendToUser\n");
	size = NLMSG_SPACE(len);		/*����Ϣͷ���ĳ���*/
	nlmsgh = NLMSG_PUT(skb, 0, 0, type, size-sizeof(*to));/*������Ϣ����*/
	pos = NLMSG_DATA(nlmsgh);/*�����Ϣ�����ݲ��ֵ�ַ*/
	memset(pos, 0, len);/*�����Ϣ*/
	memcpy(pos, data, len);/*�����ݿ�������*/

	nlmsgh->nlmsg_len = skb->tail - oldtail;/*�����Ϣ�ĳ���*/
	
	NETLINK_CB(skb).dst_group = 0;/*�����趨*/
	netlink_unicast(nlfd, skb, to->nlmsg_pid, MSG_DONTWAIT);	/*��������*/

	DBGPRINT("<==SIPFW_NLSendToUser\n");
	return 0;
 nlmsg_failure:/*NL����ʧ�ܵ�ת��*/
 	if(skb)
		kfree_skb(skb);/*�ͷ���Դ*/

	DBGPRINT("<==SIPFW_NLSendToUser\n");
	return -1;
}

/*��ù����б�����Ĵ�����*/
static int SIPFW_NLAction_RuleList(struct sipfw_rules *rule, struct nlmsghdr * to)
{
	int i ,num;
	unsigned int count = -1;
	struct sipfw_list *l = NULL;
	struct sipfw_rules *cur = NULL;
	
	DBGPRINT("==>SIPFW_NLAction_RuleList\n");
	if(rule->chain == SIPFW_CHAIN_ALL)/*���ȫ���������Ĺ�����Ϣ*/
	{
		i = 0;/*��ʼ��*/
		num = 3;/*������*/
		count = sipfw_tables[0].number /*�������*/
				+ sipfw_tables[1].number 
				+sipfw_tables[2].number ;
	}
	else
	{
		i = rule->chain;/*�������ĵ�ַ*/
		num = i+1;/*����*/
		count = sipfw_tables[i].number;/*�������*/
	}

	/*����ͻ��˷��͹������������Ӧ�ó������*/
	SIPFW_NLSendToUser( to, &count, sizeof(count), SIPFW_MSG_RULE);

	for(;i<num; i++)/*ѭ����ȡ���򲢷��͸��û��ռ�*/
	{		
		l = &sipfw_tables[i];/*������ʼ��ַ*/
		for(cur = l->rule; cur != NULL; cur = cur->next)/*����*/
		{
			if(cur)/*�ǿ�*/
			{
				/*���͸��û��ռ�˹���*/
				SIPFW_NLSendToUser( to, cur, sizeof(*cur), SIPFW_MSG_RULE);
			}
		}
	}
	/*��մ������Ĺ���*/
	kfree(rule);

	DBGPRINT("<==SIPFW_NLAction_RuleList\n");
	return 0;
}

/*���ӹ����б�*/
static int SIPFW_NLAction_RuleAddpend(struct sipfw_rules *rule)
{
	struct sipfw_list *l = &sipfw_tables[rule->chain];/*�������*/

	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;
	DBGPRINT("==>SIPFW_NLAction_RuleAddpend\n");
	DBGPRINT("addpend to chain:%d==>%s,source:%x,dest:%x\n",
		rule->chain,
		(char*)sipfw_chain_name[rule->chain].ptr,
		rule->source,
		rule->dest);
	if(l->rule == NULL)/*��Ϊ��*/
	{
		l->rule = rule;/*�ӵ�ͷ��*/
	}
	else/*��Ϊ��*/
	{
		/*�ҵ��������ĩβ*/
		for(cur = l->rule; cur != NULL; prev = cur, cur = cur->next)
			;
		prev->next = rule;/*�ҽ�*/
	}	

	l->number++;/*�����ϵĹ����������*/

	DBGPRINT("<==SIPFW_NLAction_RuleAddpend\n");
	return 0;
}

/*ɾ�������б����number��Ч��ɾ���˹���
*�����մ���Ĺ���ɾ��*/
static int SIPFW_NLAction_RuleDelete(struct sipfw_rules *rule, int number)
{
	struct sipfw_list *l = &sipfw_tables[rule->chain];/*���ĸ�����ɾ��*/
	int i = 0;

	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;
	DBGPRINT("==>SIPFW_NLAction_RuleDelete\n");
	if(number > l->number)/*λ�ò�������*/
	{
		kfree(rule);/*������*/
	}
	else if(number != -1)/*λ�ò�����Ч*/
	{
		/*���Һ��ʵ�λ��*/
		for(cur = l->rule, i= 1; cur != NULL && i !=number; prev = cur, cur = cur->next,i++)
			;
		if(cur != NULL)/*����ĩβ*/
		{
			if(prev == NULL)/*ɾ����һ������*/
			{
				l->rule = cur->next;/*����ͷ��ָ��*/
			}
			else/*�м�Ĺ���*/
			{
				prev->next = cur->next;
			}
			kfree(cur);/*�ͷ���Դ*/
			l->number--;		/*�����Ĺ��������1*/
		}
		kfree(rule);/*�ͷŴ���Ĺ���*/
	}	 
	else/*λ�ò���û������*/
	{
		/*�����ϲ��ҹ���ƥ�������ɾ��*/
		for(cur=l->rule; cur != NULL; prev = cur, cur=cur->next)
		{
			if(	cur->action == rule->action/*����ƥ��*/
				&&cur->addtion.valid == rule->addtion.valid/*������ƥ��*/
				&&cur->chain ==rule->chain/*������ƥ��*/
				&&cur->source == rule->source /*Դ��ַƥ��*/
				&&cur->dest == rule->dest/*Ŀ�ĵ�ַƥ��*/
				&&cur->sport==rule->sport/*Դ�˿�ƥ��*/
				&&cur->dport==rule->dport/*Ŀ�Ķ˿�ƥ��*/
				&&cur->protocol==rule->protocol)/*Э��ƥ��*/
			{
				if(!prev)/*ͷ��*/
				{
					l->rule = cur->next;/*ͷ��ָ�����*/
				}
				else/*�м�*/
				{
					prev->next = cur->next;
				}

				kfree(cur);/*�ͷ���Դ*/
				l->number --;/*��������1*/
				kfree(rule);/*�ͷŴ���Ĺ���*/
				break;
			}
		}
	}

	DBGPRINT("<==SIPFW_NLAction_RuleDelete\n");
	return 0;
}

/*�滻�����б�*/
static int SIPFW_NLAction_RuleReplace(struct sipfw_rules *rule, int number)
{
	struct sipfw_list *l = &sipfw_tables[rule->chain];
	int i = 0;

	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;
	DBGPRINT("==>SIPFW_NLAction_RuleReplace\n");
	if(number != -1)/*��ֵ��ȷ*/
	{
		/*���Һ��ʵ�λ��*/
		for(cur = l->rule, i= 1; cur != NULL && i !=number; prev = cur, cur = cur->next,i++)
			;
		if(cur != NULL)/*�ҵ����滻��*/
		{
			if(prev == NULL)/*ͷ��*/
			{
				l->rule = rule;
			}
			else/*�м�*/
			{
				prev->next = rule;
			}
			rule->next = cur->next;/*ժ�����滻��*/
			kfree(cur);/*�ͷ���Դ*/
		}
	}
	else if(number > l->number)
	{
		kfree(rule);/*û���ҵ����ͷŴ���ָ��*/
	}
	
	DBGPRINT("<==SIPFW_NLAction_RuleReplace\n");
	return 0;
}

/*������򵽹����б�ĳ��λ��*/
static int SIPFW_NLAction_RuleInsert(struct sipfw_rules *rule, int number)
{
	struct sipfw_list *l = &sipfw_tables[rule->chain];
	int i = 0;

	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;

	DBGPRINT("==>SIPFW_NLAction_RuleInsert\n");
	if(number == 1)/*����ͷ��*/
	{
		rule->next = l->rule;
		l->rule = rule;
		goto EXITSIPFW_NLAction_RuleInsert;
	}

	if(number > l->number)/*����β��*/
	{
		/*���Ҹ�λ��*/
		for(cur = l->rule; cur != NULL; prev = cur, cur = cur->next)
			;
		prev->next = rule;
		goto EXITSIPFW_NLAction_RuleInsert;
	}
	
	if(number != -1)/*λ����ȷ*/
	{
		for(cur = l->rule, i= 1; cur != NULL && i <number; prev = cur, cur = cur->next,i++)
			;
		prev->next = rule;
		rule->next = cur->next;
	}
EXITSIPFW_NLAction_RuleInsert:	
	DBGPRINT("<==SIPFW_NLAction_RuleInsert\n");
	return 0;
}

/*��չ����б�*/
static int SIPFW_NLAction_RuleFlush(struct sipfw_rules *rule)
{
	struct sipfw_list *l = NULL;
	struct sipfw_rules *prev = NULL;
	struct sipfw_rules *cur = NULL;
	int i ,num;
	DBGPRINT("==>SIPFW_NLAction_RuleFlush\n");
	if(rule->chain == SIPFW_CHAIN_ALL)/*ȫ�����*/
	{
		i = 0;
		num = 3;
	}
	else/*���һ������*/
	{
		i = rule->chain;
		num = i+1;
	}

	for(;i<num; i++)/*ѭ�����*/
	{
		
		l = &sipfw_tables[i];
		for(cur = l->rule; cur != NULL; prev = cur, cur = cur->next)
		{
			if(prev)
			{
				kfree(prev);
			}
		}
		l->rule = NULL;
		l->number = 0;
	}

	kfree(rule);

	DBGPRINT("<==SIPFW_NLAction_RuleFlush\n");
	return 0;
}

/*��Ӧ�ͻ��˵Ķ���*/
static int SIPFW_NLDoAction(void *payload, struct nlmsghdr* nlmsgh)
{
	struct sipfw_cmd_opts *cmd_opt = NULL;
	int cmd = -1;
	int number = -1;
	vec NLSUCCESS={"SUCCESS",8};
	vec NLFAILRE={"FAILURE",8};

	struct sipfw_rules *rule = NULL;
	DBGPRINT("==>SIPFW_NLDoAction\n");
	cmd_opt = (struct sipfw_cmd_opts *)payload;
	cmd = cmd_opt->command.v_uint;
	/*ÿ������֮ǰ������һ����Ԫ��Ź������ݣ�
	�Դ˵�Ԫ���ڴ洦���ɸ����������Լ�����
	������ڲ���Ĺ��򣬴��ڴ�ֱ���ɷ���ʹ���ˣ�
	��ɾ������Ķ���������Ҫ�ͷ�������Ԫ���ڴ�*/
	rule = (struct sipfw_rules*)kmalloc(sizeof(struct sipfw_rules), GFP_KERNEL);
	if(!rule)
	{
		DBGPRINT("Malloc rule struct failure\n");
	}

	rule->next = NULL;

	/*��ʼ��ΪĬ����Ϣ*/
	rule->chain = cmd_opt->chain.v_int;
	rule->source= cmd_opt->source.v_uint;
	rule->source= cmd_opt->source.v_uint;
	rule->sport = cmd_opt->sport.v_uint;
	rule->dport = cmd_opt->dport.v_uint;
	rule->protocol = cmd_opt->protocol.v_uint;
	rule->action = cmd_opt->action.v_uint;
	rule->addtion.valid = cmd_opt->addtion.valid;
	number = cmd_opt->number.v_int;
	
//static int SIPFW_NLSendToUser(struct sock *s, struct nlmsghdr *to, void *data, int len);

	switch(cmd)
	{
		int err = -1;
		case SIPFW_CMD_INSERT:	/*��������в����¹���*/
			err = SIPFW_NLAction_RuleInsert(rule, number);
			if(!err)
			{
				SIPFW_NLSendToUser( nlmsgh,NLSUCCESS.ptr, NLSUCCESS.len, SIPFW_MSG_SUCCESS);
			}
			else
			{
				SIPFW_NLSendToUser( nlmsgh,NLFAILRE.ptr, NLFAILRE.len, SIPFW_MSG_FAILURE);
			}
			break;
		case SIPFW_CMD_DELETE:	/*�ӹ�������ɾ��ĳ����*/
			err = SIPFW_NLAction_RuleDelete(rule, number);
			if(!err)
			{
				SIPFW_NLSendToUser( nlmsgh,NLSUCCESS.ptr, NLSUCCESS.len, SIPFW_MSG_SUCCESS);
			}
			else
			{
				SIPFW_NLSendToUser( nlmsgh,NLFAILRE.ptr, NLFAILRE.len, SIPFW_MSG_FAILURE);
			}
			break;
		case SIPFW_CMD_REPLACE:/*����ĳ������*/
			err = SIPFW_NLAction_RuleReplace(rule,number);
			if(!err)
			{
				SIPFW_NLSendToUser( nlmsgh,NLSUCCESS.ptr, NLSUCCESS.len, SIPFW_MSG_SUCCESS);
			}
			else
			{
				SIPFW_NLSendToUser( nlmsgh,NLFAILRE.ptr, NLFAILRE.len, SIPFW_MSG_FAILURE);
			}
			break;
		case SIPFW_CMD_APPEND:	/*���¹���ӵ�������ĩβ*/
			err = SIPFW_NLAction_RuleAddpend(rule);
			if(!err)
			{
				SIPFW_NLSendToUser( nlmsgh,NLSUCCESS.ptr, NLSUCCESS.len, SIPFW_MSG_SUCCESS);
			}
			else
			{
				SIPFW_NLSendToUser( nlmsgh,NLFAILRE.ptr, NLFAILRE.len, SIPFW_MSG_FAILURE);
			}
			break;
		case SIPFW_CMD_LIST:	/*�г��������еĹ���*/
			SIPFW_NLAction_RuleList(rule,nlmsgh);
			break;
		case SIPFW_CMD_FLUSH:/*��չ���*/
			err = SIPFW_NLAction_RuleFlush(rule);
			if(!err)
			{
				SIPFW_NLSendToUser( nlmsgh,NLSUCCESS.ptr, NLSUCCESS.len, SIPFW_MSG_SUCCESS);
			}
			else
			{
				SIPFW_NLSendToUser( nlmsgh,NLFAILRE.ptr, NLFAILRE.len, SIPFW_MSG_FAILURE);
			}
			break;
		default:
			break;
	}

	DBGPRINT("<==SIPFW_NLDoAction\n");
	return 0;
}

static void SIPFW_NLInput(struct sock *sk, int len)
{	
	__u8 *payload = NULL;
	DBGPRINT("==>SIPFW_NLInput\n");

	/*�������Ϊ:
	*�����ն��в�Ϊ�յ�ʱ��
	*������ժ���������ݣ�
	*��ȡIPͷ���͸��ز��ֵ�ָ��Ȼ��
	*���͸�������
	*/
	do{
		struct sk_buff *skb;
		/*������ժ����������*/
		while((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
		{
			struct nlmsghdr *nlh = NULL;
			if(skb->len >= sizeof(struct nlmsghdr))/*���ݳ��Ȳ���*/
			{
				nlh = (struct nlmsghdr *)skb->data;/*�����Ϣͷ��*/
				if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
					&& (skb->len >= nlh->nlmsg_len))/*�Ϸ�����*/
				{
					payload = NLMSG_DATA(nlh);/*���ز���*/
					SIPFW_NLDoAction(payload, nlh);/*��������*/
				}
			}
			kfree_skb(skb);
		}
	}while(nlfd && nlfd->sk_receive_queue.qlen);
	DBGPRINT("<==SIPFW_NLInput\n");
	return ;
}

/* ����netlink�׽��� */
int SIPFW_NLCreate(void)
{
	/*����Netlink�׽��֣��䴦��Ļص�����ΪSIPFW_NLInput*/
	nlfd = netlink_kernel_create(NL_SIPFW,  1, SIPFW_NLInput,  THIS_MODULE);
	if(!nlfd)
	{
		return -1;
	}
	
	return 0;
}
/*����netlink�׽���*/
int SIPFW_NLDestory(void)
{
	if(nlfd)
	{
		sock_release(nlfd->sk_socket);
	}	
	return 0;
}



