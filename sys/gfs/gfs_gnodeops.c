#ifndef lint
static	char	*sccsid = "@(#)gfs_gnodeops.c	1.24	(ULTRIX)	3/26/87";
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
 * 23 Mar 87 -- prs
 *	Fixed gno_close to decrement reference count in gnode
 *	before returning from special cases.
 *
 * 03 Mar 87 -- chase
 *	refresh gnode attributes before an append mode write
 *
 * 03 Mar 87 -- prs
 *	Changed gno_close to verify block device was not mounted before
 *	flushing and invalidating all its buffers.
 *
 * 29 Jan 87 -- chet
 *	moved gput() after GCLOSE in gno_close().
 *
 * 19 Sep 86 -- lp
 *	fixed bug in ioctl code when enabling n-buff.
 * 11 Sep 86 -- koehler
 *	fixed close routine
 *
 ***********************************************************************/


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/cmap.h"
#include "../h/stat.h"
#include "../h/kernel.h"


int	gno_rw(), gno_ioctl(), gno_select(), gno_close();
struct 	fileops gnodeops =
	{ gno_rw, gno_ioctl, gno_select, gno_close };

int 	divorce_rw(), divorce_ioctl(), divorce_select(), divorce_close();
struct	fileops divorceops =
	{ divorce_rw, divorce_ioctl, divorce_select, divorce_close };

#ifdef GFSDEBUG
extern short GFS[];
#endif

gno_rw(fp, rw, uio)
	register struct file *fp;
	register enum uio_rw rw;
	register struct uio *uio;
{
	register struct gnode *gp = (struct gnode *)fp->f_data;
	register int error;
	
	if ((gp->g_mode&GFMT) == GFREG) {
		gfs_lock(gp);
		if (fp->f_flag&FAPPEND && rw == UIO_WRITE) {
			(void)GGETVAL(gp);
			uio->uio_offset = fp->f_offset = gp->g_size;
		}

		/*
		 * If synchronous write flag was passed in the file
		 * pointer from the open or fcntl system calls,
		 * then do a synchronous write.
		 */

		if (fp->f_flag & O_FSYNC)
			error = rwgp(gp, uio, rw, IO_SYNC, fp->f_cred);
		else
			error = rwgp(gp, uio, rw, IO_ASYNC, fp->f_cred);

		gfs_unlock(gp);			
	} else
		error = rwgp(gp, uio, rw, IO_ASYNC, fp->f_cred);

	return (error);
}

rdwri(rw, gp, base, len, offset, segflg, aresid)
	register struct gnode *gp;
	register caddr_t base;
	int len, offset, segflg;
	register int *aresid;
	enum uio_rw rw;
{
	struct uio _auio;
	register struct uio *auio = &_auio;
	struct iovec _aiov;
	register struct iovec *aiov = &_aiov;
	register int error;

	auio->uio_iov = aiov;
	auio->uio_iovcnt = 1;
	aiov->iov_base = base;
	aiov->iov_len = len;
	auio->uio_resid = len;
	auio->uio_offset = offset;
	auio->uio_segflg = segflg;
	error = rwgp(gp, auio, rw, IO_ASYNC, u.u_cred);
	if (aresid)
		*aresid = auio->uio_resid;
	else
		if (auio->uio_resid)
			error = EIO;
	return (error);
}

rwgp(gp, uio, rw, ioflag, cred)
	register struct gnode *gp;
	register struct uio *uio;
	register enum uio_rw rw;
	int ioflag;
	struct ucred *cred;
{
	dev_t dev = (dev_t)gp->g_rdev;
	register int type;
	extern int mem_no;
	register int ret;
	
	if (rw != UIO_READ && rw != UIO_WRITE)
		panic("rwgp");
	if (rw == UIO_READ && uio->uio_resid == 0)
		return (0);
	if (uio->uio_offset < 0 &&
	    ((gp->g_mode&GFMT) != GFCHR || mem_no != major(dev)))
		return (EINVAL);
	
	
	if (rw == UIO_WRITE && type == GFREG && uio->uio_offset +
	uio->uio_resid > u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		return (EFBIG);
	}
	
	ret = GRWGP(gp, uio, rw, ioflag, cred);
	if (!ret && rw == UIO_READ) {
		if(ISLOCAL(gp->g_mp))
			gp->g_flag |= GACC;
	}
	return(ret);
}


gno_ioctl(fp, com, data, cred)
	register struct file *fp;
	register int com;
	register caddr_t data;
	struct ucred *cred;
{
 	register struct gnode *gp = ((struct gnode *)fp->f_data);
	register int fmt = gp->g_mode & GFMT;
	dev_t dev;
	register int flag;
	
