/*�ļ�ex03-fcntl-04.c,
ʹ��fcntl��ý����źŵĽ���ID*/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


#define NEWFD 8
int main(void)
{
	int uid;	
	/*���ļ�test.txt*/
	int fd = open("test.txt", O_RDWR);
	
	/*��ý����źŵĽ���ID*/	
	uid = fcntl(fd, F_GETOWN);
	printf("the SIG recv ID is %d\n",uid);
	
  close(fd);
 	
	return 0;
}

