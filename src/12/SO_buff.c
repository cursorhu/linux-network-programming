	intfd, cork = 1; 
	�� 
	/* ��ʼ��*/ 
	�� 
	setsockopt (fd, SOL_TCP, TCP_CORK, &cork, sizeof (cork)); /* ��ˮͰ��ˮ */ 
	write (fd, ��); 
	senddata (fd, ��); 
	.../* �������� */
	write (fd, ��); 
	senddata (fd, ��); 
	�� 
	cork = 0; 
	setsockopt (fd, SOL_TCP, TCP_CORK, &on, sizeof (on)); /* ����ˮͰ */ 
