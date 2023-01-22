#ifndef lint
static	char	*sccsid = "@(#)ufs_mount.c	1.21	(ULTRIX)	3/3/87";
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
 * 15 Jan 87 -- prs
 *	Added or'ing in the M_QUOTA bit to the mount structures flags
 *	field.
 *
 * 04 Dec 86 -- prs
 *	Made changes to not set FS_CLEAN byte if a file system
 *	was mounted with -o force in ufs_sbupdat.
 *
 * 11 Sep 86 -- koehler
 *	made changes for synchronous fs and user level mounts
 *
 ***********************************************************************/


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../ufs/fs.h"
#include "../h/fs_types.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/file.h"
#include "../h/kernel.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/devio.h"
#include "../ufs/ufs_mount.h"

int		ufs_umount(),	ufs_sbupdat();
struct gnode	*ufs_gget(),	*ufs_namei();
int		ufs_link(),	ufs_unlink(),	ufs_rmdir();
struct gnode	*ufs_mkdir();
struct gnode	*ufs_maknode();
int		ufs_rename(),	ufs_getdirent(),ufs_grele();
struct gnode	*ufs_galloc();
int		ufs_syncgp(),	ufs_gfree(),	ufs_gtrunc();
int		ufs_rwgp();
struct filock 	*ufs_rlock();
int		ufs_seek(),	ufs_stat(),	ufs_glock();
int		ufs_gunlock(),	ufs_gupdat(),	ufs_open();
int		ufs_close(),	ufs_select(),	ufs_readlink();
int		ufs_symlink();
struct fs_data	*ufs_getfsdata();
int		ufs_fcntl(),	ufs_bmap();

struct	mount_ops Ufs_ops = {
/* begin mount ops */
	ufs_umount,
	ufs_sbupdat,
	ufs_gget,
	ufs_namei,
	ufs_link,
	ufs_unlink,
	ufs_mkdir,
	ufs_rmdir,
	ufs_maknode,
	ufs_rename,
	ufs_getdirent,
	ufs_grele,
	ufs_syncgp,
	ufs_gtrunc,
	0, 		/*getval*/
	ufs_rwgp,
	ufs_rlock,
	ufs_seek,
	ufs_stat,
	ufs_glock,
	ufs_gunlock,
	ufs_gupdat,
	ufs_open,
	ufs_close,
	ufs_select,
	ufs_readlink,
	ufs_symlink,
	ufs_getfsdata,
	ufs_fcntl,
	0,		/* gfreegn */
	ufs_bmap
};

extern int turn_on_sync;

struct  mount_ops  *ufs_ops = &Ufs_ops;
gno_t rootino = (gno_t) ROOTINO;

#ifdef GFSDEBUG
extern short GFS[];
#endif

/* this routine has lousy error codes */
/* this routine has races if running twice */
struct mount *
ufs_mount(devname, name, ronly, mp, fs_specific)
	char *devname;
	char *name;
	int ronly;
	register struct mount *mp;
	struct ufs_specific *fs_specific;
{
	dev_t	dev;
	register struct buf *tp = NULL;
	register struct buf *bp = NULL;
	register struct fs *fs;
	register struct mount *nmp;
	register struct gnode *gp;
	int blks;
	caddr_t space;
	int i, size;
	int root;
	struct gnode gnode;
	struct devget devget;
	struct ufs_specific ufs_sp;
	int force = 0;

	/* special case this when we are mounting the root fs */
	
	root = devname == NULL;
	
