#ifndef __KERNEL__
#define __KERNEL__
#endif /*__KERNEL__*/
#ifndef MODULE
#define MODULE
#endif /*MODULE*/
#include "sipfw.h"

/*���ļ�*/
struct file *SIPFW_OpenFile(const char *filename, int flags, int mode)
{
	struct file *f = NULL;

	DBGPRINT("==>SIPFW_OpenFile\n");

	f = filp_open(filename, flags, 0);	/*filp_open���ļ�*/
	if (!f || IS_ERR(f))				/*�жϴ���*/
	{
		f = NULL;
	}

	DBGPRINT("<==SIPFW_OpenFile\n");
	return f;
}


/*���ļ��ж�ȡһ�У����Ϊlen�ֽڣ��ŵ�buf��*/
ssize_t SIPFW_ReadLine(struct file *f, char *buf, size_t len)
{
#define EOF (-1)/*�ļ�����*/

	ssize_t count = -1;
	mm_segment_t oldfs;/*�ϵĵ�ַ�ռ����÷�ʽ*/
	struct inode *inode;/*�ڵ�*/
	DBGPRINT("==>SIPFW_ReadLine\n");

	/*�ж������������ȷ��*/
	if (!f || IS_ERR(f) || !buf || len <= 0) 
	{
		goto out_error;
	}

	/*�ж��ļ�ָ���Ƿ���ȷ*/
	if (!f || !f->f_dentry || !f->f_dentry->d_inode)
	{
		goto out_error;
	}
	/*�ļ��ڵ�*/
	inode = f->f_dentry->d_inode;

	/*�ж��ļ�Ȩ���Ƿ�ɶ�*/
	if (!(f->f_mode & FMODE_READ))
	{
		goto out_error;
	}

	/*�Ƿ����ļ���������*/
	if (f->f_op && f->f_op->read) 
	{
		oldfs = get_fs();			/*��õ�ַ����*/
		set_fs(KERNEL_DS);		/*����Ϊ�ں�ģʽ*/
		count = 0;

		if (f->f_op->read(f, buf, 
			1, &f->f_pos) == 0)	/*��ȡ����ʧ��*/
		{
			DBGPRINT("file read failure\n");
			goto out;
		}

		if (*buf == EOF)			/*�ļ�����*/
		{
			DBGPRINT("file EOF\n");
			goto out;
		}
		count = 1;
		while (*buf != EOF		/*�ļ�����*/
			&& *buf != '\0' 		/*��*/
			&& *buf != '\n' 		/*�س�*/
			&& *buf != '\r'		/*����*/
		       && count < len		/*������д��*/
		       && f->f_pos <= inode->i_size) /*�ļ���������*/
		{
			buf 		+= 1;		/*��������ַ�ƶ�*/
			count 	+= 1;		/*��������*/
			if (f->f_op->read(f, buf, 1, &f->f_pos) <= 0) 
			{
				count -= 1;
				break;
			}
		}
	} 
	else							/*û�в�������*/
	{
		goto out_error;
	}

	if (*buf == '\r' 				/*����β�������ַ�*/
		|| *buf =='\n' 
		||*buf == EOF ) 
	{
		*buf = '\0';				/*�޸�Ϊ���ַ�*/
		count -= 1;				/*�ַ�����1*/
	} 
	else							/*β���ַ������滻*/
	{
		buf += 1;				/*�ƶ�һλ*/
		*buf = '\0';				/*��Ϊ���ַ�*/
	}
	
out:
	set_fs(oldfs);					/*�ظ�ԭ���ĵ�ַ���÷�ʽ*/
out_error:
	DBGPRINT("<==SIPFW_ReadLine\n");
	return count;
}

/*���ļ���д��һ��*/
ssize_t SIPFW_WriteLine(struct file *f, char *buf, size_t len)
{
	ssize_t count = -1;
	mm_segment_t oldfs;
	struct inode *inode;
	DBGPRINT("==>SIPFW_WriteLine\n");

	/*�ж������������ȷ��*/
	if (!f || IS_ERR(f) || !buf || len <= 0) 
	{
		goto out_error;
	}
	/*�ж��ļ�ָ���Ƿ���ȷ*/
	if (!f || !f->f_dentry || !f->f_dentry->d_inode)
	{
		goto out_error;
	}

	inode = f->f_dentry->d_inode;

	/*�ж��ļ�Ȩ���Ƿ��д*/
	if (!(f->f_mode & FMODE_WRITE) || !(f->f_mode & FMODE_READ) )
	{
		goto out_error;
	}

	/*�Ƿ����ļ���������*/
	if (f->f_op && f->f_op->read && f->f_op->write) 
	{
		//f->f_pos = f->f_count;
		oldfs = get_fs();			/*��õ�ַ����*/
		set_fs(KERNEL_DS);		/*����Ϊ�ں�ģʽ*/
		count = 0;

		count = f->f_op->write(f, buf, len, &f->f_pos) ;

		if (count == -1)			/*д������ʧ��*/
		{
			goto out;
		}		
	} 
	else							/*û�в�������*/
	{
		goto out_error;
	}

out:
	set_fs(oldfs);					/*�ظ�ԭ���ĵ�ַ���÷�ʽ*/
out_error:
	DBGPRINT("<==SIPFW_WriteLine\n");
	return count;
}

