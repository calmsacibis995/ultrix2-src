#ifndef lint
static char *sccsid = "@(#)gfs_subr.c	1.2	ULTRIX	10/3/86";
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

/* ------------------------------------------------------------------------
 * Modification History: /sys/sys/ufs_subr.c
 *
 * 11 Sep 86 -- koehler
 *	dev_t can't be a register
 *
 * 06 Nov 84 -- jrs
 *	Add small fix to update and Berkeley change to syncip() to
 *	cut overhead when syncing large files.
 *
 * 26 Oct 84 -- jrs
 *	Add small change for nami cache support
 *
 * 17 Jul 84 -- jmcg
 *	Added code to keep track of inode lockers and unlockers as a
 *	debugging aid.  Conditionally compiled with RECINODELOCK
 *
 * 17 Jul 84 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		ufs_subr.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#ifdef KERNEL
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/gnode.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/quota.h"
#include "../h/kernel.h"
#else
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/fs.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/gnode.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/quota.h>
#endif

#ifdef KERNEL
/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "syncprt"
 * above.
 */
bufstats()
{
	register int s, i, j, count;
	register struct buf *bp, *dp;
	int counts[MAXBSIZE/CLBYTES+1];
	static char *bname[BQUEUES] = { "LOCKED", "LRU", "AGE", "EMPTY" };

	for (bp = bfreelist, i = 0; bp < &bfreelist[BQUEUES]; bp++, i++) {
		count = 0;
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			counts[j] = 0;
		s = spl6();
		for (dp = bp->av_forw; dp != bp; dp = dp->av_forw) {
			counts[dp->b_bufsize/CLBYTES]++;
			count++;
		}
		splx(s);
		printf("%s: total-%d", bname[i], count);
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			if (counts[j] != 0)
				printf(", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}
#endif

#ifdef notdef
getfsx(dev)
	dev_t dev;
	
{
	return(getmp(dev) - mount);
}
#endif
