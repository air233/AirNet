//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//  
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#include "ikcp.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>



//=====================================================================
// KCP BASIC
//=====================================================================
const IUINT32 IKCP_RTO_NDL = 30;		// no delay min rto
const IUINT32 IKCP_RTO_MIN = 100;		// normal min rto
const IUINT32 IKCP_RTO_DEF = 200;
const IUINT32 IKCP_RTO_MAX = 60000;
const IUINT32 IKCP_CMD_PUSH = 81;		// cmd: push data
const IUINT32 IKCP_CMD_ACK  = 82;		// cmd: ack
const IUINT32 IKCP_CMD_WASK = 83;		// cmd: window probe (ask)
const IUINT32 IKCP_CMD_WINS = 84;		// cmd: window size (tell)
const IUINT32 IKCP_ASK_SEND = 1;		// need to send IKCP_CMD_WASK
const IUINT32 IKCP_ASK_TELL = 2;		// need to send IKCP_CMD_WINS
const IUINT32 IKCP_WND_SND = 32;
const IUINT32 IKCP_WND_RCV = 128;       // must >= max fragment size
const IUINT32 IKCP_MTU_DEF = 1400;
const IUINT32 IKCP_ACK_FAST	= 3;
const IUINT32 IKCP_INTERVAL	= 100;
const IUINT32 IKCP_OVERHEAD = 24;
const IUINT32 IKCP_DEADLINK = 20;
const IUINT32 IKCP_THRESH_INIT = 2;
const IUINT32 IKCP_THRESH_MIN = 2;
const IUINT32 IKCP_PROBE_INIT = 7000;		// 7 secs to probe window size
const IUINT32 IKCP_PROBE_LIMIT = 120000;	// up to 120 secs to probe window
const IUINT32 IKCP_FASTACK_LIMIT = 5;		// max times to trigger fastack


//---------------------------------------------------------------------
// encode / decode
//---------------------------------------------------------------------

/* encode 8 bits unsigned int */
static inline char *ikcp_encode8u(char *p, unsigned char c)
{
	*(unsigned char*)p++ = c;
	return p;
}

/* decode 8 bits unsigned int */
static inline const char *ikcp_decode8u(const char *p, unsigned char *c)
{
	*c = *(unsigned char*)p++;
	return p;
}

/* encode 16 bits unsigned int (lsb) */
static inline char *ikcp_encode16u(char *p, unsigned short w)
{
#if IWORDS_BIG_ENDIAN || IWORDS_MUST_ALIGN
	*(unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	memcpy(p, &w, 2);
#endif
	p += 2;
	return p;
}

/* decode 16 bits unsigned int (lsb) */
static inline const char *ikcp_decode16u(const char *p, unsigned short *w)
{
#if IWORDS_BIG_ENDIAN || IWORDS_MUST_ALIGN
	*w = *(const unsigned char*)(p + 1);
	*w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
	memcpy(w, p, 2);
#endif
	p += 2;
	return p;
}

/* encode 32 bits unsigned int (lsb) */
static inline char *ikcp_encode32u(char *p, IUINT32 l)
{
#if IWORDS_BIG_ENDIAN || IWORDS_MUST_ALIGN
	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	memcpy(p, &l, 4);
#endif
	p += 4;
	return p;
}

/* decode 32 bits unsigned int (lsb) */
static inline const char *ikcp_decode32u(const char *p, IUINT32 *l)
{
#if IWORDS_BIG_ENDIAN || IWORDS_MUST_ALIGN
	*l = *(const unsigned char*)(p + 3);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	memcpy(l, p, 4);
#endif
	p += 4;
	return p;
}

static inline IUINT32 _imin_(IUINT32 a, IUINT32 b) {
	return a <= b ? a : b;
}

static inline IUINT32 _imax_(IUINT32 a, IUINT32 b) {
	return a >= b ? a : b;
}

static inline IUINT32 _ibound_(IUINT32 lower, IUINT32 middle, IUINT32 upper) 
{
	return _imin_(_imax_(lower, middle), upper);
}

static inline long _itimediff(IUINT32 later, IUINT32 earlier) 
{
	return ((IINT32)(later - earlier));
}

//---------------------------------------------------------------------
// manage segment
//---------------------------------------------------------------------
typedef struct IKCPSEG IKCPSEG;

static void* (*ikcp_malloc_hook)(size_t) = NULL;
static void (*ikcp_free_hook)(void *) = NULL;

// internal malloc
static void* ikcp_malloc(size_t size) {
	if (ikcp_malloc_hook) 
		return ikcp_malloc_hook(size);
	return malloc(size);
}

// internal free
static void ikcp_free(void *ptr) {
	if (ikcp_free_hook) {
		ikcp_free_hook(ptr);
	}	else {
		free(ptr);
	}
}

// redefine allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void (*new_free)(void*))
{
	ikcp_malloc_hook = new_malloc;
	ikcp_free_hook = new_free;
}

// allocate a new kcp segment
static IKCPSEG* ikcp_segment_new(ikcpcb *kcp, int size)
{
	return (IKCPSEG*)ikcp_malloc(sizeof(IKCPSEG) + size);
}

// delete a segment
static void ikcp_segment_delete(ikcpcb *kcp, IKCPSEG *seg)
{
	ikcp_free(seg);
}

// write log
void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...)
{
	char buffer[1024];
	va_list argptr;
	if ((mask & kcp->logmask) == 0 || kcp->writelog == 0) return;
	va_start(argptr, fmt);
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	kcp->writelog(buffer, kcp, kcp->user);
}

