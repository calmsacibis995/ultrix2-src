#ifndef lint
static char *sccsid = "@(#)tm.c	1.4	(ULTRIX)	4/18/86";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 * TM11/TE??
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vaxuba/ubareg.h"
#include "../vaxuba/tmreg.h"

#include "saio.h"

u_short	tmstd[] = { 0172520 };

tmopen(io)
	register struct iob *io;
{
	register skip;

	tmstrategy(io, TM_REW);
	skip = io->i_boff;
	while (skip--) {
		io->i_cc = 0;
		tmstrategy(io, TM_SFORW);
	}
}

tmclose(io)
	register struct iob *io;
{

	tmstrategy(io, TM_REW);
}

tmstrategy(io, func)
	register struct iob *io;
{
	register int com, unit, errcnt;
	register struct tmdevice *tmaddr =
	    (struct tmdevice *)ubamem(io->i_unit, tmstd[0]);
	int word, info;

	unit = io->i_unit;
	errcnt = 0;
retry:
	tmquiet(tmaddr);
	com = (unit<<8);
	info = ubasetup(io, 1);
	tmaddr->tmbc = -io->i_cc;
	tmaddr->tmba = info;
	if (func == READ)
		tmaddr->tmcs = com | TM_RCOM | TM_GO;
	else if (func == WRITE)
		tmaddr->tmcs = com | TM_WCOM | TM_GO;
	else if (func == TM_SREV) {
		tmaddr->tmbc = -1;
		tmaddr->tmcs = com | TM_SREV | TM_GO;
		return (0);
	} else
		tmaddr->tmcs = com | func | TM_GO;
	for (;;) {
		word = tmaddr->tmcs;
		DELAY(100);
		if (word & TM_CUR)
			break;
	}
	ubafree(io, info);
	word = tmaddr->tmer;
	if (word & TMER_EOT)
		return (0);
	if (word & TM_ERR) {
		if (word & TMER_EOF)
			return (0);
		if (errcnt == 0)
			printf("te error: er=%b", word, TMER_BITS);
		if (errcnt == 10) {
			printf("\n");
			return (-1);
		}
		errcnt++;
		tmstrategy(io, TM_SREV);
		goto retry;
	}
	if (errcnt)
		printf(" recovered by retry\n");
	if (word & TMER_EOF)
		return (0);
	return (io->i_cc + tmaddr->tmbc);
}

tmquiet(tmaddr)
	register struct tmdevice *tmaddr;
{
	register word;
	for (;;) {
		word = tmaddr->tmcs;
		DELAY(100);
		if (word&TM_CUR)
			break;
	}
	for (;;) {
		word = tmaddr->tmer;
		DELAY(100);
		if ((word&TMER_TUR) && (word&TMER_SDWN)==0)
			break;
	}
}
