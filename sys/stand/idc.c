#ifndef lint
static	char	*sccsid = "@(#)idc.c	1.7		(ULTRIX)	4/18/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1983, 1986 by			*
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
/*	idc.c	6.1	83/07/29	*/

/*
 *	Modification History
 *
 * 25-Mar-85 --tresvik
 *	Fixed bug where idcwait happens on drive 0 when issuing a
 *	CRDY (initialization) instead of the selected drive
 *
 * 10-Dec-84  p.keilty
 *	Added io->i_error = EHER on harderr and moved ccleft -= thiscc
 *	so the retry on error would work correctly.
 *
 */

/*
 * IDC (RB730)
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vaxuba/idcreg.h"
#include "../vaxuba/ubareg.h"

#include "saio.h"

u_short	idcstd[] = { 0175606 };
short	rb02_off[] = { 0, 400, 0, -1, -1, -1, -1, -1 };
short	rb80_off[] = { 0, 37, 0, -1, -1, -1, 115, 305 };

int idc_type[4];

idcopen(io)
	register struct iob *io;
{
	register struct idcdevice *idcaddr;
	register int i;

	idcaddr = (struct idcdevice *)((caddr_t)ubauba(io->i_unit) + 0x200);
	if (io->i_boff < 0 || io->i_boff > 7)
		_stop("idc bad unit");
	idcaddr->idcmpr = IDCGS_GETSTAT;
	idcaddr->idccsr = IDC_GETSTAT|(io->i_unit<<8);
	idcwait(idcaddr);
	i = idcaddr->idcmpr;
	idcaddr->idccsr = IDC_CRDY|(io->i_unit<<8)|(1<<(io->i_unit+16));
	idcwait(idcaddr);
	idcaddr->idccsr = (io->i_unit<<8)|IDC_RHDR;
	idcwait(idcaddr);
	if (idcaddr->idccsr & IDC_ERR) {
		printf("idc error: idccsr %x\n", idcaddr->idccsr);
		_stop("idc fatal error");
	}
	i = idcaddr->idcmpr;
	i = idcaddr->idcmpr;
	if (idcaddr->idccsr & IDC_R80) {
		idc_type[io->i_unit] = 1;
		io->i_boff = rb80_off[io->i_boff] * NRB80SECT * NRB80TRK;
	} else {
		idc_type[io->i_unit] = 0;
		io->i_boff = rb02_off[io->i_boff] * NRB02SECT * NRB02TRK;
	}
	if (io->i_boff < 0)
		_stop("idc%d: bad unit type", io->i_unit);
}

idcstrategy(io, func)
	register struct iob *io;
{
	register struct idcdevice *idcaddr;
	int com;
	daddr_t bn;
	short dn, cn, sn, tn;
	short ccleft, thiscc = 0;
	int ubinfo, errcnt = 0;

	idcaddr = (struct idcdevice *)((caddr_t)ubauba(io->i_unit) + 0x200);
	ubinfo = ubasetup(io, 1);
	bn = io->i_bn;
	ccleft = io->i_cc;
retry:
	dn = io->i_unit;
	if (idc_type[dn]) {
		cn = bn/(NRB80SECT*NRB80TRK);
		sn = bn%NRB80SECT;
		tn = (bn / NRB80SECT) % NRB80TRK;
		thiscc = (NRB80SECT - sn) * 512;
	} else {
		cn = 2*bn/(NRB02SECT*NRB02TRK);
		sn = (2*bn)%NRB02SECT;
		tn = (2*bn / NRB02SECT) % NRB02TRK;
		thiscc = (NRB02SECT - sn) * 256;
	}
	thiscc = MIN(thiscc, ccleft);
/*	cn += io->i_boff; */
	idcaddr->idccsr = IDC_CRDY|IDC_SEEK|(dn<<8)|(1<<(dn+16));
	idcaddr->idcdar = (cn<<16)|(tn<<8)|sn;
	idcaddr->idccsr = IDC_SEEK|(dn<<8);
	idcwait(idcaddr);
	idcaddr->idccsr &= ~IDC_ATTN;
	com = dn<<8;
	if (func == READ)
		com |= IDC_READ;
	else
		com |= IDC_WRITE;
	idcaddr->idccsr = IDC_CRDY|com;
	idcaddr->idcbar = ubinfo&0x3ffff;
	idcaddr->idcbcr = -thiscc;
	idcaddr->idcdar = (cn<<16)|(tn<<8)|sn;
	idcaddr->idccsr = com;
	idcwait(idcaddr);
	if (idcaddr->idccsr & IDC_ERR) {
		printf("idc error: (cyl,trk,sec)=(%d,%d,%d) csr=%b\n",
		    cn, tn, sn, idcaddr->idccsr, IDCCSR_BITS);
		if (errcnt == 10) {
			printf("idc: unrecovered error\n");
			ubafree(io, ubinfo);
			io->i_error = EHER;
			return (-1);
		}
		errcnt++;
		goto retry;
	}
	if (errcnt)
		printf("idc: recovered by retry\n");
	ccleft -= thiscc;
	if (ccleft) {
		bn += thiscc/NBPG;
		ubinfo += thiscc;
		goto retry;
	}
	ubafree(io, ubinfo);
	return (io->i_cc);
}

idcwait(idcaddr)
	register struct idcdevice *idcaddr;
{
	register int i;

	while ((idcaddr->idccsr & (IDC_CRDY|IDC_DRDY)) != (IDC_CRDY|IDC_DRDY))
		for (i = 10; i; i--)
			;
}

/*ARGSUSED*/
idcioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}
