/*�ļ�ex03-lseek-02.c,
ʹ��lseek���������ն��ļ�*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
  int fd = -1,i;
  ssize_t size = -1;
  off_t offset = -1;  
  /*������ݵĻ�����*/
  char buf1[]="01234567";
  char buf2[]="ABCDEFGH";
  /*�ļ���*/
  char filename[] = "hole.txt";  
  int len = 8;
  
  /*�����ļ�hole.txt*/
	fd = open(filename,O_RDWR|O_CREAT,S_IRWXU);
	if(-1 == fd){
		/*�����ļ�ʧ��*/
	  return -1;
	}	

	/*��buf1�е�����д�뵽�ļ�Hole.txt��*/
	size = write(fd, buf1,len);
	if(size != len){
		/*д������ʧ��*/
		return -1;	
	}
	
	/*�����ļ�ƫ����Ϊ����ֵ��32*/
	offset = lseek(fd, 32, SEEK_SET);
	if(-1 == offset){
		/*����ʧ��*/
		return -1;
	}
	/*��buf2�е�����д�뵽�ļ�hole.txt��*/
	size = write(fd, buf2,len);
	if(size != len){
		/*д������ʧ��*/
		return -1;	
	}
		
	/*�ر��ļ�*/
	close(fd);

	return 0;
}
