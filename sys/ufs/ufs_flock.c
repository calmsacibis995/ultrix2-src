#ifndef lint
static char *sccsid = "@(#)ufs_flock.c	1.2	ULTRIX	10/3/86";
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
 *	Stephen Reilly, 09-Sept-85
 *	Created to handle the lockf call.
 *
 ***********************************************************************/
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
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
					wakeup((caddr_t)ptr); \
					ptr->stat.wakeflg = 0 ; \
				}
#define l_end 		l_len
#define MAXEND  	017777777777

extern	struct	flckinfo flckinfo;	/* configuration and acct info	*/
extern struct	filock	*frlock;	/* pointer to record lock free list*/
extern struct	flino	*frfid;		/* file id free list		*/
extern struct	flino	*fids;		/* file id head list		*/
extern struct	flino	sleeplcks;	/* head of chain of sleeping locks*/

struct filock	*ufs_insflck(),		*ufs_delflck(),		*ufs_flckadj();
struct filock	*ufs_blocked(),		*ufs_deadlock();

struct _rlock {
	struct filock *(*function)();
};

struct _rlock ufs_flock_routines[] = {
	ufs_insflck,
	ufs_delflck,
	ufs_flckadj,
	ufs_blocked,
	ufs_deadlock
};

#define RLOCKROUTINE(num)	ufs_flock_routines[(num)].function

/* ufs_rlock is the entry point for all of the ufs region/file locking
 * routines
 */
struct filock *
ufs_rlock(gp, cmd, flino, filock, flock)
	register struct gnode *gp;
	register int cmd;
	register struct flino *flino;
	register struct filock *filock;
	register struct flock *flock;
{
	register struct filock *(*routine)();
	
	if((routine = RLOCKROUTINE(cmd)) == NULL)
		return((struct filock *)EOPNOTSUPP);
		
	return((*routine)(flino, filock, flock));
}


/* insert lock after given lock using locking data */

struct filock *
ufs_insflck(flip, lckdat, fl)
	register struct	flino	*flip;
	register struct	filock	*fl;
	register struct	flock	*lckdat;
{
	register struct filock *new;
	register struct	filock	*f;

	new = frlock;
	if (new != NULL) {
		++flckinfo.reccnt;
		++flckinfo.rectot;
		frlock = new->next;
		if (frlock != NULL)
			frlock->prev = NULL;
		new->set = *lckdat;
		new->set.l_pid = u.u_procp->p_pid;
		new->stat.wakeflg = 0;
		if (fl == NULL) {
			new->next = flip->fl_flck;
			if (flip->fl_flck != NULL)
				flip->fl_flck->prev = new;
			flip->fl_flck = new;
		} else {
			new->next = fl->next;
			if (fl->next != NULL)
				fl->next->prev = new;
			fl->next = new;
		}
		new->prev = fl;
	}
	return (new);
}

/* delete lock
 */
struct filock *
ufs_delflck(flip, fl)
	register struct flino *flip;
	register struct filock *fl;
{
	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		flip->fl_flck = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;
	WAKEUP(fl);

	--flckinfo.reccnt;
	if (frlock == NULL) {
		fl->next = fl->prev = NULL;
		frlock = fl;
	} else {
		fl->next = frlock;
		fl->prev = NULL;
		frlock = (frlock->prev = fl);
	}
}


/* Adjust file lock from region specified by 'ld' starting at lock 'insrtp'
 */
