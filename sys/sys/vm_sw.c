#ifndef lint
static char *sccsid = "@(#)vm_sw.c	1.5	ULTRIX	10/3/86";
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
/************************************************************************
 *
 *			Modification History
 *
 *	Koehler	11 Sept 86
 *	made changes to gfs namei interface
 *
 *	Stephen Reilly, 09-Sept-85
 *	Modified to handle the new 4.3BSD namei code.
 *
 *	vm_sw.c	6.1	83/07/29
 ***********************************************************************/
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/map.h"
#include "../h/uio.h"
#include "../h/file.h"
#include "../h/kmalloc.h"

struct	buf rswbuf;
/*
 * Indirect driver for multi-controller paging.
 */
swstrategy(bp)
	register struct buf *bp;
{
	int sz, off, seg;
	dev_t dev;

#ifdef GENERIC
	/*
	 * A mini-root gets copied into the front of the swap
	 * and we run over top of the swap area just long
	 * enough for us to do a mkfs and restor of the real
	 * root (sure beats rewriting standalone restor).
	 */
#define	MINIROOTSIZE	4096
	if (rootdev == dumpdev)
		bp->b_blkno += MINIROOTSIZE;
#endif
	sz = howmany(bp->b_bcount, DEV_BSIZE);
	if (bp->b_blkno+sz > nswap) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	if (nswdev > 1) {
		off = bp->b_blkno % dmmax;
		if (off+sz > dmmax) {
			bp->b_flags |= B_ERROR;
			iodone(bp);
			return;
		}
		seg = bp->b_blkno / dmmax;
		dev = swdevt[seg % nswdev].sw_dev;
		seg /= nswdev;
		bp->b_blkno = seg*dmmax + off;
	} else
		dev = swdevt[0].sw_dev;
	bp->b_dev = dev;
	if (dev == 0)
		panic("swstrategy");
	(*bdevsw[major(dev)].d_strategy)(bp);
}

swread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (physio(swstrategy, &rswbuf, dev, B_READ, minphys, uio));
}

swwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (physio(swstrategy, &rswbuf, dev, B_WRITE, minphys, uio));
}

/*
 * System call swapon(name) enables swapping on device name,
 * which must be in the swdevsw.  Return EBUSY
 * if already swapping on this device.
 */
swapon()
{
	struct a {
		char	*name;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	dev_t dev;
 	register struct nameidata *ndp = &u.u_nd;
	register struct swdevt *sp;

 	if (!suser())
 		return;
 	ndp->ni_nameiop = LOOKUP | FOLLOW;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		return;
	}
 	if(u.u_error = copyinstr(uap->name, ndp->ni_dirp, MAXPATHLEN, (u_int *)
	0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
 	gp = gfs_namei(ndp);
	km_free(ndp->ni_dirp, MAXPATHLEN);
	if (gp == NULL)
		return;
	if ((gp->g_mode&GFMT) != GFBLK) {
		u.u_error = ENOTBLK;
		gput(gp);
		return;
	}
	dev = (dev_t)gp->g_rdev;
	gput(gp);
	if (major(dev) >= nblkdev) {
		u.u_error = ENXIO;
		return;
	}
	/*
	 * Search starting at second table entry,
	 * since first (primary swap area) is freed at boot.
	 */
	for (sp = &swdevt[1]; sp->sw_dev; sp++)
		if (sp->sw_dev == dev) {
			if (sp->sw_freed) {
				u.u_error = EBUSY;
				return;
			}
			swfree(sp - swdevt);
			return;
		}
	u.u_error = ENODEV;
}

/*
 * Swfree(index) frees the index'th portion of the swap map.
 * Each of the nswdev devices provides 1/nswdev'th of the swap
 * space, which is laid out with blocks of dmmax pages circularly
 * among the devices.
 */
swfree(index)
	int index;
{
	register swblk_t vsbase;
	register long blk;
	dev_t dev;
	register swblk_t dvbase;
	register int nblks;

	dev = swdevt[index].sw_dev;
	(*bdevsw[major(dev)].d_open)(dev, FREAD|FWRITE);
	swdevt[index].sw_freed = 1;
	nblks = swdevt[index].sw_nblks;
	for (dvbase = 0; dvbase < nblks; dvbase += dmmax) {
		blk = nblks - dvbase;
		if ((vsbase = index*dmmax + dvbase*nswdev) >= nswap)
			panic("swfree");
		if (blk > dmmax)
			blk = dmmax;
		if (vsbase == 0) {
			/*
			 * Can't free a block starting at 0 in the swapmap
			 * but need some space for argmap so use 1/2 this
			 * hunk which needs special treatment anyways.
			 */
			argdev = swdevt[0].sw_dev;
			rminit(argmap, (long)(blk/2-ctod(CLSIZE)),
			    (long)ctod(CLSIZE), "argmap", ARGMAPSIZE);
			/*
			 * First of all chunks... initialize the swapmap
			 * the second half of the hunk.
			 */
			rminit(swapmap, (long)blk/2, (long)blk/2,
			    "swap", nswapmap);
		} else
			rmfree(swapmap, blk, vsbase);
	}
}
