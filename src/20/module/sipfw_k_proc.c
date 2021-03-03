#ifndef __KERNEL__
#define __KERNEL__
#endif /*__KERNEL__*/
#ifndef MODULE
#define MODULE
#endif /*MODULE*/
#include "sipfw.h"


#define MAX_COOKIE_LENGTH       PAGE_SIZE
static struct proc_dir_entry *sipfw_proc_dir;				/*PROC��Ŀ¼*/
static struct proc_dir_entry *sipfw_proc_info;			/*����ǽ����Ϣ*/
static struct proc_dir_entry *sipfw_proc_defaultaction;	/*Ĭ�϶���*/
static struct proc_dir_entry *sipfw_proc_logpause;		/*ֹͣ��־д��*/
static struct proc_dir_entry *sipfw_proc_invalid;			/*����ǽ��ֹ*/
static char *cookie_pot;  

/*����ǽ��Ϣ������*/
int SIPFW_ProcInfoRead( 	char *buffer, 
					char **start, 
					off_t offset,
					int length, 
					int *eof, 
					void *data )
{
	int len;
	if (offset > 0) /*����ҳ*/
	{
		*eof = 1;
		return 0;
	}
	/*����Ϣ�����û��ռ��û�*/
	len = sprintf(buffer, 		/*��Ϣ����*/
		"DefaultAction:%s\n"	/*Ĭ�϶���*/
		"RulesFile:%s\n"		/*�����ļ�λ��*/
		"LogFile:%s\n"		/*��־�ļ�λ��*/
		"RulesNumber:%d\n"	/*�����ܸ���*/
		"HitNumber:%d\n"		/*�������й���������*/
		"FireWall:%s\n",		/*����ǽ�Ƿ���ֹ*/
		(char*)sipfw_action_name[cf.DefaultAction].ptr,
		cf.RuleFilePath,
		cf.LogFilePath,
		sipfw_tables[0].number+sipfw_tables[1].number+sipfw_tables[2].number,
		cf.HitNumber,
		cf.Invalid?"INVALID":"VALID");
	return len;
}

int SIPFW_ProcLogRead( 	char *buffer, 
					char **start, 
					off_t offset,
					int length, 
					int *eof, 
					void *data )
{
	int len;
	if (offset > 0) 
	{
		*eof = 1;
		return 0;
	}
	/*����־д����ֹ�����ø��û�*/
	len = sprintf(buffer, "%d\n",cf.LogPause);
	return len;
}
ssize_t SIPFW_ProcLogWrite( struct file *filp, 
					const char __user *buff,
					unsigned long len, 
					void *data )
{
	/*�����ݿ����뻺����*/
	if (copy_from_user( cookie_pot, buff, len )) 
	{
		return -EFAULT;
	}
	/*��ʽ��ȡ����ֵ*/
	sscanf(cookie_pot,"%d\n",&cf.LogPause);

	return len;
}

int SIPFW_ProcActionRead( 	char *buffer, 
					char **start, 
					off_t offset,
					int length, 
					int *eof, 
					void *data )
{
	int len;
	if (offset > 0) 
	{
		*eof = 1;
		return 0;
	}
	/*Ĭ�Ϲ��������Ƹ��û�*/
	len = sprintf(buffer, "%s\n",	(char*)sipfw_action_name[cf.DefaultAction].ptr);
	return len;
}
ssize_t SIPFW_ProcActionWrite( struct file *filp, 
					const char __user *buff,
					unsigned long len, 
					void *data )
{
	/*��ȡ�û�д�������*/
	if (copy_from_user( cookie_pot, buff, len )) 
	{
		return -EFAULT;
	}

	/*�Ƚ�д����ַ���*/
	if(!strcmp(cookie_pot, "ACCEPT"))
	{
		cf.DefaultAction = SIPFW_ACTION_ACCEPT;
	}
	else if(!strcmp(cookie_pot, "DROP"))
	{
		cf.DefaultAction = SIPFW_ACTION_DROP;
	}

	return len;
}
int SIPFW_ProcInvalidRead( 	char *buffer, 
					char **start, 
					off_t offset,
					int length, 
					int *eof, 
					void *data )
{
	int len;
	if (offset > 0) 
	{
		*eof = 1;
		return 0;
	}
	len = sprintf(buffer, "%d\n",	cf.Invalid);
	return len;
}
ssize_t SIPFW_ProcInvalidWrite( struct file *filp, 
					const char __user *buff,
					unsigned long len, 
					void *data )
{
	if (copy_from_user( cookie_pot, buff, len )) 
	{
		return -EFAULT;
	}

	sscanf(cookie_pot,"%d\n",&cf.Invalid);

	return len;
}

/*PROC�����ļ���ʼ������*/
int SIPFW_Proc_Init( void )
{
	int ret = 0;
	/*�����ڴ汣���û�д�������*/
	cookie_pot = (char *)vmalloc( MAX_COOKIE_LENGTH );
	if (!cookie_pot) /*����ʧ��*/
	{
		ret = -ENOMEM;
	} 
	else 
	{
		memset( cookie_pot, 0, MAX_COOKIE_LENGTH );/*���㻺����*/
		sipfw_proc_dir = proc_mkdir("sipfw",  proc_net);/*��/proc/net�½���sipfwĿ¼*/
		sipfw_proc_info = create_proc_entry( "information", 0644, sipfw_proc_dir );/*��Ϣ��*/
		sipfw_proc_defaultaction = create_proc_entry( "defaultaction", 0644, sipfw_proc_dir );/*Ĭ�϶�����*/
		sipfw_proc_logpause = create_proc_entry( "logpause", 0644, sipfw_proc_dir );/*��־��ֹ��*/
		sipfw_proc_invalid= create_proc_entry( "invalid", 0644, sipfw_proc_dir );/*����ǽ��ֹ��*/
		if (sipfw_proc_info == NULL /*�ж��Ƿ����ɹ�*/
			|| sipfw_proc_defaultaction == NULL 
			||sipfw_proc_logpause == NULL 
			||sipfw_proc_invalid == NULL) 
		{/*���лָ�����*/
			ret = -ENOMEM;
			vfree(cookie_pot);
		} 
		else 
		{
			sipfw_proc_info->read_proc = SIPFW_ProcInfoRead;/*��Ϣ������*/
			sipfw_proc_info->owner = THIS_MODULE;

			sipfw_proc_defaultaction->read_proc = SIPFW_ProcActionRead;/*����������*/
			sipfw_proc_defaultaction->write_proc= SIPFW_ProcActionWrite;/*����д����*/
			sipfw_proc_defaultaction->owner = THIS_MODULE;

			sipfw_proc_logpause->read_proc = SIPFW_ProcLogRead;/*��־������*/
			sipfw_proc_logpause->write_proc= SIPFW_ProcLogWrite;/*��־д����*/
			sipfw_proc_logpause->owner = THIS_MODULE;

			sipfw_proc_invalid->read_proc = SIPFW_ProcInvalidRead;/*����ǽ������*/
			sipfw_proc_invalid->write_proc= SIPFW_ProcInvalidWrite;/*����ǽд����*/
			sipfw_proc_invalid->owner = THIS_MODULE;
		}
	}
	return ret;
}

/*PROC�����ļ�������*/
void SIPFW_Proc_CleanUp( void )
{
	remove_proc_entry("defaultaction", sipfw_proc_dir);
	remove_proc_entry("logpause", sipfw_proc_dir);
	remove_proc_entry("invalid", sipfw_proc_dir);
	remove_proc_entry("information", sipfw_proc_dir);
	remove_proc_entry("sipfw", proc_net);
	
	vfree(cookie_pot);
}

