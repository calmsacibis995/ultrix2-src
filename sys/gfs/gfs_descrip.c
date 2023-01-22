#ifndef lint
static char *sccsid = "@(#)gfs_descrip.c	1.7	ULTRIX	10/3/86";
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

/************************************************************************
 *
 *			Modification History
 *
 * 	Paul Shaughnessy, 10/02/86
 * 005- Added code to clear the close_on_exec bit when the F_DUPFD
 *	command is specified with the fcntl system call.
 *
 *	Paul Shaughnessy, 12/23/85
 * 004- Added commands to fcntl system calls to turn on/off
 * 	the syncronous write option to a file.
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 *	Stephen Reilly, 9/09/85
 * 002-	Modified to handle the new lockf code.				
 *
 *	Stephen Reilly,	2/19/85
 * 001- Process group or process ID we not stored correctly into the
 *	socket structure.
 *
 * 	Larry Cohen, 4/4/85
 * 002- Changes for block in use capability.
 *	Also changed calls to getf to GETF macro
 *
 * 15 Mar 85 -- funding
 *	Added named pipe support (re. System V named pipes)
 *
 * 	Larry Cohen, 4/13/85
 *	call ioctl FIOCINUSE when closing inuse desriptor
 *
 * 05-May-85 - Larry Cohen
 *	keep track of the highest number file descriptor opened
 *
 * I moved a line around as part of the inode count going negative
 * Rich
 *
 *	kern_descrip.c	6.2	83/09/25
 *
 ***********************************************************************/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/mount.h"
#include "../h/stat.h"
#include "../h/ioctl.h"
#include "../h/flock.h"

/*
 * Descriptor management.
 */

/*
 * TODO:
 *	increase NOFILE
 *	eliminate u.u_error side effects
 */

/*
 * System calls on descriptors.
 */
getdtablesize()
{

	u.u_r.r_val1 = NOFILE;
}

getdopt()
{

}

setdopt()
{

}

dup()
{
	register struct a {
		int	i;
	} *uap = (struct a *) u.u_ap;
	register struct file *fp;
	register int j;

	if (uap->i &~ 077) { uap->i &= 077; dup2(); return; }	/* XXX */

	GETF(fp, uap->i);
	j = ufalloc(0);
	if (j < 0)
		return;
	dupit(j, fp, u.u_pofile[uap->i]);
}

dup2()
{
	register struct a {
		int	i, j;
	} *uap = (struct a *) u.u_ap;
	register struct file *fp;

	GETF(fp, uap->i);
	if (uap->j < 0 || uap->j >= NOFILE) {
		u.u_error = EBADF;
		return;
	}
	u.u_r.r_val1 = uap->j;
	if (uap->i == uap->j)
		return;
	if (u.u_ofile[uap->j]) {
		if (u.u_pofile[uap->j] & UF_MAPPED)
			munmapfd(uap->j);
		closef(u.u_ofile[uap->j]);
		if (u.u_error)
			return;
	}
	if (uap->j > u.u_omax)  /* track largest file pointer number */
		u.u_omax = uap->j;
	dupit(uap->j, fp, u.u_pofile[uap->i]);
}

dupit(fd, fp, flags)
	register int fd;
	register struct file *fp;
	register int flags;
{

	u.u_ofile[fd] = fp;
#ifdef notdef
	flags &= ~(UF_INUSE); /* remove INUSE reference to inode   *002*/ 
#endif
	u.u_pofile[fd] = flags;
	fp->f_count++;
}

