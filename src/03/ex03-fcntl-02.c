/*�ļ�ex03-fcntl-02.c,
ʹ��fcntl�޸��ļ���״ֵ̬*/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>/*strlen����*/

int main(void)
{
	int flags = -1;
	char buf[] = "FCNTL";
	
	int fd = open("test.txt", O_RDWR);
	/*����ļ�״̬*/	
	flags = fcntl(fd, F_GETFL, 0);
	/*����״̬Ϊ��׷��*/
	flags |= O_APPEND;
	/*��״̬д��*/
	flags = fcntl(fd, F_SETFL, &flags);
  if( flags < 0 ){
  	/*������*/
  	printf("failure to use fcntl\n");
  	return -1;
  }
  /*���ļ���д���ַ���*/
  write(fd, buf, strlen(buf));
  close(fd);
 	
	return 0;
}