// check log mask
static int ikcp_canlog(const ikcpcb *kcp, int mask)
{
	if ((mask & kcp->logmask) == 0 || kcp->writelog == NULL) return 0;
	return 1;
}

// output segment
static int ikcp_output(ikcpcb *kcp, const void *data, int size)
{
	assert(kcp);
	assert(kcp->output);
	if (ikcp_canlog(kcp, IKCP_LOG_OUTPUT)) 
	{
		ikcp_log(kcp, IKCP_LOG_OUTPUT, "[RO] %ld bytes", (long)size);
	}
	if (size == 0) return 0;

	return kcp->output((const char*)data, size, kcp, kcp->user);
}

// output queue
void ikcp_qprint(const char *name, const struct IQUEUEHEAD *head)
{
#if 1
	const struct IQUEUEHEAD *p;
	printf("<%s>: [", name);
	for (p = head->next; p != head; p = p->next) {
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		printf("(%lu %d)", (unsigned long)seg->sn, (int)(seg->ts % 10000));
		if (p->next != head) printf(",");
	}
	printf("]\n");
#endif
}


//---------------------------------------------------------------------
// create a new kcpcb
//---------------------------------------------------------------------
ikcpcb* ikcp_create(IUINT32 conv, void *user)
{
	ikcpcb *kcp = (ikcpcb*)ikcp_malloc(sizeof(struct IKCPCB));
	if (kcp == NULL) return NULL;
	kcp->conv = conv;
	kcp->user = user;
	kcp->snd_una = 0;
	kcp->snd_nxt = 0;
	kcp->rcv_nxt = 0;
	kcp->ts_recent = 0;
	kcp->ts_lastack = 0;
	kcp->ts_probe = 0;
	kcp->probe_wait = 0;
	kcp->snd_wnd = IKCP_WND_SND;
	kcp->rcv_wnd = IKCP_WND_RCV;
	kcp->rmt_wnd = IKCP_WND_RCV;
	kcp->cwnd = 0;
	kcp->incr = 0;
	kcp->probe = 0;
	kcp->mtu = IKCP_MTU_DEF;//MTU长度
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;//最大报文长度
	kcp->stream = 0;

	kcp->buffer = (char*)ikcp_malloc((kcp->mtu + IKCP_OVERHEAD) * 3);
	if (kcp->buffer == NULL) {
		ikcp_free(kcp);
		return NULL;
	}

	iqueue_init(&kcp->snd_queue);
	iqueue_init(&kcp->rcv_queue);
	iqueue_init(&kcp->snd_buf);
	iqueue_init(&kcp->rcv_buf);
	kcp->nrcv_buf = 0;
	kcp->nsnd_buf = 0;
	kcp->nrcv_que = 0;
	kcp->nsnd_que = 0;
	kcp->state = 0;
	kcp->acklist = NULL;
	kcp->ackblock = 0;
	kcp->ackcount = 0;
	kcp->rx_srtt = 0;
	kcp->rx_rttval = 0;
	kcp->rx_rto = IKCP_RTO_DEF;
	kcp->rx_minrto = IKCP_RTO_MIN;
	kcp->current = 0;
	kcp->interval = IKCP_INTERVAL;
	kcp->ts_flush = IKCP_INTERVAL;
	kcp->nodelay = 0;
	kcp->updated = 0;
	kcp->logmask = 0;
	kcp->ssthresh = IKCP_THRESH_INIT;
	kcp->fastresend = 0;
	kcp->fastlimit = IKCP_FASTACK_LIMIT;
	kcp->nocwnd = 0;
	kcp->xmit = 0;
	kcp->dead_link = IKCP_DEADLINK;
	kcp->output = NULL;
	kcp->writelog = NULL;

	return kcp;
}


//---------------------------------------------------------------------
// release a new kcpcb
//---------------------------------------------------------------------
void ikcp_release(ikcpcb *kcp)
{
	assert(kcp);
	if (kcp) 
	{
		IKCPSEG *seg;
		while (!iqueue_is_empty(&kcp->snd_buf)) {
			seg = iqueue_entry(kcp->snd_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_buf)) {
			seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->snd_queue)) {
			seg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_queue)) {
			seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		if (kcp->buffer) {
			ikcp_free(kcp->buffer);
		}
		if (kcp->acklist) {
			ikcp_free(kcp->acklist);
		}

		kcp->nrcv_buf = 0;
		kcp->nsnd_buf = 0;
		kcp->nrcv_que = 0;
		kcp->nsnd_que = 0;
		kcp->ackcount = 0;
		kcp->buffer = NULL;
		kcp->acklist = NULL;
		ikcp_free(kcp);
	}
}


//---------------------------------------------------------------------
// set output callback, which will be invoked by kcp
//---------------------------------------------------------------------
void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len,
	ikcpcb *kcp, void *user))
{
	kcp->output = output;
}


