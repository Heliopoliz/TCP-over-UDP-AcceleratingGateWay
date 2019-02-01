#pragma once

#include <signal.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <time.h>

#else
#include <sys/time.h>
#include <sys/types.h>
#include <stddef.h>
#endif

#define MINRTO 1
#define MAXRTO 30
#define MAXRESENDTIME 3 
 
struct rtt_info
{
	float rtt_rtt;
	float rtt_srtt;
	float rtt_val;
	float rtt_rto;
	int rtt_resend_times;
	uint32_t rtt_base;
};

void rtt_init(struct rtt_info *);
void rtt_newdata(struct rtt_info *);
int rtt_start(struct rtt_info *);
void rtt_stopandcalc(struct rtt_info *, uint32_t);
int rtt_timeout(struct rtt_info *);
uint32_t rtt_ts(struct rtt_info *);

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return 0;
}
#endif




