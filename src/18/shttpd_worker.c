#include "shttpd.h"

static int workersnum = 0;/*�����̵߳�����*/
static struct worker_ctl *wctls = NULL;/*�߳�ѡ��*/
extern struct conf_opts conf_para;
pthread_mutex_t thread_init = PTHREAD_MUTEX_INITIALIZER;
static int WORKER_ISSTATUS(int status);
static void Worker_Init();
static int Worker_Add(int i);
static void Worker_Delete(int i);
static void Worker_Destory();



static void do_work(struct worker_ctl *wctl)
{
	DBGPRINT("==>do_work\n");
	struct timeval tv;		/*��ʱʱ��*/
	fd_set rfds;			/*���ļ���*/
	int fd = wctl->conn.cs;/*�ͻ��˵��׽���������*/
	struct vec *req = &wctl->conn.con_req.req;/*���󻺳�������*/
	
	int retval = 1;

	for(;retval > 0;)
	{
		/*����ļ���,���ͻ�������
			������������ļ���*/
		FD_ZERO(&rfds);	
		FD_SET(fd, &rfds);	

		/*���ó�ʱ*/
		tv.tv_sec = 300;//conf_para.TimeOut;
		tv.tv_usec = 0;

		/*��ʱ������*/
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);
		switch(retval)
		{
			case -1:/*����*/
				close(fd);
				break;
			case 0:/*��ʱ*/
				close(fd);
				break;
			default:
				printf("select retval:%d\n",retval);
				if(FD_ISSET(fd, &rfds))/*����ļ�*/
				{
					memset(wctl->conn.dreq, 0, sizeof(wctl->conn.dreq));
					/*��ȡ�ͻ�������*/
					req->len = 
						read(wctl->conn.cs, wctl->conn.dreq, sizeof(wctl->conn.dreq));
					req->ptr = wctl->conn.dreq;
					DBGPRINT("Read %d bytes,'%s'\n",req->len,req->ptr);
					if(req->len > 0)
					{
						/*�����ͻ��˵�����*/
						wctl->conn.con_req.err =	Request_Parse(wctl);
						/*������Ӧ�ͻ�������*/
						Request_Handle(wctl);				
					}
					else
					{
						close(fd);
						retval = -1;
					}
				}
		}
	}
	DBGPRINT("<==do_work\n");
}

/*�̴߳�����*/
static void *worker(void *arg)
{
	DBGPRINT("==>worker\n");
	struct worker_ctl *ctl = (struct worker_ctl *)arg;
	struct worker_opts *self_opts = &ctl->opts;

	pthread_mutex_unlock(&thread_init);
	/*��ʼ���߳�Ϊ���У��ȴ�����*/
	self_opts->flags = WORKER_IDEL;

	/*��������߳�û���ô��߳��˳�����ѭ����������*/
	for(;self_opts->flags != WORKER_DETACHING;)
	{
		//DBGPRINT("work:%d,status:%d\n",(int)self_opts->th,self_opts->flags );
		/*�鿴�Ƿ����������*/
		int err = pthread_mutex_trylock(&self_opts->mutex);
		if(err)
		{
			//DBGPRINT("NOT LOCK\n");
			sleep(1);
			continue;
		}
		else
		{
			/*������do it*/
			DBGPRINT("Do task\n");
			self_opts->flags = WORKER_RUNNING;
			do_work(ctl);
			close(ctl->conn.cs);
			ctl->conn.cs = -1;
			if(self_opts->flags == WORKER_DETACHING)
				break;
			else
				self_opts->flags = WORKER_IDEL;
		}
	}

	/*���ط����˳�����*/
	/*����״̬Ϊ��ж��*/
	self_opts->flags = WORKER_DETACHED;
	workersnum--;/*�����߳�-1*/

	DBGPRINT("<==worker\n");
	return NULL;
}


/*�߳�״̬*/
static int WORKER_ISSTATUS(int status)
{
	int i = 0;
	for(i = 0; i<conf_para.MaxClient;i++)
	{
		if(wctls[i].opts.flags == status)
			return i;
	}

	return -1;
}


/*��ʼ���߳�*/
static void Worker_Init()
{
	DBGPRINT("==>Worker_Init\n");
	int i = 0;
	/*��ʼ���ܿز���*/
	wctls = (struct worker_ctl*)malloc(sizeof(struct worker_ctl)*conf_para.MaxClient);
	memset(wctls, 0, sizeof(*wctls)*conf_para.MaxClient);/*����*/

	/*��ʼ��һЩ����*/
	for(i = 0; i<conf_para.MaxClient;i++)
	{
		/*opts&conn�ṹ��worker_ctl�ṹ�γɻ�ָ��*/
		wctls[i].opts.work = &wctls[i];
		wctls[i].conn.work = &wctls[i];

		/*opts�ṹ���ֵĳ�ʼ��*/
		wctls[i].opts.flags = WORKER_DETACHED;
		//wctls[i].opts.mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&wctls[i].opts.mutex,NULL);
		pthread_mutex_lock(&wctls[i].opts.mutex);

		/*conn���ֵĳ�ʼ��*/
		/*con_req&con_res��conn�ṹ�γɻ�ָ*/
		wctls[i].conn.con_req.conn = &wctls[i].conn;
		wctls[i].conn.con_res.conn = &wctls[i].conn;
		wctls[i].conn.cs = -1;/*�ͻ���socket����Ϊ��*/
		/*con_req���ֳ�ʼ��*/
		wctls[i].conn.con_req.req.ptr = wctls[i].conn.dreq;
		wctls[i].conn.con_req.head = wctls[i].conn.dreq;
		wctls[i].conn.con_req.uri = wctls[i].conn.dreq;
		/*con_res���ֳ�ʼ��*/
		wctls[i].conn.con_res.fd = -1; 
		wctls[i].conn.con_res.res.ptr = wctls[i].conn.dres;
	}
	
	for(i = 0; i<conf_para.InitClient;i++)
	{
		/*���ӹ涨���������߳�*/
		Worker_Add(i);
	}

	DBGPRINT("<==Worker_Init\n");
}


