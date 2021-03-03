#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif
#include "sipfw.h"
#include "sipfw_para.h"

/*һЩ����*/
MODULE_DESCRIPTION("Simple IP FireWall module ");
MODULE_AUTHOR("songjingbin<flyingfat@163.com>");

/*���뱾�����ݵĹ��Ӵ�����*/
static unsigned int
SIPFW_HookLocalIn(unsigned int hook,
	struct sk_buff **pskb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct sipfw_rules *l = NULL;/*������ָ��*/
	struct sk_buff *skb = *pskb;/*�������ݽṹ*/
	struct sipfw_rules *found = NULL;/*�ҵ��Ĺ���*/
	int	retval = 0;/*����ֵ*/
	DBGPRINT("==>SIPFW_HookLocalIn\n");
	if(cf.Invalid)/*����ǽ�Ƿ��ֹ*/
	{
		retval = NF_ACCEPT;/*����ǽ�ر�,������ͨ��*/
		goto EXITSIPFW_HookLocalIn;
	}
	
	
	l = sipfw_tables[SIPFW_CHAIN_INPUT].rule;/*INPUT��*/
	found = SIPFW_IsMatch(skb, l);/*���ݺ����й����Ƿ�ƥ��*/
	if(found)/*��ƥ�����*/
	{
		SIPFW_LogAppend(skb, found);/*��¼*/
		cf.HitNumber++;/*����������*/
	}
	/*���·���ֵ*/
	retval = found?found->action:cf.DefaultAction;
EXITSIPFW_HookLocalIn:	
	DBGPRINT("<==SIPFW_HookLocalIn\n");
	return retval ;
}

/*�ӱ��ط������������ݴ����Ӻ���*/
static unsigned int
SIPFW_HookLocaOut(unsigned int hook,
	struct sk_buff **pskb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct sipfw_rules *l = NULL;/*������ָ��*/
	struct sk_buff *skb = *pskb;/*�������ݽṹ*/
	struct sipfw_rules *found = NULL;/*�ҵ��Ĺ���*/
	int	retval = 0;/*����ֵ*/
	DBGPRINT("==>SIPFW_HookLocaOut\n");
	if(cf.Invalid)/*����ǽ�Ƿ��ֹ*/
	{
		retval = NF_ACCEPT;/*����ǽ�ر�,������ͨ��*/
		goto EXITSIPFW_HookLocaOut;
	}
	
	l = sipfw_tables[SIPFW_CHAIN_OUTPUT].rule;/*OUTPUT��*/
	found = SIPFW_IsMatch(skb, l);/*���ݺ����й����Ƿ�ƥ��*/
	if(found)
	{
		SIPFW_LogAppend(skb, found);/*��¼*/
		cf.HitNumber++;/*����������*/
	}
	/*���·���ֵ*/
	retval = found?found->action:cf.DefaultAction;
	
EXITSIPFW_HookLocaOut:
	DBGPRINT("<==SIPFW_HookLocaOut\n");
	return retval;
}

