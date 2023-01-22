#ifndef lint
static char *sccsid = "@(#)ut.c	1.5	(ULTRIX)	4/18/86";
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

/*	ut.c	6.1	83/07/29	*/

/*
 * SI Model 9700 -- emulates TU45 on the UNIBUS
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vaxuba/ubareg.h"
#include "../vaxuba/utreg.h"

#include "saio.h"

#define	MASKREG(reg)	((reg)&0xffff)

u_short	utstd[] = { 0172440 };		/* non-standard */

utopen(io)
	register struct iob *io;
{
	int skip;

	utstrategy(io, UT_REW);
	skip = io->i_boff;
	while (skip-- > 0)
		utstrategy(io, UT_SFORWF);
}

utclose(io)
	register struct iob *io;
{

	utstrategy(io, UT_REW);
}

#define utwait(addr)	{do word=addr->utcs1; while((word&UT_RDY)==0);}

utstrategy(io, func)
	register struct iob *io;
{
	register u_short word;
	register int errcnt;
	register struct utdevice *addr =
	    (struct utdevice *)ubamem(io->i_unit, utstd[0]);
	int info, resid;
	u_short dens;

	dens = (io->i_unit&07) | UTTC_PDP11FMT | UTTC_PE;
	errcnt = 0;
retry:
	utquiet(addr);
	addr->uttc = dens;
	info = ubasetup(io, 1);
	addr->utwc = -((io->i_cc+1) >> 1);
	addr->utfc = -io->i_cc;
	if (func == READ) {
		addr->utba = info;
		addr->utcs1 = UT_RCOM | ((info>>8) & 0x30) | UT_GO;
	} else if (func == WRITE) {
		addr->utba = info;
		addr->utcs1 = UT_WCOM | ((info>>8) & 0x30) | UT_GO;
	} else if (func == UT_SREV) {
		addr->utcs1 = UT_SREV | UT_GO;
		return (0);
	} else
		addr->utcs1 = func | UT_GO;
	utwait(addr);
	ubafree(io, info);
	word = addr->utds;
	if (word&(UTDS_EOT|UTDS_TM)) {
		addr->utcs1 = UT_CLEAR | UT_GO;
		goto done;
	}
	if ((word&UTDS_ERR) || (addr->utcs1&UT_TRE)) {
		if (errcnt == 0)
			printf("tj error: cs1=%b er=%b cs2=%b ds=%b",
			  addr->utcs1, UT_BITS, addr->uter, UTER_BITS,
			  addr->utcs2, UTCS2_BITS, word, UTDS_BITS);
		if (errcnt == 10) {
			printf("\n");
			return (-1);
		}
		errcnt++;
		if (addr->utcs1&UT_TRE)
			addr->utcs2 |= UTCS2_CLR;
		addr->utcs1 = UT_CLEAR | UT_GO;
		utstrategy(io, UT_SREV);
		utquiet(addr);
		if (func == WRITE) {
			addr->utcs1 = UT_ERASE | UT_GO;
			utwait(addr);
		}
		goto retry;
	}
	if (errcnt)
		printf(" recovered by retry\n");
done:
	if (func == READ) {
		resid = 0;
		if (io->i_cc > MASKREG(addr->utfc))
			resid = io->i_cc - MASKREG(addr->utfc);
	} else
		resid = MASKREG(-addr->utfc);
	return (io->i_cc - resid);
}

utquiet(addr)
	register struct utdevice *addr;
{
	register u_short word;

	utwait(addr);
	do
		word = addr->utds;
	while ((word&UTDS_DRY) == 0 && (word&UTDS_PIP));
}