	switch (fmt) {
		case GFREG:
		case GFDIR:
			if (com == FIONREAD) 
				flag = fp->f_offset;
			break;
		case GFCHR:
			flag = fp->f_flag;
			dev = gp->g_rdev;
		       /*
			* cdevsw[].d_strat implies this device can
			*  do multi-buffered operations. Otherwise
		 	*  just fall into device ioctl.
			*/
			if(cdevsw[major(dev)].d_strat) {
				if(com == FIONBUF) {
					int *acount = (int *)data;
					if(*acount < 0)
						return(ENXIO);
				   	if(*acount > 0) {
						fp->f_flag |= FNBUF;
						gp->g_flag |= ASYNC;
						startasync(dev, acount,
						fp->f_flag);
				   	} else if(fp->f_flag&FNBUF) {
						if(!asyncclose(dev, fp->f_flag))
							gp->g_flag &= ~ASYNC;
						fp->f_flag &= ~FNBUF;
				   	}
			 	  	return(0);
				}
				if(com == FIONBDONE)
					return(aiodone(dev, *(int *)data,
					fp->f_flag));
				if(com == FIONBIO || com == FIOASYNC)
						return(0); 
			} else if(com == FIONBUF)
				return(ENXIO);

	}

	if((flag = GFNCTL(gp, com, data, flag, fp->f_cred)) == GNOFUNC)
		flag = EOPNOTSUPP;
	return(flag);
}

gno_select(fp, which)
	register struct file *fp;
	register int which;
{
	register struct gnode *gp = (struct gnode *)fp->f_data;

	return (GSELECT(gp, which, fp->f_cred));
}

#ifdef notdef
gno_clone()
{

	return (EOPNOTSUPP);
}
#endif

gno_stat(gp, sb)
	register struct gnode *gp;
	register struct stat *sb;
{

	return (GSTAT(gp, sb));
}

gno_close(fp)
	register struct file *fp;
{
	register struct file *ffp;
	register struct gnode *gp = (struct gnode *)fp->f_data;
	register int flag;
	dev_t dev;
	register int mode;	
	register struct gnode *tgp;
	register struct mount *mp;
	
#ifdef GFSDEBUG
	if(GFS[0])
		cprintf("gno_close: fp 0x%x gp 0x%x\n", fp, gp);
#endif
	flag = fp->f_flag;
	if (flag & (FSHLOCK|FEXLOCK))
		gno_unlock(fp, FSHLOCK|FEXLOCK);
	dev = (dev_t)gp->g_rdev;
	mode = gp->g_mode & GFMT;
			/* call to CLOSEP is here instead of	*/
			/* in the following "switch" statement	*/
			/* because it must occur before the	*/
			/* call to IPUT.			*/
	if(mode == GFPORT)
		closep(gp, flag);
	fp->f_data = (caddr_t) 0;
	if((mode != GFREG) && (mode != GFDIR)) {
		/*
		 * Check that another inode for the same device isn't active.
		 * This is because the same device can be referenced by
		 * two different inodes (can only happen with 2 mknods of same
	 	 * major/minor numbers)
		 */
		for (ffp = file; ffp < fileNFILE; ffp++) {
			if(ffp->f_count == 0)
				continue;
			if (ffp == fp)
				continue;
			if (ffp->f_type != DTYPE_INODE		/* XXX */
				&&  ffp->f_type != DTYPE_PORT)
				continue;
			if ((tgp = (struct gnode *)ffp->f_data) &&
			    tgp->g_rdev == dev && (tgp->g_mode&GFMT) == mode) {
#ifdef GFSDEBUG
				if(GFS[0])
					cprintf("gno_close: 2 major/minor pairs\n");
#endif
				/*
				 * Decrement ref count in gnode
				 */
				GRELE(gp);
				return;
			}
		}
		if (mode == GFBLK) {
			/*
			 * On last close of a block device (that isn't mounted)
			 * we must invalidate any in core blocks, so that
			 * we can, for instance, change floppy disks.
			 */
			GETMP(mp, dev);
			/*
			 * If GETMP returns a mp, then the block device
			 * is mounted on, so return.
			 */
			if (mp != NULL) {
				/*
				 * Decrement ref count in gnode
				 */
				GRELE(gp);
				return;
			}
			bflush(dev);
			binval(dev, (struct gnode *) 0);
		}
		if (setjmp(&u.u_qsave)) {
			/*
			 * If device close routine is interrupted,
			 * must return so closef can clean up.
			 */
			 if (u.u_error == 0)
			 	u.u_error = EINTR;	/* ??? */
			return;
		}
	}
	if(GCLOSE(gp, flag) == GNOFUNC)
		u.u_error = EOPNOTSUPP;

	gfs_lock(gp);
	gput(gp);

}

