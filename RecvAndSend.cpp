#include "RecvAndSend.h"

static struct hdr sendhdr, recvhdr, echohdr;

int send_reliable(int fd, char *sendbuf, size_t sendbytes, sockaddr *dest_addr, socklen_t dest_len)
{
	printf("calling send_reliable\n");
	struct iovec iovsend[2], iovrecv[1];

	sendhdr.seq = send_seq;
	if (rttisinited == 0)
	{
		rtt_init(&rtt_info);
		rttisinited = 1;
	}

	
	msgsend.msg_name = dest_addr;
	msgsend.msg_namelen = dest_len;
	msgsend.msg_iov = iovsend;
	msgsend.msg_iovlen = 2;
	iovsend[0].iov_base = &sendhdr;
	iovsend[0].iov_len = sizeof(struct hdr);
	iovsend[1].iov_base = sendbuf;
	iovsend[1].iov_len = sendbytes;

	msgrecv.msg_name = dest_addr;
	msgrecv.msg_namelen = dest_len;
	msgrecv.msg_iov = iovrecv;
	msgrecv.msg_iovlen = 1;
	iovrecv[0].iov_base = &recvhdr;
	iovrecv[0].iov_len = sizeof(struct hdr);

	signal(SIGALRM, sig_alrm);
	rtt_newdata(&rtt_info);
	
sendagain:
	sendhdr.ts = rtt_ts(&rtt_info);
	sendmsg(fd, &msgsend, 0);

	alarm(rtt_start(&rtt_info));
	if (sigsetjmp(jmpbuf, 1) != 0)
	{
		if (rtt_timeout(&rtt_info) < 0)
		{
			perror("Packet given up!");
			rttisinited = 0;
			return (0);
		}
		goto sendagain;
	}

	do {
		// if(recvhdr.seq != sendhdr.seq)
		// {
		// 	printf("recvhdr.seq != sendhdr.seq\n");
		// }
		recvmsg(fd, &msgrecv, 0);
	} while (iovrecv[0].iov_len < sizeof(struct hdr) || recvhdr.seq != sendhdr.seq);
	alarm(0);
	rtt_stopandcalc(&rtt_info, rtt_ts(&rtt_info) - sendhdr.ts);
	send_seq++;
	return 0;
}

static void sig_alrm(int sig)
{
	siglongjmp(jmpbuf, 1);
}

int recv_reliable(int fd, char *recvbuf, size_t recvbytes, sockaddr *from, socklen_t *fromlen)
{
	struct iovec iovsend[1], iovrecv[2];

	res_msgsend.msg_name = from;
	res_msgsend.msg_namelen = *fromlen;
	res_msgsend.msg_iov = iovsend;
	res_msgsend.msg_iovlen = 1;
	iovsend[0].iov_base = &echohdr;
	iovsend[0].iov_len = sizeof(struct hdr);
	

	res_msgrecv.msg_name = from;
	res_msgrecv.msg_namelen = *fromlen;
	res_msgrecv.msg_iov = iovrecv;
	res_msgrecv.msg_iovlen = 2;
	iovrecv[0].iov_base = &echohdr;
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = recvbuf;
	iovrecv[1].iov_len = recvbytes;

	char echobuf[sizeof(struct hdr)];
	if (recvmsg(fd, &res_msgrecv, 0) < 0)
	{
		perror("echo:recv error.\n");
		return -1;
	}
	printf("recv_reliable:package received, length:%d, recvbytes is:%d\n", strlen(recvbuf), recvbytes);
	//memcpy(echobuf, recvbuf, sizeof(struct hdr));
	printf("echohdr:%d, recv_seq:%d\n", echohdr.seq, recv_seq);
	if (echohdr.seq != recv_seq)
	{

		recv_seq = echohdr.seq;
		if (sendmsg(fd, &res_msgsend, 0) < 0)
		{
			perror("echo error.\n");
			return -1;
		}
		printf("response information sent!\n");
	}
	else
	{
		printf("Repeated pack!\n");
		return -1;
	}
	
	
}

