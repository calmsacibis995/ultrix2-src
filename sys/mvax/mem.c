
#ifndef lint
static	char	*sccsid = "@(#)mem.c	1.4	(ULTRIX)	8/10/86";
#endif lint

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
/*	mem.c	6.1	83/07/29	*/
/*
 * Modification History
 *
 * 06-Aug-86 -- jaw	fixed baddaddr to work on running system.
 *
 * tresvik - 13-Jun-86
 *	Changed UNIcopy to test the address before reading or writing.
 *	Uses BADADDR.
 */

/*
 * Memory special file
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/uio.h"
#include "../h/cpuconf.h"

#include "../vax/mtpr.h"

mmread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (mmrw(dev, uio, UIO_READ));
}

mmwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (mmrw(dev, uio, UIO_WRITE));
}

mmrw(dev, uio, rw)
	dev_t dev;
	struct uio *uio;
	enum uio_rw rw;
{
	register int o;
	register u_int c, v;
	register struct iovec *iov;
	int error = 0;
	extern int umbabeg, umbaend;


	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

/* minor device 0 is physical memory */
		case 0:
			v = btop(uio->uio_offset);
			if (v >= physmem)
				goto fault;
			*(int *)mmap = v | PG_V |
				(rw == UIO_READ ? PG_KR : PG_KW);
			mtpr(TBIS, vmmap);
			o = (int)uio->uio_offset & PGOFSET;
			c = min((u_int)(NBPG - o), (u_int)iov->iov_len);
			c = min(c, (u_int)(NBPG - ((int)iov->iov_base&PGOFSET)));
			error = uiomove((caddr_t)&vmmap[o], (int)c, rw, uio);
			continue;

/* minor device 1 is kernel memory */
		case 1:
			if ((caddr_t)uio->uio_offset < (caddr_t)&umbabeg &&
			    (caddr_t)uio->uio_offset + uio->uio_resid >= (caddr_t)&umbabeg)
				goto fault;
			if ((caddr_t)uio->uio_offset >= (caddr_t)&umbabeg &&
			    (caddr_t)uio->uio_offset < (caddr_t)&umbaend)
				goto fault;
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, rw == UIO_READ ? B_READ : B_WRITE))
				goto fault;
			error = uiomove((caddr_t)uio->uio_offset, (int)c, rw, uio);
			continue;

/* minor device 2 is EOF/RATHOLE */
		case 2:
			if (rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			break;

/* minor device 3 is unibus memory (addressed by shorts) */
		case 3:
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, rw == UIO_READ ? B_READ : B_WRITE))
				goto fault;
			if (!useracc(iov->iov_base, c, rw == UIO_READ ? B_WRITE : B_READ))
				goto fault;
			error = UNIcpy((caddr_t)uio->uio_offset, iov->iov_base,
			    (int)c, rw);
			break;
		}
		if (error)
			break;
		iov->iov_base += c;
		iov->iov_len -= c;
		uio->uio_offset += c;
		uio->uio_resid -= c;
	}
	return (error);
fault:
	return (EFAULT);
}

/*
 * UNIBUS Address Space <--> User Space transfer
 */
UNIcpy(uniadd, usradd, n, rw)
	caddr_t uniadd, usradd;
	register int n;
	enum uio_rw rw;
{
	register short *from, *to;
	extern char Sysbase[];
 
	if (rw == UIO_READ) {
		from = (short *)uniadd;
		to = (short *)usradd;
	} else {
		from = (short *)usradd;
		to = (short *)uniadd;
	}
	for (n >>= 1; n > 0; n--) {
		int bad;
		extern	int cpu;

 		bad = BADADDR((rw == UIO_READ) ? from : to, 2);
		if (bad)
			return (EFAULT);
		*to++ = *from++;
	}
	return (0);
}
