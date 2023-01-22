#ifndef lint
static	char	*sccsid = "@(#)gfs_mount.c	1.17	(ULTRIX)	3/18/87";
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
 * 10 Mar 87 -- chet
 *	turn off user-level mounts unless GFSDEBUG is on
 *
 * 13 Feb 87 -- prs
 *	Changed call to bflush in update to only pass the mp parameter
 *
 * 15 Jan 87 -- prs
 *	Added nulling out the fs_data pointer in the mount structure if
 *	the sfs mount call fails
 *
 * 11 Sep 86 -- koehler
 *	changed name interface, allow the new options, fix umount 
 *	problems
 *
 * 16 Oct 86 -- koehler
 *	fixed a problem with mounting hp0a (major/minor 0/0)
 *
 ***********************************************************************/


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/file.h"
#include "../h/kernel.h"
#include "../h/conf.h"
#include "../h/kmalloc.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

#ifdef GFSDEBUG
int turn_on_sync = 1;

int turn_off_usrmnt = 0;
#else
int turn_on_sync = 0;

int turn_off_usrmnt = 1;
#endif

int nmount = 0;

smount()
{
	register struct a {
		char	*fspecial;
		char	*fpathname;
		int	ronly;
		int	fs_type;	/* new gfs entry */
		char	*opts;		/* new gfs entry */
	} *uap;
	register struct gnode	*gp;
	register struct nameidata *ndp = &u.u_nd;
	register struct mount	*mp;
	register struct fs_data *fs_data;
	int			num;
	struct mount		*(*mountsfs)();
	char *pathname;
	char *devname;
	extern struct gnode *rootdir;
	
	uap = (struct a *)u.u_ap;

#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("gfs_mount: special 0x%x path 0x%x ronly %d type %d\n",
		uap->fspecial, uap->fpathname, uap->ronly, uap->fs_type);