/*�����߳�*/
static int Worker_Add(int i)
{
	DBGPRINT("==>Worker_Add\n");
	pthread_t th;
	int err = -1;
	if( wctls[i].opts.flags == WORKER_RUNNING)
		return 1;

	pthread_mutex_lock(&thread_init);
	wctls[i].opts.flags = WORKER_INITED;/*״̬Ϊ�ѳ�ʼ��*/
	err = pthread_create(&th, NULL, worker, (void*)&wctls[i]);/*�����߳�*/
	
	pthread_mutex_lock(&thread_init);
	pthread_mutex_unlock(&thread_init);
	/*�����߳�ѡ��*/	
	wctls[i].opts.th = th;/*�߳�ID*/
	workersnum++;/*�߳���������1*/

	DBGPRINT("<==Worker_Add\n");
	return 0;
}

/*�����߳�*/
static void Worker_Delete(int i)
{
	DBGPRINT("==>Worker_Delete\n");
	wctls[i].opts.flags = WORKER_DETACHING;/*�߳�״̬��Ϊ����ж��*/
	DBGPRINT("<==Worker_Delete\n");
}

/*�����߳�*/
static void Worker_Destory()
{
	DBGPRINT("==>Worker_Destory\n");
	int i = 0;
	int clean = 0;
	
	for(i=0;i<conf_para.MaxClient;i++)
	{
		DBGPRINT("thread %d,status %d\n",i,wctls[i].opts.flags );
		if(wctls[i].opts.flags != WORKER_DETACHED)
			Worker_Delete(i);
	}

	while(!clean)
	{
		clean = 1;
		for(i = 0; i<conf_para.MaxClient;i++){
			DBGPRINT("thread %d,status %d\n",i,wctls[i].opts.flags );
			if(wctls[i].opts.flags==WORKER_RUNNING 
				|| wctls[i].opts.flags == WORKER_DETACHING)
				clean = 0;
		}
		if(!clean)
			sleep(1);
	}
	DBGPRINT("<==Worker_Destory\n");
}


#define STATUS_RUNNING 1
#define STATSU_STOP 0
static int SCHEDULESTATUS = STATUS_RUNNING;
/*�����ȹ���,
*	���пͻ������ӵ�����ʱ��
*	���ͻ������ӷ�������пͻ���
*	�ɿͻ��˴�����������
*/
int Worker_ScheduleRun(int ss)
{
	DBGPRINT("==>Worker_ScheduleRun\n");
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	Worker_Init();
	
	int i = 0;

	for(;SCHEDULESTATUS== STATUS_RUNNING;)
	{
		struct timeval tv;		/*��ʱʱ��*/
		fd_set rfds;			/*���ļ���*/
		//printf("SCHEDULESTATUS:%d\n",SCHEDULESTATUS);
		int retval = -1;

		/*����ļ���,���ͻ�������
			������������ļ���*/
		FD_ZERO(&rfds);	
		FD_SET(ss, &rfds);	

		/*���ó�ʱ*/
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		/*��ʱ������*/
		retval = select(ss + 1, &rfds, NULL, NULL, &tv);
		switch(retval)
		{
			case -1:/*����*/
			case 0:/*��ʱ*/
				continue;
				break;
			default:
				if(FD_ISSET(ss, &rfds))/*����ļ�*/
				{
					int sc = accept(ss, (struct sockaddr*)&client, &len);
					printf("client comming\n");
					i = WORKER_ISSTATUS(WORKER_IDEL);
					if(i == -1)
					{
						i =  WORKER_ISSTATUS(WORKER_DETACHED);
						if(i != -1)
							Worker_Add( i);
					}
					if(i != -1)
					{
						wctls[i].conn.cs = sc;
						pthread_mutex_unlock(&wctls[i].opts.mutex);
					}				
				}
		}		
	}
	
	DBGPRINT("<==Worker_ScheduleRun\n");
	return 0;
}

/*ֹͣ���ȹ���*/
int Worker_ScheduleStop()
{
	DBGPRINT("==>Worker_ScheduleStop\n");
	SCHEDULESTATUS = STATSU_STOP;
	int i =0;

	Worker_Destory();
	int allfired = 0;
	for(;!allfired;)
	{
		allfired = 1;
		for(i = 0; i<conf_para.MaxClient;i++)
		{
			int flags = wctls[i].opts.flags;
			if(flags == WORKER_DETACHING || flags == WORKER_IDEL)
				allfired = 0;
		}
	}
	
	pthread_mutex_destroy(&thread_init);
	for(i = 0; i<conf_para.MaxClient;i++)
		pthread_mutex_destroy(&wctls[i].opts.mutex);
	free(wctls);
	
	DBGPRINT("<==Worker_ScheduleStop\n");
	return 0;
}

