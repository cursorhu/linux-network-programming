/*�ļ�ex03-lseek-01.c,
ʹ��lseek�������Ա�׼�����Ƿ���Խ���seek����*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
  off_t offset = -1;  
  
 	/*����׼�����ļ����������ļ�ƫ������Ϊ��ǰֵ*/
	offset = lseek(stdin, 0, SEEK_CUR);
	if(-1 == offset){
		/*����ʧ�ܣ���׼���벻�ܽ���seek����*/
		printf("STDIN can't seek\n");
		return -1;
	}else{
		/*���óɹ�����׼������Խ���seek����*/
		printf("STDIN CAN seek\n");	
	};
	

	return 0;
}
