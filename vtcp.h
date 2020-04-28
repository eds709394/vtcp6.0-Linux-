
#ifndef VTCP_H
#define VTCP_H
//---------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define VTCP_TIMER						(15)							//����ʱ��
#define VTCP_TIMER_TIMES				(1000 / VTCP_TIMER)				//����ʱ��ÿ�����
#define VTCP_ASYNC_CACHE_COUNT			(32)							//�첽����
#define VTCP_ASYNC_CACHE_COUNT_MASK		(VTCP_ASYNC_CACHE_COUNT - 1)		//�첽��������
#define VTCP_PACKET_CACHE_COUNT			(128)							//������
#define VTCP_PACKET_CACHE_COUNT_MASK	(VTCP_PACKET_CACHE_COUNT - 1)	//����������
#define VTCP_PACKET_GROUP_COUNT			(3)

#define VTCP_CALC_SN_INTERVAL(s1,s2)	(int((unsigned int)(s1)-(unsigned int)(s2)))		//�������кż��
#define VTCP_CALC_RTO(r)				(3 * (r) + VTCP_TIMER)			//�������ʱʱ��
#define VTCP_CALC_S2W(s,r)				(s * (r + 1) / 65536)			//�����ٶȵ�����
#define VTCP_CALC_S2B(s)				(s * (1000 ) / 65536)			//�����ٶȵ�����
#define VTCP_CALC_B2S(b)				(b * (65535) / 1000)			//��������ٶ�

//������
enum VTCP_PKTCMD
{
	VTCP_PKTCMD_CONNECT = (0x0010),//����
	VTCP_PKTCMD_CONNECT_ACK = (0x0011),//���ӽ���
	VTCP_PKTCMD_CONNECT_ACK_DELAY = (0x0012),//�����Ƴ�(������)
	VTCP_PKTCMD_CONNECT_ACK_REFUSE = (0x0013),//���Ӿܾ�(��������)
	VTCP_PKTCMD_DATA = (0x0020),//������
	VTCP_PKTCMD_DATA_ACK = (0x0021),//������Ӧ��
	VTCP_PKTCMD_RESET = (0x0030),//�ر�ͨ��
	VTCP_PKTCMD_RESET_ACK = (0x0031),//�ر�ͨ��Ӧ��
	VTCP_PKTCMD_SYNC = (0x0040),//ͬ�����ư�
	VTCP_PKTCMD_SYNC_ACK = (0x0041),//ͬ�����ư�

};


enum EVTcpPktMsk
{
	VTCP_PKTMSK_SEND = (0x0100),		//���Ѿ�����
	VTCP_PKTMSK_SEND_REPEAT = (0x0200),		//���ظ�����

};

//״̬����
enum vtcp_states
{
	VTCP_STATE_NULL = 0x00,//��ʼ��
	VTCP_STATE_CREATED = 0x01,//����
	VTCP_STATE_BINDED = 0x02,//��
	VTCP_STATE_LISTENED = 0x03,//����
	VTCP_STATE_CONNECTING = 0x04,//���ӽ���
	VTCP_STATE_CONNECTED = 0x05,//�������
	VTCP_STATE_CONNRESET = 0x06,//���ӱ����Ͽ�(��λ)
	VTCP_STATE_CLOSED = 0x07,//�ر�

};

// SVTcpAsyncRecv
struct vtcp_buffer
{
	unsigned char *buffer;
	unsigned int length;
	unsigned int offset;
};

#pragma pack(push)
#pragma pack(1)

//4
struct vtcp_pkthdr
{
	//��������
	uint16_t cmd;
	//SOCKET ID = socket ���
	uint16_t index;
};

#define VTCP_PACKET_DATA_SIZE (1024 - sizeof(struct vtcp_pkthdr) - 2 - 4 - 4)

//1024
struct vtcp_pktdata
{
	uint16_t ack_frequence;				//�ذ�Ƶ��

	uint32_t tickcount;						//ʱ��
	uint32_t sn;						//��ǰ���

