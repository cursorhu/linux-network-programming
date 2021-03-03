01	
02		#define CMD_DISCOVERY "CMD_DISCOVERY"        /*IP��������*/
03		#define CMD_DISCOVERY_ACK "CMD_DISCOVERY_ACK"/*IP����Ӧ������*/
04		void	HandleIPFound(void *arg)
05		{
06		#define BUFFER_LEN 32
07			int ret = -1;
08			SOCKET sock = -1;
09			struct sockaddr_in local_addr;	/*���ص�ַ*/
10			struct sockaddr_in from_addr;	/*�ͻ��˵�ַ*/
11	    	int from_len;
12			int count = -1;
13			fd_set readfd;
14			char buff[BUFFER_LEN];
15			struct timeval timeout;	
16			timeout.tv_sec = 2;	/*��ʱʱ��2��*/
17			timeout.tv_usec = 0;
18		
19			DBGPRINT("==>HandleIPFound\n");
20			
21	    	sock = socket(AF_INET, SOCK_DGRAM, 0);/*�������ݱ��׽���*/
22			if( sock < 0 )
23			{
24				DBGPRINT("HandleIPFound: socket init error\n");
25				return;
26			}
27			
28			/*��������*/
29			memset((void*)&local_addr, 0, sizeof(struct sockaddr_in));
30			local_addr.sin_family = AF_INET;/*Э����*/
31	    	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);/*���ص�ַ*/
32			local_addr.sin_port = htons(MCAST_PORT);/*�����˿�*/
33			/*��*/
34			ret = bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr));
35			if(ret != 0)
36			{
37				DBGPRINT("HandleIPFound:bind error\n");
38				return;
39			}
40		
41	    	/*���������*/
42			while(1)
43			{
44				/*�ļ���������������*/
45				FD_ZERO(&readfd);
46				/*���׽����ļ����������������*/
47				FD_SET(sock, &readfd);
48				/*select�����Ƿ������ݵ���*/
49				ret = selectsocket(sock+1, &readfd, NULL, NULL, &timeout);
50				switch(ret)
51	    		{
52					case -1:
53						/*��������*/
54						break;
55					case 0:
56						/*��ʱ*/
57						//DO timeout THING
58						
59						break;
60					default:
61	    				/*�����ݵ���*/
62						if( FD_ISSET( sock, &readfd ) )
63						{
64							/*��������*/
65							count = recvfrom( sock, buff, BUFFER_LEN, 0, ( struct sockaddr * ) &from_addr, &from_len );
66							DBGPRINT( "Recv msg is %s\n", buff );
67							if( strstr( buff, CMD_DISCOVERY ) )/*�ж��Ƿ��Ǻ�*/
68							{
69								/*��Ӧ�����ݿ���*/
70								memcpy(buff, CMD_DISCOVERY_ACK,strlen(CMD_DISCOVERY_ACK)+1);
71	    						/*���͸��ͻ���*/
72								count = sendto( sock, buff, strlen( buff ), 0, ( struct sockaddr * ) &from_addr, from_len );
73							}
74						}
75				}
76			}
77			PRINT("<==HandleIPFound\n");
78
79			return;
80		}