struct filock *
ufs_flckadj(flip, insrtp, ld)
	struct flino	*flip;
	struct filock	*insrtp;
	register struct flock	*ld;
{
	struct	flock	td;			/* lock data for severed lock */
	register struct	filock	*flp, *nflp, *tdi, *tdp;
	int	insrtflg, rv = 0;
	register int	regtyp;

	insrtflg = (ld->l_type != F_UNLCK) ? 1 : 0;

	nflp = (insrtp == NULL) ? flip->fl_flck : insrtp;
	while (flp = nflp) {
		nflp = flp->next;
		if (flp->set.l_pid == u.u_procp->p_pid) {

			regtyp = regflck(ld, flp);

			/* release already locked region if necessary */
			
			switch (regtyp) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
			if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					if (insrtp == flp)
						insrtp = flp->prev;
					ufs_delflck(flip, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return((struct filock *)rv);
			case S_START|E_AFTER:
				insrtp = flp->prev;
				ufs_delflck(flip, flp);
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				if (insrtp == flp)
					insrtp = flp->prev;
				ufs_delflck(flip, flp);
				break;
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					td = flp->set;
					td.l_start = ld->l_end;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start < ld->l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (ufs_insflck(flip, &td, tdi) == NULL)
						return((struct filock *)ENOSPC);
				}
				if (insrtp == flp)
					insrtp = flp->prev;
				ufs_delflck(flip, flp);
				nflp = NULL;
				break;
			case S_START|E_MIDDLE:
			case S_MIDDLE|E_MIDDLE:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return((struct filock *)rv);
				/* setup piece after end of (un)lock */
				td = flp->set;
				td.l_start = ld->l_end;
				tdp = tdi = flp;
				do {
					if (tdp->set.l_start < ld->l_start)
						tdi = tdp;
					else
						break;
				} while (tdp = tdp->next);
				if (ufs_insflck(flip, &td, tdi) == NULL)
					return((struct filock *)ENOSPC);
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start;
					WAKEUP(flp);
					insrtp = flp;
				} else {
					insrtp = flp->prev;
					ufs_delflck(flip, flp);
				}
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return((struct filock *)rv);
				flp->set.l_end = ld->l_start;
				WAKEUP(flp);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					ufs_delflck(flip, flp);
				} else {
					flp->set.l_end = ld->l_start;
					WAKEUP(flp);
					insrtp = flp;
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		} else {
			if (flp->set.l_start > ld->l_end)
				nflp = NULL;
		}
	}

	if (insrtflg) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (ufs_insflck(flip, ld, insrtp) == NULL)
			rv = ENOSPC;
	}

	return ((struct filock *)rv);
}

/* blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * When blocked is called, 'flp' should point
 * to the record from which the search should begin.
 * Insrt is set to point to the lock before which the new lock
 * is to be placed.
 */
struct filock *
ufs_blocked(flp, lckdat, insrt)
	register struct filock *flp;
	register struct flock *lckdat;
	register struct filock **insrt;
{
	register struct filock *f;

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if (f->set.l_pid == u.u_procp->p_pid) {
			if (lckdat->l_start <= f->set.l_end
			    && lckdat->l_end >= f->set.l_start) {
				*insrt = f;
				break;
			}
		} else	if (lckdat->l_start < f->set.l_end
			    && lckdat->l_end > f->set.l_start
			    && (f->set.l_type == F_WRLCK
				|| (f->set.l_type == F_RDLCK
				    && lckdat->l_type == F_WRLCK)))
				return(f);
	}

	for ( ; f != NULL; f = f->next) {
		if (lckdat->l_start < f->set.l_end
		    && lckdat->l_end > f->set.l_start
		    && f->set.l_pid != u.u_procp->p_pid
		    && (f->set.l_type == F_WRLCK
			|| (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK)))
			return(f);
		if (f->set.l_start > lckdat->l_end)
			break;
	}

	return(NULL);
}


/* deadflck does the deadlock detection for the given record */
struct filock *
ufs_deadlock(flp)
	register struct filock *flp;
{
	register struct filock *blck, *sf;
	register int blckpid;

	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	do {
		if (blckpid == u.u_procp->p_pid)
			return((struct filock *)1);
		/* if the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks.fl_flck; sf != NULL; sf = sf->next) {
			if (blckpid == sf->set.l_pid) {
				blckpid = sf->stat.blkpid;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return((struct filock*) 0);
}

