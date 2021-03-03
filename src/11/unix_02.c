01		#include <sys/types.h>
02		#include <sys/socket.h>
03		#include <linux/un.h>
04		#include <string.h>
05		#include <signal.h>
06		#include <stdio.h>
07		#include <errno.h>
08		#include <unistd.h>
09		
10		ssize_t send_fd(int fd, void *data, size_t bytes, int sendfd)
11		{
12			struct msghdr msghdr_send;	/*������Ϣ*/
13			struct iovec iov[1];		/*����*/
14			size_t n;					/*��С*/
15			int newfd;					/*�ļ�������*/
16			/* �������msg�Ľṹ */
17			union{
18				struct cmsghdr cm;		/*control msg�ṹ*/
19				char control[CMSG_SPACE(sizeof(int))];	/*�ַ�ָ�룬�������*/
20			}control_un;
21			struct cmsghdr *pcmsghdr=NULL;/*����ͷ����ָ��*/
22			msghdr_send.msg_control = control_un.control;/*������Ϣ*/
23			msghdr_send.msg_controllen = sizeof(control_un.control);/*����*/
24			
25			pcmsghdr = CMSG_FIRSTHDR(&msghdr_send);/*ȡ�õ�һ����Ϣͷ*/
26			pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));/*��ó���*/
27			pcmsghdr->cmsg_level = SOL_SOCKET;/*���ڿ�����Ϣ*/
28			pcmsghdr->cmsg_type = SCM_RIGHTS;
29			*((int*)CMSG_DATA(pcmsghdr))= sendfd;/*socketֵ*/
30			
31			
32			msghdr_send.msg_name = NULL;/*����*/
33			msghdr_send.msg_namelen = 0;/*���Ƴ���*/
34			
35			iov[0].iov_base = data;/*����ָ��*/
36			iov[0].iov_len = bytes;/*���ݳ���*/
37			msghdr_send.msg_iov = iov;/*�����Ϣ*/
38			msghdr_send.msg_iovlen = 1;
39			
40			return (sendmsg(fd, &msghdr_send, 0));/*������Ϣ*/
41		}
42		
43		
44		int main(int argc, char *argv[])
45		{
46			int fd;
47			ssize_t n;
48			
49			if(argc != 4){
50				printf("socketpair error\n");
51			if((fd = open(argv[2],atoi(argv[3])))<0)
52				return(0);
53				
54			if((n = put_fd(atoi(argv[1]),"",1,fd))<0)
55				return(0);
56		}