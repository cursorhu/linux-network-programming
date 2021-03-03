/*�ļ�ex03-mmap-01.c,
ʹ��mmap���ļ����в���*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>/*mmap*/
#include <string.h>/*memset warning*/
#include <stdio.h>

#define FILELENGTH 80
int main(void)
{
  int fd = -1;
  /*��Ҫд���ļ����ַ���*/
  char buf[]="quick brown fox jumps over the lazy dog";
  char *ptr = NULL;
  
  /*���ļ�mmap.txt�������ļ�������СΪ0��
  ����ļ��������򴴽�����Ȩ��Ϊ�ɶ�дִ��*/
  fd = open("mmap.txt", O_RDWR/*�ɶ�д*/|O_CREAT/*�����ڣ�����*/|O_TRUNC/*��СΪ0*/, S_IRWXU);  
  if( -1 == fd){
  	/*���ļ�ʧ�ܣ��˳�*/
  	return -1;
  }  
  /*����Ĵ��뽫�ļ��ĳ�������Ϊ80*/
  /*���ƫ���ļ���ƫ������79*/
  lseek(fd, FILELENGTH-1, SEEK_SET);
  /*����д��һ���ַ�����ʱ�ļ��ĳ���Ϊ80*/
  write(fd, 'a', 1);
  
  /*���ļ�mmap.txt�е����ݶδӿ�ͷ��1M������ӳ�䵽�ڴ��У����ļ��Ĳ���������ʾ���ļ��ϣ��ɶ�д*/
  ptr = (char*)mmap(NULL, FILELENGTH, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
  if( (char*)-1 == ptr){
  	/*���ӳ��ʧ�ܣ����˳�*/
  	printf("mmap failure\n");  
  	close(fd);
  	return -1;
  }
  
  /*��buf�е��ַ���������ӳ�������У���ʼ��ַΪƫ��16*/
  memcpy(ptr+16, buf, strlen(buf));
  /*ȡ���ļ�ӳ���ϵ*/
  munmap(ptr, FILELENGTH);
  /*�ر��ļ�*/
  close(fd);
 	
	return 0;
}
