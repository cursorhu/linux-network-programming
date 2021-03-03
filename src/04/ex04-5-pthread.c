/*
* ex04-pthread.c
* �߳�ʵ��
*/
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

static int run = 1;
static int retvalue ;

void *start_routine(void *arg)
{
		int *running = arg;
		printf("���̳߳�ʼ����ϣ��������Ϊ:%d\n",*running);
		while(*running)
		{
			printf("���߳���������\n");
			usleep(1);	
		}
		printf("���߳��˳�\n");
		
		retvalue = 8;
		pthread_exit( (void*)&retvalue);
}
static char stack[1024*64];
static size_t stacksize = 1024*64;
int main(void)
{
	pthread_t pt;
	int ret = -1;
	int times = 3;
	int i = 0; 
	int *ret_join = NULL;
	
	
	pthread_attr_t attr;
	struct sched_param sch;
	pthread_attr_init(&attr);
	pthread_attr_getschedparam(&attr, &sch);
	sch.sched_priority = 256;
	pthread_attr_setschedparam(&attr, &sch);
	
	printf("�߳����Ե�ջ��СΪ:%d\n",attr.__stacksize);
	attr.__stackaddr = stack;
	attr.__stacksize = stacksize;
	ret = pthread_create(&pt, &attr, (void*)start_routine, &run);
	if(ret != 0)
	{
		printf("�����߳�ʧ��\n");
		return 1;		
	}	
	pthread_attr_getschedparam(&attr, &sch);
	printf("�߳����Ե�ջ��СΪ:%d,���ȼ�Ϊ:%d\n",attr.__stacksize,sch.sched_priority);
	usleep(1);
	for(;i<times;i++)
	{
		printf("���̴߳�ӡ\n");
		usleep(1);
	}
	run = 0;
	pthread_join(pt,(void*)&ret_join);
	printf("�̷߳���ֵΪ:%d\n",*ret_join);
	return 0;
	
}