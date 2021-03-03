001		#include <sys/types.h>
002		#include <sys/socket.h>
003		#include <linux/un.h>
004		#include <string.h>
005		#include <signal.h>
006		#include <stdio.h>
007		#include <errno.h>
008		#include <unistd.h>
009		
010		/*
011		*	��fd�н�����Ϣ�������ļ�����������ָ��recvfd��
012		*/
013		ssize_t recv_fd(int fd, void *data, size_t bytes, int *recvfd)
014		{
015			struct msghdr msghdr_recv;/*������Ϣ����*/
016			struct iovec iov[1];/*�������ݵ�����*/
017			size_t n;
018			int newfd;
019			
020			union{
021				struct cmsghdr cm;
022				char control[CMSG_SPACE(sizeof(int))];	
023			}control_un;
024			struct cmsghdr *pcmsghdr;/*��Ϣͷ��*/
025			msghdr_recv.msg_control = control_un.control;/*������Ϣ*/
026			msghdr_recv.msg_controllen = sizeof(control_un.control);/*������Ϣ�ĳ���*/
027			
028			msghdr_recv.msg_name = NULL;/*��Ϣ������Ϊ��*/
029			msghdr_recv.msg_namelen = 0;/*��Ϣ�ĳ���Ϊ��*/
030			
031			iov[0].iov_base = data;/*����������Ϊ���������*/
032			iov[0].iov_len = bytes;/*�����ĳ���Ϊ�������ݵĳ���*/
033			msghdr_recv.msg_iov = iov;/*��Ϣ����ָ��*/
034			msghdr_recv.msg_iovlen = 1;/*��Ϣ�����ĸ���Ϊ1��*/
035			if((n = recvmsg(fd, &msghdr_recv, 0))<=0)/*������Ϣ*/
036				return n;
037				
038			if((pcmsghdr = CMSG_FIRSTHDR(&msghdr_recv))!= NULL &&/*�����Ϣ��ͷ��*/
039				pcmsghdr->cmsg_len == CMSG_LEN(sizeof(int))){/*���С�ܵĳ���Ϊint*/
040				if(pcmsghdr->cmsg_level != SOL_SOCKET)/*��Ϣ��levelӦ��ΪSOL_SOCKET*/
041					printf("control level != SOL_SOCKET\n");
042				
043				if(pcmsghdr->cmsg_type != SCM_RIGHTS)/*��Ϣ�������ж�*/
044					printf("control type != SCM_RIGHTS\n");
045					
046					*recvfd = *((int*)CMSG_DATA(pcmsghdr));		/*��ô��ļ���������*/
047			}else
048				*recvfd = -1;
049				
050			return n;/*���ؽ�����Ϣ�ĳ���*/
051		}
052		
053		int my_open(const char *pathname, int mode)
054		{
055			int fd, sockfd[2],status;
056			pid_t childpid;
057			char c, argsockfd[10],argmode[10];
058			
059			socketpair(AF_LOCAL,SOCK_STREAM,0,sockfd);/*����socket*/
060			if((childpid = fork())==0){/*�ӽ���*/
061				close(sockfd[0]);/*�ر�sockfd[0]*/
062				snprintf(argsockfd, sizeof(argsockfd),"%d",sockfd[1]);/*socket������*/
063				snprintf(argmode, sizeof(argmode),"%d",mode);/*���ļ��ķ�ʽ*/
064				execl("./openfile","openfile",argsockfd, pathname,argmode,(char*)NULL)	;/*ִ�н���A*/
065				printf("execl error\n");
066			}	
067			/*������*/
068			close(sockfd[1]);
069			/*�ȴ��ӽ��̽���*/
070			waitpid(childpid, &status,0);
071			
072			if(WIFEXITED(status)==0){/*�ж��ӽ����Ƿ����*/
073				printf("child did not terminate\n")	;
074			if((status = WEXITSTATUS(status))==0){/*�ӽ��̽���*/
075				recv_fd(sockfd[0],&c,1,&fd);/*���ս���A�򿪵��ļ�������*/
076			}	else{
077				errno = status;
078				fd = -1;	
079			}	
080			
081			close(sockfd[0]);/*�ر�sockfd[0]*/
082			return fd;/*���ؽ���A���ļ���������*/
083		
084			}
085		}
086		
087		#define BUFFSIZE 256	/*���յĻ�������С*/
088		int main(int argc, char *argv[])
089		{
090			int fd, n;
091			char buff[BUFFSIZE];	/*���ջ�����*/
092			
093			if(argc !=2)
094				printf("error argc\n");
095				
096			if((fd = my_open(argv[1], O_RDONLY))<0)/*��ý���A�򿪵��ļ�������*/
097				printf("can't open %s\n",argv[1]);
098				
099			while((n = read(fd, buff, BUFFSIZE))>0)/*��ȡ����*/
100			write(1,buff,n);/*д���׼���*/
101			
102			return(0);	
103		}