/*
 * The file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register i;
	register char *pop;
	struct flock bf;
	register struct gnode *gp;		/* 004 */

	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fdes);
	pop = &u.u_pofile[uap->fdes];

	/* 004 - Fill gnode structure */

	gp = (struct gnode *)fp->f_data;

	switch(uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i < 0 || i >= NOFILE) {
			u.u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		dupit(i, fp, *pop &~ 1); /* 005 - Set close_on_exec flag to 0 */
		break;

	case F_GETFD:
		u.u_r.r_val1 = *pop & 1;
		break;

	case F_SETFD:
		*pop = (*pop &~ 1) | (uap->arg & 1);
		break;

	case F_GETFL:
		u.u_r.r_val1 = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		fp->f_flag &= FCNTLCANT;
		fp->f_flag |= (uap->arg-FOPEN) &~ FCNTLCANT;
		u.u_error = fset(fp, FNDELAY, fp->f_flag & FNDELAY);
		if (u.u_error)
			break;
		u.u_error = fset(fp, FASYNC, fp->f_flag & FASYNC);
		if (u.u_error)
			(void) fset(fp, FNDELAY, 0);
		break;

	case F_GETOWN:
		u.u_error = fgetown(fp, &u.u_r.r_val1);
		break;

	case F_SETOWN:
		u.u_error = fsetown(fp, uap->arg);
		break;

	case F_GETLK:
		/* get record lock */
		if (copyin(uap->arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=getflck(fp, &bf)) != 0)
			u.u_error = i;
		else if (copyout((caddr_t)&bf, uap->arg, sizeof bf))
			u.u_error = EFAULT;
		break;

	case F_SETLK:
		/* set record lock and return if blocked */
		if (copyin(uap->arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=setflck(fp, &bf, 0)) != 0)
			u.u_error = i;
		break;

	case F_SETLKW:
		/* set record lock and wait if blocked */
		if (copyin(uap->arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=setflck(fp, &bf, 1)) != 0)
			u.u_error = i;
		break;

	case F_SETSYN:	/* 004 */
		/* 004 - set syncronous write if regular file */
		if ((gp->g_mode&GFMT) != GFREG)
			if ((gp->g_mode&GFMT) == GFSOCK) {

				/*
				 * 004 - If a socket was given, return
				 * appropriate error.
				 */
				u.u_error = EOPNOTSUPP;
				return;
			}
			else {
			
				/*	
				 * 004 - Not a regular file or socket. Return
				 * appropriate error.
				 */

				u.u_error = EINVAL;
				return;
			}

		/*
		 * 004 - Now its a regular file, now check fp flag to
		 * verify that file is open for writing. If not, return
		 * an error.
		 */

		if ((fp->f_flag&FWRITE) == 0) {
			u.u_error = EINVAL;
			return;
		}

		/*
		 * 004 - File was validly open for syncronous write !
		 * Now set file pointer flag.
		 */

		fp->f_flag |= FSYNCRON;
		break;

	case F_CLRSYN:
		/* 004 - Clear syncronous write flag */
		/* First verify it is a regular file. */
		if ((gp->g_mode&GFMT) != GFREG)
			if ((gp->g_mode&GFMT) == GFSOCK) {

				/*
				 * 004 - If a socket was given, return
				 * appropriate error.
				 */
				u.u_error = EOPNOTSUPP;
				return;
			}
			else {
			
				/*	
				 * 004 - Not a regular file or socket. Return
				 * appropriate error.
				 */

				u.u_error = EINVAL;
				return;
			}

		/* Regular file ! */

		fp->f_flag &= ~FSYNCRON;
		break;

	default:
		u.u_error = EINVAL;
	}
}

fset(fp, bit, value)
	register struct file *fp;
	register int bit;
	int  value;
{

	if (value)
		fp->f_flag |= bit;
	else
		fp->f_flag &= ~bit;
	return (fioctl(fp, (int)(bit == FNDELAY ? FIONBIO : FIOASYNC),
	    (caddr_t)&value));
}

fgetown(fp, valuep)
	register struct file *fp;
	register int *valuep;
{
	register int error;

	switch (fp->f_type) {

	case DTYPE_SOCKET:
		*valuep = -((struct socket *)fp->f_data)->so_pgrp; /*001*/
		return (0);

	case DTYPE_PORT:
		if(fp->f_flag & FREAD)
			*valuep = -(((struct gnode *)(fp->f_data))
					-> g_rso) -> so_pgrp;
		else
			*valuep = -(((struct gnode *)(fp->f_data))
					-> g_wso) -> so_pgrp;
		return(0);

	default:
		error = fioctl(fp, (int)TIOCGPGRP, (caddr_t)valuep);
		*valuep = -*valuep;
		return (error);
	}
}

fsetown(fp, value)
	register struct file *fp;
	int value;
{

	if (fp->f_type == DTYPE_SOCKET) {
		((struct socket *)fp->f_data)->so_pgrp = -value; /*001*/
		return (0);
	}
	if(fp->f_type == DTYPE_PORT){
		if(fp->f_flag & FREAD)
			(((struct gnode *)(fp->f_data))->g_rso)
						-> so_pgrp = -value;
		else
			(((struct gnode *)(fp->f_data))->g_wso)
						-> so_pgrp = -value;
		return(0);
	}
	if (value > 0) {
		struct proc *p = pfind(value);
		if (p == 0)
			return (EINVAL);
		value = p->p_pgrp;
	} else
		value = -value;
	return (fioctl(fp, (int)TIOCSPGRP, (caddr_t)&value));
}

fioctl(fp, cmd, value)
	register struct file *fp;
	register int cmd;
	register caddr_t value;
{

	return ((*fp->f_ops->fo_ioctl)(fp, cmd, value, u.u_cred));
}

