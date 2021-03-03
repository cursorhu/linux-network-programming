/*
 * ����Э��ṹ,IPv4���μ�RFC 791
 */
struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN 	/* ���ΪС�� */
	unsigned int ip_hl:4;								/* ͷ������ */
	unsigned int ip_v:4;								/* �汾 */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN 			/*���Ϊ���*/
	unsigned int ip_v:4;								/* �汾 */
	unsigned int ip_hl:4;								/* ͷ������ */
#endif
	u_int8_t ip_tos;										/* TOS���������� */
	u_short ip_len;											/* �ܳ��� */
	u_short ip_id;											/* ��ʶֵ */
	u_short ip_off;											/* ��ƫ��ֵ */
	...
	...
	u_int8_t ip_ttl;										/* TTL������ʱ�� */
	u_int8_t ip_p;											/* Э������ */
	u_short ip_sum;											/* У��� */
	struct in_addr ip_src, ip_dst;			/* Դ��ַ��Ŀ�ĵ�ַ */
};



struct icmp
{
	u_int8_t  icmp_type;	/* ��Ϣ���� */
  u_int8_t  icmp_code;	/* ��Ϣ���͵����� */
  u_int16_t icmp_cksum;	/* У��� */
  union
  {
    u_char ih_pptr;		/* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr;	/* ���ص�ַ */
    struct ih_idseq				/* ��ʾ���ݱ� */
    {
      u_int16_t icd_id;		/* ���ݱ�ID */
      u_int16_t icd_seq;	/* ���ݱ������ */
    } ih_idseq;
    u_int32_t ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu
    {
      u_int16_t ipm_void;
      u_int16_t ipm_nextmtu;
    } ih_pmtu;

    struct ih_rtradv
    {
      u_int8_t irt_num_addrs;
      u_int8_t irt_wpa;
      u_int16_t irt_lifetime;
    } ih_rtradv;
  } icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
  union
  {
    struct
    {
      u_int32_t its_otime;
      u_int32_t its_rtime;
      u_int32_t its_ttime;
    } id_ts;
    struct
    {
      struct ip idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    struct icmp_ra_addr id_radv;
    u_int32_t   id_mask;
    u_int8_t    id_data[1];	/* ���� */
  } icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};

struct icmp
{
	u_int8_t  icmp_type;	/* ��Ϣ���� */
  u_int8_t  icmp_code;	/* ��Ϣ���͵����� */
  u_int16_t icmp_cksum;	/* У��� */
  union
  {    
    struct ih_idseq				/* ��ʾ���ݱ� */
    {
      u_int16_t icd_id;		/* ���ݱ�ID */
      u_int16_t icd_seq;	/* ���ݱ������ */
    } ih_idseq;
  } icmp_hun;

#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
  union
  {
    u_int8_t    id_data[1];	/* ���� */
  } icmp_dun;
#define	icmp_data	icmp_dun.id_data
};


#ifdef __FAVOR_BSD			/* ���ϲ��BSD��ʽ */
struct udphdr
{
  u_int16_t uh_sport;		/* Դ��ַ�˿� */
  u_int16_t uh_dport;		/* Ŀ�ĵ�ַ�˿� */
  u_int16_t uh_ulen;		/* UDP���� */
  u_int16_t uh_sum;			/* UDPУ��� */
};
#else										/* Linux��ʽ */
struct udphdr
{
  u_int16_t source;			/* Դ��ַ�˿� */
  u_int16_t dest;				/* Ŀ�ĵ�ַ�˿� */
  u_int16_t len;				/* UDP���� */
  u_int16_t check;			/* UDPУ��� */
};
#endif

struct tcphdr 
{
	__u16	source;		/* Դ��ַ�˿� */
	__u16	dest;			/* Ŀ�ĵ�ַ�˿� */
	__u32	seq;			/* ���к� */
	__u32	ack_seq;	/* ȷ�����к� */
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16	res1:4,		/* ���� */
		doff:4,				/*  */
		fin:1,				/* �ر����ӱ�־ */
		syn:1,				/* �������ӱ�־ */
		rst:1,				/* �������ӱ�־ */
		psh:1,				/* ���շ����콫���ݷŵ�Ӧ�ò��־ */
		ack:1,				/* ȷ����ű�־ */
		urg:1,				/* ����ָ���־ */
		ece:1,				/* ӵ����־λ */
		cwr:1;				/* ӵ����־λ */
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16	doff:4,
		res1:4,				/* ���� */
		cwr:1,				/* ӵ����־λ */
		ece:1,				/* ӵ����־λ */
		urg:1,				/* ����ָ���־ */
		ack:1,				/* ȷ����ű�־ */
		psh:1,				/* ���շ����콫���ݷŵ�Ӧ�ò��־ */
		rst:1,				/* �������ӱ�־ */
		syn:1,				/* �������ӱ�־ */
		fin:1;				/* �ر����ӱ�־ */
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif	
	__u16	window;			/* �������ڴ�С */
	__u16	check;			/* У��� */
	__u16	urg_ptr;		/* �����ֶ�ָ�� */
};
