#ifndef lint
static char *sccsid = "@(#)gfs_sysquota.c	1.9	ULTRIX	2/12/87";
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
 *	Robin 23-Jan-87
 *	changed define QUOTA so the login limit code was not part
 *	of the ifdef.
 *
 *	Robin 04-Dec-86
 *	Added code to set login limit.
 *
 *	koehler 11 Sep 86
 *	namei interface change
 *
 *	Robin Lewis, 11-Mar-86
 *	Added code to check for at&t login limit requirements.
 *
 *	Stephen Reilly, 30-Oct-85
 *	Modified code to reflect the new way getmdev is call.
 *
 *	Stephen Reilly, 14-Oct-85
 *	Modified user.h
 *
 *	Stephen Reilly, 09-Sept-85
 *	Modified to handle the new 4.3BSD namei code.
 *
 ***********************************************************************/
/*
 * MELBOURNE QUOTAS
 *
 * System calls.
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/gnode.h"
#include "../h/quota.h"
#include "../h/mount.h"
#include "../h/uio.h"
#include "../h/buf.h"
#include "../h/kmalloc.h"

int mlil = 0;
int tta = 2;

/*
 * The sys call that tells the system about a quota file.
 */
setquota()
{
	register struct a {
		char	*fblk;
		char	*fname;
	} *uap = (struct a *)u.u_ap;
	register struct mount *mp;
	register char *name;
	dev_t dev;
	
#ifndef QUOTA
	u.u_error = EINVAL;
	return;
#else

	if((name = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fblk, name, MAXPATHLEN, (u_int *)
	0)) {
		km_free(name, MAXPATHLEN);
		return;
	}

	u.u_error = getmdev(&dev, name, NULL);
	
	if (u.u_error) {
		km_free(name, MAXPATHLEN);		
		return;
	}

	
	GETMP(mp, dev);
	if (uap->fname == NULL)
		closedq(mp);
	else {
 		if(u.u_error = copyinstr(uap->fname, name, MAXPATHLEN, (u_int *)
			0)) {
			km_free(name, MAXPATHLEN);
			u.u_error = EFAULT;
			return;
		}
		opendq(mp, name);
	}
	km_free(name, MAXPATHLEN);		
	return;
#endif
}

/*
 * Sys call to allow users to find out
 * their current position wrt quota's
 * and to allow super users to alter it.
 */
qquota()
{
	register struct a {
		int	cmd;
		int	uid;
		int	arg;
		caddr_t	addr;
	} *uap = (struct a *)u.u_ap;
#ifdef QUOTA
	register struct quota *q;
#endif QUOTA
	extern int tta;

				/* at&t login limit hack */
	if (uap->cmd == Q_ULIMIT)
	{
		u.u_error = qulim();
		return;
	}
	 if (uap->cmd == Q_SLIMIT)
	{
		if (!suser())
			return (u.u_error);			/* XXX */
		tta = (uap->arg > 1) ? uap->arg  : 0;
		return;
	}

#ifdef QUOTA
	if (uap->uid < 0)
		uap->uid = u.u_ruid;
	if (uap->uid != u.u_ruid && uap->uid != u.u_quota->q_uid && !suser())
		return;
	if (uap->cmd != Q_SYNC && uap->cmd != Q_SETUID) {
		q = getquota(uap->uid, uap->cmd == Q_DOWARN, 0);
		if (q == NOQUOTA) {
			u.u_error = ESRCH;
			return;
		}
		if (u.u_error)
			goto bad;
	}
	switch (uap->cmd) {

	case Q_SETDLIM:
		u.u_error = setdlim(q, (dev_t)uap->arg, uap->addr);
		break;

	case Q_GETDLIM:
		u.u_error = getdlim(q, (dev_t)uap->arg, uap->addr);
		break;

	case Q_SETDUSE:
		u.u_error = setduse(q, (dev_t)uap->arg, uap->addr);
		break;

	case Q_SETWARN:
		u.u_error = setwarn(q, (dev_t)uap->arg, uap->addr);
		break;

	case Q_DOWARN:
		u.u_error = dowarn(q, (dev_t)uap->arg);
		break;
	
	case Q_SYNC:
		u.u_error = qsync((dev_t)uap->arg);
		return;

	case Q_SETUID:
		u.u_error = qsetuid(uap->uid, uap->arg);
		return;

	default:
		u.u_error = EINVAL;
		break;
	}
bad:
	delquota(q);
#endif
}