//---------------------------------------------------------------------
// user/upper level recv: returns size, returns below zero for EAGAIN
//---------------------------------------------------------------------
int ikcp_recv(ikcpcb *kcp, char *buffer, int len)
{
	struct IQUEUEHEAD *p;
	int ispeek = (len < 0)? 1 : 0;
	int peeksize;
	int recover = 0;
	IKCPSEG *seg;
	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue))
		return -1;

	if (len < 0) len = -len;

	peeksize = ikcp_peeksize(kcp);//rcv_queue 中第一个包的大小. 这需要考虑到分片

	if (peeksize < 0) 
		return -2;//不满足长度

	if (peeksize > len) 
		return -3;//buff不足

	if (kcp->nrcv_que >= kcp->rcv_wnd)
		recover = 1;

	// merge fragment
	for (len = 0, p = kcp->rcv_queue.next; p != &kcp->rcv_queue; ) {
		int fragment;
		seg = iqueue_entry(p, IKCPSEG, node);
		p = p->next;

		if (buffer) {
			memcpy(buffer, seg->data, seg->len);
			buffer += seg->len;
		}

		len += seg->len;
		fragment = seg->frg;

		if (ikcp_canlog(kcp, IKCP_LOG_RECV)) {
			ikcp_log(kcp, IKCP_LOG_RECV, "recv sn=%lu", (unsigned long)seg->sn);
		}

		if (ispeek == 0) {
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
			kcp->nrcv_que--;
		}

		if (fragment == 0) 
			break;
	}

	assert(len == peeksize);

	// move available data from rcv_buf -> rcv_queue
	// 从rcv_buf->rcv_queue移动可用数据
	while (! iqueue_is_empty(&kcp->rcv_buf)) 
	{
		seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
		//保证收到的数据是有序的
		if (seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) 
		{
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
			kcp->nrcv_que++;
			kcp->rcv_nxt++;//期望接收到的下一个数据段的序号
		}	
		else 
		{
			break;
		}
	}

	// fast recover
	// 快速恢复
	if (kcp->nrcv_que < kcp->rcv_wnd && recover) {
		// ready to send back IKCP_CMD_WINS in ikcp_flush
		// tell remote my window size
		//准备在IKCP_flush中发回IKCP_CMD_WINS 告诉远端我的窗口大小
		kcp->probe |= IKCP_ASK_TELL;
	}

	return len;
}


//---------------------------------------------------------------------
// peek data size
// 查看数据大小
//---------------------------------------------------------------------
int ikcp_peeksize(const ikcpcb *kcp)
{
	struct IQUEUEHEAD *p;
	IKCPSEG *seg;
	int length = 0;

	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue)) return -1;

	seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
	if (seg->frg == 0) return seg->len;

	//接收未满足
	if (kcp->nrcv_que < seg->frg + 1) return -1;

	for (p = kcp->rcv_queue.next; p != &kcp->rcv_queue; p = p->next) {
		seg = iqueue_entry(p, IKCPSEG, node);
		length += seg->len;//计算所有的总长度大小
		if (seg->frg == 0) break;
	}

	return length;
}


//---------------------------------------------------------------------
// user/upper level send, returns below zero for error
//---------------------------------------------------------------------
int ikcp_send(ikcpcb *kcp, const char *buffer, int len)
{
	IKCPSEG *seg;
	int count, i;

	assert(kcp->mss > 0);
	if (len < 0) return -1;

	// append to previous segment in streaming mode (if possible)
	//在流模式下附加到前一段（如果可能）
	if (kcp->stream != 0) 
	{
		//前一发送节点不为空
		if (!iqueue_is_empty(&kcp->snd_queue)) 
		{
			IKCPSEG *old = iqueue_entry(kcp->snd_queue.prev, IKCPSEG, node);
			if (old->len < kcp->mss) //前数据节点 小于分片长度
			{
				int capacity = kcp->mss - old->len;//剩余空间
				int extend = (len < capacity)? len : capacity;//需要空间
				seg = ikcp_segment_new(kcp, old->len + extend);
				assert(seg);
				if (seg == NULL) 
				{
					return -2;
				}
				iqueue_add_tail(&seg->node, &kcp->snd_queue);
				memcpy(seg->data, old->data, old->len);
				if (buffer) 
				{
					memcpy(seg->data + old->len, buffer, extend);
					buffer += extend;
				}
				seg->len = old->len + extend;
				seg->frg = 0;
				len -= extend;
				iqueue_del_init(&old->node);
				ikcp_segment_delete(kcp, old);
			}
		}

		//剩余长度为0
		if (len <= 0) 
		{
			return 0;
		}
	}

	//包 分片
	if (len <= (int)kcp->mss) 
		count = 1;
	else 
		count = (len + kcp->mss - 1) / kcp->mss;

	//分片太大
	if (count >= (int)IKCP_WND_RCV) 
		return -2;

	if (count == 0) 
		count = 1;

	// fragment
	for (i = 0; i < count; i++) 
	{
		//分片长度
		int size = len > (int)kcp->mss ? (int)kcp->mss : len;
		seg = ikcp_segment_new(kcp, size);
		assert(seg);
		if (seg == NULL) 
		{
			return -2;
		}

		//拷贝分片数据
		if (buffer && len > 0) 
		{
			memcpy(seg->data, buffer, size);
		}
		seg->len = size;
		//分片编号(0为最后一个分片)
		seg->frg = (kcp->stream == 0)? (count - i - 1) : 0;
		iqueue_init(&seg->node);
		//插入发送队列
		iqueue_add_tail(&seg->node, &kcp->snd_queue);
		//增长发送队列计数
		kcp->nsnd_que++;
		if (buffer) 
		{
			buffer += size;
		}
		//总长度减去当前分片长度
		len -= size;
	}

	return 0;
}


