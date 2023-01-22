#ifndef lint
static	char	*sccsid = "@(#)gfs_syscalls.c	1.24	(ULTRIX)	3/3/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986,87 by			*
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
 * 03 Mar 87 -- chet
 *	Unlocked gnode before doing GSTAT so that locks won't
 *	propagate up file tree when a slow operation takes place.
 *
 * 13 Feb 87 -- prs
 *	Removed check for m_dev to be null in sync. The check prevented
 *	sync on a device with major number 0.
 *
 * 15 January 86 -- Chase
 *	added code to check for failure on truncate in copen()
 *
 * 11 Sept 86 -- koehler
 *	changes for new namei interface
 *
 * 12 Sept 86 -- koehler
 *	symlink errno change
 *
 * 2  Oct 86 -- Larry Cohen
 *	refix shared line bug in copen
 ***********************************************************************/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/file.h"
#include "../h/stat.h"
#include "../h/gnode.h"
#include "../h/buf.h"
#include "../h/proc.h"
#include "../h/quota.h"
#include "../h/uio.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/mount.h"
#include "../h/ioctl.h"
#include "../h/kmalloc.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

link()
{
        register struct gnode *source_gp, *target_pgp;
        register struct a {
                char    *source;
                char    *target;
        } *uap = (struct a *)u.u_ap;
        register struct nameidata *ndp = &u.u_nd;

        ndp->ni_nameiop = LOOKUP | FOLLOW;
	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->source, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

        source_gp = gfs_namei(ndp);   
	km_free(ndp->ni_dirp, MAXPATHLEN);
        if (source_gp == NULL)
                return;
	if (((source_gp->g_mode & GFMT ) == GFDIR) && !suser()) {
		gput(source_gp);
		return;
	}
	
	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		gput(source_gp);
		return;
	}
 	if(u.u_error = copyinstr(uap->target, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		gput(source_gp);
		return;
	}

	/* this stuff is done here to avoid races in the fs */
	
	source_gp->g_nlink++;
	source_gp->g_flag |= (GCHG | GCLINK);
	(void) GUPDATE(source_gp, &time, &time, 1,  u.u_cred);
	gfs_unlock(source_gp);	
        ndp->ni_nameiop = CREATE;

        target_pgp = gfs_namei(ndp);

	if(u.u_error) {
		source_gp->g_nlink--;
		source_gp->g_flag |= (GCHG | GCLINK);
		(void) GUPDATE(source_gp, &time, &time, 1,  u.u_cred);
		GRELE(source_gp);
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
        if (target_pgp != NULL) {
		u.u_error = EEXIST;
		source_gp->g_nlink--;
		source_gp->g_flag |= (GCHG | GCLINK);		
		(void) GUPDATE(source_gp, &time, &time, 1,  u.u_cred);
		km_free(ndp->ni_dirp, MAXPATHLEN);
		gput(target_pgp);
		GRELE(source_gp);
		return;
	}
		
	target_pgp = ndp->ni_pdir;

	if(target_pgp->g_mp != source_gp->g_mp) {
		u.u_error = EXDEV;
		source_gp->g_nlink--;
		source_gp->g_flag |= (GCHG | GCLINK);		
		(void) GUPDATE(source_gp, &time, &time, 1,  u.u_cred);
		km_free(ndp->ni_dirp, MAXPATHLEN);
		gput(target_pgp);
		GRELE(source_gp);
		return;
	}
	/*
	 * LINK should take three arguments, gp of source, gp of parent 
	 * of source, and source component.
	 */
	
	 /* link does the grele */
	if(GLINK(source_gp, ndp) == GNOFUNC) {
		u.u_error = EOPNOTSUPP;
		gput(target_pgp);
		GRELE(source_gp);
	}
	km_free(ndp->ni_dirp, MAXPATHLEN);	
}

unlink()
{
	register struct a {
		char	*fname;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp, *dp;
	register struct nameidata *ndp = &u.u_nd;
	register int ret;
	
	ndp->ni_nameiop = DELETE | LOCKPARENT;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	gp = gfs_namei(ndp);

	if (gp == NULL) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	dp = (struct gnode *)ndp->ni_pdir;
	/* only root can unlink a directory */

	if ((gp->g_mode&GFMT) == GFDIR && !suser())
		goto out;
	/*
	 * Don't unlink a mounted file.
	 */
	if (gp->g_dev != dp->g_dev) {
		u.u_error = EBUSY;
		goto out;
	}
	if (gp->g_flag&GTEXT)
		xrele(gp);	/* try once to free text */

	if(GUNLINK(gp, ndp) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
out:
#ifdef GFSDEBUG
	if(GFS[10] && !u.u_error)
		cprintf("unlink: gp 0x%x (%d), nlink %d\n", gp, gp->g_number,
		gp->g_nlink);
#endif
	if (dp == gp) {
		ret = GRELE(gp);
		gput(gp);
	} else {
		gput(dp);
		gput(gp);
	}
	km_free(ndp->ni_dirp, MAXPATHLEN);
}


/*
 * Mkdir system call
 */
mkdir()
{
	register struct a {
		char	*name;
		int	dmode;
	} *uap = (struct a *) u.u_ap;
	register struct gnode *gp;
	register struct nameidata *ndp = &u.u_nd;

	uap = (struct a *)u.u_ap;
	ndp->ni_nameiop = CREATE;
	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->name, ndp->ni_dirp, MAXPATHLEN, (u_int *)
	0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	gp = gfs_namei(ndp);
	if (u.u_error) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	if (gp != NULL) {
		u.u_error = EEXIST;
		gput(gp);
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

#ifdef GFSDEBUG
	if(GFS[13])
		cprintf("mkdir: pdir 0x%x\n", ndp->ni_pdir);
#endif
	if((gp = GMKDIR(ndp->ni_pdir, ndp->ni_dirp, uap->dmode)) == 
	(struct gnode *) GNOFUNC)
		u.u_error = EOPNOTSUPP;
	else
		if(!u.u_error)
			GRELE(gp);
	km_free(ndp->ni_dirp, MAXPATHLEN);
}

/*
 * Rmdir system call.
 */
rmdir()
{
	register struct a {
		char	*name;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	register struct nameidata *ndp = &u.u_nd;

	ndp->ni_nameiop = DELETE | LOCKPARENT;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->name, ndp->ni_dirp, MAXPATHLEN, (u_int *)
	0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	gp = gfs_namei(ndp);
	if (gp == NULL) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	if(GRMDIR(gp, ndp) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
	km_free(ndp->ni_dirp, MAXPATHLEN);
}


/*
 * mode mask for creation of files
 */
umask()
{
	register struct a {
		int	mask;
	} *uap = (struct a *)u.u_ap;

	u.u_r.r_val1 = u.u_cmask;
	u.u_cmask = uap->mask & 07777;
}

struct gnode *
maknode(mode, ndp)
	register int mode;
	register struct nameidata *ndp;
{
	if(GMAKNODE(mode, ndp) == (struct gnode *) GNOFUNC)
		u.u_error = EOPNOTSUPP;
}

/*
 * Rename system call.
 * 	rename("foo", "bar");
 * is essentially
 *	unlink("bar");
 *	link("foo", "bar");
 *	unlink("foo");
 * but ``atomically''.  Can't do full commit without saving state in the
 * inode on disk which isn't feasible at this time.  Best we can do is
 * always guarantee the target exists.
 *
 * Basic algorithm is:
 *
 * 1) Bump link count on source while we're linking it to the
 *    target.  This also insure the gnode won't be deleted out
 *    from underneath us while we work (it may be truncated by
 *    a concurrent `trunc' or `open' for creation).
 * 2) Link source to destination.  If destination already exists,
 *    delete it first.
 * 3) Unlink source reference to gnode if still around.
 *    If a directory was moved and the parent of the destination
 *    is different from the source, patch the ".." entry in the
 *    directory.
 *
 * Source and destination must either both be directories, or both
 * not be directories.  If target is a directory, it must be empty.
 */
rename()
{
	register struct a {
		char	*from;
		char	*to;
	} *uap;
	register struct gnode *source_gp;
	register struct nameidata *source_ndp = &u.u_nd;
	struct nameidata target_ndp;
	
	/* get us a pointer to the from name */
	
	uap = (struct a *)u.u_ap;

#ifdef GFSDEBUG
	if(GFS[16])
		cprintf("rename: lookup source\n");
#endif
	source_ndp->ni_nameiop = DELETE | LOCKPARENT;

	if((source_ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->from, source_ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(source_ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	if((target_ndp.ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		km_free(source_ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
 	if(u.u_error = copyinstr(uap->to, target_ndp.ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(target_ndp.ni_dirp, MAXPATHLEN);		
		km_free(source_ndp->ni_dirp, MAXPATHLEN);		
		return;
	}
	
	source_gp = gfs_namei(source_ndp);
	
	if (source_gp == NULL || u.u_error) {
		km_free(target_ndp.ni_dirp, MAXPATHLEN);
		km_free(source_ndp->ni_dirp, MAXPATHLEN);
#ifdef GFSDEBUG
		if(GFS[16])
			cprintf("rename: error is %d\n", u.u_error);
#endif
		return;
	}
	
	target_ndp.ni_nameiop = CREATE | LOCKPARENT | NOCACHE;
	
	if(GRENAMEG(source_gp, u.u_cdir, source_ndp, u.u_cdir, &target_ndp, 0)
	== GNOFUNC) {
		u.u_error = EOPNOTSUPP;
		gput(source_gp);
		gput(source_ndp->ni_pdir);
	}
	km_free(target_ndp.ni_dirp, MAXPATHLEN);
	km_free(source_ndp->ni_dirp, MAXPATHLEN);
}

extern	struct fileops gnodeops;
extern	int	soo_rw(), soo_ioctl(), soo_select(), gno_close();
struct	fileops	portops =
    { soo_rw, soo_ioctl, soo_select, gno_close };
struct	file *getgnode();

/*
 * Change current working directory (``.'').
 */
chdir()
{

	chdirec(&u.u_cdir);
}

/*
 * Change notion of root (``/'') directory.
 */
chroot()
{

	if (suser())
		chdirec(&u.u_rdir);
}

/*
 * Common routine for chroot and chdir.
 */
chdirec(gpp)
	register struct gnode **gpp;
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
	} *uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;
	register int ret;
	
	ndp->ni_nameiop = LOOKUP | FOLLOW;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	gp = gfs_namei(ndp);
	km_free(ndp->ni_dirp, MAXPATHLEN);
	
	if (gp == NULL || u.u_error) 
		return;
	if ((gp->g_mode&GFMT) != GFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if (access(gp, GEXEC))
		goto bad;
	gfs_unlock(gp);
	if (*gpp)
		ret = GRELE(*gpp);
	*gpp = gp;
	return;

bad:
	gput(gp);
}

/*
 * Open system call.
 */
open()
{
	register struct a {
		char	*fname;
		int	mode;
		int	crtmode;
	} *uap = (struct a *) u.u_ap;

	copen(uap->mode-FOPEN, uap->crtmode, uap->fname);
}

/*
 * Creat system call.
 */
creat()
{
	register struct a {
		char	*fname;
		int	fmode;
	} *uap = (struct a *)u.u_ap;

	copen(FWRITE|FCREAT|FTRUNC, uap->fmode, uap->fname);
}

/*
 * Common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
copen(mode, arg, fname)
	register int mode;
	int arg;
	caddr_t fname;
{
	register struct gnode *gp;
	register struct file *fp;
	register struct nameidata *ndp = &u.u_nd;
	register int i;
	caddr_t value;
	int ret;
	
#ifdef notdef
	if ((mode&(FREAD|FWRITE)) == 0) {
		u.u_error = EINVAL;
		return;
	}
#endif

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(fname, ndp->ni_dirp, MAXPATHLEN, (u_int *)
	0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	if (mode&FCREAT) {
		if (mode & FEXCL)
			ndp->ni_nameiop = CREATE;
		else
			ndp->ni_nameiop = CREATE | FOLLOW;
		gp = gfs_namei(ndp);
		if (gp == NULL) {
			if (u.u_error) {
				km_free(ndp->ni_dirp, MAXPATHLEN);
				return;
			}
			if((gp = GMAKNODE((arg & 07777) | GFREG, ndp))
			== (struct gnode *) GNOFUNC) {
				km_free(ndp->ni_dirp, MAXPATHLEN);
				u.u_error = EOPNOTSUPP;
				return;
			}
			km_free(ndp->ni_dirp, MAXPATHLEN);
			if (gp == NULL) {
				return;
			}
			mode &= ~FTRUNC;
		} else {
			km_free(ndp->ni_dirp, MAXPATHLEN);

			if (mode&FEXCL) {
				u.u_error = EEXIST;
				gput(gp);
				return;
			}
			mode &= ~FCREAT;
		}
	} else {
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		gp = gfs_namei(ndp);

		km_free(ndp->ni_dirp, MAXPATHLEN);
		
		if (gp == NULL) {
			return;
		}
		
	}
	if ((gp->g_mode & GFMT) == GFSOCK) {
		u.u_error = EOPNOTSUPP;
		goto bad;
	}
	if ((mode&FCREAT) == 0) {
		if (mode&FREAD)
			if (access(gp, GREAD))
				goto bad;
		if (mode&(FWRITE|FTRUNC)) {	/* To close a nasty hole */
			if (access(gp, GWRITE))	/* Rich */
				goto bad;
			if ((gp->g_mode&GFMT) == GFDIR) {
				u.u_error = EISDIR;
				goto bad;
			}
		}
	}
waitinuse:
	while ((mode & FBLKINUSE) && (gp->g_flag & GINUSE))  { /*002*/
		if (mode & FNDELAY) {
			u.u_error = EWOULDBLOCK;
			goto bad;
		}

		gfs_unlock(gp);
		sleep((caddr_t)&gp->g_flag, PLOCK);
		gfs_lock(gp);
		
	}
	if((gp->g_mode & GFMT) == GFPORT){
		int created = 0;

			/* if the port is being opened for	*/
			/* reading (writing) and the read	*/
			/* (write) socket does not exist then	*/
			/* create it.				*/
		if((mode & FREAD)  &&  (gp->g_rso == 0)){
			u.u_error = socreate(AF_UNIX, &gp->g_rso,
						SOCK_STREAM, 0);
			if(u.u_error){
				gp->g_rso = 0;
				goto bad;
			}
			created++;
		}
		if((mode & FWRITE)  &&  (gp->g_wso == 0)){
			u.u_error = socreate(AF_UNIX, &gp->g_wso,
						SOCK_STREAM, 0);
			if(u.u_error){
				if(created){
					gp->g_rso->so_state |= 
							SS_NOFDREF;
					sofree(gp->g_rso);
					gp->g_rso = 0;
				}
				gp->g_wso = 0;
				goto bad;
			}
			created++;
		}
			/* if one or both of the sockets on	*/
			/* this port were just created so that	*/
			/* both sockets now exist then connect	*/
			/* them to create a socketpair.		*/
		if(created  &&  gp->g_rso != 0  &&  gp->g_wso != 0){
			if(piconnect(gp->g_wso, gp->g_rso) == 0){
				gp->g_wso->so_state |= SS_NOFDREF;
				sofree(gp->g_wso);
				gp->g_wso = 0;
				gp->g_rso->so_state |= SS_NOFDREF;
				sofree(gp->g_rso);
				gp->g_rso = 0;
				goto bad;
			}
		}

		mode &= ~FTRUNC;
	}

	if (mode&FTRUNC) {
		if(GTRUNC(gp, (u_long)0, u.u_cred) == GNOFUNC)
			u.u_error = EOPNOTSUPP;
		if (u.u_error)
			goto bad;
	}

	fp = falloc();
	if (fp == NULL)
		goto bad;
		
	gfs_unlock(gp);
	fp->f_flag = mode&FMASK;
	if((gp->g_mode & GFMT) == GFPORT){
		fp->f_type = DTYPE_PORT;
		fp->f_ops = &portops;
	}
	else {
		fp->f_type = DTYPE_INODE;
		fp->f_ops = &gnodeops;
	}
	fp->f_data = (caddr_t)gp;
#ifdef GFS
	if(GFS[0])
		cprintf("copen: fp 0x%x count %d gp 0x%x (gp) flags 0%o count %d\n",
		fp, fp->f_count, gp, gp->g_flag, gp->g_count);
#endif
	i = u.u_r.r_val1;
	if (setjmp(&u.u_qsave)) {
		if (u.u_error == 0)
			u.u_error = EINTR;
		u.u_ofile[i] = NULL;
		if(fp)
			closef(fp);
		return;
	}

	u.u_error = GOPEN(gp, mode);
#ifdef GFS
	if(GFS[0])
		cprintf ("copen: u.u_error %d\n", u.u_error);
#endif
	if (u.u_error == 0) {
		if ((mode&FBLKANDSET)==FBLKANDSET) { /*002*/
			gp->g_flag |= GINUSE;
			u.u_pofile[i] |= UF_INUSE;
			(*fp->f_ops->fo_ioctl)(fp, FIOSINUSE, value, u.u_cred);
		}
		return;
	}
	u.u_ofile[i] = NULL;
	fp->f_count--;
	fp = NULL;
	if (u.u_error==EALREADY && (gp->g_flag&GINUSE)) { 
				   /*  gnode was grabbed while we were */
		u.u_error = 0;	   /*  blocked.  wait to free up again.*/
		gfs_lock(gp);		/* need to lock again */
		goto waitinuse;	    
	}
	ret = GRELE(gp);
	return;
bad:
	gput(gp);
}


/*
 * Mknod system call
 */
mknod()
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;
	register struct nameidata *ndp = &u.u_nd;
	register int ret;
	
	uap = (struct a *)u.u_ap;
	if (!suser() && ((uap->fmode & GFMT) != GFPORT))
		return;
			/* if a non-privileged user is making	*/
			/* a port node then negate the error	*/
			/* posted by SUSER.			*/
	u.u_error = 0;
	ndp->ni_nameiop = CREATE;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	gp = gfs_namei(ndp);
	if (gp != NULL) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		u.u_error = EEXIST;
		goto out;
	}
	if (u.u_error) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	if((gp = GMAKNODE(uap->fmode, ndp)) == (struct gnode *) GNOFUNC) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		u.u_error = EOPNOTSUPP;
		return;
	}
	km_free(ndp->ni_dirp, MAXPATHLEN);
	if (gp == NULL) {
		return;
	}
	
	switch (gp->g_mode & GFMT) {

	case GFMT:	/* used by badsect to flag bad sectors */
	case GFCHR:
	case GFBLK:
		/*
		 * Want to be able to use this to make badblock
		 * gnodes, so don't truncate the dev number.
		 */
		gp->g_rdev = uap->dev;
		gp->g_flag |= GACC|GUPD|GCHG;
		if(!ISLOCAL(gp->g_mp)) {
			cprintf("maknode: trying to make a remote spec 0x%x\n",
			gp);
		}
		ret = GUPDATE(gp, &time, &time, 0, u.u_cred);
	}

out:
	gput(gp);
}


/*
 * Synch an open file.
 */
fsync()
{
	register struct a {
		int	fd;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	register struct file *fp;
	
	fp = getgnode(uap->fd);
	if (fp == NULL)
		return;
	if ((gp = (struct gnode *)fp->f_data) == 0) {
		u.u_error = EBADF;
		return;
	}

	gfs_lock(gp);
	if(GSYNCG(gp, fp->f_cred) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
	gfs_unlock(gp);
}



/*
 * Change mode of a file given path name.
 */
chmod()
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap = (struct a *)u.u_ap;

	/* where should owner be? */
	
	if ((gp = owner(uap->fname, FOLLOW)) == NULL)
		return;
	u.u_error = chmod1(gp, uap->fmode, u.u_cred);
	gput(gp);
}

/*
 * Change mode of a file given a file descriptor.
 */
fchmod()
{
	register struct a {
		int	fd;
		int	fmode;
	} *uap;
	register struct gnode *gp;
	register struct file *fp;
	
	uap = (struct a *)u.u_ap;
	fp = getgnode(uap->fd);
	if (fp == NULL)
		return;
	if ((gp = (struct gnode *)fp->f_data) == 0) {
		u.u_error = EBADF;
		return;
	}
	if (u.u_uid != gp->g_uid && !suser())
		return;

	gfs_lock(gp);
	u.u_error = chmod1(gp, uap->fmode, fp->f_cred);
	gfs_unlock(gp);
}

/*
 * Change the mode on a file.
 * Gnode must be locked before calling.
 */
chmod1(gp, mode, cred)
	register struct gnode *gp;
	register int mode;
	struct	ucred	*cred;
{
        register int sticky = gp->g_mode & GSVTX;
	register int ret;

	if(ISREADONLY(gp->g_mp))
		return(EROFS);
		
	if (u.u_uid) {
		if ((gp->g_mode & GFMT) != GFDIR)
			mode &= ~GSVTX;
		if (!groupmember(gp->g_gid))
			mode &= ~GSGID;
	}

        /* for the new text table sticky changes blow away unref-ed text */
        if((gp->g_flag&GTEXT) && (mode&GSVTX) && sticky == 0)
                xrele(gp);
        gp->g_mode &= ~07777;
	gp->g_mode |= mode&07777;
	gp->g_flag |= (GCHG | GCMODE);
	if(!ISLOCAL(gp->g_mp))
		if(ret = GUPDATE(gp, 0, 0, 0, cred)) {
			(void) GGETVAL(gp);
			return(ret);
		}
	if (gp->g_flag&GTEXT && (gp->g_mode&GSVTX)==0 && sticky)
		xrele(gp);
	return(0);
}

/*
 * Set ownership given a path name.
 */
chown()
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
		int	uid;
		int	gid;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (!suser() || (gp = owner(uap->fname, NOFOLLOW)) == NULL)
		return;
	u.u_error = chown1(gp, uap->uid, uap->gid, u.u_cred);
	gput(gp);
}

/*
 * Set ownership given a file descriptor.
 */
fchown()
{
	register struct a {
		int	fd;
		int	uid;
		int	gid;
	} *uap;
	register struct gnode *gp;
	register struct file *fp;
	
	uap = (struct a *)u.u_ap;
	fp = getgnode(uap->fd);
	if (fp == NULL)
		return;
	if ((gp = (struct gnode *)fp->f_data) == 0) {
		u.u_error = EBADF;
		return;
	}
	if (!suser())
		return;

	gfs_lock(gp);
	
	/*
	 * NFS stupidly sends the credentials of the person opening
	 * the file rather than the uid of process calling this 
	 * routine
	 */
	u.u_error = chown1(gp, uap->uid, uap->gid, fp->f_cred);
	gfs_unlock(gp);
}

/*
 * Perform chown operation on gnode gp;
 * gnode must be locked prior to call.
 */
chown1(gp, uid, gid, cred)
	register struct gnode *gp;
	struct	ucred	*cred;
	register int uid, gid;
{
	register int ret = 0;
#ifdef QUOTA
	register long change;
#endif

	if(ISREADONLY(gp->g_mp))
		return(EROFS);
	if (uid == -1)
		uid = gp->g_uid;
	if (gid == -1)
		gid = gp->g_gid;
#ifdef QUOTA
	if (gp->g_uid == uid)		/* this just speeds things a little */
		change = 0;
	else
		change = gp->g_blocks;
	(void) chkdq(gp, -change, 1);
	(void) chkiq(gp->g_dev, gp, gp->g_uid, 1);
	dqrele(gp->g_dquot);
#endif
	gp->g_uid = uid;
	gp->g_gid = gid;
	gp->g_flag |= (GCHG | GCID);
		
	if (u.u_ruid != 0)
		gp->g_mode &= ~(GSUID|GSGID);
	
	if(!ISLOCAL(gp->g_mp)) {
		if(ret = GUPDATE(gp, 0, 0, 0, cred)) {
			(void) GGETVAL(gp);
			return(ret);
		}
	}
		
#ifdef QUOTA
	gp->g_dquot = inoquota(gp);
	(void) chkdq(gp, change, 1);
	(void) chkiq(gp->g_dev, (struct gnode *)NULL, uid, 1);
	return (u.u_error);	
#else
	return (ret);
#endif
}

utimes()
{
	register struct a {
		char	*fname;
		struct	timeval *tptr;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	struct timeval tv[2];

	if ((gp = owner(uap->fname, FOLLOW)) == NULL)
		return;
	if(ISREADONLY(gp->g_mp)) {
		u.u_error = EROFS;
		gput(gp);
		return;
	}
	
	u.u_error = copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof (tv));
	if (u.u_error == 0) {
		gp->g_flag |= GACC|GUPD|GCHG;
		if(GUPDATE(gp, &tv[0], &tv[1], 0, u.u_cred) == GNOFUNC)
			u.u_error = EOPNOTSUPP;
	}
	gput(gp);
}

/*
 * Flush any pending I/O.
 */
sync()
{
	register struct mount *mp;
	
	for(mp = mount; mp < &mount[nmount]; mp++) {
		if(mp->m_dev == (dev_t) NODEV)
			continue;
		update(mp);
	}
	gfs_gupdat();
}

/*
 * Truncate a file given its path name.
 */
truncate()
{
	register struct a {
		char	*fname;
		u_long	length;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	register struct nameidata *ndp = &u.u_nd;

	ndp->ni_nameiop = LOOKUP | FOLLOW;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	gp = gfs_namei(ndp);
	km_free(ndp->ni_dirp, MAXPATHLEN);
	
	if (gp == NULL)
		return;
	if (access(gp, GWRITE))
		goto bad;
	if ((gp->g_mode&GFMT) == GFDIR) {
		u.u_error = EISDIR;
		goto bad;
	}
	if(GTRUNC(gp, uap->length, u.u_cred) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
bad:
	gput(gp);
}

/*
 * Truncate a file given a file descriptor.
 */
ftruncate()
{
	register struct a {
		int	fd;
		u_long	length;
	} *uap = (struct a *)u.u_ap;
	register struct gnode *gp;
	register struct file *fp;

	fp = getgnode(uap->fd);
	if (fp == NULL)
		return;
	if ((fp->f_flag&FWRITE) == 0) {
		u.u_error = EINVAL;
		return;
	}

	if ((gp = (struct gnode *)fp->f_data) == 0) {
		u.u_error = EBADF;
		return;
	}
	if(ISREADONLY(gp->g_mp)) {
		u.u_error = EROFS;
		return;
	}
	
	gfs_lock(gp);
#ifdef GFSDEBUG
	if(GFS[2])
		cprintf("ftrunc: gp 0x%x (%d) length %d\n", gp, gp->g_number,
		uap->length);
#endif
	if(GTRUNC(gp, uap->length, fp->f_cred) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
	gfs_unlock(gp);
	return;
}


/*
 * symlink -- make a symbolic link
 */
symlink()
{
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;
	register struct gnode *gp;
	register struct nameidata *ndp = &u.u_nd;
	register char *target_cp;
	register char *source_cp;
	
	uap = (struct a *)u.u_ap;
	
	/* get us the target name */
	
	if((target_cp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}

	if(u.u_error = copyinstr(uap->target, target_cp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(target_cp, MAXPATHLEN);
		return;
	}
	
	/* get us the source name */
	
	if((source_cp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		km_free(target_cp, MAXPATHLEN);
		u.u_error = EIO;
		return;
	}

	if(u.u_error = copyinstr(uap->linkname, source_cp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(target_cp, MAXPATHLEN);		
		km_free(source_cp, MAXPATHLEN);
		return;
	}
	
	/* we may not create a symlink when the source exists */
	
	ndp->ni_nameiop = CREATE;
	ndp->ni_dirp = source_cp;
	gp = gfs_namei(ndp);
	if (gp) {
		gput(gp);
		km_free(source_cp, MAXPATHLEN);
		km_free(target_cp, MAXPATHLEN);
		u.u_error = EEXIST;
		return;
	}
	if (u.u_error) {
		km_free(source_cp, MAXPATHLEN);
		km_free(target_cp, MAXPATHLEN);
		return;
	}

#ifdef GFSDEBUG
	if(GFS[17])
		cprintf("symlink: ndp 0x%x ni_pdir 0x%x\n", ndp,
		ndp->ni_pdir);
#endif
	/* create the special file type for a symlink */
	
	
	if(GSYMLINK(ndp, target_cp) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
	km_free(source_cp, MAXPATHLEN);
	km_free(target_cp, MAXPATHLEN);
}

/*
 * Seek system call
 */
lseek()
{
	register struct file *fp;
	register struct a {
		int	fd;
		off_t	off;
		int	sbase;
	} *uap;
	register struct gnode *gp;
	register long where;
	register int ret = 0;
	
	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fd);
	if (fp->f_type != DTYPE_INODE) {
		u.u_error = ESPIPE;
		return;
	}
	if (fp->f_data == (caddr_t)0) {
		u.u_error = EBADF;
		return;
	}
	
	/* should seeks just arbitrarily be done?, should we check
	 * with the correct sfs to see if the operation can be done?
	 */
	
	gp = (struct gnode *)fp->f_data;

	switch (uap->sbase) {

	case L_INCR:
		where = fp->f_offset + uap->off;
		ret = GSEEK(gp, where);
		break;

	case L_XTND:
		where = uap->off + ((struct gnode *)fp->f_data)->g_size;
		ret = GSEEK(gp, where);
		break;

	case L_SET:
		where = uap->off;
		ret = GSEEK(gp, where);
		break;

	default:
		u.u_error = EINVAL;
		return;
	}
	
	/* 
	 * there needs to be a documentation change here, if
   	 * the seek is not successful (which is not possible for
	 * ufs), fp->offset does not change
	 */

	if(u.u_error) {
		u.u_error = EINVAL;
		where = -1;
	 } else {
		if(ret) {
			u.u_error = EOPNOTSUPP;
			return;
		}
		fp->f_offset = where;
	}
	u.u_r.r_off = where;
}

/*
 * Access system call
 */
saccess()
{
	register svuid, svgid;
	register struct gnode *gp;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;
	register struct nameidata *ndp = &u.u_nd;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ndp->ni_nameiop = LOOKUP | FOLLOW;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	gp = gfs_namei(ndp);
	km_free(ndp->ni_dirp, MAXPATHLEN);
	if (gp != NULL) {
		
		/*
		 * make checks for M_NODEV, M_NOSUID, and M_NOEXEC
		 * flags 
		 */
		
		if ((uap->fmode&R_OK) && access(gp, GREAD))
			goto done;
		if ((uap->fmode&W_OK) && access(gp, GWRITE))
			goto done;

		/*
		 * convoluted as it seems, the following test
		 * needs to be checked:
		 *	if a regular file is being checked for exec
		 *		and the filesystem is NOSUID
		 *		and the file is SUID or SGID
		 *		and we are not the super user
		 *	or if a regular file is being checked for exec
		 *		and the filesystem is NOEXEC
		 *		and we are not the super user
		 * we may not permit access to the file
		 */
		 
		if((gp->g_mp->m_flags & M_NOEXEC) && u.u_uid) {
			u.u_error = EROFS;
			goto done;
		}
		
		if((gp->g_mp->m_flags & M_NOSUID) && (gp->g_mode & 
		(GSUID | GSGID)) && (gp->g_mode & GFREG) && u.u_uid) {
			u.u_error = EROFS;
			goto done;
		}
		if ((uap->fmode&X_OK) && access(gp, GEXEC))
			goto done;
done:
		gput(gp);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}


/*
 * Stat system call.  This version follows links.
 */

stat()
{

	stat1(FOLLOW);
}


/*
 * Lstat system call.  This version does not follow links.
 */

lstat()
{

	stat1(NOFOLLOW);
}


stat1(follow)
	register int follow;
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
		struct stat *ub;
	} *uap;
	struct stat sb;
	register struct nameidata *ndp = &u.u_nd;

	uap = (struct a *)u.u_ap;
	ndp->ni_nameiop = LOOKUP | follow;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
	(u_int *) 0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}


	gp = gfs_namei(ndp);

	km_free(ndp->ni_dirp, MAXPATHLEN);
	if (gp == NULL)
		return;
	gfs_unlock(gp);
	(void) GSTAT(gp, &sb);
	GRELE(gp);
	if (!u.u_error)
	 	u.u_error = copyout((caddr_t)&sb, (caddr_t)uap->ub, sizeof (sb));
}


/*
 * Return target name of a symbolic link
 */

readlink()
{
	register struct gnode *gp;
	register struct a {
		char	*name;
		char	*buf;
		int	count;
	} *uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;
	struct uio _auio;
	register struct uio *auio = &_auio;
	struct iovec _aiov;
	register struct iovec *aiov = &_aiov;
	
	ndp->ni_nameiop = LOOKUP;

	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		u.u_error = EIO;
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

	/* if the sfs doesn't allow links everything is cool */
		
	if ((gp->g_mode&GFMT) != GFLNK) {
		u.u_error = ENXIO;
		goto out;
	}
	auio->uio_iov = aiov;
	auio->uio_iovcnt = 1;
	aiov->iov_base = uap->buf;
	aiov->iov_len = auio->uio_resid = uap->count;
	auio->uio_segflg = auio->uio_offset = 0;
	if(GREADLINK(gp, auio) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
out:
	gput(gp);
	if(u.u_error == NULL)
		u.u_r.r_val1 = uap->count - auio->uio_resid;
}

struct file *
getgnode(fdes)
	register int fdes;
{
	register struct file *fp;

	if ((unsigned)fdes >= NOFILE || (fp = u.u_ofile[fdes]) == NULL) {
		u.u_error = EBADF;
		return ((struct file *)0);
	}
	if (fp->f_type != DTYPE_INODE && fp->f_type != DTYPE_PORT) {
		u.u_error = EINVAL;
		return ((struct file *)0);
	}
	return (fp);
}

getmnt() {
	register struct a {
		u_int	*cookie;
		struct 	fs_data	*buf;
		u_int	nbytes;
	} *uap = (struct a *) u.u_ap;
	register u_int number;
	register struct mount *mp;
	int cookie;	
	register struct fs_data *fs_data;
	register int count;
	
	/* insure we can get all the stuff out. */
	
	if((count = number = uap->nbytes / sizeof(struct fs_data)) < 1) {
#ifdef GFSDEBUG
		if(GFS[14])
			cprintf("getmnt: number %d too small bytes %d\n",
			number, uap->nbytes);
#endif
		u.u_error = EINVAL;
		return;
	}
#ifdef GFSDEBUG
	if(GFS[14])
		cprintf("getmnt: number %d bytes %d\n", number, uap->nbytes);
	
#endif
	if(u.u_error = copyin(uap->cookie, &cookie, sizeof(cookie)))
		return;
		
#ifdef GFSDEBUG
	if(GFS[14])
		cprintf("getmnt: cookie %d NMOUNT %d\n", cookie, NMOUNT);
#endif
	if(cookie < 0 || cookie > NMOUNT) {
		u.u_error = EINVAL;
		return;
	}
	for(mp = &mount[cookie]; mp < &mount[NMOUNT] && number; mp++, cookie++) {
		if(mp->m_bufp && mp->m_flags & M_DONE) {
			number--;
			fs_data = GGETFSDATA(mp);
#ifdef GFSDEBUG
			if(GFS[14])
				cprintf("getmnt: fs_data 0x%x type %d\n",
				fs_data, fs_data->fd_fstype);
#endif
			if (u.u_error = copyout((caddr_t) fs_data,
			(caddr_t) uap->buf, sizeof(struct fs_data))) {
				return;
			}
			uap->buf++;
		}
	}
	if(cookie == NMOUNT)
		cookie = 0;
	u.u_r.r_val1 = count - number;
	
	copyout(&cookie, uap->cookie, sizeof(cookie));
	return;
}


getdirentries()
{
	register struct a {
		int fd;
		struct gen_dir *buf;
		u_int nbytes;
		u_int *cookie;
	} *uap = (struct a *) u.u_ap;
	register struct file *fp;
	register struct gnode *gp;
	struct uio _auio;
	register struct uio *auio = &_auio;
	struct iovec _aiov;
	register struct iovec *aiov = &_aiov;
	register int ret;
	
	if((fp = getgnode(uap->fd)) == NULL) {   /* bad file descriptor */
#ifdef GFSDEBUG
		if(GFS[15])
			cprintf("getdirents: bad fd\n");
#endif
		/* 
		 * getgnode returns EINVAL if not an INODE or PORT
		 * we just interpret that to mean it isn't a directory
		 * either
		 */ 

		if (u.u_error == EINVAL)
			u.u_error = ENOTDIR;
		return;
	}

	if(fp->f_type != DTYPE_INODE) {		/* this must be a gnode */
		u.u_error = EOPNOTSUPP;	
#ifdef GFSDEBUG
		if(GFS[15])
			cprintf("getdirents: not a gnode\n");
#endif
		return;
	}
	
	gp = (struct gnode *) fp->f_data;
	if((gp->g_mode & GFMT) != GFDIR) {	/* this must be a directory */
#ifdef GFSDEBUG
		if(GFS[15])
			cprintf("getdirents: not a dir\n");
#endif
		u.u_error = ENOTDIR;
		return;
	}
	
	if(access(gp, GREAD)) {		/* we must be able to read the dir */
#ifdef GFSDEBUG
		if(GFS[15])
			cprintf("getdirents: access check\n");
#endif
		u.u_error = EPERM;
		return;
	}
	
	/* check for valid buffer */
	if (uap->nbytes < DIRBLKSIZ || (uap->nbytes & (DIRBLKSIZ-1))
		|| !useracc(uap->buf,uap->nbytes,B_WRITE)) {
		u.u_error = EINVAL;
		return;
	}

	aiov->iov_base = (caddr_t) uap->buf;
	aiov->iov_len = uap->nbytes;
	auio->uio_iov = aiov;
	auio->uio_iovcnt = 1;
	auio->uio_segflg = UIO_USERSPACE;
	auio->uio_offset = fp->f_offset;
	auio->uio_resid = uap->nbytes;

	ret = GGETDIRENTS(gp, auio, fp->f_cred);

	if (u.u_error)
		return;
#ifdef GFSDEBUG
	if(GFS[15])
		cprintf("getdirentries: nbytes %d resid %d offset %d\n",
			uap->nbytes, auio->uio_resid,auio->uio_offset);
#endif
	u.u_error = copyout((caddr_t)&fp->f_offset,
				(caddr_t)uap->cookie,sizeof(long));
	u.u_r.r_val1 = uap->nbytes - auio->uio_resid;
	fp->f_offset = auio->uio_offset;	/* for lseek and next read */
}
