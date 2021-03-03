/*�ļ�ex03-write-01.c,
O_CREAT��O_EXCL��ʹ��*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
  int fd = -1,i;
  ssize_t size = -1;
  int input = 0;
  /*������ݵĻ�����*/
  char buf[]="quick brown fox jumps over the lazy dog";
  char filename[] = "test.txt";
  /*���ļ�������ļ������ڣ��򱨴�*/
	fd = open(filename,O_RDWR|O_TRUNC);
	if(-1 == fd){
		/*�ļ��Ѿ�����*/
	  printf("Open file %s failure,fd:%d\n",filename,fd);
	}	else	{
		/*�ļ������ڣ���������*/
	  printf("Open file %s success,fd:%d\n",filename,fd);
	}

	/*������д�뵽�ļ�test.txt��*/
	size = write(fd, buf,strlen(buf));
	printf("write %d bytes to file %s\n",size,filename);
	/*�ر��ļ�*/
	close(fd);

	return 0;
}