#endif
	
	/* get us a gnode so we can later fill in mp->m_gnodp */
	ndp->ni_nameiop = LOOKUP | FOLLOW | NOCACHE;
	if((pathname = ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL)
	{
		u.u_error = EIO;
		return;
	}
 	if(u.u_error = copyinstr(uap->fpathname, pathname, MAXPATHLEN,
		(u_int *) 0)) {
		km_free(pathname, MAXPATHLEN);
		return;
	}
	if(*pathname != '/') {		/* we must have absolute pathnames */
		u.u_error = EINVAL;
		km_free(pathname, MAXPATHLEN);
		return;
	}
	if((devname = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		km_free(pathname, MAXPATHLEN);
		u.u_error = EIO;
		return;
	}
	if(u.u_error = copyinstr(uap->fspecial, devname, MAXPATHLEN,
		(u_int *) 0)) {
		km_free(devname, MAXPATHLEN);
		km_free(pathname, MAXPATHLEN);
		return;
	}
	if((gp = GNAMEI(ndp)) == NULL) {
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
	if(gp->g_count != 1) {		/* someone has this inode open */
		u.u_error = EBUSY;
		nchinval(gp->g_dev);
		gput(gp);
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
	if((gp->g_mode & GFMT) != GFDIR) { /* this mount point is not a dir */
		u.u_error = ENOTDIR;
		nchinval(gp->g_dev);		
		gput(gp);
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
	if(u.u_uid) 
		if(turn_off_usrmnt || (gp->g_uid != u.u_uid)) {
			u.u_error = EPERM;
			nchinval(gp->g_dev);		
			gput(gp);
			km_free(pathname, MAXPATHLEN);
			km_free(devname, MAXPATHLEN);
			return;
		}
	/* I can't believe that I am writing new code and not doing
	 * the mount cacheing now!!!
	 */
	
	for(mp = mount; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp == 0)
			break;
	
	if(mp == &mount[NMOUNT]) {
		u.u_error = EMFILE;
		nchinval(gp->g_dev);
		gput(gp);
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
	
	mp->m_bufp = (struct buf *) NODEV;	/* for reservation */
	mp->m_dev = NODEV;

	num = uap->fs_type;
	if(num < 0 || num >= NUM_FS) {	/* fs_type not in range 0-0xff */
		u.u_error = ENXIO;	/* this is a new error return */
#ifdef GFSDEBUG
		if(GFS[6])
			cprintf("gfs_mount: bad fs type\n");
#endif
		mp->m_bufp = NULL;
		nchinval(gp->g_dev);		
		gput(gp);
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
	mountsfs = MOUNTFS(num);
	if(!mountsfs) {			/* fs_type not configured */
		u.u_error = EOPNOTSUPP;	/* this is another new error */
		mp->m_bufp = NULL;
		nchinval(gp->g_dev);		
		gput(gp);
		km_free(pathname, MAXPATHLEN);
		km_free(devname, MAXPATHLEN);
		return;
	}
#ifdef GFSDEBUG
	if(GFS[6]) {
		cprintf("gfs_mount: calling sfs routine with ");
		cprintf("gp 0x%x (%d) pathname '%s' devname 0x%x ronly %d mp 0x%x\n", gp,
		gp->g_number, pathname, uap->fspecial, uap->ronly, mp);
	}
#endif
	fs_data = mp->m_fs_data = (struct fs_data *)
			km_alloc(sizeof(struct fs_data), KM_CLRSG|KM_SLEEP);
	if(mp == &mount[nmount])
		nmount++;
	fs_data->fd_uid = u.u_uid;		/* who is mounting */
	if(mp != (*mountsfs)(devname, pathname, uap->ronly, mp, uap->opts)) {
		/*  something went wrong in the sfs, it will return
		 *  the error
		 */
		mp->m_bufp = NULL;
		mp->m_dev = NODEV;
		km_free(mp->m_fs_data, sizeof(struct fs_data));
		mp->m_fs_data = NULL;
		if(mp == &mount[nmount - 1])
			nmount--;
		nchinval(gp->g_dev);		
		gput(gp);
		km_free(devname, MAXPATHLEN);
		km_free(pathname, MAXPATHLEN);
		return;
	}
	bcopy(pathname, fs_data->fd_path, MAXPATHLEN);
	bcopy(devname, fs_data->fd_devname, MAXPATHLEN);
	km_free(pathname, MAXPATHLEN);
	km_free(devname, MAXPATHLEN);
	fs_data->fd_dev = mp->m_dev;
	mp->m_gnodp = gp;
	
	if (u.u_uid)
		mp->m_flags |= M_USRMNT;	/* nosuid & nodev */

	gp->g_mpp = mp;
	mp->m_fstype = num;
#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("gfs_mount: mp 0x%x gp 0x%x (%d)\n", mp, gp,
		gp->g_number);
#endif
	gp->g_flag |= GMOUNT;
	gfs_unlock(gp);
	mp->m_flags |= M_DONE;
}

umount()
{
	register struct a {
		dev_t	fdev;
	} *uap = (struct a *) u.u_ap;
	dev_t	 		dev = uap->fdev;	
	register struct mount	*mp;

	GETMP(mp, dev);
	if((mp == NULL) || (mp == (struct mount *) MSWAPX)) {
		u.u_error = EINVAL;
		return;
	}

	/* check perm and allow user level mount and umount */
	if(u.u_uid) {
		if(u.u_uid != mp->m_fs_data->fd_uid) {
#ifdef GFSDEBUG
			cprintf("umount: uid %d != fd_uid %d\n",
			u.u_uid, mp->m_fs_data->fd_uid);
#endif
			u.u_error = EPERM;
			return;
		}
	}

	gfs_gupdat();
	bflush(mp->m_dev);

#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("gfs_umount: mp 0x%x dev 0x%x\n", mp, mp->m_dev);
#endif

	if((u.u_error = GUMOUNT(mp, 0)) == 0) {
		if (mp->m_bufp != (struct buf *) (NODEV))
			brelse(mp->m_bufp);
		km_free(mp->m_fs_data, sizeof(struct fs_data));
		mpurge(mp - &mount[0]);
		bzero((caddr_t)mp, sizeof(struct mount));
		mp->m_dev = NODEV;
		binval(dev, (struct gnode *) 0);
		if((mp - mount) == (nmount - 1))
			nmount--;
	}
}

update(mp)
	register struct  mount *mp;
{
	register int ret;

#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("update: updating mp 0x%x flags %x\n", mp, mp->m_flags);
#endif
	if((mp->m_flags & M_MOD) && (mp->m_flags & M_DONE)){
		if(ISREADONLY(mp)) {
			printf("fs= %s\n",mp->m_path);
			panic("update: Read only file system");
	  	}
		ret = GSBUPDATE(mp, 0);
	} 
#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("update: calling bflush on dev 0x%x\n", mp->m_dev);
#endif
	bflush(mp->m_dev);
}

gfs_gupdat()
{
        register struct gnode *gp;
	register int ret;
	
	for (gp = gnode; gp < gnodeNGNODE; gp++) {
		if((gp->g_flag & GLOCKED) != 0 || gp->g_count == 0 ||
		((gp->g_flag & (GMOD|GACC|GUPD|GCHG)) == 0))
                        continue;

		if(!ISLOCAL(gp->g_mp)) {
#ifdef GFSDEBUG
			if(GFS[9])
				cprintf("gfs_updat: update remote gnode 0x%x\n",
					gp);
#endif GFSDEBUG
			continue;
		}
		
		/* to maintain the integrity of the gnode while updating it,
		 * make sure it is locked
		 */

		gfs_lock(gp);
		gp->g_count++;

#ifdef GFSDEBUG
		if(GFS[9])
			cprintf("gfs_update: gp 0x%x dev 0x%x gno %d\n", gp,
			gp->g_dev, gp->g_number);
#endif
		ret = GUPDATE(gp, &time, &time, 0, u.u_cred);
                gput(gp);

		/* we need to see if the gnode is still locked */
	}
}

#ifdef notdef
struct mount *
getmp(dev)
	register dev_t dev;
{
	register struct mount *mp;
	for(mp = mount; mp < &mount[NMOUNT]; mp++) 
		if((mp->m_bufp != 0) && (dev == mp->m_dev)) {
			return(mp);
		}
	return(NULL);
}
#endif

int pdev_major = 0;
int pdev_minor = 0;

dev_t
getpdev()
{
	if (pdev_major == 0) pdev_major = (nblkdev+nchrdev+1);
	if (pdev_minor > (1<<7)-1) {
		pdev_major++;
		pdev_minor=0;
	}
	return(makedev(pdev_major,pdev_minor++));
}


/*
 * Common code for ufs_mount, ufs_umount and setquota.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */


getmdev(pdev, fname, usr)
	dev_t *pdev;
	char *fname;
	int usr;
{
	register dev_t dev;
	register struct gnode *gp;
 	register struct nameidata *ndp = &u.u_nd;

	if (u.u_uid && !usr) {
		u.u_error = EPERM;
		return (EPERM);
	}

 	ndp->ni_nameiop = LOOKUP | FOLLOW;
 	ndp->ni_dirp = fname;
 	if((gp = GNAMEI(ndp)) == NULL)
		return (u.u_error);

	/* user level mount -- user must have read permission on dev */

	if(usr)
		if(access(gp, GEXEC)) {
			gput(gp);
			u.u_error = EPERM;
			return(EPERM);
		}

	if ((gp->g_mode&GFMT) != GFBLK) {
		gput(gp);
		return (ENOTBLK);
	}
	dev = gp->g_rdev;
	if (major(dev) >= nblkdev) {
		gput(gp);
		return (ENXIO);
	}
	gput(gp);
	*pdev = dev;
	return (0);
}