	uint8_t data[VTCP_PACKET_DATA_SIZE];	//���ݰ�����
};

//14+1+32
struct vtcp_pktack
{
	uint32_t tickcount;						//ʱ��
	uint32_t sn;						//��ǰ���

	//tcp_sn_recv_min
	uint16_t minimum;
	//tcp_sn_recv_max
	uint16_t maximum;
	//tcp_sn_recv_cur
	uint16_t current;

	//ӳ��ͼ�ֽڴ�С
	uint8_t bitssize;
	//ӳ��ͼ
	uint8_t bits[VTCP_PACKET_CACHE_COUNT / 8];
};

//10+1+32
struct vtcp_pktsyncack
{
	uint32_t sn;

	//tcp_sn_recv_min
	uint16_t minimum;
	//tcp_sn_recv_max
	uint16_t maximum;
	//tcp_sn_recv_cur
	uint16_t current;
};

struct vtcp_pkt
{
	// ���ݰ�ͷ, 4 �ֽ�
	struct vtcp_pkthdr hdr;

	union
	{
		struct vtcp_pktdata data;
		struct vtcp_pktack ack;
		struct vtcp_pktsyncack synack;
	};
};

//����չ(�ڴ�)
struct vtcp_pkt_ext
{
	//int64	speed;						//��ǰ�ٶ�

	int cb;								//����С��0���հ���
	int cbdata;							//�����ݴ�С(���ݰ���Ч��
	int cboffset;						//������ƫ��(���ݰ���Ч,��������ʱʹ��)

	struct vtcp_pkt pkt;
};

#pragma pack(pop)

#define VTCP_SEND											1
#define VTCP_LOAD_SEND										2
#define VTCP_SENT											3
#define VTCP_RECV											4
#define VTCP_CONNECT										5
#define VTCP_ACCEPT											6
#define VTCP_LISTEN											7
#define VTCP_ADDRESSES_COMPARE								8
#define VTCP_ADDRESS_READ									9
#define VTCP_CANCEL											10
#define VTCP_TIMEOUT										11
// ����ռ�
#define VTCP_REQUEST										12
// �ͷſռ�
#define VTCP_RELEASE										13
#define VTCP_LOCK											14
// �����Ƿ�֧
#define VTCP_LOCK_SESSION									0
#define VTCP_LOCK_DOOR										1

typedef int (*t_vtcp_procedure)(void *parameter, unsigned int sid, unsigned int fd, unsigned char number, const unsigned char *address, unsigned int addresssize, void **packet, unsigned char *buffer, unsigned int bufferlength);

struct vtcp_queue
{
	unsigned char queue[sizeof(struct vtcp_buffer) * VTCP_ASYNC_CACHE_COUNT];

	unsigned int index;
	unsigned int count;
};

struct vtcp_door
{
	//

	unsigned char address[20];
};

struct vtcp_packet
{
	struct vtcp_pkt_ext packets[VTCP_PACKET_CACHE_COUNT];

	unsigned int count;
};

struct vtcp_session
{
	//��ʶ(������)
	// Զ��
	unsigned short index1;

	//״̬
	unsigned char state;
	//��־
	unsigned char flags;

	unsigned int last_send;
	unsigned int last_recv;

	// ��ַ
	unsigned char address[20];

	// ���������б�
	struct vtcp_queue queue1;
	// ���հ�
	struct vtcp_packet packet0;
	// ���Ͱ�
	struct vtcp_packet packet1;

	int	errorcode;							//�ڲ��������

	//��ŷ�����
	unsigned int sn;

	//������ŵ׶�(��ȷ�����)
	unsigned int minimum1;
	//������Ÿ߶�(���������)
	unsigned int maximum1;
	//������ŵ�ǰ(���͵�δȷ�����)
	unsigned int current1;
	//������Ÿ���(���)
	unsigned int update;

	//������ŵ׶�(ȷ����ŵ�δ������ͻ�)
	unsigned int minimum0;
	//������Ÿ߶�(���������)
	unsigned int maximum0;
	//������ŵ�ǰ(ȷ�����)
	unsigned int current0;

