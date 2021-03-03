#include "shttpd.h"
static char *l_opt_arg;
static char *shortopts = "c:d:f:ho:l:m:t:";
/*
struct option{
     char *name;
     int has_arg;
     int *flag;
     int val;
};
*/
static struct option longopts[] = {
	{"CGIRoot", 		required_argument, NULL, 'c'},
	{"ConfigFile", 		required_argument, NULL, 'f'},	
	{"DefaultFile", 	required_argument, NULL, 'd'},	
	{"DocumentRoot",	required_argument, NULL, 'o'},
	{"ListenPort", 		required_argument, NULL, 'l'},
	{"MaxClient", 		required_argument, NULL, 'm'},	
	{"TimeOut",		required_argument, NULL, 't'},
	{"Help",			no_argument, NULL, 'h'},
	{0, 0, 0, 0},
};


extern struct conf_opts conf_para;
static void display_usage(void)
{
	printf("sHTTPD -l number  -m number      -o path    -c  path   -d  filename   -t seconds  -o filename\n");
	printf("sHTTPD --ListenPort number\n");
	printf("       --MaxClient number\n");
	printf("       --DocumentRoot) path\n");
	printf("       --DefaultFile) filename\n");
	printf("       --CGIRoot path \n");
	printf("       --DefaultFile filename\n");
	printf("       --TimeOut seconds\n");
	printf("       --ConfigFile filename\n");
}

static void display_para()
{
	printf("sHTTPD ListenPort: %d\n",conf_para.ListenPort);
	printf("       MaxClient: %d\n", conf_para.MaxClient);
	printf("       DocumentRoot: %s\n",conf_para.DocumentRoot);
	printf("       DefaultFile:%s\n",conf_para.DefaultFile);
	printf("       CGIRoot:%s \n",conf_para.CGIRoot);
	printf("       DefaultFile:%s\n",conf_para.DefaultFile);
	printf("       TimeOut:%d\n",conf_para.TimeOut);
	printf("       ConfigFile:%s\n",conf_para.ConfigFile);
}

static int conf_readline(int fd, char *buff, int len)
{
	int n = -1;
	int i = 0;
	int begin = 0;

	/*�建����*/
	memset(buff, 0, len);
	for(i =0; i<len;begin?i++:i)/*����ͷ���ֲ�Ϊ'\r'����'\n'ʱi����*/
	{
		n = read(fd, buff+i, 1);/*��һ���ַ�*/
		if(n == 0)/*�ļ�ĩβ*/{
			*(buff+i) = '\0';
			break;
		}else if(*(buff+i) == '\r' ||*(buff+i) == '\n'){/*�س�����*/
			if(begin){/*Ϊһ��*/
				*(buff+i) = '\0';	
				break;
			}
		}else{
			begin = 1;
		}
	}

	return i;
}


static int Para_CmdParse(int argc, char *argv[])
{
	int c;
	int len;
	int value;

	/*�����������,�������ò���*/
	while ((c = getopt_long (argc, argv, shortopts, longopts, NULL)) != -1)
	{
		switch (c)
		{
			case 'c': 	/*CGI��·��*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					memcpy(conf_para.CGIRoot, l_opt_arg, len +1);
				}
				
				break;
			case 'd': 	/*Ĭ���ļ�����*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					memcpy(conf_para.DefaultFile, l_opt_arg, len +1);
				}
				
				break;
			case 'f': 	/*�����ļ����ƺ�·��*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					memcpy(conf_para.ConfigFile, l_opt_arg, len +1);
				}
				
				break;
			case 'o': /*���ļ�·��*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					memcpy(conf_para.DocumentRoot, l_opt_arg, len +1);
				}
				
				break;
			case 'l': 	/*�����˿�*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if(value != LONG_MAX && value != LONG_MIN)
						conf_para.ListenPort = value;
				}
				
				break;
			case 'm': /*���ͻ�������*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if(value != LONG_MAX && value != LONG_MIN)
						conf_para.MaxClient= value;
				}				
				
				break;
			case 't': 	/*��ʱʱ��*/
				l_opt_arg = optarg;
				if(l_opt_arg && l_opt_arg[0]!=':'){
					printf("TIMEOUT\n");
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if(value != LONG_MAX && value != LONG_MIN)
						conf_para.TimeOut = value;
				}
				
				break;
			case '?':/*�������*/
				printf("Invalid para\n");
			case 'h':	/*����*/
				display_usage();
				break;
		}
	}
	return 0;
}
/*
*  ���������ļ�����
*/
void Para_FileParse(char *file)
{
#define LINELENGTH 256
	char line[LINELENGTH];
	char *name = NULL, *value = NULL;
	int fd = -1;
	int n = 0;

	fd = open(file, O_RDONLY);
	if(fd == -1)
	{
		goto EXITPara_FileParse;
	}
	
	/*
	*�����ʽ����:
	*[#ע��|[�ո�]�ؼ���[�ո�]=[�ո�]value]
	*/
	while( (n = conf_readline(fd, line, LINELENGTH)) !=0)
	{
		char *pos = line;	
		/* ����һ�п�ͷ���ֵĿո� */
		while(isspace(*pos)){
			pos++;
		}
		/*ע��?*/
		if(*pos == '#'){
			continue;
		}
		
		/*�ؼ��ֿ�ʼ����*/
		name = pos;
		/*�ؼ��ֵ�ĩβ*/
		while(!isspace(*pos) && *pos != '=')
		{
			pos++;
		}
		*pos = '\0';/*���ɹؼ����ַ���*/

		/*value����ǰ��ո�*/
		while(isspace(*pos))
		{
			pos++;
		}
		/*value��ʼ*/
		value = pos;
		/*������*/
		while(!isspace(*pos) && *pos != '\r' && *pos != '\n')
		{
			pos++;
		}
		*pos = '\0';/*����ֵ���ַ���*/


		/*���ݹؼ��ֲ��֣����value���ֵ�ֵ*/
		int ivalue;
		/*"CGIRoot","DefaultFile","DocumentRoot","ListenPort","MaxClient","TimeOut"*/
		if(strncmp("CGIRoot", name, 7))	{/*CGIRoot����*/
			memcpy(conf_para.CGIRoot, value, strlen(value)+1);
		}else if(strncmp("DefaultFile", name, 11)){/*DefaultFile����*/
			memcpy(conf_para.DefaultFile, value, strlen(value)+1);
		}else if(strncmp("DocumentRoot", name, 12)){/*DocumentRoot����*/
			memcpy(conf_para.DocumentRoot, value, strlen(value)+1);
		}else if(strncmp("ListenPort", name, 10)){/*ListenPort����*/
			ivalue = strtol(value, NULL, 10);
			conf_para.ListenPort = ivalue;
		}else if(strncmp("MaxClient", name, 9)){/*MaxClient����*/
			ivalue = strtol(value, NULL, 10);
			conf_para.MaxClient = ivalue;
		}else if(strncmp("TimeOut", name, 7)){/*TimeOut����*/
			ivalue = strtol(value, NULL, 10);
			conf_para.TimeOut = ivalue;
		}		
	}
	close(fd);

EXITPara_FileParse:
	return;	
}


void Para_Init(int argc, char *argv[])
{
	/*�����������������*/
	Para_CmdParse(argc, argv);

	/*���������ļ����ò���*/
	if(strlen(conf_para.ConfigFile))
		Para_FileParse(conf_para.ConfigFile);

	display_para();

	/*�������ò���*/
	return ;
}