/*
 * Place an advisory lock on an inode.
 */
gno_lock(fp, cmd)
	register struct file *fp;
	register int cmd;
{
	register int priority = PLOCK;
	register struct gnode *gp = (struct gnode *)fp->f_data;

	if ((cmd & LOCK_EX) == 0)
		priority++;
	/*
	 * If there's a exclusive lock currently applied
	 * to the file, then we've gotta wait for the
	 * lock with everyone else.
	 */
again:
	while (gp->g_flag & GEXLOCK) {
		if (cmd & LOCK_NB)
			return (EWOULDBLOCK);
		/*
		 * If we're holding an exclusive
		 * lock, then release it.
		 */
		if (fp->f_flag & FEXLOCK) {
			gno_unlock(fp, FEXLOCK);
			goto again;
		}
		gp->g_flag |= GLWAIT;
		sleep((caddr_t)&gp->g_exlockc, priority);
	}
	if (cmd & LOCK_EX) {
		cmd &= ~LOCK_SH;
		/*
		 * Must wait for any shared locks to finish
		 * before we try to apply a exclusive lock.
		 */
		while (gp->g_flag & GSHLOCK) {
			if (cmd & LOCK_NB)
				return (EWOULDBLOCK);
			/*
			 * If we're holding a shared
			 * lock, then release it.
			 */
			if (fp->f_flag & FSHLOCK) {
				gno_unlock(fp, FSHLOCK);
				goto again;
			}
			gp->g_flag |= GLWAIT;
			sleep((caddr_t)&gp->g_shlockc, PLOCK);

			/*
			 * If an exclusive lock was put on the file
			 * before we woke up, try again.
			 */
			if (gp->g_flag & GEXLOCK)
				goto again;
		}
	}
	if (fp->f_flag & (FSHLOCK|FEXLOCK))
		panic("gno_lock");
	if (cmd & LOCK_SH) {
		gp->g_shlockc++;
		gp->g_flag |= GSHLOCK;
		fp->f_flag |= FSHLOCK;
	}
	if (cmd & LOCK_EX) {
		gp->g_exlockc++;
		gp->g_flag |= GEXLOCK;
		fp->f_flag |= FEXLOCK;
	}
	return (0);
}

/*
 * Unlock a file.
 */
gno_unlock(fp, kind)
	register struct file *fp;
	register int kind;
{
	register struct gnode *gp = (struct gnode *)fp->f_data;
	register int flags;

	kind &= fp->f_flag;
	if (gp == NULL || kind == 0)
		return;
	flags = gp->g_flag;
	if (kind & FSHLOCK) {
		if ((flags & GSHLOCK) == 0)
			panic("gno_unlock: SHLOCK");
		if (--gp->g_shlockc == 0) {
			gp->g_flag &= ~GSHLOCK;
			if (flags & GLWAIT)
				wakeup((caddr_t)&gp->g_shlockc);
		}
		fp->f_flag &= ~FSHLOCK;
	}
	if (kind & FEXLOCK) {
		if ((flags & GEXLOCK) == 0)
			panic("gno_unlock: EXLOCK");
		if (--gp->g_exlockc == 0) {
			gp->g_flag &= ~(GEXLOCK|GLWAIT);
			if (flags & GLWAIT)
				wakeup((caddr_t)&gp->g_exlockc);
		}
		fp->f_flag &= ~FEXLOCK;
	}
}


/*
 * Revoke access the current tty by all processes.
 * Used only by the super-user in init
 * to give ``clean'' terminals at login.
 */
vhangup()
{

	if (!suser())
		return;
	if (u.u_ttyp == NULL)
		return;
	forceclose(u.u_ttyd);
	if ((u.u_ttyp->t_state) & TS_ISOPEN)
		gsignal(u.u_ttyp->t_pgrp, SIGHUP);
}

forceclose(dev)
	dev_t dev;
{
	register struct file *fp;
	register struct gnode *gp;
	register caddr_t value;
	struct 	fileops saveops;

	for (fp = file; fp < fileNFILE; fp++) {
		if (fp->f_count == 0)
			continue;
		if (fp->f_type != DTYPE_INODE)
			continue;
		gp = (struct gnode *)fp->f_data;
		if (gp == 0)
			continue;
		if ((gp->g_mode & GFMT) != GFCHR)
			continue;
		if (gp->g_rdev != dev)
			continue;
		

		if (gp->g_flag & GINUSE) {
			(*fp->f_ops->fo_ioctl)(fp, FIOCINUSE, value, fp->f_cred);
			wakeup((caddr_t)&gp->g_flag);
			gp->g_flag &= ~(GINUSE);
		}

		fp->f_flag &= ~(FREAD|FWRITE);
		saveops.fo_close = fp->f_ops->fo_close;
		fp->f_ops = &divorceops;
		(*saveops.fo_close)(fp);
		fp->f_data = (caddr_t) 0;
	}
}