	if(!root) {			/* we are not mounting / */
		if(u.u_error = getmdev(&dev, devname, u.u_uid)) {
#ifdef GFSDEBUG
			if(GFS[6])
				cprintf("ufs_mount: getmdev failure\n");
#endif
			return(NULL);
		}
#ifdef GFSDEBUG
		if(GFS[6])
			cprintf("ufs_mount: dev 0x%x\n", dev);
#endif
		GETMP(nmp, dev);
		if(nmp != NULL) {
#ifdef GFSDEBUG
			if(GFS[6])
				cprintf("ufs_mount: already mounted\n");
#endif
			u.u_error = EBUSY; /* someone has us mounted */
			return(NULL);
		}
		if(fs_specific != NULL) {
			u.u_error = copyin((caddr_t) fs_specific, (caddr_t)
			&ufs_sp, sizeof(ufs_sp));
			if(u.u_error)
				return(NULL);
			fs_specific = &ufs_sp;
		}
	} else {
		force = 1;
		dev = rootdev;		/* map the root device from config */
	}
		
	/* 
	 * this is a hack but I can't see a way around it
	 * we need a gnode before we have the system mounted so we push 
	 * one on the stack and hand craft it. The gget at the end replaces
	 * this fake gnode with the real one.
	 */
		
	gnode.g_mode = GFBLK;
	gnode.g_dev = gnode.g_rdev = dev;
	gnode.g_mp = mp;
	mp->m_rootgp = &gnode;
	
	/* set up some stuff that smount cannot accomplish */
	
	mp->m_dev = dev;
	mp->iostrat = bdevsw[major(dev)].d_strategy;	/* set this now! */
	mp->m_ops = ufs_ops;	/* set pointer to file ops */
	mp->m_flags = ((ronly) ? M_RONLY : 0) | M_LOCAL;
	if(fs_specific != NULL) {
		int pg_thresh = fs_specific->ufs_pgthresh * 1024;
		mp->m_flags |= (fs_specific->ufs_flags & (M_RONLY | M_NOEXEC |
			M_QUOTA | M_NOSUID | M_NODEV | M_FORCE | M_SYNC));

		mp->m_fs_data->fd_pgthresh = 
			clrnd(btoc((pg_thresh > MINPGTHRESH) ?
						pg_thresh : MINPGTHRESH));
		
		/* only the superuser can mount forceably */

		force = (fs_specific->ufs_flags & M_FORCE) && (u.u_uid == 0);
	} else
		/*
		 * if we don't specify the thresh hold, use 64kB
		 */
		mp->m_fs_data->fd_pgthresh = clrnd(btoc(MINPGTHRESH * 8));

	if(turn_on_sync == 0)
		mp->m_flags &= ~M_SYNC;


#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("ufs_mount: flags 0%o, name '%s' mp 0x%x dev 0x%x thresh 0x%x\n",
		mp->m_flags, name, mp, dev, mp->m_fs_data->fd_pgthresh);
#endif

	if(u.u_error = (*bdevsw[major(dev)].d_open)(dev, ronly ?
	FREAD : FREAD|FWRITE))
		return(NULL);

	u.u_error = (*bdevsw[major(dev)].d_ioctl)(dev, DEVIOCGET, &devget, 0);

	if (u.u_error) {
		if (!root) {
			printf("file system device ioctl failure");
		} else {
			printf("root file system device ioctl failure");
		}
	}

	if(devget.stat & DEV_OFFLINE) {
		u.u_error = ENODEV;
		goto ERROR;
	}
	if((devget.stat & DEV_WRTLCK) && !ronly) {
		if(!root) {
			u.u_error = EROFS;
			goto ERROR;
		} else {
			(*bdevsw[major(dev)].d_close)(dev,
			 ronly ? FREAD : FREAD | FWRITE);
			ronly = 1;
			if(u.u_error = (*bdevsw[major(dev)].d_open)
					(dev, ronly ? FREAD : FREAD|FWRITE)) {
				return(NULL);
			}
			u.u_error = EROFS;
		}
	}

	/* get us the superblock */
	
	tp = bread(dev, SBLOCK, SBSIZE, (struct gnode *) NULL);
	if (tp->b_flags & B_ERROR)
		goto ERROR;

	fs = tp->b_un.b_fs;

	/*
	 *	Check the magic number and see 
	 *	if we have a valid filesystem.
	 */
	if (fs->fs_magic != FS_MAGIC ) {		/*001*/
 		u.u_error = EINVAL;		/* also needs translation */
		goto ERROR;
	}
	
	/* 
	 * only root can mount a non-cleaned filesystem and then only
	 * forcibly
	 */

	if((fs->fs_fmod != FS_CLEAN) && !force) {
		uprintf("ufs_mount: fs %s not cleaned -- please fsck\n",
		devname);
		u.u_error = EINVAL;
		goto ERROR;
	}

	bp = geteblk((int)fs->fs_sbsize);
	mp->m_bufp = bp;
