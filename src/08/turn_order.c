#include <stdio.h>

/* �������͵ı������ͣ����ڲ����ֽ���
*  	��Աvalue�ĸߵͶ��ֽڿ����ɳ�Աtype���ֽڷ���
 */
/* 16λ */
typedef union{
	unsigned short int value;
	unsigned char byte[2];	
}to16;
/* 32λ */
typedef union{
	unsigned long int value;
	unsigned char byte[4];	
}to32;

#define BITS16 16	/*16λ*/
#define BITS32 32	/*32λ*/
/* �����ֽڴ�ӡ��beginΪ�ֽڿ�ʼ��
*  flagΪBITS16��ʾ16λ��
*  flagΪBITS32��ʾ32λ�� 
*/
void showvalue(unsigned char *begin, int flag)
{
	int num = 0, i = 0;
	if(flag == BITS16){
		num = 2;	
	}else if(flag == BITS32){
		num = 4;	
	}
	
	for(i = 0; i< num; i++)
	{
		printf("%x ",*(begin+i));	
	}
	printf("\n");
}

int main(int argc, char *argv)
{	
	to16 v16_orig, v16_turn1,v16_turn2;	/*һ��to16���ͱ���*/
	to32 v32_orig, v32_turn1,v32_turn2; /*һ��to32���ͱ���*/
	
	
	v16_orig.value = 0xabcd;		/* ��ֵΪ0xabcd */
	v16_turn1.value = htons(v16_orig.value);/*��һ��ת��*/
	v16_turn2.value = ntohs(v16_turn1.value);/*�ڶ���ת��*/
	
	v32_orig.value = 0x12345678;	/* ��ֵΪ0x12345678 */
	v32_turn1.value = htonl(v32_orig.value);/*��һ��ת��*/
	v32_turn2.value = ntohl(v32_turn1.value);/*�ڶ���ת��*/
	
	/* ��ӡ��� */
	printf("16 host to network byte order change:\n");	
	printf("\torig:\t");showvalue(v16_orig.byte, BITS16);	/* 16λ��ֵ��ԭʼֵ */
	printf("\t1 times:");showvalue(v16_turn1.byte, BITS16); /* 16λ��ֵ�ĵ�һ��ת�����ֵ */
	printf("\t2 times:");showvalue(v16_turn2.byte, BITS16); /* 16λ��ֵ�ĵڶ���ת�����ֵ */
	
	printf("32 host to network byte order change:\n");
	printf("\torig:\t");showvalue(v32_orig.byte, BITS32);   /* 32λ��ֵ��ԭʼֵ */
	printf("\t1 times:");showvalue(v32_turn1.byte, BITS32); /* 32λ��ֵ�ĵ�һ��ת�����ֵ */
	printf("\t2 times:");showvalue(v32_turn2.byte, BITS32); /* 32λ��ֵ�ĵڶ���ת�����ֵ */
	
	
	return 0;	
}
