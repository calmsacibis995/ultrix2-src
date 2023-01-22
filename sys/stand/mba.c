#ifndef lint
static char *sccsid = "@(#)mba.c	1.4	(ULTRIX)	4/22/86";
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


#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/vm.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vax/mtpr.h"
#include "../vaxmba/mbareg.h"
#include "../vaxmba/hpreg.h"

#include "saio.h"

mbastart(io, func)
	register struct iob *io;
	int func;
{
	struct mba_regs *mba = mbamba(io->i_unit);
	struct mba_drv *drv = mbadrv(io->i_unit);
	register struct pte *pte = mba->mba_map;
	int npf;
	unsigned v;
	int o;
	int vaddr;

	v = btop(io->i_ma);
	o = (int)io->i_ma & PGOFSET;
	npf = btoc(io->i_cc + o);
	vaddr = o;
	while (--npf >= 0)
		*(int *)pte++ = v++ | PG_V;
	mba->mba_sr = -1;
	mba->mba_bcr = -io->i_cc;
	mba->mba_var = vaddr;
	if (io->i_flgs&F_SSI)
		drv->mbd_of |= HPOF_SSEI;
	switch (io->i_flgs & F_TYPEMASK) {

	case F_RDDATA:			/* standard read */
		drv->mbd_cs1 = MB_RCOM|MB_GO;
		mbawait(io);
		return(0);

	case F_WRDATA:			/* standard write */
		drv->mbd_cs1 = MB_WCOM|MB_GO;
		mbawait(io);
		return(0);

	/* the following commands apply to disks only */

	case F_HDR|F_RDDATA:	
		drv->mbd_cs1 = HP_RHDR|HP_GO;
		break;

	case F_HDR|F_WRDATA:
		drv->mbd_cs1 = HP_WHDR|HP_GO;
		break;

	case F_CHECK|F_WRDATA:
	case F_CHECK|F_RDDATA:
		drv->mbd_cs1 = HP_WCDATA|HP_GO;
		break;

	case F_HCHECK|F_WRDATA:
	case F_HCHECK|F_RDDATA:
		drv->mbd_cs1 = HP_WCHDR|HP_GO;
		break;

	default:
		goto error;
	}
	mbawait(io);
	if ((drv->mbd_dt & MBDT_TAP) == 0)
		return (0);
error:
	io->i_error = ECMD;
	io->i_flgs &= ~F_TYPEMASK;
	return (1);
}

mbawait(io)
	register struct iob *io;
{
	struct mba_regs *mba = mbamba(io->i_unit);
	struct mba_drv *drv = mbadrv(io->i_unit);

	while (mba->mba_sr & MBSR_DTBUSY)
		DELAY(100);
}

mbainit(mbanum)
	int mbanum;
{
	register struct mba_regs *mba = mbaddr[mbanum];

	/* SHOULD BADADDR IT */
	if (mbaact & (1<<mbanum))
		return;
	mba->mba_cr = MBCR_INIT;
	mbaact |= 1<<mbanum;
}