	//TCP-RTT����
	unsigned int rtt;							//�����������ڣ�RTT��
	unsigned int rtt_prev;						//�����������ڣ�RTT��

	//TCP�����ٶȿ���
	unsigned long long	send_data_speed_surplus;		//�����ٶ�ʣ�ࣨ65536�ࣩ
	unsigned long long	send_data_speed;				//�����ٶȣ�65536����ÿ����
	unsigned long long	send_data_speed_prev;			//�����ٶȣ�65536����ÿ����
	unsigned long long	send_data_speed_change;		//�����ٶȱ仯��
	unsigned long long	send_data_speed_change_prev;	//�����ٶȱ仯��
	int send_data_speed_level;		//�����ٶ�ˮƽ��0~15��

	// �ط�����
	unsigned int repeat;
	unsigned int send_count;
	// ����ӵ������
	unsigned int cwnd_prev_prev;
	// ����ӵ������
	unsigned int cwnd_prev;
	// ����ӵ������
	unsigned int cwnd;

	// TCP�ذ��ٶȿ���
	// �������ݰ�����
	unsigned int recv_count;
	// �������ݰ���ӦƵ��
	unsigned int recv_ack_freq;

	// public://TCP����Ԥ��
	// 	int		m_tcp_recv_data_series_speed;		
	// 	int		m_tcp_recv_data_series_space;
	// 	int		m_tcp_recv_data_series_count;
	// 	int		m_tcp_recv_data_series_count_temp;
	// 	int		m_tcp_recv_data_series_time0;
	// 	int		m_tcp_recv_data_series_time1;
	// 	int		m_tcp_recv_data_series_time2;

	//TCP����
	//����
	unsigned int linger;
	// ����ʱ��
	unsigned int linger_timeout;
	//����(�ڲ�ʹ��)
	unsigned int linger_timeout_tick;
	// �ʱ��
	unsigned int keepalive;
	// �ʱ����
	unsigned int keepalive_internal;
	// ���ӳ�ʱ
	unsigned int connect_timeout;
	// ���ӳ�ʱ(�ڲ�ʹ��)
	unsigned int connect_timeout_tick;

	//ͳ�Ʊ���
	unsigned long long count_do_send_data;
	unsigned long long count_do_send_data_ack;
	unsigned long long count_do_send_data_ack_lost;
	unsigned long long count_do_send_data_repeat;
	unsigned long long count_do_send_sync;
	unsigned long long count_do_send_sync_ack;
	unsigned long long count_do_send_reset;
	unsigned long long count_do_send_reset_ack;

	//ͳ�Ʊ���
	unsigned long long count_on_recv_data;
	unsigned long long count_on_recv_data_ack;
	unsigned long long count_on_recv_data_ack_lost;
	unsigned long long count_on_recv_sync;
	unsigned long long count_on_recv_sync_ack;
	unsigned long long count_on_recv_reset;
	unsigned long long count_on_recv_reset_ack;

	//ͳ���ٶ�
	unsigned long long count_recv;
	unsigned long long count_send;
	unsigned long long count_send_bytes;
};

struct vtcp
{
	// ��� 32 ������, ʵ����ʹ��ͨ���Ѿ�̫����
	struct vtcp_door doors[32];
	unsigned int door_count;

	struct vtcp_session *sessions;
	unsigned int count;

	void *parameter;

	t_vtcp_procedure p_procedure;

	unsigned int fd;
};

void vtcp_initialize(struct vtcp *pvtcp, void *parameter, t_vtcp_procedure p_procedure);
void vtcp_uninitialize(struct vtcp *pvtcp);

unsigned int vtcp_set_ranks(struct vtcp *pvtcp, unsigned int count);

struct vtcp_door *vtcp_search_door(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize);

struct vtcp_door *vtcp_door_open(struct vtcp *pvtcp, void *pointer, const unsigned char *address, unsigned int addresssize);
int vtcp_door_close(struct vtcp *pvtcp, struct vtcp_door *pdoor);

#endif