#ifdef QUOTA
/*
 * Q_SETDLIM - assign an entire dqblk structure.
 */
setdlim(q, dev, addr)
	register struct quota *q;
	dev_t dev;
	caddr_t addr;
{
	register struct gnode *gp;
	register struct dquot *dq, *odq;
	register struct mount *mp;	
	register int index;
	struct dqblk newlim;
	int  error = 0;

	if (!suser())
		return (u.u_error);			/* XXX */
	GETMP(mp, dev);
	index = mp - mount;
	if (index < 0 || index >= NMOUNT)
		return (EINVAL);
	dq = dqp(q, dev);
	if (dq == NODQUOT) {
		dq = dqalloc(q->q_uid, dev);
		if (dq == NODQUOT)
			return (error);
		dq->dq_cnt++;
		dq->dq_own = q;
		q->q_dq[index] = dq;
		odq = NODQUOT;
	} else
		odq = dq;

	if (dq->dq_uid != q->q_uid)
		panic("setdlim");
	while (dq->dq_flags & DQ_LOCK) {
		dq->dq_flags |= DQ_WANT;
		sleep((caddr_t)dq, PINOD+1);
	}
	error = copyin(addr, (caddr_t)&newlim, sizeof (struct dqblk));
	if (error) {
		if (dq != odq) {
			q->q_dq[index] = odq;
			dq->dq_cnt--;
		}
		dqrele(dq);
		return (error);
	}
	dq->dq_dqb = newlim;
	dq->dq_flags |= DQ_MOD;
	dqrele(dq);
	if (dq->dq_isoftlimit == 0 && dq->dq_bsoftlimit == 0) {
		q->q_dq[index] = NODQUOT;
		dq->dq_own = NOQUOTA;
		dqrele(dq);
		if (dq->dq_cnt == 0)	/* no files open using quota */
			return (error);
		dq = NODQUOT;
	}
	if (dq == odq)
		return (error);
	for (gp = gnode; gp < gnodeNGNODE; gp++)
		if (gp->g_uid == q->q_uid && gp->g_dev == dev) {
			if (dq == NODQUOT)
				dqrele(gp->g_dquot);
			else
				dq->dq_cnt++;
			gp->g_dquot = dq;
		}
	return (error);
}

/*
 * Q_GETDLIM - return current values in a dqblk structure.
 */
getdlim(q, dev, addr)
	register struct quota *q;
	dev_t dev;
	register caddr_t addr;
{
	register struct dquot *dq;
	register int error;

	dq = dqp(q, dev);
	if (dq == NODQUOT) {
		u.u_r.r_val1 = 1;
		return (0);
	}
	error = copyout((caddr_t)&dq->dq_dqb, addr, sizeof (struct dqblk));
	dqrele(dq);
	return (error);
}

/*
 * Q_SETDUSE - set current inode and disc block totals.
 * Resets warnings and associated flags.
 */
setduse(q, dev, addr)
	register struct quota *q;
	dev_t dev;
	register caddr_t addr;
{
	register struct dquot *dq;
	struct dqusage usage;
	register int error = 0;

	if (!suser())
		return (u.u_error);
	dq = dqp(q, dev);
	if (dq == NODQUOT) {
		u.u_r.r_val1 = 1;
		return (error);
	}
	while (dq->dq_flags & DQ_LOCK) {
		dq->dq_flags |= DQ_WANT;
		sleep((caddr_t)dq, PINOD+1);
	}
	if (dq->dq_uid != q->q_uid)
		panic("setduse");
	error = copyin(addr, (caddr_t)&usage, sizeof (usage));
	if (error == 0) {
		dq->dq_curinodes = usage.du_curinodes;
		dq->dq_curblocks = usage.du_curblocks;
		if (dq->dq_curinodes < dq->dq_isoftlimit)
			dq->dq_iwarn = MAX_IQ_WARN;
		if (dq->dq_curblocks < dq->dq_bsoftlimit)
			dq->dq_bwarn = MAX_DQ_WARN;
		dq->dq_flags &= ~(DQ_INODS | DQ_BLKS);
		dq->dq_flags |= DQ_MOD;
	}
	dqrele(dq);
	return (error);
}