//---------------------------------------------------------------------
// parse ack
// 处理ack时间
//---------------------------------------------------------------------
static void ikcp_update_ack(ikcpcb *kcp, IINT32 rtt)
{
	IINT32 rto = 0;
	if (kcp->rx_srtt == 0) 
	{
		kcp->rx_srtt = rtt;
		kcp->rx_rttval = rtt / 2;
	}
	else 
	{
		long delta = rtt - kcp->rx_srtt;
		if (delta < 0) delta = -delta;
		kcp->rx_rttval = (3 * kcp->rx_rttval + delta) / 4;
		kcp->rx_srtt = (7 * kcp->rx_srtt + rtt) / 8;
		if (kcp->rx_srtt < 1) kcp->rx_srtt = 1;
	}
	rto = kcp->rx_srtt + _imax_(kcp->interval, 4 * kcp->rx_rttval);
	kcp->rx_rto = _ibound_(kcp->rx_minrto, rto, IKCP_RTO_MAX);
}

static void ikcp_shrink_buf(ikcpcb *kcp)
{
	struct IQUEUEHEAD *p = kcp->snd_buf.next;
	if (p != &kcp->snd_buf) 
	{
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		kcp->snd_una = seg->sn;
	}	
	else 
	{
		kcp->snd_una = kcp->snd_nxt;
	}
}

//ACK确认
static void ikcp_parse_ack(ikcpcb *kcp, IUINT32 sn)
{
	struct IQUEUEHEAD *p, *next;

	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) 
	{
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (sn == seg->sn) 
		{
			//删除已经确认的send_buf中的数据
			iqueue_del(p);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
			break;
		}
		
		if (_itimediff(sn, seg->sn) < 0) 
		{
			break;
		}
	}
}

//UNA确认 小于UNA的包 全部确认
static void ikcp_parse_una(ikcpcb *kcp, IUINT32 una)
{
	struct IQUEUEHEAD *p, *next;
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) 
	{
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (_itimediff(una, seg->sn) > 0) 
		{
			iqueue_del(p);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
		}	
		else 
		{
			break;
		}
	}
}

//处理fast ack
static void ikcp_parse_fastack(ikcpcb *kcp, IUINT32 sn, IUINT32 ts)
{
	struct IQUEUEHEAD *p, *next;

	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) 
	{
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (_itimediff(sn, seg->sn) < 0) 
		{
			break;
		}
		else if (sn != seg->sn) //收到的包失序
		{
		#ifndef IKCP_FASTACK_CONSERVE
			seg->fastack++;
		#else
			if (_itimediff(ts, seg->ts) >= 0)
				seg->fastack++;
		#endif
		}
	}
}


//---------------------------------------------------------------------
// ack append
//---------------------------------------------------------------------
static void ikcp_ack_push(ikcpcb *kcp, IUINT32 sn, IUINT32 ts)
{
	IUINT32 newsize = kcp->ackcount + 1;
	IUINT32 *ptr;

	//长度不够时 开辟新的acklist
	if (newsize > kcp->ackblock) 
	{
		IUINT32 *acklist;
		IUINT32 newblock;

		for (newblock = 8; newblock < newsize; newblock <<= 1);
			acklist = (IUINT32*)ikcp_malloc(newblock * sizeof(IUINT32) * 2);

		if (acklist == NULL) 
		{
			assert(acklist != NULL);
			abort();
		}

		if (kcp->acklist != NULL) 
		{
			IUINT32 x;
			for (x = 0; x < kcp->ackcount; x++) 
			{
				acklist[x * 2 + 0] = kcp->acklist[x * 2 + 0];
				acklist[x * 2 + 1] = kcp->acklist[x * 2 + 1];
			}
			ikcp_free(kcp->acklist);
		}

		kcp->acklist = acklist;
		kcp->ackblock = newblock;
	}

	//序列:时间戳
	ptr = &kcp->acklist[kcp->ackcount * 2];
	ptr[0] = sn;
	ptr[1] = ts;

	kcp->ackcount++;
}

static void ikcp_ack_get(const ikcpcb *kcp, int p, IUINT32 *sn, IUINT32 *ts)
{
	if (sn) 
		sn[0] = kcp->acklist[p * 2 + 0];
	
	if (ts) 
		ts[0] = kcp->acklist[p * 2 + 1];
}