int vdebug = 0;

divorce_rw(fp, rw, uio)
	register struct file *fp;
	register enum uio_rw rw;
	register struct uio *uio;
{
	if (vdebug)
		mprintf("divorce_rw ,pid=%d\n", u.u_procp->p_pid);
	return(EBADF);
}


divorce_ioctl(fp, com, data)
	register struct file *fp;
	register int com;
	register caddr_t data;
{
	if (vdebug)
		mprintf("divorce_ioctl ,pid=%d\n", u.u_procp->p_pid);
	return(EBADF);
}


divorce_select(fp, which)
	register struct file *fp;
	register int which;
{
	if (vdebug)
		mprintf("divorce_select ,pid=%d\n", u.u_procp->p_pid);
	return(EBADF);
}

divorce_close(fp)
	register struct file *fp;
{
	if (vdebug)
		mprintf("divorce_close ,pid=%d\n", u.u_procp->p_pid);
	return(0);  /* why not!? */
}

gfs_update(gp, t1, t2, wait, cred)
	register struct gnode *gp;
	register struct timeval *t1, *t2;
	register int wait;
	register struct ucred *cred;
{
	register int ret;
	
	ret = GUPDATE(gp, t1, t2, wait, cred);
}


struct gnode *
getegnode(hash, dev, gno)
	register int	hash;
	dev_t	dev;
	register gno_t	gno;
{
	register struct gnode *gp;
	register struct gnode *gq;
	register struct gnode *xp;
	
	if((gp = gfreeh) == NULL) {
		u.u_error = ENFILE;
		tablefull("gnode");
		return(NULL);
	}
	if (gp->g_count) {
		cprintf("getegnode: gp 0x%x (%d)\n", gp, gp->g_number);
		panic("getegnode: free gnode isn't");
	}
	if((gp->g_mp) && (gp->g_mp->m_dev != NODEV))
		(void)GFREEGN(gp);
	if (gq = gp->g_freef)
		gq->g_freeb = &gfreeh;
		
	/*
	 * Now to take gnode off the hash chain it was on
	 * (initially, or after an iflush, it is on a "hash chain"
	 * consisting entirely of itself, and pointed to by no-one,
	 * but that doesn't matter), and put it on the chain for
	 * its new (gno, dev) pair
	 */
		
	gfreeh = gq;
	gp->g_freef = NULL;
	gp->g_freeb = NULL;
	gp->g_textp = NULL;
	gp->g_rdev = 0;
	gp->g_blocks = 0;
	remque(gp);
#ifdef GFSDEBUG
	if(GFS[5])
		cprintf("ggrab: gp 0x%x (%d), hash %d &ghead 0x%x\n", gp,
		gp->g_number, hash, &ghead[hash]);
#endif
	insque(gp, &ghead[hash]);
	gp->g_dev = dev;
	gp->g_number = gno;
	gp->g_id = ++nextgnodeid;
	bzero((caddr_t)gp->g_in.pad, sizeof(gp->g_in));
	
	if(nextgnodeid < 0)
		for(nextgnodeid = 0, xp = gnode; xp < gnodeNGNODE; xp++)
			xp->g_id = 0;
			
	return(gp);
}


freegnode(gp)
	register struct gnode *gp;
{
	if(gp->g_count != 0) {
		cprintf("freegnode: gp 0x%x (%d)\n", gp, gp->g_number);
		panic("freegnode: freeing active gnode");
	}
#ifdef GFSDEBUG
	{
		register struct gnode *ngp;
		
		ngp = gfreeh;
		do {
			if(ngp == gp) {
				cprintf("nfs_inactive: gp 0x%x (%d)\n", gp,
					gp->g_number);
				panic("freeing gnode already on free list");
			}
			ngp = ngp->g_freef;
		} while (ngp);
	}
#endif
	
	if (gfreeh) {
		*gfreet = gp;
		gp->g_freeb = gfreet;
	} else {
		gfreeh = gp;
		gp->g_freeb = &gfreeh;
		wakeup((caddr_t)&gfreeh);
	}
	gp->g_freef = NULL;
	gfreet = &gp->g_freef;
}


/*
 * Remove a gnode from the gnode cache (make the gnode active )
 */

void
gremque(gp)
	register struct gnode *gp;
{
	register struct gnode *gq;

	gq = gp->g_freef;
	*(gp->g_freeb) = gq;
	if(gq)
		gq->g_freeb = gp->g_freeb;
	else
		gfreet = gp->g_freeb;
	gp->g_freef = NULL;
	gp->g_freeb = NULL;
}
