#ifndef lint
static char *sccsid = "@(#)gfs_flock.c	1.4	ULTRIX	10/3/86";
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
 *	Paul Shaughnessy (prs), 15-Sept-86
 *	Changed DEADLOCK index to DELLOCK.
 *
 *	koehler 11 Sep 86
 *	unregisterized dev_t
 *
 *	Stephen Reilly, 09-Sept-85
 *	Created to handle the lockf call.
 *
 ***********************************************************************/
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/gnode.h"
#include "../h/mount.h"
#include "../h/proc.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/flock.h"

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

#define	SLEEP(ptr, pri)		sleep(ptr, pri)
#define	WAKEUP(ptr)		if (ptr->stat.wakeflg) { \
					wakeup(ptr); \
					ptr->stat.wakeflg = 0 ; \
				}
#define l_end 		l_len
#define MAXEND  	017777777777

extern	struct	flckinfo flckinfo;	/* configuration and acct info		*/
struct	filock	*frlock;		/* pointer to record lock free list	*/
struct	flino	*frfid;			/* file id free list			*/
struct	flino	*fids;			/* file id head list			*/
struct	flino	sleeplcks;		/* head of chain of sleeping locks	*/

/* find file id */
struct flino *
findfid(fp)
	register struct file *fp;
{
	register struct flino *flip;
	dev_t d;
	register gno_t n;
	register struct gnode *gp = (struct gnode *)fp->f_data;

	d = gp->g_dev;
	n = gp->g_number;
	flip = fids;
	while (flip != NULL) {
		if (flip->fl_dev == d && flip->fl_number == n) {
			flip->fl_refcnt++;
			break;
		}
		flip = flip->next;
	}
	return (flip);
}

struct flino *
allocfid(fp)
	register struct file *fp;
{
	register struct flino *flip;
	register struct gnode *gp = (struct gnode *)fp->f_data;

	flip = frfid;
	if (flip != NULL) {
		++flckinfo.filcnt;
		++flckinfo.filtot;
		/* remove from free list */
		frfid = flip->next;
		if (frfid != NULL)
			frfid->prev = NULL;

		/* insert into allocated file identifier list */
		if (fids != NULL)
			fids->prev = flip;
		flip->next = fids;
		fids = flip;

		/* set up file identifier info */
		++flip->fl_refcnt;
		flip->fl_dev = gp->g_dev;
		flip->fl_number = gp->g_number;
	}
	return (flip);
}

freefid(flip)
	register struct flino *flip;
{
	if (--flip->fl_refcnt <= 0 && flip->fl_flck == NULL) {
		--flckinfo.filcnt;
		if (flip->prev != NULL)
			flip->prev->next = flip->next;
		else
			fids = flip->next;
		if (flip->next != NULL)
			flip->next->prev = flip->prev;
		flip->fl_dev = 0;
		flip->fl_number = 0;
		flip->fl_refcnt = 0;
		if (frfid != NULL)
			frfid->prev = flip;
		flip->next = frfid;
		flip->prev = NULL;
		frfid = flip;
	}
}
	

/* build file lock free list
 */
flckinit()
{
	register i;

	for (i=0; i<flckinfo.fils; i++) {
		freefid(&flinotab[i]);
	}
	flckinfo.filcnt = 0;

	for (i=0; i<flckinfo.recs; i++) {
		if (frlock == NULL) {
			flox[i].next = flox[i].prev = NULL;
			frlock = &flox[i];
		} else {
			flox[i].next = frlock;
			flox[i].prev = NULL;
			frlock = (frlock->prev = &flox[i]);
		}
	}
	flckinfo.reccnt = 0;
}

/* regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
int
regflck(ld, flp)
	register struct flock *ld;
	register struct filock *flp;
{
	register int regntype;

	if (ld->l_start > flp->set.l_start) {
		if (ld->l_start > flp->set.l_end)
			return(S_AFTER|E_AFTER);
		else if (ld->l_start == flp->set.l_end)
			return(S_END|E_AFTER);
		else
			regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end > flp->set.l_start) {
		if (ld->l_end > flp->set.l_end)
			regntype |= E_AFTER;
		else if (ld->l_end == flp->set.l_end)
			regntype |= E_END;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_start)
		regntype |= E_START;
	else
		regntype |= E_BEFORE;

	return (regntype);
}

/* locate overlapping file locks
 */
getflck(fp, lckdat)
	register struct file *fp;
	register struct flock *lckdat;
{
	register struct flino *flip;
	struct filock *found, *insrt = NULL;
	register int retval = 0;
	int inwhence;		/* whence of request */

	/*
	 *	Locks can only happen to disk inodes.
	 */
	if ( fp->f_type != DTYPE_INODE )
		return(EOPNOTSUPP);

	/*
	 *	Make sure the inode pointer is valid
	 */
	if ( fp->f_data == NULL )
		return(EFAULT);

	/* get file identifier and file lock list pointer if there is one */
	flip = findfid(fp);
	if (flip == NULL) {
		lckdat->l_type = F_UNLCK;
		return (0);
	}

	/* convert start to be relative to beginning of file */
	inwhence = lckdat->l_whence;
	if (retval=convoff(fp, lckdat, 0))
		return (retval);
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* find overlapping lock */
	found = GRLOCK((struct gnode *)fp->f_data, BLOCKLOCK,
	(struct flino *) flip->fl_flck, (struct filock *) lckdat,
	(struct flock *) &insrt);
	if (found != NULL)
		*lckdat = found->set;
	else
		lckdat->l_type = F_UNLCK;
	freefid(flip);

	/* restore length */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= lckdat->l_start;

	retval = convoff(fp, lckdat, inwhence);
	return (retval);
}

