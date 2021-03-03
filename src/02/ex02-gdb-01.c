/*�ļ�����ex02-gdb-01.c*/
#include <stdio.h>/* ����printf*/
#include <stdlib.h>/*����malloc*/

/* ��������sumΪstatic int���� */
static int sum(int value);

/* ���ڿ�����������Ľṹ */
struct inout{
		int value;
		int result;
};
int main(int argc, char *argv[]){
	/*�����ڴ�*/
	struct inout *io = (struct inout*)malloc(sizeof(struct inout));
	/*�ж��Ƿ�ɹ�*/
	if(NULL == io)
	{
		/*ʧ�ܷ���*/
		printf("�����ڴ�ʧ��\n");
		return -1;		
	}
	
	/*�ж���������Ƿ���ȷ*/
	if(argc !=2)
	{
		printf("�����������!\n");
		return -1;		
	}
	
  /* �������Ĳ��� */
  io->value = *argv[1]-'0';
  /* ��value�����ۼ���� */
  io->result = sum(io->value);
  printf("�������ֵΪ��%d,������Ϊ��%d\n",io->value,io->result);

  return 0;
}

/* �ۼ���ͺ��� */
static int sum(int value){
  int result = 0;
  int i = 0;
  /* ѭ�������ۼ�ֵ */
  for(i=0;i<value;i++)
    result += (i+1);
  /*���ؽ��*/
  return result;
}