#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("ufs_mount: superblock at 0x%x\n", mp->m_bufp);
#endif

	/* map the superblock into a buffer not connected with a device */
	
	bcopy((caddr_t)tp->b_un.b_addr, (caddr_t)bp->b_un.b_addr,
	   (u_int)fs->fs_sbsize);
	brelse(tp);
	tp = 0;
	fs = bp->b_un.b_fs;
	if(!root)
		bcopy(name, fs->fs_fsmnt, MAXMNTLEN);
	else
		bcopy("/", fs->fs_fsmnt, sizeof("/"));
		
#ifdef GFSDEBUG
	if(GFS[6])
		cprintf ("ufs_mount: FS size %d bmask %d\n",
		fs->fs_size, fs->fs_bmask);
#endif
	fs->fs_ronly = (ronly != 0);
		
	/*	needs to check writtability of device */

	if (ronly == 0)
		mp->m_flags |= M_MOD;
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = wmemall(vmemall, (int)fs->fs_cssize);

	if (space == 0) {
 		u.u_error = ENOMEM;
		goto ERROR;
	}
	
	/* get us the cylinder groups */
	
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		tp = bread(dev, fsbtodb(fs, fs->fs_csaddr + i), size,
		(struct gnode *) NULL);
		if (tp->b_flags&B_ERROR) {
			wmemfree(space, (int)fs->fs_cssize);
			goto ERROR;
		}
		bcopy((caddr_t)tp->b_un.b_addr, space, (u_int)size);
		fs->fs_csp[i / fs->fs_frag] = (struct csum *)space;
		space += size;
		brelse(tp);
		tp = 0;
	}
	
	/* gfs has no knowledge of the following parameters, we must set them */
	
	mp->m_bsize = fs->fs_bsize;		/* set it once and for all */
	mp->m_fstype = GT_ULTRIX;
	
	(void) ufs_getfsdata(mp);
	
	/* this gget is very order dependent, do it last */
	
	if((gp = ufs_gget(dev, mp, ROOTINO, 0)) == NULL) 
		panic("ufs_mount: cannot find root inode");
	ufs_gunlock(gp);
	
	/* point the mount table toward the root of the filesystem */
	
	mp->m_rootgp = gp;
	
	
#ifdef GFSDEBUG
	if(GFS[6]) {
		cprintf("ufs_mount: rootgp 0x%x (%d)\n", gp, gp->g_number);
		cprintf("ufs_mount:	g_mp 0x%x ops 0x%x\n", gp->g_mp,
		gp->g_mp->m_ops);
	}
#endif
	/*
	 * once we are mounted, make no presumptions on the cleanliness
	 * of the filesystem.
	 */
	
	fs->fs_fmod = 0;
	if(!(ronly || root))
		ufs_sbupdat(mp, 0);
	if(root) {
		devname = mp->m_fs_data->fd_devname;
		bcopy("/dev/",devname,5);
		bcopy(devget.dev_name,&devname[5],strlen(devget.dev_name));
		i = strlen(devname);
		if(devget.unit_num > 9) {
			devname[i++] = '0' + devget.unit_num/10;
		}
		devname[i++] = '0' + devget.unit_num%10;
		devname[i++] = 'a' + devget.category_stat;
		devname[i++] = '\0';
		bcopy("/", mp->m_fs_data->fd_path, sizeof("/"));
		inittodr(fs->fs_time);
	}

	return (mp);