//---------------------------------------------------------------------
// parse data
// 处理收到数据
//---------------------------------------------------------------------
void ikcp_parse_data(ikcpcb *kcp, IKCPSEG *newseg)
{
	struct IQUEUEHEAD *p, *prev;
	IUINT32 sn = newseg->sn;
	int repeat = 0;
	
	//超出接收窗口 或 重复包 丢弃
	if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) >= 0 ||
		_itimediff(sn, kcp->rcv_nxt) < 0) 
	{
		ikcp_segment_delete(kcp, newseg);
		return;
	}

	//从后往前遍历
	for (p = kcp->rcv_buf.prev; p != &kcp->rcv_buf; p = prev) 
	{
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		prev = p->prev;
		if (seg->sn == sn) 
		{
			repeat = 1;
			break;
		}
		
		if (_itimediff(sn, seg->sn) > 0) 
		{
			break;
		}
	}

	if (repeat == 0) 
	{
		//加入到接收列表
		iqueue_init(&newseg->node);
		iqueue_add(&newseg->node, p);
		kcp->nrcv_buf++;
	}	
	else 
	{
		//重复数据 丢弃
		ikcp_segment_delete(kcp, newseg);
	}

#if 1
	ikcp_qprint("rcvbuf", &kcp->rcv_buf);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

	// move available data from rcv_buf -> rcv_queue
	// 从rcv_buf->rcv_queue 移动可用数据
	while (! iqueue_is_empty(&kcp->rcv_buf)) 
	{
		IKCPSEG *seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
		//当sn与下一个nxt不同时 直接退出  保证接收窗口大小不小于 0
		if (seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) 
		{
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
			kcp->nrcv_que++;
			kcp->rcv_nxt++;
		}	
		else 
		{
			break;
		}
	}

#if 1
	ikcp_qprint("queue", &kcp->rcv_queue);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

#if 1
//	printf("snd(buf=%d, queue=%d)\n", kcp->nsnd_buf, kcp->nsnd_que);
//	printf("rcv(buf=%d, queue=%d)\n", kcp->nrcv_buf, kcp->nrcv_que);
#endif
}


//---------------------------------------------------------------------
// input data
// 处理收到的输入数据
//---------------------------------------------------------------------
int ikcp_input(ikcpcb *kcp, const char *data, long size)
{
	IUINT32 prev_una = kcp->snd_una;
	IUINT32 maxack = 0, latest_ts = 0;
	int flag = 0;

	if (ikcp_canlog(kcp, IKCP_LOG_INPUT)) 
	{
		ikcp_log(kcp, IKCP_LOG_INPUT, "[RI] %d bytes", (int)size);
	}

	if (data == NULL || (int)size < (int)IKCP_OVERHEAD) return -1;

	while (1) 
	{
		IUINT32 ts, sn, len, una, conv;
		IUINT16 wnd;
		IUINT8 cmd, frg;
		IKCPSEG *seg;

		if (size < (int)IKCP_OVERHEAD) break;

		data = ikcp_decode32u(data, &conv);
		if (conv != kcp->conv) return -1;
		//读取头部数据
		data = ikcp_decode8u(data, &cmd);
		data = ikcp_decode8u(data, &frg);
		data = ikcp_decode16u(data, &wnd);
		data = ikcp_decode32u(data, &ts);
		data = ikcp_decode32u(data, &sn);
		data = ikcp_decode32u(data, &una);//未确认序号
		data = ikcp_decode32u(data, &len);
		//减去头部数据长度
		size -= IKCP_OVERHEAD;

		if ((long)size < (long)len || (int)len < 0) return -2;

		if (cmd != IKCP_CMD_PUSH && cmd != IKCP_CMD_ACK &&
			cmd != IKCP_CMD_WASK && cmd != IKCP_CMD_WINS) 
			return -3;

		//远端窗口大小
		kcp->rmt_wnd = wnd;
		//优先处理una
		ikcp_parse_una(kcp, una);
		//
		ikcp_shrink_buf(kcp);

		if (cmd == IKCP_CMD_ACK) 
		{
			if (_itimediff(kcp->current, ts) >= 0) 
			{
				ikcp_update_ack(kcp, _itimediff(kcp->current, ts));
			}

			//处理ack序列
			ikcp_parse_ack(kcp, sn);

			ikcp_shrink_buf(kcp);
			if (flag == 0) 
			{
				flag = 1;
				maxack = sn;
				latest_ts = ts;
			}	
			else 
			{
				if (_itimediff(sn, maxack) > 0) 
				{
				#ifndef IKCP_FASTACK_CONSERVE
					maxack = sn;
					latest_ts = ts;
				#else
					if (_itimediff(ts, latest_ts) > 0) 
					{
						maxack = sn;
						latest_ts = ts;
					}
				#endif
				}
			}
			if (ikcp_canlog(kcp, IKCP_LOG_IN_ACK)) 
			{
				ikcp_log(kcp, IKCP_LOG_IN_ACK, 
					"input ack: sn=%lu rtt=%ld rto=%ld", (unsigned long)sn, 
					(long)_itimediff(kcp->current, ts),
					(long)kcp->rx_rto);
			}
		}
		else if (cmd == IKCP_CMD_PUSH) 
		{
			if (ikcp_canlog(kcp, IKCP_LOG_IN_DATA)) 
			{
				ikcp_log(kcp, IKCP_LOG_IN_DATA, 
					"input psh: sn=%lu ts=%lu", (unsigned long)sn, (unsigned long)ts);
			}
			if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) < 0) 
			{
				//收到数据 将收到的sn加入到ack确认队列
				ikcp_ack_push(kcp, sn, ts);

				if (_itimediff(sn, kcp->rcv_nxt) >= 0) 
				{
					seg = ikcp_segment_new(kcp, len);
					seg->conv = conv;
					seg->cmd = cmd;
					seg->frg = frg;
					seg->wnd = wnd;
					seg->ts = ts;
					seg->sn = sn;
					seg->una = una;
					seg->len = len;

					if (len > 0) 
					{
						memcpy(seg->data, data, len);
					}

					ikcp_parse_data(kcp, seg);
				}
			}
		}
		else if (cmd == IKCP_CMD_WASK) 
		{
			// ready to send back IKCP_CMD_WINS in ikcp_flush
			// tell remote my window size
			//准备在IKCP_flush中发回IKCP_CMD_WINS告诉远端我的窗口大小
			kcp->probe |= IKCP_ASK_TELL;
			if (ikcp_canlog(kcp, IKCP_LOG_IN_PROBE)) 
			{
				ikcp_log(kcp, IKCP_LOG_IN_PROBE, "input probe");
			}
		}
		else if (cmd == IKCP_CMD_WINS) 
		{
			// do nothing
			if (ikcp_canlog(kcp, IKCP_LOG_IN_WINS)) 
			{
				ikcp_log(kcp, IKCP_LOG_IN_WINS,
					"input wins: %lu", (unsigned long)(wnd));
			}
		}
		else 
		{
			return -3;
		}

		data += len;
		size -= len;
	}

	if (flag != 0) 
	{
		//目前最大ack
		ikcp_parse_fastack(kcp, maxack, latest_ts);
	}

	if (_itimediff(kcp->snd_una, prev_una) > 0) 
	{
		//当前窗口大小 小于 
		if (kcp->cwnd < kcp->rmt_wnd) 
		{
			IUINT32 mss = kcp->mss;
			//拥塞窗口 小于 慢启动阈值
			if (kcp->cwnd < kcp->ssthresh) 
			{
				kcp->cwnd++;
				kcp->incr += mss;
			}	
			else 
			{
				if (kcp->incr < mss) kcp->incr = mss;
					kcp->incr += (mss * mss) / kcp->incr + (mss / 16);
				if ((kcp->cwnd + 1) * mss <= kcp->incr) 
				{
				#if 1
					kcp->cwnd = (kcp->incr + mss - 1) / ((mss > 0)? mss : 1);
				#else
					kcp->cwnd++;
				#endif
				}
			}
			
			if (kcp->cwnd > kcp->rmt_wnd) 
			{
				kcp->cwnd = kcp->rmt_wnd;
				kcp->incr = kcp->rmt_wnd * mss;
			}
		}
	}

	return 0;
}


