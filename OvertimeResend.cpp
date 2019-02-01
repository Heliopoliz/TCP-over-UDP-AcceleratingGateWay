#include "OvertimeResend.h"

#define RTOCALC(p) ((p)->rtt_srtt+ 4.0 * ((p)->rtt_val))  

static float rtt_minmax(float rto)
{
	if (rto < MINRTO) {
		return MINRTO;
	}
	else if (rto > MAXRTO) {
		return MAXRTO;
	}
	else
		return rto;
}

void rtt_init(rtt_info *p)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	p->rtt_base = tv.tv_sec;
	p->rtt_rtt = 0;
	p->rtt_srtt = 0;
	p->rtt_val = 0.75;
	p->rtt_rto = rtt_minmax(RTOCALC(p));
}

void rtt_newdata(rtt_info *p)
{
	p->rtt_resend_times = 0;
}

int rtt_start(rtt_info *p)
{
	return ((int)(p->rtt_rto + 0.5));
}

void rtt_stopandcalc(rtt_info *p, uint32_t ms)
{
	double delta;
	p->rtt_rtt = ms / 1000;
	delta = p->rtt_rtt - p->rtt_srtt;
	p->rtt_srtt += delta / 8;
	if (delta < 0)
	{
		delta = -delta;
	}
	p->rtt_val += (delta - p->rtt_val) / 4;
	p->rtt_rto = rtt_minmax(RTOCALC(p));
}

int rtt_timeout(rtt_info *p)
{
	p->rtt_rto *= 2;
	if (++(p->rtt_resend_times) > MAXRESENDTIME)
	{ return (-1); }
	return (0);
}
  

uint32_t rtt_ts(rtt_info *p)
{
	uint32_t ts;
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	return ((tv.tv_sec-p->rtt_base) * 1000) + (tv.tv_usec /1000);
}

