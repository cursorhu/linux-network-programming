#include <signal.h>
#include <stdio.h>

static void sig_handle(int signo)
{
	if( SIGSTOP== signo)
	{
		printf("���յ��ź�SIGSTOP\n");		
	}	
	else if(SIGKILL==signo)
	{
		printf("���յ��ź�SIGKILL\n");		
	}
	else
	{
		printf("���յ��ź�:%d\n",signo);		
	}
	
	return;
}

int main(void)
{
	sighandler_t ret;
	ret = signal(SIGSTOP, sig_handle);
	if(SIG_ERR == ret)
	{
		printf("ΪSIGSTOP�ҽ��źŴ�����ʧ��\n");
		return -1;		
	}	
	
	ret = signal(SIGKILL, sig_handle);
	if(SIG_ERR == ret)
	{
		printf("ΪSIGKILL�ҽ��źŴ�����ʧ��\n");
		return -1;		
	}
	
	for(;;);
	
}