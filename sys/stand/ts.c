/*	ts.c	6.1	83/07/29	*/

#ifndef lint
static	char	*sccsid = "@(#)ts.c	1.4	(ULTRIX)	4/18/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,1986 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History
 *
 * 10-Jul-84 --tresvik-001
 *	Unable to copy from a ts11 connected to other than the first
 *	unibus adapter.  Plugged in the unit number in tsopen routine.
 */

/*
 * TS11 tape driver
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vaxuba/tsreg.h"
#include "../vaxuba/ubareg.h"

#include "saio.h"

u_short	tsstd[] = { 0772520 };

struct	iob	ctsbuf;

u_short	ts_uba;			/* Unibus address of ts structure */

struct tsdevice *tsaddr = 0;

struct ts {
	struct ts_cmd ts_cmd;
	struct ts_char ts_char;
	struct ts_sts ts_sts;
} ts;

tsopen(io)
	register struct iob *io;
{
	static struct ts *ts_ubaddr;
	long i = 0;

	if (tsaddr == 0)
		tsaddr = (struct tsdevice *)ubamem(io->i_unit, tsstd[0]);
	tsaddr->tssr = 0;
	while ((tsaddr->tssr & TS_SSR)==0) {
		DELAY(10);
		if (++i > 1000000) {
			printf("ts: not ready\n");
			return;
		}
	}
	if (tsaddr->tssr&TS_OFL) {
		printf("ts: offline\n");
		return;
	}
	if (tsaddr->tssr&TS_NBA) {
		int i;

		ctsbuf.i_ma = (caddr_t) &ts;
		ctsbuf.i_cc = sizeof(ts);
		ctsbuf.i_unit = io->i_unit;	/* tresvik-001 */
		if (ts_ubaddr == 0)
			ts_ubaddr = (struct ts *)ubasetup(&ctsbuf, 2);
		ts_uba = (u_short)((long)ts_ubaddr + (((long)ts_ubaddr>>16)&03));
		ts.ts_char.char_addr = (int)&ts_ubaddr->ts_sts;
		ts.ts_char.char_size = sizeof(ts.ts_sts);
		ts.ts_char.char_mode = TS_ESS;
		ts.ts_cmd.c_cmd = TS_ACK|TS_SETCHR;
		i = (int)&ts_ubaddr->ts_char;
		ts.ts_cmd.c_loba = i;
		ts.ts_cmd.c_hiba = (i>>16)&3;
		ts.ts_cmd.c_size = sizeof(ts.ts_char);
		tsaddr->tsdb = ts_uba;
	}
	tsstrategy(io, TS_REW);
	if (io->i_cc = io->i_boff)
		tsstrategy(io, TS_SFORWF);
}

tsclose(io)
	register struct iob *io;
{

	tsstrategy(io, TS_REW);
}

tsstrategy(io, func)
	register struct iob *io;
{
	register int errcnt, info = 0;

	errcnt = 0;
retry:
	while ((tsaddr->tssr & TS_SSR) == 0)
		DELAY(100);
	if (func == TS_REW || func == TS_SFORWF)
		ts.ts_cmd.c_repcnt = io->i_cc;
	else {
		info = ubasetup(io, 1);
		ts.ts_cmd.c_size = io->i_cc;
		ts.ts_cmd.c_loba = info;
		ts.ts_cmd.c_hiba = (info>>16)&3;
	}
	if (func == READ)
		func = TS_RCOM;
	else if (func == WRITE)
		func = TS_WCOM;
	ts.ts_cmd.c_cmd = TS_ACK|TS_CVC|func;
	tsaddr->tsdb = ts_uba;
	do
		DELAY(100)
	while ((tsaddr->tssr & TS_SSR) == 0);
	if (info)
		ubafree(io, info);
	if (ts.ts_sts.s_xs0 & TS_TMK)
		return (0);
	if (tsaddr->tssr & TS_SC) {
		printf("ts tape error: er=%b, xs0=%b\n",
		    tsaddr->tssr, TSSR_BITS,
		    ts.ts_sts.s_xs0, TSXS0_BITS);
		if (errcnt==10) {
			printf("ts: unrecovered error\n");
			return (-1);
		}
		errcnt++;
		if (func == TS_RCOM || func == TS_WCOM)
			func |= TS_RETRY;
		goto retry;
	}
	if (errcnt)
		printf("ts: recovered by retry\n");
	return (io->i_cc - ts.ts_sts.s_rbpcr);
}