/* clear and set file locks
 */
setflck(fp, lckdat, slpflg)
	register struct file *fp;
	struct flock *lckdat;
	int slpflg;
{
	register struct flino *flip;
	register struct filock *found, *sf;
	struct filock *insrt = NULL;
	register int retval = 0;
	register int contflg = 0;

	/*
	 *	Locks can only happen to disk inodes.
	 */
	if ( fp->f_type != DTYPE_INODE )
		return(EOPNOTSUPP);

	/*
	 *	Make sure the inode pointer is valid
	 */
	if ( fp->f_data == NULL )
		return(EFAULT);

	/* check access permissions */
	if ((lckdat->l_type == F_RDLCK && (fp->f_flag&FREAD) == 0)
	    || (lckdat->l_type == F_WRLCK && (fp->f_flag&FWRITE) == 0))
		return (EBADF);
	
	/* convert start to be relative to beginning of file */
	if (retval=convoff(fp, lckdat, 0))
		return (retval);
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* get or create a file/inode record lock header */
	flip = findfid(fp);
	if (flip == NULL) {
		if (lckdat->l_type == F_UNLCK)
			return (0);
		if ((flip=allocfid(fp)) == NULL)
			return (EMFILE);
	}

	do {
		contflg = 0;
		switch (lckdat->l_type) {
		case F_RDLCK:
		case F_WRLCK:
			if((found = (struct filock *)GRLOCK((struct gnode *)
			fp->f_data, BLOCKLOCK, (struct flino *) flip->fl_flck,
			(struct filock *) lckdat, (struct flock *) &insrt))
			== NULL)
				retval = (int)GRLOCK((struct gnode *) fp->f_data,
				ADJLOCK, flip, insrt, lckdat);
			else if (slpflg) {
				/* do deadlock detection here */
				if(GRLOCK((struct gnode *)fp->f_data,
				DEADLOCK, (struct flino *) found,
				(struct filock *) 0, (struct flock *) 0))
					retval = EDEADLK;
				else
					if((sf = GRLOCK((struct gnode *) fp->f_data,
					MAKELOCK, &sleeplcks, lckdat, NULL))
					== NULL)
						retval = ENOSPC;
					else {
						found->stat.wakeflg++;
						sf->stat.blkpid = found->set.l_pid;
						if (SLEEP(found, PCATCH|(PZERO+1)))
							retval = EINTR;
						else
							contflg = 1;
						sf->stat.blkpid = 0;
						(void) GRLOCK((struct gnode *) fp->f_data, DELLOCK,
						&sleeplcks, sf, (struct flock *)NULL);
					}
			} else
				retval = EACCES;
			break;
		case F_UNLCK:
			/* removing a file record lock */
			retval = (int) GRLOCK((struct gnode *)fp->f_data,
			ADJLOCK, flip, flip->fl_flck, lckdat);
			break;
		default:
			retval = EINVAL;	/* invalid lock type */
			break;
		}
	} while (contflg);
	freefid(flip);
	return(retval);
}

/* convoff - converts the given data (start, whence) to the
 * given whence.
 */
int
convoff(fp, lckdat, whence)
	register struct file *fp;
	register struct flock *lckdat;
	register int whence;
{
	register struct gnode *gp = (struct gnode *)fp->f_data;

	if (lckdat->l_whence == 1)
		lckdat->l_start += fp->f_offset;
	else if (lckdat->l_whence == 2)
		lckdat->l_start += gp->g_size;
	else if (lckdat->l_whence != 0)
		return (EINVAL);
	if (lckdat->l_start < 0)
		return (EINVAL);
	if (whence == 1)
		lckdat->l_start -= fp->f_offset;
	else if (whence == 2)
		lckdat->l_start -= gp->g_size;
	else if (whence != 0)
		return (EINVAL);
	lckdat->l_whence = whence;
	return (0);
}


/* Clean up record locks left around by process (called in closef) */
cleanlocks(fp)
	register struct file *fp;
{
	register struct filock *flp, *nflp;
	register struct flino *flip;

	/*
	 *	Locks can only happen to disk inodes.
	 */
	if ( fp->f_type != DTYPE_INODE )
		return;

	/*
	 *	Make sure the inode pointer is valid
	 */
	if ( fp->f_data == NULL )
		return;

	flip = findfid(fp);
	if (flip == NULL)
		return;

	for (flp=flip->fl_flck; flp!=NULL; flp=nflp) {
		nflp = flp->next;
		if (flp->set.l_pid == u.u_procp->p_pid)
			GRLOCK((struct gnode *)fp->f_data, DELLOCK, flip, flp,
			(struct flock *) 0);
	}
	freefid(flip);

	return;
}