ERROR:
	(*bdevsw[major(dev)].d_close)(dev, ronly ? FREAD : FREAD | FWRITE);

	/* something happened and we need to invalidate the buffer cache
	 * so that later we may re-use the device
	 */
	binval(dev, (struct gnode *) 0);
	return(NULL);
}

ufs_umount(mp, force)
	register struct	mount	*mp;
	register int force;
{
	register struct gnode *gp = mp->m_gnodp;
	register struct fs  *fs;
	register int stillopen;
	dev_t	dev = mp->m_dev;

	
#ifdef GFSDEBUG
	if(GFS[6])
		cprintf("ufs_umount: tryping to unmount mp 0x%x dev 0x%x\n",
		mp, dev);
#endif		
		
	nchinval(dev);			/* flush the name cache */
	xumount(dev);			/* get rid of the sticky bitted files */
	if(!ISREADONLY(mp))
		ufs_sbupdat(mp, 0);		/* flush the superblock */

	
#ifdef QUOTA
	if((stillopen = gflush(dev, mp->m_qinod, mp->m_rootgp)) < 0 && !force)
#else
	if((stillopen = gflush(dev, mp->m_rootgp)) < 0 &&
	!force)					 /* try to flush gnodes */
#endif
		return(EBUSY);		/* someone has a file open)  */
	
 	if(stillopen < 0) 
		return(EBUSY);

	(void) GRELE(mp->m_rootgp);
#ifdef QUOTA
	closedq(mp);

	/* there is a nasty piece of baggage with quotas, we must reflush
	 * all the gnodes for the device to get rid of the quota gnode
	 */
	
	(void) gflush(dev, (struct gnode *) NULL, (struct gnode *) NULL);
#endif
	
	/* make the mounted directory accessible again */
	
	gp->g_flag &= ~GMOUNT;
	(void) GRELE(gp);
	
	/* mark the filesystem as clean */
	
	if(!ISREADONLY(mp))
		ufs_sbupdat(mp, 1);		
	/* free the cylinder group stuff */

	fs = mp->m_bufp->b_un.b_fs;
	wmemfree((caddr_t)fs->fs_csp[0], (int)fs->fs_cssize);
	
	if(!stillopen) 
		(*bdevsw[major(dev)].d_close)(dev, !fs->fs_ronly);
	return(NULL);
}

ufs_sbupdat(mp, flag)
	register struct mount *mp;
	int flag;
{
	register struct fs *fs = mp->m_bufp->b_un.b_fs;
	register struct buf *bp;
	register int blks, i, size;
	caddr_t space;
	
#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("ufs_sbupdat: fs 0x%x mp 0x%x rootgp 0x%x dev %x\n", fs,
		mp, mp->m_rootgp, mp->m_dev);
#endif GFSDEBUG
	mp->m_flags &= ~ M_MOD;
	fs->fs_time = time.tv_sec;
	/*
	 * Don't set FS_CLEAN byte if file system was force mounted.
	 */
	fs->fs_fmod = (flag == 1 && !(mp->m_fs_data->fd_flags & M_FORCE))
	  ? FS_CLEAN : 1;

	/*
	 * we use 0 instead of gp because the superblock and such is not
	 * really in the buffer cache
	 */
	
	bp = getblk(mp->m_dev, SBLOCK, (int)fs->fs_sbsize, (struct gnode *) 0);
	bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)fs->fs_sbsize);
	bwrite(bp);
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = (caddr_t)fs->fs_csp[0];
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		bp = getblk(mp->m_dev, fsbtodb(fs, fs->fs_csaddr + i), size, 
		(struct gnode *) 0);
		bcopy(space, bp->b_un.b_addr, (u_int)size);
		space += size;
		bwrite(bp);
	}
#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("ufs_sbupdat: sb flushed\n");
#endif GFSDEBUG
}
