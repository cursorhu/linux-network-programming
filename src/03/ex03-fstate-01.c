/*�ļ�ex03-fstat-01.c,
ʹ��stat����ļ���״̬*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
	struct stat st;
  if( -1 == stat("test.txt", &st)){
  	printf("����ļ�״̬ʧ��\n");
  	return -1;
  }
  
  printf("�������ļ����豸ID��%d\n",st.st_dev);
  printf("���ļ��Ľڵ㣺%d\n",st.st_ino);
  printf("���ļ��ı���ģʽ��%d\n",st.st_mode);
  printf("���ļ���Ӳ��������%d\n",st.st_nlink);
  printf("���ļ���������ID��%d\n",st.st_uid);
  printf("���ļ��������ߵ���ID��%d\n",st.st_gid);
  printf("�豸ID��������ļ�Ϊ�����豸����%d\n",st.st_rdev);
  printf("���ļ��Ĵ�С��%d\n",st.st_size);
  printf("���ļ��������ļ�ϵͳ���С��%d\n",st.st_blksize);
  printf("���ļ���ռ�ÿ�������%d\n",st.st_blocks);
  printf("���ļ���������ʱ�䣺%d\n",st.st_atime);
  printf("���ļ�������޸�ʱ�䣺%d\n",st.st_mtime);
  printf("���ļ������״̬�ı�ʱ�䣺%d\n",st.st_ctime);
  
 	
	return 0;
}

�������ļ����豸ID��17
���ļ��Ľڵ㣺207
���ļ��ı���ģʽ��33279
���ļ���Ӳ��������
���ļ���������ID��0
���ļ��������ߵ���ID��0
�豸ID��������ļ�Ϊ�����豸����0
���ļ��Ĵ�С��50
���ļ��������ļ�ϵͳ���С��1024
���ļ���ռ�ÿ�������1
���ļ���������ʱ�䣺1222786995
���ļ�������޸�ʱ�䣺1222768248
���ļ������״̬�ı�ʱ�䣺1222768248


