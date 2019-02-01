#ifndef RECV_AND_SEND_H


#include "stdafx.h"
#include "OvertimeResend.h"
#include <signal.h>
#include <setjmp.h>

#ifdef _WIN32
typedef jmp_buf sigjmp_buf;
struct iovec {
	const void *iov_base;
	size_t iov_len;
};
#endif

static struct rtt_info rtt_info;
static int rttisinited = 0;
static uint32_t recv_seq = 0;
static uint32_t send_seq = 1;
static struct msghdr msgsend, msgrecv, res_msgsend, res_msgrecv;


struct hdr
{
	uint32_t seq;
	uint32_t ts;
};


static void sig_alrm(int sig);
static sigjmp_buf jmpbuf;



int send_reliable(int fd, char *sendbuf, size_t sendbytes, sockaddr *dest_addr, socklen_t dest_len);
int recv_reliable(int fd, char *recvbuf, size_t recvbytes, sockaddr *from, socklen_t *fromlen);
 
#define RECV_AND_SEND_H
#endif

