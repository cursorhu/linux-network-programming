#include <stdio.h>

/* �������͵ı������ͣ����ڲ����ֽ���
*  	��Աvalue�ĸߵͶ��ֽڿ����ɳ�Աtype���ֽڷ���
 */
typedef union{
	unsigned short int value;								/*�����ͱ���*/
	unsigned char byte[2];								/*�ַ�����*/
}to;

int main(int argc, char *argv)
{	
	to typeorder ;										/*һ��to���ͱ���*/
	typeorder.value = 0xabcd;								/* ��typeorder������ֵΪ0xabcd */
	
	/* С���ֽ����� */
	if(typeorder.byte[0] == 0xcd && typeorder.byte[1]==0xab){		/*���ֽ���ǰ*/
		printf("Low endian byte order"
				"byte[0]:0x%x,byte[1]:0x%x\n",
				typeorder.byte[0],
				typeorder.byte[1]);	
	}
	
	/* ����ֽ����� */
	if(typeorder.byte[0] == 0xab && typeorder.byte[1]==0xcd){		/*���ֽ���ǰ*/
		printf("High endian byte order"
				"byte[0]:0x%x,byte[1]:0x%x\n",
				typeorder.byte[0],
				typeorder.byte[1]);	
	}
	
	return 0;	
}