//---------------------------------------------------------------------
// ikcp_encode_seg
//---------------------------------------------------------------------
static char *ikcp_encode_seg(char *ptr, const IKCPSEG *seg)
{
	ptr = ikcp_encode32u(ptr, seg->conv);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->cmd);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->frg);
	ptr = ikcp_encode16u(ptr, (IUINT16)seg->wnd);
	ptr = ikcp_encode32u(ptr, seg->ts);
	ptr = ikcp_encode32u(ptr, seg->sn);
	ptr = ikcp_encode32u(ptr, seg->una);
	ptr = ikcp_encode32u(ptr, seg->len);
	return ptr;
}

//获取窗口未使用长度
static int ikcp_wnd_unused(const ikcpcb *kcp)
{
	if (kcp->nrcv_que < kcp->rcv_wnd) 
	{
		//可接收窗口长度 - 已经接收长度 
		return kcp->rcv_wnd - kcp->nrcv_que;
	}
	return 0;
}


//---------------------------------------------------------------------
// ikcp_flush
//---------------------------------------------------------------------
void ikcp_flush(ikcpcb *kcp)
{
	//当前时间
	IUINT32 current = kcp->current;
	
	//准备发送的buff
	char *buffer = kcp->buffer;
	char *ptr = buffer;
	
	int count, size, i;
	IUINT32 resent, cwnd;
	IUINT32 rtomin;
	struct IQUEUEHEAD *p;
	int change = 0; // 是否执行过快速重传
	int lost = 0;	// 是否执行过超时重传
	IKCPSEG seg;

	// 'ikcp_update' haven't been called. 
	if (kcp->updated == 0) return;

	seg.conv = kcp->conv;
	seg.cmd = IKCP_CMD_ACK;
	seg.frg = 0;
	seg.wnd = ikcp_wnd_unused(kcp);
	seg.una = kcp->rcv_nxt;//下一个待确认的序号
	seg.len = 0;
	seg.sn = 0;
	seg.ts = 0;

	// flush acknowledges
	//刷新确认 发送ack指令
	count = kcp->ackcount;
	for (i = 0; i < count; i++) 
	{
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) 
		{
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		//获取ack的sn和ts
		ikcp_ack_get(kcp, i, &seg.sn, &seg.ts);
		//编码
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->ackcount = 0;

	// probe window size (if remote window size equals zero)
	// 探测窗口大小（如果远程窗口大小等于零）
	if (kcp->rmt_wnd == 0) 
	{
		if (kcp->probe_wait == 0) 
		{
			kcp->probe_wait = IKCP_PROBE_INIT;
			kcp->ts_probe = kcp->current + kcp->probe_wait;
		}	
		else 
		{
			if (_itimediff(kcp->current, kcp->ts_probe) >= 0) 
			{
				if (kcp->probe_wait < IKCP_PROBE_INIT) 
					kcp->probe_wait = IKCP_PROBE_INIT;
				kcp->probe_wait += kcp->probe_wait / 2;
				if (kcp->probe_wait > IKCP_PROBE_LIMIT)
					kcp->probe_wait = IKCP_PROBE_LIMIT;
				kcp->ts_probe = kcp->current + kcp->probe_wait;
				kcp->probe |= IKCP_ASK_SEND;
			}
		}
	}	
	else 
	{
		kcp->ts_probe = 0;
		kcp->probe_wait = 0;
	}

	// flush window probing commands
	// 刷新窗口探测命令
	if (kcp->probe & IKCP_ASK_SEND) 
	{
		seg.cmd = IKCP_CMD_WASK;//发送询问窗口指令
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) 
		{
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	// flush window probing commands
	// 刷新窗口探测命令
	if (kcp->probe & IKCP_ASK_TELL) {
		seg.cmd = IKCP_CMD_WINS;//发送告诉远端大小指令
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) 
		{
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->probe = 0;

	// calculate window size
	// 计算窗口大小  发送窗口 or 远端窗口
	cwnd = _imin_(kcp->snd_wnd, kcp->rmt_wnd);

	if (kcp->nocwnd == 0) 
		cwnd = _imin_(kcp->cwnd, cwnd);

	// move data from snd_queue to snd_buf
	// 将数据从sndqueue移动到sndbuf 
	//发送cwmd条消息
	while (_itimediff(kcp->snd_nxt, kcp->snd_una + cwnd) < 0) 
	{
		IKCPSEG *newseg;
		if (iqueue_is_empty(&kcp->snd_queue)) break;

		newseg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);

		iqueue_del(&newseg->node);
		iqueue_add_tail(&newseg->node, &kcp->snd_buf);
		kcp->nsnd_que--;
		kcp->nsnd_buf++;

		newseg->conv = kcp->conv;
		newseg->cmd = IKCP_CMD_PUSH;//发送数据指令
		newseg->wnd = seg.wnd;
		newseg->ts = current;
		newseg->sn = kcp->snd_nxt++;
		newseg->una = kcp->rcv_nxt;
		newseg->resendts = current;
		newseg->rto = kcp->rx_rto;
		newseg->fastack = 0;
		newseg->xmit = 0;
	}

	// calculate resent
	// 计算重新发送
	resent = (kcp->fastresend > 0) ? (IUINT32)kcp->fastresend : 0xffffffff;
	rtomin = (kcp->nodelay == 0) ? (kcp->rx_rto >> 3) : 0;//重连时间

	// flush data segments
	// 刷新数据段
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) 
	{
		IKCPSEG *segment = iqueue_entry(p, IKCPSEG, node);
		int needsend = 0;
		if (segment->xmit == 0) 
		{
			needsend = 1;
			segment->xmit++;
			segment->rto = kcp->rx_rto;
			segment->resendts = current + segment->rto + rtomin;
		}
		else if (_itimediff(current, segment->resendts) >= 0) 
		{
			//超时重传
			needsend = 1;
			segment->xmit++;
			kcp->xmit++;
			if (kcp->nodelay == 0) 
			{
				segment->rto += _imax_(segment->rto, (IUINT32)kcp->rx_rto);
			}	
			else 
			{
				IINT32 step = (kcp->nodelay < 2) ? ((IINT32)(segment->rto)) : kcp->rx_rto;
				segment->rto += step / 2;
			}
			segment->resendts = current + segment->rto;
			lost = 1;
		}
		else if (segment->fastack >= resent) //失序次数
		{
			//重传次数
			if ((int)segment->xmit <= kcp->fastlimit || kcp->fastlimit <= 0) 
			{
				needsend = 1;
				segment->xmit++;
				segment->fastack = 0;
				segment->resendts = current + segment->rto;
				change++;
			}
		}

		if (needsend) 
		{
			int need;
			segment->ts = current;
			segment->wnd = seg.wnd;
			segment->una = kcp->rcv_nxt;

			size = (int)(ptr - buffer);
			need = IKCP_OVERHEAD + segment->len;

			if (size + need > (int)kcp->mtu)
			{
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
			}

			ptr = ikcp_encode_seg(ptr, segment);

			if (segment->len > 0) 
			{
				memcpy(ptr, segment->data, segment->len);
				ptr += segment->len;
			}

			//超过最大次数
			if (segment->xmit >= kcp->dead_link) 
			{
				kcp->state = (IUINT32)-1;
			}
		}
	}

	// flash remain segments
	// 刷新剩余的段
	size = (int)(ptr - buffer);
	if (size > 0) 
	{
		ikcp_output(kcp, buffer, size);
	}

	// update ssthresh
	// 更新 慢启动阈值 (slow start threshold)
	// 发生快速重传, 进入快速恢复
	if (change) 
	{
		IUINT32 inflight = kcp->snd_nxt - kcp->snd_una;
		kcp->ssthresh = inflight / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = kcp->ssthresh + resent;
		kcp->incr = kcp->cwnd * kcp->mss;
	}

	// 发生超时重传, 进入慢启动
	if (lost) 
	{
		kcp->ssthresh = cwnd / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = 1;//丢包后 重置窗口大小
		kcp->incr = kcp->mss;
	}

	if (kcp->cwnd < 1) 
	{
		kcp->cwnd = 1;
		kcp->incr = kcp->mss;
	}
}


//---------------------------------------------------------------------
// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
//---------------------------------------------------------------------
void ikcp_update(ikcpcb *kcp, IUINT32 current)
{
	IINT32 slap;

	kcp->current = current;

	if (kcp->updated == 0) 
	{
		kcp->updated = 1;
		kcp->ts_flush = kcp->current;
	}

	slap = _itimediff(kcp->current, kcp->ts_flush);

	if (slap >= 10000 || slap < -10000) {
		kcp->ts_flush = kcp->current;
		slap = 0;
	}

	if (slap >= 0) 
	{
		kcp->ts_flush += kcp->interval;
		if (_itimediff(kcp->current, kcp->ts_flush) >= 0) 
		{
			kcp->ts_flush = kcp->current + kcp->interval;
		}
		ikcp_flush(kcp);
	}
}


//---------------------------------------------------------------------
// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
//确定何时调用ikcp_update：
//如果存在，则返回应以毫秒为单位调用ikcpupdate的时间
//没有ikcp_input/_send调用。你可以这样调用ikcpupdate
//而不是重复调用更新。
//重要的是要减少不必要的ikcp_update调用。用它来
//调度ikcp_ update（例如，
//或在处理大量kcp连接时优化ikcp_update）
//---------------------------------------------------------------------
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current)
{
	IUINT32 ts_flush = kcp->ts_flush;
	IINT32 tm_flush = 0x7fffffff;
	IINT32 tm_packet = 0x7fffffff;
	IUINT32 minimal = 0;
	struct IQUEUEHEAD *p;

	if (kcp->updated == 0) {
		return current;
	}

	if (_itimediff(current, ts_flush) >= 10000 ||
		_itimediff(current, ts_flush) < -10000) {
		ts_flush = current;
	}

	if (_itimediff(current, ts_flush) >= 0) {
		return current;
	}

	tm_flush = _itimediff(ts_flush, current);

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) 
	{
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		IINT32 diff = _itimediff(seg->resendts, current);
		if (diff <= 0) 
		{
			return current;
		}
		if (diff < tm_packet) 
			tm_packet = diff;
	}

	minimal = (IUINT32)(tm_packet < tm_flush ? tm_packet : tm_flush);

	if (minimal >= kcp->interval) minimal = kcp->interval;

	return current + minimal;
}



