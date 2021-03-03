/*�ļ�ex03-fcntl-02.c,
ʹ��fcntl�޸��ļ���״ֵ̬*/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>/*strlen����*/

#define NEWFD 8
int main(void)
{
	char buf[] = "FCNTL";
	
	int fd = open("test.txt", O_RDWR);
	printf("the file test.txt ID is %d\n",fd);
	/*����ļ�״̬*/	
	fd = fcntl(fd, F_GETFD);
	printf("the file test.txt ID is %d\n",fd);
	fd = NEWFD;
	/*��״̬д��*/
	fcntl(NEWFD, F_SETFL, &fd);
  
  /*���ļ���д���ַ���*/
  write(NEWFD, buf, strlen(buf));
  close(NEWFD);
 	
	return 0;
}