/*�ر��ļ�*/
void SIPFW_CloseFile(struct file *f)
{
	DBGPRINT("==>SIPFW_CloseFile\n");
	if(!f)
		return;
	
	filp_close(f, current->files);
	DBGPRINT("<==SIPFW_CloseFile\n");
}

/*����������������Ϣд����־�ļ�����ʽΪ
*from [IP:port] to [IP:port] protocol [string] was [Action name]
*/
int SIPFW_LogAppend(struct sk_buff *skb, struct sipfw_rules *r)
{
	char buff[2048];		/*����д���ļ�������*/
	struct file *f;			/*��־�ļ�*/
	int retval = 0;		/*����ֵ*/
	struct tm cur;		/*��ǰ����ָ��*/
	unsigned long time;	/*��ǰ������*/
	const struct vec *proto;/*Э������*/
	struct iphdr *iph = skb->nh.iph;/*�������ݵ�IPͷ��*/

	if(cf.LogPause)		/*��ͣ����־��д���ļ�*/
	{
		retval = -1;		/*����*/
		goto EXITSIPFW_LogAppend;
	}

	/*����־�ļ�*/
	f = SIPFW_OpenFile(cf.LogFilePath, O_CREAT|O_RDWR|O_APPEND, 0);
	if(f == NULL)
	{
		retval = -1;
		goto EXITSIPFW_LogAppend;
	}
	time = get_seconds();		/*��õ�ǰʱ��*/
	SIPFW_Localtime(&cur, time);/*ת��Ϊ���������*/

	/*����Э������*/
	for(proto = &sipfw_protocol_name[0];
		proto->ptr != NULL && proto->value != iph->protocol;
		proto++)
		;

	/*����д����־�ļ���������Ϣ*/
	snprintf(buff, 						/*��Ϣ������*/
		2048,							/*����������*/
		"Time: %04d-%02d-%02d "			/*���ڵ�������*/
		"%02d:%02d:%02d  "				/*���ڵ�ʱ����*/
		"From %d.%d.%d.%d "				/*��ԴIP*/
		"To %d.%d.%d.%d "				/*Ŀ��IP*/
		" %s PROTOCOL "					/*Э������*/
		"was %sed\n",					/*����ʽ����*/
		cur.year,	cur.mon, cur.mday,		/*������*/
		cur.hour,  cur.min,  cur.sec,			/*ʱ����*/
		(iph->saddr & 0x000000FF)>>0,		/*Դ��ַ��һ��*/
		(iph->saddr & 0x0000FF00)>>8,		/*Դ��ַ�ڶ���*/
		(iph->saddr & 0x00FF0000)>>16,	/*Դ��ַ������*/
		(iph->saddr & 0xFF000000)>>24,	/*Դ��ַ���Ķ�*/
		(iph->daddr & 0x000000FF)>>0,		/*Ŀ�ĵ�ַ��һ��*/
		(iph->daddr & 0x0000FF00)>>8,		/*Ŀ�ĵ�ַ�ڶ���*/
		(iph->daddr & 0x00FF0000)>>16,	/*Ŀ�ĵ�ַ������*/
		(iph->daddr & 0xFF000000)>>24,	/*Ŀ�ĵ�ַ���Ķ�*/
		(char*)proto->ptr,					/*Э������*/
		(char*)sipfw_action_name[r->action].ptr);/*��������*/
	SIPFW_WriteLine(f, buff, strlen(buff));	/*д���ļ�*/
	SIPFW_CloseFile( f);					/*�ر��ļ�*/
	
EXITSIPFW_LogAppend:
	return retval;	
}

/*�������ļ��ж�ȡ������Ϣ*/
int SIPFW_HandleConf(void)
{
	int retval = 0,count;
	char *pos = NULL;
	struct file *f = NULL;
	char line[256];
	DBGPRINT("==>SIPFW_HandleConf\n");
	f = SIPFW_OpenFile("/etc/sipfw.conf", /*���ļ�*/
				O_CREAT|O_RDWR|O_APPEND, 0);
	if(f == NULL)/*ʧ��*/
	{
		retval = -1;
		goto EXITSIPFW_HandleConf;
	}	

	while((count = SIPFW_ReadLine(f, line, 256))>0)/*��ȡһ��*/
	{
		pos = line;							/*����ͷ*/
		
		if(!strncmp(pos, "DefaultAction",13))		/*Ĭ�϶���?*/
		{
			pos += 13+1;						/*����λ��*/
			if(!strncmp(pos, "ACCEPT",6))		/*�Ƿ�ACCEPT*/
			{
				cf.DefaultAction = SIPFW_ACTION_ACCEPT;
			}
			else if(!strncmp(pos, "DROP",4))	/*�Ƿ�DROP*/
			{
				cf.DefaultAction = SIPFW_ACTION_DROP;
			}
		}
		else if(!strncmp(pos, "RulesFile",9))		/*�����ļ�·��*/
		{
			pos += 10;
			strcpy(cf.RuleFilePath, pos);		/*����*/
		}
		else if(!strncmp(pos, "LogFile",7))		/*��־�ļ�·��*/
		{
			pos += 8;
			strcpy(cf.LogFilePath,pos );			/*����*/
		}
	}
	SIPFW_CloseFile(f);						/*�ر��ļ�*/
EXITSIPFW_HandleConf:
	DBGPRINT("<==SIPFW_HandleConf\n");
	return retval;
}




