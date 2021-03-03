#ifndef __SHTTPD_H__
#define __SHTTPD_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>   /* for sockaddr_in */
#include <netdb.h>        /* for hostent */ 
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>        /* we want to catch some of these after all */
#include <unistd.h>       /* protos for read, write, close, etc */
#include <dirent.h>       /* for MAXNAMLEN */
#include <limits.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stddef.h>


/*�̵߳�״ֵ̬*/
enum{WORKER_INITED, WORKER_RUNNING,WORKER_DETACHING, WORKER_DETACHED,WORKER_IDEL};

struct conf_opts{
	char CGIRoot[128];		/*CGI��·��*/
	char DefaultFile[128];		/*Ĭ���ļ�����*/
	char DocumentRoot[128];	/*���ļ�·��*/
	char ConfigFile[128];		/*�����ļ�·��������*/
	int ListenPort;			/*�����˿�*/
	int MaxClient;			/*���ͻ�������*/
	int TimeOut;				/*��ʱʱ��*/
	int InitClient;				/*��ʼ���߳�����*/
};



/* HTTPЭ��ķ��� */
typedef enum SHTTPD_METHOD_TYPE{
	METHOD_GET, 		/*GET     ����*/
	METHOD_POST, 		/*POST   ����*/
	METHOD_PUT, 		/*PUT     ����*/
	METHOD_DELETE, 	/*DELETE����*/
	METHOD_HEAD,		/*HEAD   ����*/
	METHOD_CGI,		/**CGI����*/
	METHOD_NOTSUPPORT
}SHTTPD_METHOD_TYPE;

enum {HDR_DATE, HDR_INT, HDR_STRING};	/* HTTP header types		*/

typedef struct shttpd_method{
	SHTTPD_METHOD_TYPE type;
	int name_index;
	
}shttpd_method;

/*
 * For parsing. This guy represents a substring.
 */
typedef struct vec 
{
	char	*ptr;
	int			len;
	SHTTPD_METHOD_TYPE type;
}vec;

/*
 * This thing is aimed to hold values of any type.
 * Used to store parsed headers' values.
 */
#define big_int_t long

struct http_header {
	int		len;		/* Header name length		*/
	int		type;		/* Header type			*/
	size_t		offset;		/* Value placeholder		*/
	char	*name;		/* Header name			*/
};

/*
 * This structure tells how HTTP headers must be parsed.
 * Used by parse_headers() function.
 */
#define	OFFSET(x)	offsetof(struct headers, x)

/*
 * This thing is aimed to hold values of any type.
 * Used to store parsed headers' values.
 */
union variant {
	char		*v_str;
	int		v_int;
	big_int_t	v_big_int;
	time_t		v_time;
	void		(*v_func)(void);
	void		*v_void;
	struct vec	v_vec;
};


#define	URI_MAX		16384		/* Default max request size	*/
/*
 * This guy holds parsed HTTP headers
 */
struct headers {
	union variant	cl;		/* Content-Length:		*/
	union variant	ct;		/* Content-Type:		*/
	union variant	connection;	/* Connection:			*/
	union variant	ims;		/* If-Modified-Since:		*/
	union variant	user;		/* Remote user name		*/
	union variant	auth;		/* Authorization		*/
	union variant	useragent;	/* User-Agent:			*/
	union variant	referer;	/* Referer:			*/
	union variant	cookie;		/* Cookie:			*/
	union variant	location;	/* Location:			*/
	union variant	range;		/* Range:			*/
	union variant	status;		/* Status:			*/
	union variant	transenc;	/* Transfer-Encoding:		*/
};

struct cgi{
	int iscgi;
	struct vec bin;
	struct vec para;	
};

struct worker_ctl;
struct worker_opts{
	pthread_t th;			/*�̵߳�ID��*/
	int flags;				/*�߳�״̬*/
	pthread_mutex_t mutex;/*�߳����񻥳�*/

	struct worker_ctl *work;	/*���̵߳��ܿؽṹ*/
};

struct worker_conn ;
/*����ṹ*/
struct conn_request{
	struct vec	req;		/*��������*/
	char *head;			/*����ͷ��\0'��β*/
	char *uri;			/*����URI,'\0'��β*/
	char rpath[URI_MAX];	/*�����ļ�����ʵ��ַ\0'��β*/

	int 	method;			/*��������*/

	/*HTTP�İ汾��Ϣ*/
	unsigned long major;	/*���汾*/
	unsigned long minor;	/*���汾*/

	struct headers ch;	/*ͷ���ṹ*/

	struct worker_conn *conn;	/*���ӽṹָ��*/
	int err;
};

/* ��Ӧ�ṹ */
struct conn_response{
	struct vec	res;		/*��Ӧ����*/
	time_t	birth_time;	/*����ʱ��*/
	time_t	expire_time;/*��ʱʱ��*/

	int		status;		/*��Ӧ״ֵ̬*/
	int		cl;			/*��Ӧ���ݳ���*/

	int 		fd;			/*�����ļ�������*/
	struct stat fsate;		/*�����ļ�״̬*/

	struct worker_conn *conn;	/*���ӽṹָ��*/	
};
struct worker_conn 
{
#define K 1024
	char		dreq[16*K];	/*���󻺳���*/
	char		dres[16*K];	/*��Ӧ������*/

	int		cs;			/*�ͻ����׽����ļ�������*/
	int		to;			/*�ͻ�������Ӧʱ�䳬ʱ�˳�ʱ��*/

	struct conn_response con_res;
	struct conn_request con_req;

	struct worker_ctl *work;	/*���̵߳��ܿؽṹ*/
};

struct worker_ctl{
	struct worker_opts opts;
	struct worker_conn conn;
};
struct mine_type{
	char	*extension;
	int 			type;
	int			ext_len;
	char	*mime_type;
};
void Para_Init(int argc, char *argv[]);

int Request_Parse(struct worker_ctl *wctl);
int Request_Handle(struct worker_ctl* wctl);


int Worker_ScheduleRun();
int Worker_ScheduleStop();
void Method_Do(struct worker_ctl *wctl);
void uri_parse(char *src, int len);

struct mine_type* Mine_Type(char *uri, int len, struct worker_ctl *wctl);



#define DBGPRINT printf


#endif /*__SHTTPD_H__*/

