 #include <netdb.h>
 #include <string.h>
 #include <stdio.h>
 #include <sys/socket.h> 
 #if 0
 struct hostent {
                      char    *h_name;        /* official name of host */
                      char    **h_aliases;    /* alias list */
                      int     h_addrtype;     /* host address type */
                      int     h_length;       /* length of address */
                      char    **h_addr_list;  /* list of addresses */
              }
              #define h_addr  h_addr_list[0]  /* for backward compatibility */
struct hostent *
       gethostbyaddr(const void *addr, int len, int type);
#endif
 int main(int argc, char *argv[])
 {
 	struct hostent *ht=NULL;
 
 	/* ��ѯ���������� */
 	char host[]="www.sina.com.cn";
 	
 #if 1
 	struct hostent *ht1=NULL, *ht2=NULL;
 	char host1[]="www.sohu.com";
 	/* ��ѯ����www.sina.com.cn */
 	ht1 = gethostbyname(host);
 	ht2 = gethostbyname(host1);
 	int j = 0;
 	
 #else
 	struct in_addr in;
 	in.s_addr = inet_addr("60.215.128.140");
 	ht = gethostbyaddr(&in, sizeof(in), AF_INET);
 #endif
 for(j = 0;j<2;j++){
 	if(j == 0)
 		ht = ht1;
 	else
 		ht =ht2;
 		
 	printf("----------------------\n");
 	
 	if(ht){
 		int i = 0;
 		printf("get the host:%s addr\n",host);	/* ԭʼ���� */
 		printf("name:%s\n",ht->h_name);			/* ���� */
 		
 		/*Э����AF_INETΪIPv4����AF_INET6ΪIPv6*/
 		printf("type:%s\n",ht->h_addrtype==AF_INET?"AF_INET":"AF_INET6");
 		
 		/* IP��ַ�ĳ��� */
 		printf("legnth:%d\n",ht->h_length);	
 		/* ��ӡIP��ַ */
 		for(i=0;;i++){
 			if(ht->h_addr_list[i] != NULL){/* ����IP��ַ����Ľ�β */
 				printf("IP:%s\n",inet_ntoa((unsigned int*)ht->h_addr_list[i]));	/*��ӡIP��ַ*/
 			}	else{/*�ﵽ��β*/
 				break;	/*�˳�forѭ��*/
 			}
 		}
 		
 		/* ��ӡ������ַ */
 		for(i=0;;i++){/*ѭ��*/
 			if(ht->h_aliases[i] != NULL){/* û�е�����������Ľ�β */
 				printf("alias %d:%s\n",i,ht->h_aliases[i]);	/* ��ӡ���� */
 			}	else{/*��β*/
 				break;	/*�˳�ѭ��*/
 			}
 		}
 	}	
}
 	return 0;
 }
 