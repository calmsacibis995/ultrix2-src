#ifndef lint
static char *sccsid = "@(#)gfs_gnode.c	1.5	ULTRIX	10/16/86";
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


/***********************************************************************
 *
 *		Modification History
 *
 * 11 Sep 86 -- koehler 
 *	changed gflush
 *
 * 16 Oct 86 -- koehler
 *	ggrab needs to make sure that the last free gnode is not being
 *	consumed.
 *
 ***********************************************************************/


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/conf.h"
#include "../h/buf.h"
#ifdef QUOTA
#include "../h/quota.h"
#endif
#include "../h/kernel.h"

union ghead ghead[GNOHSZ];
#ifdef GFSDEBUG
extern short GFS[];
#endif
 
struct gnode *gfreeh, **gfreet;

/*
 * Initialize hash links for gnodes
 * and build gnode free list.
 */
ghinit()
{
	register int i;
	register struct gnode *gp = gnode;
	register union  ghead *gh = ghead;

	for (i = GNOHSZ; --i >= 0; gh++) {
		gh->gh_head[0] = gh;
		gh->gh_head[1] = gh;
	}
	gfreeh = gp;
	gfreet = &gp->g_freef;
	gp->g_freeb = &gfreeh;
	gp->g_forw = gp;
	gp->g_back = gp;
	for (i = ngnode; --i > 0; ) {
		++gp;
		gp->g_forw = gp;
		gp->g_back = gp;
		*gfreet = gp;
		gp->g_freeb = gfreet;
		gfreet = &gp->g_freef;
	}
	gp->g_freef = NULL;
}

#ifdef notdef
/*
 * Find an gnode if it is incore.
 * This is the equivalent, for gnodes,
 * of ``incore'' in bio.c or ``pfind'' in subr.c.
 */
struct gnode *
gfind(dev, ino)
	dev_t dev;
	register gno_t gno;
{
	register struct gnode *gp;
	register union  ghead *gh;

	gh = &ghead[GNOHASH(dev, gno)];
	for (gp = gh->gh_chain[0]; gp != (struct gnode *)gh; gp = gp->g_forw)
		if (gno==gp->g_number && dev==gp->g_dev)
			return (gp);
	return ((struct gnode *)0);
}
#endif notdef


/*
 * Convert a pointer to an gnode into a reference to an gnode.
 *
 * This is basically the internal piece of iget (after the
 * gnode pointer is located) but without the test for mounted
 * filesystems.  It is caller's responsibility to check that
 * the gnode pointer is valid.
 */
ggrab(gp)
	register struct gnode *gp;
{
	while ((gp->g_flag&GLOCKED) != 0) {
		gp->g_flag |= GWANT;
		sleep((caddr_t)gp, PINOD);
	}
	if (gp->g_count == 0) {		/* ino on free list */
		register struct gnode *gq;
#ifdef GFSDEBUG
		if(GFS[5])
			cprintf("ggrab: gp 0x%x (%d) freef 0x%x freeb 0x%x\n",
			gp, gp->g_number, gp->g_freef, gp->g_freeb);
#endif
		gq = gp->g_freef;
		if(gp == gfreeh) {
			gfreeh = gp->g_freef;
			if(gfreeh)
 				gfreeh->g_freeb = &gfreeh;
		} else {
			if (gq)
				gq->g_freeb = gp->g_freeb;
			else
				gfreet = gp->g_freeb;
		}
		*gp->g_freeb = gq;
		gp->g_freef = NULL;
		gp->g_freeb = NULL;
	}
	gp->g_count++;
	GLOCK(gp);
}

/*
 * Decrement reference count of
 * an gnode structure.
 * On the last reference,
 * write the gnode out and if necessary,
 * truncate and deallocate the file.
 */
gput(gp)
	register struct gnode *gp;
{
	register int ret;
#ifdef GFSDEBUG
	int *foo;
#endif

#ifdef GFSDEBUG
	if(GFS[5])
		cprintf("gput: gp 0x%x (%d) called by 0x%x\n", gp,
		gp->g_number, *(&foo + 5));
#endif

	if(gp->g_count < 1) {
		printf ("gput: gp 0x%x g_dev 0x%x number %d\n", gp,
		gp->g_dev, gp->g_number);
		panic("gput g_count < 1!");  /* added by Rich */
	}
	/* not all filesystem types may have a unlock routine */
	
	gfs_unlock(gp);

	ret = GRELE(gp);
}

/*
 * remove any gnodes in the gnode cache belonging to dev
 *
 * There should not be any active ones, return error if any are found
 * (nb: this is a user error, not a system err)
 *
 * Also, count the references to dev by block devices - this really
 * has nothing to do with the object of the procedure, but as we have
 * to scan the gnode table here anyway, we might as well get the
 * extra benefit.
 *
 * this is called from sumount()/sys3.c when dev is being unmounted
 */
#ifdef QUOTA
gflush(dev, gq, mgp)
	dev_t dev;
	register struct gnode *gq;
	struct gnode *mgp;
#else
gflush(dev, mgp)
	dev_t dev;
	struct gnode *mgp;
#endif
{
	register struct gnode *gp;
	register open = 0;

	for (gp = gnode; gp < gnodeNGNODE; gp++) {
#ifdef QUOTA
		if (gp != gq && gp->g_dev == dev && gp != gp->g_mp->m_gnodp) {
#else
		if ((gp->g_dev == dev) && (gp != gp->g_mp->m_gnodp)) {
#endif
			if((gp == mgp) && (mgp->g_count == 1))
				continue;
			if (gp->g_count)
				return(-1);
			else {
				remque(gp);
				gp->g_forw = gp;
				gp->g_back = gp;
				/*
				 * as g_count == 0, the gnode was on the free
				 * list already, just leave it there, it will
				 * fall off the bottom eventually. We could
				 * perhaps move it to the head of the free
				 * list, but as umounts are done so
				 * infrequently, we would gain very little,
				 * while making the code bigger.
				 */
#ifdef QUOTA
				dqrele(gp->g_dquot);
				gp->g_dquot = NODQUOT;
#endif
			}
		}
		else if (gp->g_count && (gp->g_mode&GFMT)==GFBLK &&
		   gp->g_rdev == dev) /* hack? */
			open++;
	}
	return (open);
}

struct gnode *
gfs_gget(dev, mpp, gno, flag) 
	dev_t dev;
	register struct mount *mpp;
	register gno_t gno;
	register int flag;
{
	return(GGET(dev, mpp, gno, flag));
}


int
gfs_lock(gp)
	register struct gnode *gp;
{
	GLOCK(gp);
}


int
gfs_unlock(gp)
	register struct gnode *gp;
{
	register int (*unlock)();
	
	if(unlock = OPS(gp)->go_unlock) {
		if(!(gp->g_flag & GLOCKED)) {
			printf("gfs_unlock: gp 0x%x (%d) dev 0x%x\n", gp,
			gp ? gp->g_number : -1, gp ? gp->g_dev : -1);
			panic("gfs_unlock: unlocked gnode");
		}
		(*unlock)(gp);
	} else if(gp->g_flag & GLOCKED)
		panic("gfs_unlock: locked gnode, no unlock routine");
}


void
gfs_grele(gp)
	register struct gnode *gp;
{
	register int ret;
	
	ret = GRELE(gp);
}
