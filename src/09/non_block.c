
#define PORT 9999
#define BACKLOG 4
int main(int argc, char *argv[]) 
{ 
	streuct sockaddr_in local,client;
	int len;
	int s_s  = -1,s_c= -1;
	/* ��ʼ����ַ�ṹ */
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT);
	local.sin_addr = htonl(-1);
	
	/*�����׽���������*/
	s_s = socket(AF_INET, SOCK_STREAM, 0);
	/*���÷�������ʽ*/
	fcntl(s_s,F_SETFL, O_NONBLOCK); 
	/*����*/
	listen(s_s, BACKLOG);
	for(;;)
	{
		/*��ѯ���տͻ���*/
		while(s_c < 0){/*�ȴ��ͻ��˵���*/
			s_c =accept(s_s, (struct sockaddr*)&client, &len);
		}
		
		/*��ѯ���գ������յ����ݵ�ʱ���˳�whileѭ��*/
		while(recv(s_c, buffer, 1024)<=0)
			;
		/* ���յ��ͻ��˵����� */
		if(strcmp(buffer, "HELLO")==0){/*�ж��Ƿ�ΪHELLO�ַ���*/
			send(s, "OK", 3, 0);/*������Ӧ*/
			close(s_c);			/*�ر�����*/
			continue;			/*�����ȴ��ͻ�������*/
		}
		
		if(strcmp(buffer, "SHUTDOWN")==0){/*�ж��Ƿ�ΪSHUTDOWN�ַ���*/
			send(s, "BYE", 3, 0);	/*����BYE�ַ���*/
			close(s_c);				/*�رտͻ�������*/
			break;					/*�˳���ѭ��*/
		}
		
	}
	close(s_s);
	
	return 0;
}