close()
{
	register struct a {
		int	i;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register u_char *pf;
	register struct gnode *gp;
	register int j;
	register caddr_t value;

	GETF(fp, uap->i);
	pf = (u_char *)&u.u_pofile[uap->i];
	gp = (struct gnode *)fp->f_data;
	if ((*pf & UF_INUSE) && gp) {
		*pf &= ~UF_INUSE;
		/* clear inuse only when last inuse open file */
		for (j=0; j <= u.u_omax; j++)
			if ((u.u_pofile[j] & UF_INUSE) &&
			    u.u_ofile[j] == fp)
				goto stillinuse;
		(*fp->f_ops->fo_ioctl)(fp, FIOCINUSE, value, u.u_cred);
		wakeup((caddr_t)&gp->g_flag);
		gp->g_flag &= ~(GINUSE);
	}
stillinuse:
	closef(fp);
	if (*pf & UF_MAPPED)
		munmapfd(uap->i);
	
	u.u_ofile[uap->i] = NULL;
	/* WHAT IF u.u_error ? */
	*pf = 0;
}

fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct	stat *sb;
	} *uap;
	struct stat ub;

	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fdes);
	switch (fp->f_type) {

	case DTYPE_PORT:
	case DTYPE_INODE:
		if (fp->f_data)
			u.u_error = gno_stat((struct gnode *)fp->f_data, &ub);
		else
			u.u_error = EBADF;
		break;

	case DTYPE_SOCKET:
		u.u_error = soo_stat((struct socket *)fp->f_data, &ub);
		break;

	default:
		panic("fstat");
		/*NOTREACHED*/
	}
	if (u.u_error == 0)
		u.u_error = copyout((caddr_t)&ub,(caddr_t)uap->sb,sizeof (ub));
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(i)
	register int i;
{

	for (; i < NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_r.r_val1 = i;
			u.u_pofile[i] = 0;
			if (i > u.u_omax)
				u.u_omax = i;
			return (i);
		}
	u.u_error = EMFILE;
	return (-1);
}

ufavail()
{
	register int i, avail = 0;

	for (i = 0; i < NOFILE; i++)
		if (u.u_ofile[i] == NULL)
			avail++;
	return (avail);
}

struct	file *lastf;
/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 */
struct file *
falloc()
{
	register struct file *fp;
	register i;

	i = ufalloc(0);
	if (i < 0)
		return (NULL);
	if (lastf == 0)
		lastf = file;
	for (fp = lastf; fp < fileNFILE; fp++)
		if (fp->f_count == 0)
			goto slot;
	for (fp = file; fp < lastf; fp++)
		if (fp->f_count == 0)
			goto slot;
	tablefull("file");
	u.u_error = ENFILE;
	return (NULL);
slot:
	u.u_ofile[i] = fp;
	fp->f_count = 1;
	fp->f_data = 0;
	fp->f_offset = 0;
	crhold(u.u_cred);
	fp->f_cred = u.u_cred;
	lastf = fp + 1;
	return (fp);
}

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.  Only task is to check range of the descriptor.
 * Critical paths should use the GETF macro.
 */
struct file *
getf(f)
	register int f;
{
	register struct file *fp;

	if ((unsigned)f >= NOFILE || (fp = u.u_ofile[f]) == NULL) {
		u.u_error = EBADF;
		return (NULL);
	}
	return (fp);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 * If last reference not going away, but no more
 * references except in message queues, run a
 * garbage collect.  This would better be done by
 * forcing a gc() to happen sometime soon, rather
 * than running one each time.
 */
closef(fp)
	register struct file *fp;
{
	register int othernbuf = 0;

	if (fp == NULL)
		return;
	cleanlocks(fp);			/* 002  */
	if(fp->f_flag&FNBUF) {
		struct gnode *gp = (struct gnode *)fp->f_data;
		othernbuf = asyncclose(gp->g_rdev, fp->f_flag);
	}
	if (fp->f_count > 1) {
		fp->f_count--;
		if (fp->f_count == fp->f_msgcount)
			unp_gc();
		return;
	}
	if(fp->f_flag&FNBUF) {
		struct gnode *gp = (struct gnode *)fp->f_data;
		fp->f_flag &= ~FNBUF;
		if(!othernbuf)
			gp->g_flag &= ~GSYNC;
	}

	(*fp->f_ops->fo_close)(fp);
	crfree(fp->f_cred);
	fp->f_count = 0;
}

/*
 * Apply an advisory lock on a file descriptor.
 */
flock()
{
	register struct a {
		int	fd;
		int	how;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	GETF(fp, uap->fd);
	if (fp->f_type != DTYPE_INODE && fp->f_type != DTYPE_PORT ) {
		u.u_error = EOPNOTSUPP;
		return;
	}
 	if ((uap->how & (LOCK_UN|LOCK_EX|LOCK_SH)) == 0){
 		u.u_error = EINVAL;			
 		return;
 	}
	if (uap->how & LOCK_UN) {
		gno_unlock(fp, FSHLOCK|FEXLOCK);
		return;
	}
	/* avoid work... */
	if ((fp->f_flag & FEXLOCK) && (uap->how & LOCK_EX) ||
	    (fp->f_flag & FSHLOCK) && (uap->how & LOCK_SH))
		return;
	if (fp->f_data)
		u.u_error = gno_lock(fp, uap->how);
	else
		u.u_error = EBADF;
}
