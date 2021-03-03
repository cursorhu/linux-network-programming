/*�ļ�ex03-open-03.c,
O_CREAT��O_EXCL��ʹ��*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
  int fd = -1,i;
  ssize_t size = -1;
  /*������ݵĻ�����*/
  char buf[10];
  char filename[] = "test.txt";
  /*���ļ�������ļ������ڣ��򱨴�*/
	fd = open(filename,O_RDONLY);
	if(-1 == fd){
		/*�ļ��Ѿ�����*/
	  printf("Open file %s failure,fd:%d\n",filename,fd);
	}	else	{
		/*�ļ������ڣ���������*/
	  printf("Open file %s success,fd:%d\n",filename,fd);
	}

	/*ѭ����ȡ���ݣ�ֱ���ļ�ĩβ���߳���*/
	while(size){
		/*ÿ�ζ�ȡ10���ֽ�����*/
		size = read(fd, buf,10);
		if( -1 == size)	{
			/*��ȡ���ݳ���*/
			close(fd);/*�ر��ļ�*/
			printf("read file error occurs\n");
			/*����*/
			return -1;		
		}else{
			/*��ȡ���ݳɹ�*/
			if(size >0 ){
				/*���size���ֽ�����*/
				printf("read %d bytes:",size);
				/*��ӡ����*/
				printf("\"");
				/*����ȡ�����ݴ�ӡ����*/
				for(i = 0;i<size;i++){
					printf("%c",*(buf+i));
				}
				/*��ӡ���Ų�����*/
				printf("\"\n");
			}else{
				printf("reach the end of file\n");	
			}
		}
	}

	return 0;
}
