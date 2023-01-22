# ifndef lint
static	char	*sccsid = "@(#)conf.c	1.7	(ULTRIX)	3/23/86";
# endif not lint
/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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


/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/conf.c
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 17-Jun-85 - jaw
 *	add support for rx50 on 8200.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 27 Dec 84 --afd
 *	Added tmscp routines into the device switch for tmscp tape device
 *	support.
 *
 * 10 Dec 84 --reilly
 *	The devsw table now has a new field that can be used to determine
 *	if the device is a tape drive.
 *
 * 29 Dec 83 --jmcg
 *	Commented the basis for the conditional compilation of the
 *	configuration table.  Rearranged the order to reduce the
 *	confusion.  Added MicroVAX_1 as a sort of default.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		conf.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"

#include "../vaxmba/mbareg.h"

#include "saio.h"

devread(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_RDDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.g_dev].dv_strategy)(io, READ);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devwrite(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_WRDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.g_dev].dv_strategy)(io, WRITE);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devopen(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.g_dev].dv_open)(io);
}

devclose(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.g_dev].dv_close)(io);
}

devioctl(io, cmd, arg)
	register struct iob *io;
	int cmd;
	caddr_t arg;
{

	return ((*devsw[io->i_ino.g_dev].dv_ioctl)(io, cmd, arg));
}

/*ARGSUSED*/
nullsys(io)
	struct iob *io;
{

	;
}

/*ARGSUSED*/
nullioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}

/*
 *	Configuration rules on which all the conditional compilation
 *	is based:
 *		massbus drives only on 750, 780 and 8600
 *		only 730 has an IDC
 *		MicroVAX only has ra (RQDX1) and rl
 *		tapes are not configured for booting
 */

int	nullsys(), nullioctl();
#if defined (VAX8600) || defined(VAX780) || defined(VAX750)
int	hpstrategy(), hpopen(), hpioctl();
#endif
#if defined (VAX8600) || defined(VAX780) || defined(VAX750) || defined(VAX730)
int	upstrategy(), upopen(), upioctl();
int	rkstrategy(), rkopen(), rkioctl();
#endif
int	rastrategy(), raopen(), raioctl();
int	rlstrategy(), rlopen(), rlioctl();
#if defined(VAX730)
int	idcstrategy(), idcopen(), idcioctl();
#endif
#if defined(VAX8200)
int	csstrategy(), csopen(), csioctl();
#endif
#ifndef BOOT
#if defined (VAX8600) || defined(VAX780) || defined(VAX750) || defined(VAX730)
int	tmstrategy(), tmopen(), tmclose();
int	tsstrategy(), tsopen(), tsclose();
int	utstrategy(), utopen(), utclose();
#endif
#if defined (VAX8600) || defined(VAX780) || defined(VAX750)
int	htstrategy(), htopen(), htclose();
int	mtstrategy(), mtopen(), mtclose();
#endif
int	tmscpstrategy(), tmscpopen(), tmscpclose();
#endif BOOT

struct devsw devsw[] = {
#if defined (VAX8600) || defined(VAX780) || defined(VAX750)
	{ "hp",	hpstrategy,	hpopen,		nullsys,	hpioctl,
		0 },
#endif
#if defined (VAX8600) || defined(VAX780) || defined(VAX750) || defined(VAX730)
	{ "up",	upstrategy,	upopen,		nullsys,	upioctl,
		0 },
	{ "hk",	rkstrategy,	rkopen,		nullsys,	rkioctl,
		0 },
#endif
	{ "ra",	rastrategy,	raopen,		nullsys,	raioctl,
		0 },
	{ "rl",	rlstrategy,	rlopen,		nullsys,	rlioctl,
		0 },
#if defined(VAX730)
	{ "rb",	idcstrategy,	idcopen,	nullsys,	idcioctl,
		0 },
#endif
#if defined(VAX8200)
	{ "cs", csstrategy,	csopen,		nullsys,	csioctl,
		0 },
#endif

#ifndef BOOT
#if defined (VAX8600) || defined(VAX780) || defined(VAX750) || defined(VAX730)
	{ "ts",	tsstrategy,	tsopen,		tsclose,	nullioctl,
		B_TAPE },
	{ "tm",	tmstrategy,	tmopen,		tmclose,	nullioctl,
		B_TAPE },
	{ "ut",	utstrategy,	utopen,		utclose,	nullioctl,
		B_TAPE },
#endif
#if defined (VAX8600) || defined(VAX780) || defined(VAX750)
	{ "ht",	htstrategy,	htopen,		htclose,	nullioctl,
		B_TAPE },
	{ "mt",	mtstrategy,	mtopen,		mtclose,	nullioctl,
		B_TAPE },
#endif
	{ "tms",tmscpstrategy,tmscpopen,	tmscpclose,	nullioctl,
		B_TAPE },
#endif BOOT
	{ 0, 0, 0, 0, 0, 0 },
};
