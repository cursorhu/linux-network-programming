/*�ļ�ex03-ioctl-01.c
*����CDROM*/
#include <linux/cdrom.h>
#include <stdio.h>
#include <fcntl.h>

int main(void){
	/*��CDROM�豸�ļ�*/
	int fd = open("/dev/cdrom",O_RDONLY);
	if(fd < 0){
		printf("��CDROMʧ��\n");
		return -1;
	}
	if (!ioctl(fd, CDROMEJECT,NULL)){
		printf("�ɹ�����CDROM\n");
	}else{
		printf("����CDROMʧ��\n");
	}
	return 0;
}