int ikcp_setmtu(ikcpcb *kcp, int mtu)
{
	char *buffer;
	if (mtu < 50 || mtu < (int)IKCP_OVERHEAD) 
		return -1;
	buffer = (char*)ikcp_malloc((mtu + IKCP_OVERHEAD) * 3);
	if (buffer == NULL) 
		return -2;
	kcp->mtu = mtu;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	ikcp_free(kcp->buffer);
	kcp->buffer = buffer;
	return 0;
}

int ikcp_interval(ikcpcb *kcp, int interval)
{
	if (interval > 5000) interval = 5000;
	else if (interval < 10) interval = 10;
	kcp->interval = interval;
	return 0;
}

int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc)
{
	if (nodelay >= 0) 
	{
		kcp->nodelay = nodelay;
		if (nodelay) 
		{
			kcp->rx_minrto = IKCP_RTO_NDL;	
		}	
		else 
		{
			kcp->rx_minrto = IKCP_RTO_MIN;
		}
	}

	if (interval >= 0) 
	{
		if (interval > 5000) interval = 5000;
		else if (interval < 10) interval = 10;
		kcp->interval = interval;
	}

	if (resend >= 0) 
	{
		kcp->fastresend = resend;
	}
	
	if (nc >= 0) 
	{
		kcp->nocwnd = nc;
	}

	return 0;
}


int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd)
{
	if (kcp) 
	{
		if (sndwnd > 0) 
		{
			kcp->snd_wnd = sndwnd;//发送窗口
		}

		if (rcvwnd > 0) 
		{   // must >= max fragment size
			kcp->rcv_wnd = _imax_(rcvwnd, IKCP_WND_RCV);//接收窗口
		}
	}
	return 0;
}

int ikcp_waitsnd(const ikcpcb *kcp)
{
	//已发送但未确认长度 + 待发送队列长度
	return kcp->nsnd_buf + kcp->nsnd_que;
}


// read conv
IUINT32 ikcp_getconv(const void *ptr)
{
	IUINT32 conv;
	ikcp_decode32u((const char*)ptr, &conv);
	return conv;
}