/*�ӱ��ط������������ݴ����Ӻ���*/
static unsigned int
SIPFW_HookForward(unsigned int hook,
	struct sk_buff **pskb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct sipfw_rules *l = NULL;/*������ָ��*/
	struct sk_buff *skb = *pskb;/*�������ݽṹ*/
	struct sipfw_rules *found = NULL;/*�ҵ��Ĺ���*/
	int	retval = 0;/*����ֵ*/
	DBGPRINT("==>SIPFW_HookForward\n");
	DBGPRINT("in device:%s,out device:%s\n",in->name, out->name);{
	int i =0;int len =skb->dev->addr_len;
	__u8 *mac=skb->dev->perm_addr;
	for(i=0;i<len;i++)
		DBGPRINT("%x-", mac[i]);
	mac = skb->input_dev->perm_addr;
	len = skb->input_dev->addr_len;
	DBGPRINT("\n");
	for(i=0;i<len;i++)
		DBGPRINT("%x-", mac[i]);
	DBGPRINT("\n");
	mac= skb->mac.raw;len = skb->mac_len;
	DBGPRINT("SKB MAC, len:%d\n",len);
	for(i=0;i<len;i++)
		DBGPRINT("%x-", mac[i]);
	DBGPRINT("\n");
	//00-1F-3A-B1-FA-60
	mac[6]=mac[0];
	mac[7]=mac[1];
	mac[8]=mac[2];
	mac[9]=mac[3];
	mac[10]=mac[4];
	mac[11]=0x15;
	
	mac[0]=0x00;
	mac[1]=0x1F;
	mac[2]=0x3A;
	mac[3]=0xB1;
	mac[4]=0xFA;
	mac[5]=0x60;

	
	
	

	dev_queue_xmit(skb);
	return NF_STOLEN;

	

}	if(cf.Invalid)/*����ǽ�Ƿ��ֹ*/
	{
		retval = NF_ACCEPT;/*����ǽ�ر�,������ͨ��*/
		goto EXITSIPFW_HookForward;
	}
	
	l = sipfw_tables[SIPFW_CHAIN_OUTPUT].rule;/*OUTPUT��*/
	found = SIPFW_IsMatch(skb, l);/*���ݺ����й����Ƿ�ƥ��*/
	if(found)
	{
		SIPFW_LogAppend(skb, found);/*��¼*/
		cf.HitNumber++;/*����������*/
	}
	/*���·���ֵ*/
	retval = found?found->action:cf.DefaultAction;
EXITSIPFW_HookForward:
	DBGPRINT("<==SIPFW_HookForward\n");
	return retval;
}

/*�ӱ��ط������������ݴ����Ӻ���*/
static unsigned int
SIPFW_HookPreRouting(unsigned int hook,
	struct sk_buff **pskb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct sipfw_rules *l = NULL;/*������ָ��*/
	struct sk_buff *skb = *pskb;/*�������ݽṹ*/
	struct sipfw_rules *found = NULL;/*�ҵ��Ĺ���*/
	int	retval = 0;/*����ֵ*/
	DBGPRINT("==>SIPFW_HookPreRouting\n");
	if(cf.Invalid)/*����ǽ�Ƿ��ֹ*/
	{
		retval = NF_ACCEPT;/*����ǽ�ر�,������ͨ��*/
		goto EXITSIPFW_HookPreRouting;
	}
	
	l = sipfw_tables[SIPFW_CHAIN_OUTPUT].rule;/*OUTPUT��*/
	found = SIPFW_IsMatch(skb, l);/*���ݺ����й����Ƿ�ƥ��*/
	if(found)
	{
		SIPFW_LogAppend(skb, found);/*��¼*/
		cf.HitNumber++;/*����������*/
	}
	/*���·���ֵ*/
	retval = found?found->action:cf.DefaultAction;
EXITSIPFW_HookPreRouting:
	DBGPRINT("<==SIPFW_HookPreRouting\n");
	return retval;
}

/*�ӱ��ط������������ݴ����Ӻ���*/
static unsigned int
SIPFW_HookPostRouting(unsigned int hook,
	struct sk_buff **pskb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct sipfw_rules *l = NULL;/*������ָ��*/
	struct sk_buff *skb = *pskb;/*�������ݽṹ*/
	struct sipfw_rules *found = NULL;/*�ҵ��Ĺ���*/
	int	retval = 0;/*����ֵ*/
	DBGPRINT("==>SIPFW_HookPostRouting\n");
	if(cf.Invalid)/*����ǽ�Ƿ��ֹ*/
	{
		retval = NF_ACCEPT;/*����ǽ�ر�,������ͨ��*/
		goto EXITSIPFW_HookPostRouting;
	}
	
	l = sipfw_tables[SIPFW_CHAIN_OUTPUT].rule;/*OUTPUT��*/
	found = SIPFW_IsMatch(skb, l);/*���ݺ����й����Ƿ�ƥ��*/
	if(found)
	{
		SIPFW_LogAppend(skb, found);/*��¼*/
		cf.HitNumber++;/*����������*/
	}
	/*���·���ֵ*/
	retval = found?found->action:cf.DefaultAction;
EXITSIPFW_HookPostRouting:
	DBGPRINT("<==SIPFW_HookPostRouting\n");
	return retval;
}


