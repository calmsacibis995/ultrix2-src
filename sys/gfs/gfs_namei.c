#ifndef lint
static	char	*sccsid = "@(#)gfs_namei.c	1.5	(ULTRIX)	12/16/86";
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


/***********************************************************************
 *
 *		Modification History
 *
 * 11 Sep 86 -- koehler
 *	changed the namei interface
 *
 ***********************************************************************/


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/gnode.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/kmalloc.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif


struct gnode *
gfs_namei(ndp)
	register struct nameidata *ndp;
{
	register char *cp;		/* pointer into pathname argument */
/* these variables refer to things which must be freed or unlocked */
	register struct gnode *dp = 0;	/* the directory we are searching */
	register u_int lockparent;
	register u_int flag;
	extern struct gnode *rootdir;
	
	lockparent = ndp->ni_nameiop & LOCKPARENT;
	flag = ndp->ni_nameiop &~ (LOCKPARENT|NOCACHE|FOLLOW);
	if (flag == DELETE || lockparent)
		ndp->ni_nameiop &= ~NOCACHE;


	/*
	 * pathname always exist in kernel space at this point.
	 * set up the call to the SFS and go
	 */
		
	/*
	 * Get starting directory.
	 */

	cp = ndp->ni_dirp;
	if (*cp == '/') {
		while (*cp == '/')
			cp++;
		if ((dp = u.u_rdir) == NULL)
			dp = rootdir;
	} else
		dp = u.u_cdir;

	gfs_lock(dp);
	dp->g_count++;
	ndp->ni_pdir = dp;
	ndp->ni_endoff = 0;
	ndp->ni_slcnt = 0;
	
	/* search though the path calling the sfs's when needed */

	ndp->ni_cp = cp;

#ifdef GFSDEBUG
	if(GFS[1])
		cprintf("gfs_namei: pdir 0x%x (%d)\n", dp, dp->g_number);
#endif
	dp->g_flag |= GINCOMPLETE;
	while (!u.u_error && (dp->g_flag & GINCOMPLETE)) {
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf("gfs_namei: dp 0x%x (%d) type %d %s count %d\n",
			dp, dp->g_number, dp->g_mp->m_fstype, dp->g_flag
			& GLOCKED ? "locked" : "NOT LOCKED", dp->g_count);
#endif
		dp = OPS(dp)->go_namei(ndp);

		if(dp == NULL) {
#ifdef GFSDEBUG
			if(GFS[1])
				cprintf ("gfs_namei: sfs_namei return NULL, pdir 0x%x\n",
				ndp->ni_pdir);
#endif
			break;
		}
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf ("gfs_namei: flags 0%o count %d name '%s'\n",
			dp->g_flag, dp->g_count, ndp->ni_cp);
#endif
	}
	return(dp);
}
