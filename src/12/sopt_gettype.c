#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

int main(int argc,char **argv)
{
    int err = -1;		/*����*/
    int s = -1;            /* Socket */
    int so_type = -1;        /* Socket ���� */
    socklen_t len = -1;        /* ѡ��ֵ���� */

    /*
     * ����һ����ʽ�׽���
     */
    s = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == s){
    	printf("socket error\n");
    	return -1;
	}

    /*
     * ���SO_TYPE��ֵ
     */
    len = sizeof(so_type);
    err = getsockopt(s, SOL_SOCKET, SO_TYPE, &so_type,&len);
    if(err == -1){
    	printf("getsockopt error\n");
    	close(s);
    	return -1;	
    }

    /*
     * ������
     */
    printf("socket fd: %d\n",s);
    printf(" SO_TYPE : %d\n",so_type);
    printf(" SO_STREAM = %d\n",SOCK_STREAM);

    close(s);
    return 0;
}