/* ���ӹҽӽṹ */
static struct nf_hook_ops sipfw_hooks[]  = {
	{
		.hook		= SIPFW_HookLocalIn,	/*���ؽ�������*/
		.owner		= THIS_MODULE,			/*ģ��������*/
		.pf			= PF_INET,				/*����Э��*/
		.hooknum	= NF_IP_LOCAL_IN,		/*�ҽӵ�*/
		.priority		= NF_IP_PRI_FILTER-1,		/*���ȼ�*/
	},

	{
		.hook		= SIPFW_HookLocaOut,	/*���ط���������*/
		.owner		= THIS_MODULE,			/*ģ��������*/
		.pf			= PF_INET,				/*����Э��*/
		.hooknum	= NF_IP_LOCAL_OUT,		/*�ҽӵ�*/
		.priority		= NF_IP_PRI_FILTER-1,		/*���ȼ�*/
	},
	{
		.hook		= SIPFW_HookForward,	/*���ط���������*/
		.owner		= THIS_MODULE,			/*ģ��������*/
		.pf			= PF_INET,				/*����Э��*/
		.hooknum	= NF_IP_FORWARD,		/*�ҽӵ�*/
		.priority		= NF_IP_PRI_FILTER-1,		/*���ȼ�*/
	},
	{
		.hook		= SIPFW_HookPreRouting,	/*���ط���������*/
		.owner		= THIS_MODULE,			/*ģ��������*/
		.pf			= PF_INET,				/*����Э��*/
		.hooknum	= NF_IP_PRE_ROUTING,		/*�ҽӵ�*/
		.priority		= NF_IP_PRI_FILTER-1,		/*���ȼ�*/
	},
	{
		.hook		= SIPFW_HookPostRouting,	/*���ط���������*/
		.owner		= THIS_MODULE,			/*ģ��������*/
		.pf			= PF_INET,				/*����Э��*/
		.hooknum	= NF_IP_POST_ROUTING,		/*�ҽӵ�*/
		.priority		= NF_IP_PRI_FILTER-1,		/*���ȼ�*/
	},
};



/*ģ���ʼ��*/
static int __init SIPFW_Init(void)
{
	int ret = -1;
	DBGPRINT("==>SIPFW_Init\n");
	
	ret = SIPFW_HandleConf();/*��ȡ����ǽ�����ļ�*/
	
	ret =SIPFW_NLCreate();/*����Netlink�׽���׼�����û��ռ�ͨ��*/
	if(ret)
	{
		goto error1;
	}
	
	ret =SIPFW_Proc_Init();/*����PROC�����ļ�*/
	if(ret)
	{
		goto error2;
	}	
	ret = nf_register_hooks(sipfw_hooks,ARRAY_SIZE(sipfw_hooks));
	if(ret)
	{
		goto error3;
	}

	goto error1;
error3:
	SIPFW_Proc_CleanUp();
error2:
	SIPFW_NLDestory();
error1:
	DBGPRINT("<==SIPFW_Init\n");
	return ret;
}

static void __exit SIPFW_Exit(void)
{
	DBGPRINT("==>SIPFW_Exit\n");
	SIPFW_NLDestory();
	SIPFW_ListDestroy();

	DBGPRINT("module sipfw exit\n");
	
	SIPFW_Proc_CleanUp();
	nf_unregister_hooks(sipfw_hooks,ARRAY_SIZE(sipfw_hooks));
	DBGPRINT("<==SIPFW_Exit\n");
}

module_init(SIPFW_Init);
module_exit(SIPFW_Exit);
MODULE_LICENSE("GPL/BSD");

