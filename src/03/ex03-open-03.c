/*�ļ�ex03-open-03.c,
O_CREAT��O_EXCL��ʹ��*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
  int fd = -1;
  char filename[] = "test.txt";
  /*���ļ�������ļ������ڣ��򱨴�*/
	fd = open(filename,O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
	if(-1 == fd){
		/*�ļ��Ѿ�����*/
	  printf("File %s exist!,reopen it",filename);
	  /*���´�*/
	  fd = open(filename,O_RDWR);
	  printf("fd:%d\n",fd);
	}	else	{
		/*�ļ������ڣ���������*/
	  printf("Open file %s success,fd:%d\n",filename,fd);
	}

	return 0;
}
