#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>

void msg_show_attr(int msg_id, struct msqid_ds msg_info)
{
	int ret = -1;
	sleep(1);
	ret = msgctl(msg_id, IPC_STAT, &msg_info);
	if( -1 == ret)
	{
		printf("�����Ϣ��Ϣʧ��\n");
		return ;		
	}
	
	printf("\n");
	printf("���ڶ����е��ֽ�����%d\n",msg_info.msg_cbytes);
	printf("��������Ϣ����%d\n",msg_info.msg_qnum);
	printf("����������ֽ�����%d\n",msg_info.msg_qbytes);
	printf("�������Ϣ�Ľ���pid��%d\n",msg_info.msg_lspid);
	printf("��������Ϣ�Ľ���pid��%d\n",msg_info.msg_lrpid);
	printf("�������Ϣ��ʱ�䣺%s",ctime(&(msg_info.msg_stime)));
	printf("��������Ϣ��ʱ�䣺%s",ctime(&(msg_info.msg_rtime)));
	printf("���仯ʱ�䣺%s",ctime(&(msg_info.msg_ctime)));
	printf("��ϢUID�ǣ�%d\n",msg_info.msg_perm.uid);
	printf("��ϢGID�ǣ�%d\n",msg_info.msg_perm.gid);
}


int main(void)
{
	int ret = -1;
	int msg_flags, msg_id;
	key_t key;
	struct msgmbuf{
		int mtype;
		char mtext[10];
		};	
	struct msqid_ds msg_info;
	struct msgmbuf msg_mbuf;
	
	int msg_sflags,msg_rflags;
	char *msgpath = "/ipc/msg/";
	key = ftok(msgpath,'b');
	if(key != -1)
	{
		printf("�ɹ�����KEY\n");		
	}
	else
	{
		printf("����KEYʧ��\n");		
	}
	
	msg_flags = IPC_CREAT|IPC_EXCL;
	msg_id = msgget(key, msg_flags|0x0666);
	if( -1 == msg_id)
	{
		printf("��Ϣ����ʧ��\n");
		return 0;		
	}	
	msg_show_attr(msg_id, msg_info);
	
	msg_sflags = IPC_NOWAIT;
	msg_mbuf.mtype = 10;
	memcpy(msg_mbuf.mtext,"������Ϣ",sizeof("������Ϣ"));
	ret = msgsnd(msg_id, &msg_mbuf, sizeof("������Ϣ"), msg_sflags);
	if( -1 == ret)
	{
		printf("������Ϣʧ��\n");		
	}
	msg_show_attr(msg_id, msg_info);
	
	msg_rflags = IPC_NOWAIT|MSG_NOERROR;
	ret = msgrcv(msg_id, &msg_mbuf, 10,10,msg_rflags);
	if( -1 == ret)
	{
		printf("������Ϣʧ��\n");		
	}
	else
	{
		printf("������Ϣ�ɹ������ȣ�%d\n",ret);		
	}
	msg_show_attr(msg_id, msg_info);
	
	msg_info.msg_perm.uid = 8;
	msg_info.msg_perm.gid = 8;
	msg_info.msg_qbytes = 12345;
	ret = msgctl(msg_id, IPC_SET, &msg_info);
	if( -1 == ret)
	{
		printf("������Ϣ����ʧ��\n");
		return 0;		
	}
	msg_show_attr(msg_id, msg_info);
	
	ret = msgctl(msg_id, IPC_RMID,NULL);
	if(-1 == ret)
	{
		printf("ɾ����Ϣʧ��\n");
		return 0;		
	}
	return 0;
}