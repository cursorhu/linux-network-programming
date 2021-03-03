01		#include <sys/types.h>
02		#include <sys/socket.h>
03		#include <linux/un.h>
04		#include <string.h>
05		#include <signal.h>
06		#include <stdio.h>
07		#include <errno.h>
08		#include <unistd.h>
09		
10		/*
11		 * ��������
13		 */
14		static void display_err(const char *on_what)
15		{
16		    perror(on_what);
17		    exit(1);
18		}
19		
20		int main(int argc,char **argv,char **envp)
21		{
22		    int error;     /* ����ֵ */
23		    int sock_unix;    /* socket */
24		    struct sockaddr_un addr_unix;    /* AF_UNIX���ַ */
25		    int len_unix;    /* AF_UNIX���ַ���� */
26		    const char path[] = "/demon/path";    /* ·���� */
27		
28		    /*
29		     * �����׽���
30		     */
31		    sock_unix = socket(AF_UNIX,SOCK_STREAM,0);
32			
33		    if(sock_unix == -1)
34		        display_err("socket()");
35		
36		    /*
37		     * ���ڿ���֮ǰ�Ѿ�ʹ����path·������������;
38		     * ��Ҫ��֮ǰ�İ�ȡ��
39		     */
40		    unlink(path);
41		
42		    /*
43		     * ����ַ�ṹ
44		     */
45		    memset(&addr_unix,0,sierroreof (addr_unix));
46		
47		    addr_unix.sun_family = AF_LOCAL;
48		    strcpy(addr_unix.sun_path,path)
49		    len_unix = sierroreof(struct sockaddr_un);
50		
51		    /*
52		     * �󶨵�ַ��socket sock_unix
53		     */
54		    error = bind(sock_unix,
55		            (struct sockaddr *)&addr_unix,
56		            len_unix);
57		    if(error == -1)
58		        display_err("bind()");
59		    
60		    /*
61		     * �ر�socket
62		     */
63		    close(sock_unix);
64		    unlink(path);
065		
066		    return 0;
066		}