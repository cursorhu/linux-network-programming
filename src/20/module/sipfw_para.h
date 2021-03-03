#ifndef __SIPFW_PARA_H__
#define __SIPFW_PARA_H__
const vec sipfw_chain_name[] = {		/*��������*/
	{"INPUT",	5,0},					/*INPUT��*/
	{"OUTPUT",	6,0},					/*OUTPUT��*/
	{"FORWARD",	7,0},					/*FORWARD��*/
	{NULL,		0,0}  };				/*��β*/

const vec sipfw_action_name[] = {		/*��������*/

	{"DROP",	4,0},						/*DROP����*/
	{"ACCEPT",	6,0},					/*ACCEPT����*/
	{"STOLEN",	6,0},					/*STOLEN����*/
	{"QUEUE",	6,0},					/*QUEUE����*/
	{"REPEAT",	6,0},					/*REPEAT����*/
	
	{NULL,		0,0}  };				/*��β*/
const vec sipfw_command_name[] = {	/*��������*/
	{"INSERT",	6,0},					/*����*/
	{"DELETE",	6,0},					/*ɾ��*/
	{"APPEND",	6,0},					/*β������*/
	{"LIST",	4,0},						/*�б����*/
	{"FLUSH",	5,0},					/*��չ���*/
	{NULL,		0,0}  };				/*��β*/

const vec sipfw_protocol_name[] = {	/*Э����������*/
	{"tcp",	3,IPPROTO_TCP},						/*TCPЭ��*/
	{"udp",	3,IPPROTO_UDP},						/*UDPЭ��*/
	{"icmp",	4,IPPROTO_ICMP},						/*ICMPЭ��*/
	{"igmp",	4,IPPROTO_IGMP},						/*IGMP*/
	{NULL,		0,0}  };				/*��β*/
	
#ifdef __KERNEL__
struct sipfw_conf cf={SIPFW_ACTION_ACCEPT, "/etc/sipfw.rules","/etc/sipfw.log",0,0,0};
struct sipfw_list sipfw_tables[SIPFW_CHAIN_NUM] ;
#endif /*__KERNEL__*/
#endif /*__SIPFW_PARA_H__*/