/*
 * Q_SETWARN - set warning counters.
 */
setwarn(q, dev, addr)
	register struct quota *q;
	dev_t dev;
	register caddr_t addr;
{
	register struct dquot *dq;
	register int error = 0;
	struct dqwarn warn;

	if (!suser())
		return (u.u_error);			/* XXX */
	dq = dqp(q, dev);
	if (dq == NODQUOT) {
		u.u_r.r_val1 = 1;
		return (error);
	}
	while (dq->dq_flags & DQ_LOCK) {
		dq->dq_flags |= DQ_WANT;
		sleep((caddr_t)dq, PINOD+1);
	}
	if (dq->dq_uid != q->q_uid)
		panic("setwarn");
	error = copyin(addr, (caddr_t)&warn, sizeof (warn));
	if (error == 0) {
		dq->dq_iwarn = warn.dw_iwarn;
		dq->dq_bwarn = warn.dw_bwarn;
		dq->dq_flags &= ~(DQ_INODS | DQ_BLKS);
		dq->dq_flags |= DQ_MOD;
	}
	dqrele(dq);
	return (error);
}

/*
 * Q_DOWARN - force warning(s) to user(s).
 */
dowarn(q, dev)
	register struct quota *q;
	dev_t dev;
{
	register struct dquot *dq, **dqq;

	if (!suser() || u.u_ttyp == NULL)
		return (u.u_error);			/* XXX */
	if (dev != NODEV) {
		dq = dqp(q, dev);
		if (dq != NODQUOT) {
			qwarn(dq);
			dqrele(dq);
		}
		return (0);
	}
	for (dqq = q->q_dq; dqq < &q->q_dq[NMOUNT]; dqq++) {
		dq = *dqq;
		if (dq != NODQUOT && dq != LOSTDQUOT)
			qwarn(dq);
	}
	return (0);
}

/*
 * Q_SYNC - sync quota files to disc.
 */
qsync(dev)
	dev_t dev;
{
	register struct quota *q;
	register struct mount *mp;
	register index;

	if (!suser())
		return (u.u_error);			/* XXX */
	for (mp = mount, index = 0; mp < &mount[NMOUNT]; mp++, index++)
		if (mp->m_bufp && mp->m_qinod &&
		    (dev == NODEV || dev == mp->m_dev)) {
			for (q = quota; q < quotaNQUOTA; q++)
				if (q->q_cnt) {
					q->q_cnt++;
					putdq(mp, q->q_dq[index], 0);
					delquota(q);
				}
		}
	return (0);
}

/*
 * Q_SETUID - change quota to a particular uid.
 */
qsetuid(uid, noquota)
	register int uid, noquota;
{
	register struct quota *q;

	if (uid == u.u_quota->q_uid)
		return (0);
	if (!suser())
		return (u.u_error);			/* XXX */
	q = getquota(uid, 0, noquota ? Q_NDQ : 0);
	qclean();
	qstart(q);
	return (u.u_error);
}

#endif QUOTA
qulim()	/* at&t login limit hack */
{
	register int s;
	extern int tta;
	struct proc *procptr;

	if( tta != 0 )
	{
		if(mlil < tta)
		{
			procptr=u.u_procp;
			s=spl6();
			while((procptr->p_ppid != 1) &&
			((procptr->p_flag & SLOGIN) == 0))
				procptr=procptr->p_pptr;

			splx(s);
			if((procptr->p_flag & SLOGIN) == 0)
			{
				mlil++;
				u.u_procp->p_flag |= SLOGIN;
			}
			return(0);
		}
		else
		{
			return(1);
		}
	}
	return(0);
}
