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
    int err = -1;		/* ����ֵ */
    int s = -1;			/* socket������ */
    int snd_size = 0;   /* ���ͻ�������С */
    int rcv_size = 0;	/* ���ջ�������С */
    socklen_t optlen;	/* ѡ��ֵ���� */

    /*
     * ����һ��TCP�׽���
     */
    s = socket(PF_INET,SOCK_STREAM,0);
    if( s == -1){
    	printf("�����׽��ִ���\n");
    	return -1;	
    }
    
    /*
     * �ȶ�ȡ���������õ����
     * ���ԭʼ���ͻ�������С
     */
    optlen = sizeof(snd_size);
    err = getsockopt(s, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err){
    	printf("��ȡ���ͻ�������С����\n");
    }  
    /*
     * ��ӡԭʼ�������������
     */
    printf(" ���ͻ�����ԭʼ��СΪ: %d �ֽ�\n",snd_size);
    printf(" ���ջ�����ԭʼ��СΪ: %d �ֽ�\n",rcv_size);

    /*
     * ���ԭʼ���ջ�������С
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if(err){
    	printf("��ȡ���ջ�������С����\n");
    }
    
    /*
     * ���÷��ͻ�������С
     */
    snd_size = 4096;    /* ���ͻ�������СΪ8K */
    optlen = sizeof(snd_size);
    err = setsockopt(s, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if(err){
    	printf("���÷��ͻ�������С����\n");
    }

    /*
     * ���ý��ջ�������С
     */
    rcv_size = 8192;    /* ���ջ�������СΪ8K */
    optlen = sizeof(rcv_size);
    err = setsockopt(s,SOL_SOCKET,SO_RCVBUF, &rcv_size, optlen);
    if(err){
    	printf("���ý��ջ�������С����\n");
    }

    /*
     * ����������������õ����
     * ����޸ĺ��ͻ�������С
     */
    optlen = sizeof(snd_size);
    err = getsockopt(s, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err){
    	printf("��ȡ���ͻ�������С����\n");
    }  

    /*
     * ����޸ĺ���ջ�������С
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if(err){
    	printf("��ȡ���ջ�������С����\n");
    }

    /*
     * ��ӡ���
     */
    printf(" ���ͻ�������СΪ: %d �ֽ�\n",snd_size);
    printf(" ���ջ�������СΪ: %d �ֽ�\n",rcv_size);

    close(s);
    return 0;
}
