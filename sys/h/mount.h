/*	@(#)mount.h	1.13	(ULTRIX)	1/15/87	*/

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
 *	Added MOUNT_QUOTA define
 *
 * 23 Oct 86 -- chet
 *	Add sync flag to GBMAP macro for synchronous writes
 *
 * 11 sep 86 -- koehler
 *	modifications for alignment of fs_data structure.. new function
 *	introduction, new mount options, and general fun stuff
 *
 * 16 oct 86 -- koehler
 *	returned the GETMP to its original state
 *
 ***********************************************************************/

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */

/* needs param.h */
#ifndef __MOUNT__
#define __MOUNT__
/*
 * fs_data should eliminate the need for fs.h in sys/sys and sys/gfs
 */

struct fs_data_req {	/* required part for all file systems */
		u_int	flags;		/* how mounted */
		u_int	mtsize;		/* max transfer size in bytes */
		u_int	otsize;		/* optimal transfer size in bytes */
		u_int	bsize;		/* fs block size in bytes for vm code */
		u_int	fstype;		/* see ../h/fs_types.h  */
		u_int	gtot;		/* total number of gnodes */
		u_int	gfree;		/* # of free gnodes */
		u_int	btot;		/* total number of 1K blocks */
		u_int	bfree;		/* # of free 1K blocks */
		u_int	bfreen;		/* user consumable 1K blocks */
		u_int	pgthresh;	/* min size in bytes before paging*/
		int	uid;		/* uid that mounted me */
		dev_t	dev;		/* major/minor of fs */
		dev_t	pad;		/* alignment: dev_t is a short*/
		char	devname[MAXPATHLEN + 4]; /* name of dev */
		char	path[MAXPATHLEN + 4]; /* name of mount point */
};					/* 2108 bytes */

struct fs_data {
		struct	fs_data_req fd_req;/* 2108 bytes */
		u_int	fd_spare[113];	/* this structure is exactly  */
					/* 13 * 4 + 2 * 1028 + 113 *4 = 2560 */
					/* bytes - KEEP IT THAT WAY - rr */
					/* since we malloc memory in 512 byte */
					/* chunks, the last 113 u_int's are */
					/* FREE */
};

/* these defines let anyone access the fields very simply */
/* fsdata->fd_flags or v_fs_data->fd_flags work! see nfs/vfs.h */

#define fd_flags	fd_req.flags	/* how mounted */
#define fd_mtsize	fd_req.mtsize	/* max transfer size */
#define fd_otsize	fd_req.otsize	/* optimal transfer size */
#define fd_bsize	fd_req.bsize	/* fs block size for vm code */
#define fd_fstype	fd_req.fstype	/* see ../h/fs_types.h  */
#define fd_gtot		fd_req.gtot	/* total number of gnodes */
#define fd_gfree	fd_req.gfree	/* # of free gnodes */
#define fd_btot		fd_req.btot	/* total number of blocks */
#define fd_bfree	fd_req.bfree	/* # of free blocks */
#define fd_bfreen	fd_req.bfreen	/* user consumable blocks */
#define fd_pgthresh	fd_req.pgthresh	/* min file size before paging*/
#define fd_uid		fd_req.uid	/* uid that mounted me */
#define fd_dev		fd_req.dev	/* major/minor of fs */
#define fd_pad		fd_req.pad	/* alignment: dev_t is a short*/
#define fd_devname	fd_req.devname	/* name of dev */
#define fd_path		fd_req.path	/* name of mount point */

struct	mount
{
	struct	mount	*m_forw[2];	/* must be first */
	dev_t	m_dev;			/* device mounted on */
	u_short m_pad;			/* padding */
	union {
		struct	buf	*fs;	/* pointer to superblock */
		char		*cp;	/* whatever specific fs wants */
	} m_sb_un;
	struct	fs_data *m_fs_data;
	struct	gnode	*m_gnodp;	/* pointer to mounted on gnode */
	struct	gnode	*m_rootgp;	/* pointer to root gnode */
	struct	gnode	*m_qinod;	/* QUOTA: pointer to quota file */
	struct	mount_ops {	/* begin mount ops */
/*		return value	function	arguments		*/

		int		(*go_umount)(	/* mp,force		*/ );
		int		(*go_sbupdate)(	/* mp, last 		*/ );
		struct gnode *	(*go_gget)(	/* dev, mp, ino, flag	*/ );
		struct gnode * 	(*go_namei)(	/* ndp			*/ );
		int		(*go_link)(	/* gp, ndp		*/ );
		int		(*go_unlink)(	/* gp,ndp		*/ );
		struct gnode *	(*go_mkdir)(	/* pgp,name,mode	*/ );
		int		(*go_rmdir)(	/* gp,ndp		*/ );
		struct gnode *	(*go_maknode)(	/* mode,ndp		*/ );
		int		(*go_rename)(	/* gp,from_ndp,to_ndp,flag*/ );
		int		(*go_getdirents)(/* gp, uio, cred	*/ );
		int		(*go_rele)(	/* gp			*/ );
		int		(*go_syncgp)(	/* gp			*/ );
		int		(*go_trunc)(	/* gp, newsize, cred	*/ );
		int		(*go_getval)(	/* gp			*/ );
		int		(*go_rwgp)(	/* gp,uio, rw, flag,cred*/ );
		struct filock *	(*go_rlock)(    /* gp,cmd,flino,filock,flock*/);
		int		(*go_seek)(	/* gp, loc		*/ );
		int		(*go_stat)(	/* gp, statbuf		*/ );
		int		(*go_lock)(	/* gp			*/ );
		int		(*go_unlock)(	/* gp			*/ );
		int		(*go_gupdat)(	/* gp,atime,mtime,wait,cred*/);
		int		(*go_open)(	/* gp, mode		*/ );
		int		(*go_close)(	/* gp, flag		*/ );
		int		(*go_select)(	/* gp, rw, cred		*/ );
		int		(*go_readlink)(	/* gp, uio		*/ );
		int		(*go_symlink)(	/* gp, source, dest	*/ );
		struct fs_data *(*go_getfsdata)(/* mp			*/ );
		int		(*go_fcntl)(	/* gp,cmd,args,flag,cred*/ );
		int		(*go_freegn)(	/* gp			*/ );

		int		(*go_bmap)(	/* gp,vbn,rw,size,sync	*/ )
	} *m_ops;
	/* iostrategy routine per mounted file system */
	int			(*iostrat)(	/* bp			*/ );
};

#define	m_bufp		m_sb_un.fs		/* to make old code work*/

/* careful with these next defines since they are not valid until */
/* fs_data has been km_alloc'ed in mount system call */

#define m_path		m_fs_data->fd_path
#define m_bsize		m_fs_data->fd_bsize
#define m_fstype	m_fs_data->fd_fstype
#define m_flags		m_fs_data->fd_flags

#ifdef KERNEL

#define OPS(gp)	\
		((gp)->g_mp->m_ops)
#define GNOFUNC -1

/*
 ***************************************************************************
 ******************* START GFS FS MACROS ***********************************
 ***************************************************************************
 */

/*
 * Gnode function callout macros to leave the code readable.
 */

/*
 * GGET(dev, mp, gno, flag)
 *	dev_t	dev;
 *	struct	mount	*mp;
 *	gno_t	ino;
 *	int	flag;
 *
 *	Get a gnode from a mounted file system
 */

#define GGET(dev, mp, gno, flag)	\
		((struct gnode *) (((mp)->m_ops->go_gget) ? \
		(((mp)->m_ops->go_gget) ((dev), (mp), (gno), (flag))) : \
		(struct gnode *) GNOFUNC))
		
		
/*
 * GRELE(gp)
 *	struct	gnode	*gp;
 *
 *	Release a gnode
 */
						

#define	GRELE(gp)	\
		(OPS(gp)->go_rele ? OPS(gp)->go_rele(gp) : GNOFUNC)


/*
 * GALLOC(pgp, gpref, mode)
 *	struct	gnode	*pgp;
 *	gno_t	gpref;
 *	int	mode;
 *
 * Allocate a gnode around prefered gnode gpref with parent pgp and
 * mode mode.  gpref is only advisory and may be ignored.
 */

#define	GALLOC(pgp, gpref, mode)	\
		((struct gnode *)(OPS(pgp)->go_alloc ? \
		OPS(pgp)->go_alloc((pgp), (gpref), (mode)) : \
		(struct gnode *) GNOFUNC))
		
		
/*
 * GSYNCG(gp, cred)
 *	struct	gnode	*gp;
 *	struct  ucred *cred;
 *
 * Cause any cached data to be written out.
 */

#define GSYNCG(gp,cred)	\
		((OPS(gp)->go_syncgp ? (OPS(gp)->go_syncgp)(gp,cred) : GNOFUNC))


/*
 * GFREE(gp)
 *	struct	gnode	*gp;
 *
 * Free a gnode
 */

#define GFREE(gp)	\
		((OPS(gp)->go_free ? OPS(gp)->go_free(gp)) : GNOFUNC)


/*
 * GTRUNC(gp, newsize, cred) 
 *	struct	gnode	*gp;
 *	u_long newsize;
 *	struct ucred *cred;
 *
 * Truncate a file to the specified size.
 */

#define GTRUNC(gp, newsize, cred)	\
		(OPS(gp)->go_trunc ? OPS(gp)->go_trunc((gp), (newsize), \
		(cred)) : GNOFUNC)


/*
 * GGETVAL(gp)
 *	struct	gnode	*gp;
 *
 *	Get uptodate values in the gnode_common fields of the gnode
 *	This is primarily for remote file systems.
 */
	
#define GGETVAL(gp)	\
		(OPS(gp)->go_getval ? OPS(gp)->go_getval(gp) : GNOFUNC)
		
		
/*
 * GRWGP(gp, uio, rw, ioflag, cred)
 *	struct	gnode	*gp;
 *	struct	uio	*uio;
 *	enum	uio_rw	rw;
 *	int ioflag;
 *	struct ucred 	*cred;
 *
 * Read or write data on a gnode using the uio structure passed to the
 * function.
 */
					
#define GRWGP(gp, uio, rw, ioflag, cred)	\
		((OPS(gp)->go_rwgp) ((gp), (uio), (rw), (ioflag), (cred)))


/*
 * GUMOUNT(mp, flag)
 *	struct mount *mp;
 *	int flag;
 *
 * umount the file system pointed to by mp
 */
	
#define GUMOUNT(mp,flag)	\
		(((mp)->m_ops->go_umount)((mp),(flag)))


/*
 * GSBUPDATE(mp)
 *	struct mount *mp;
 *	int last;
 *
 * flush the superblock pointed to by mp
 */

#define GSBUPDATE(mp, last)	\
		((((mp)->m_ops->go_sbupdate ? ((mp)->m_ops->go_sbupdate) \
		((mp), (last)) : GNOFUNC)))
		


/*
 * GLINK(gp, pgp, source, target)
 *	struct gnode *gp;
 *	struct gnode *pgp;
 *	char *source
 *	char *target;
 *
 * link the file target whose parent is pgp to source which is
 * pointed to by gp
 */

#define	GLINK(gp, ndp)	\
		(OPS(gp)->go_link ? OPS(gp)->go_link((gp), (ndp)) \
		: GNOFUNC)
	

/*
 * GUNLINK(gp, ndp)
 *	struct gnode *gp;
 *	struct nameidata *ndp
 *
 * unlink the file pointed to by gp having the ndp set for us
 */
	
#define GUNLINK(gp, ndp)	\
		(OPS(gp)->go_unlink ? OPS(gp)->go_unlink((gp), (ndp)) \
		: GNOFUNC)


/*
 * GMKDIR(pgp, name, mode)
 *	struct gnode *gp;
 *	char *name;
 *	u_int mode;
 *
 * create a directory whose parent is pointed to by pgp with modes mode
 */

#define GMKDIR(pgp, name, mode)	\
		(OPS(pgp)->go_mkdir ? OPS(pgp)->go_mkdir((pgp), (name), \
		(mode)) : (struct gnode *) GNOFUNC)


/*
 * GRMDIR(gp, ndp)
 *	struct gnode *gp;
 *	struct nameidata *ndp;
 *
 * remove the directory pointed to by gp with ndp set up
 */
	
#define GRMDIR(gp, ndp)	\
		(OPS(gp)->go_rmdir ? OPS(gp)->go_rmdir((gp), (ndp)) : GNOFUNC)


/*
 * GMAKNODE(mode, ndp)
 *	u_int mode;
 *	struct nameidata *ndp;
 *
 * make a special file with modes mode (note mode has GFMT meaning)
 */

#define GMAKNODE(mode,ndp)	\
		(OPS((ndp)->ni_pdir)->go_maknode ? \
		(OPS((ndp)->ni_pdir)->go_maknode)((mode),(ndp)) : \
		(struct gnode *) GNOFUNC)
	

/*
 * GRENAMEG(gp, ssd, source_ndp, tsd, target_ndp, flag)
 *	struct gnode *gp;
 *	struct gnode *ssd, *tsd;
 *	struct nameidata *source_ndp;
 *	struct nameidata *target_ndp;
 *	int flag;
 *
 * rename the file pointed to by gp with source_ndp set up from the namei
 * obtained from the call for gp and move it to target_ndp (note that dirp
 * is used).  ssd and tsd are the starting directories for parsing the
 * source and target respectively.
 */
	
#define GRENAMEG(gp, ssd, source_ndp, tsd, target_ndp, flag)	\
		(OPS(gp)->go_rename ? OPS(gp)->go_rename((gp), (ssd), \
		(source_ndp), (tsd), (target_ndp), (flag)) : GNOFUNC)


/*
 * GGETDIRENTS(gp, uio, cred)
 *	struct gnode *gp;
 *	struct uio *uio;
 *	struct ucred *cred
 *
 * read the directory pointed to by gp, format the output according to
 * the generic directory format, and return it to the place pointed to
 * by uio
 */
	
#define GGETDIRENTS(gp, uio, cred)	\
		((OPS(gp)->go_getdirents)((gp),(uio), (cred)))


/* 
 * GSYMLINK(ndp, target)
 *	struct nameidata *ndp;
 *	char *target;
 *
 * create a symbolic link named source using pgp as the pointer to the parent
 * and point it towards the name target
 */

#define GSYMLINK(ndp, target)	\
		(OPS(ndp->ni_pdir)->go_symlink ? OPS(ndp->ni_pdir)\
		->go_symlink((ndp), (target)) : GNOFUNC)
					

/*
 * GNAMEI(ndp)
 *	struct nameidata *ndp;
 *
 * parse the pathname pointed to by ndp and return a gnode pointer on
 * success
 */

#define GNAMEI(ndp)	\
		(gfs_namei(ndp))


/*
 * GRLOCK(gp, cmd, flino, filock, flock)
 *	struct gnode *gp;
 *	int cmd;
 *	struct flino *flino;
 *	struct filock *flockp;
 *	struct flock *flock;
 *
 * perform the command cmd on the file/region pointed to by gp
 * using the given structures
 */

#define GRLOCK(gp, cmd, flino, flockp, flock) \
		(OPS(gp)->go_rlock ? OPS(gp)->go_rlock((gp), (cmd), \
		(flino), (flockp), (flock)) : (struct filock *) GNOFUNC)


/*
 * GSEEK(gp, loc)
 *	struct gnode *gp;
 *	int loc;
 *
 * seek to loc on the file pointed to by gp
 */

#define GSEEK(gp, loc)	\
		(OPS(gp)->go_seek ? OPS(gp)->go_seek((gp), (loc)) : GNOFUNC)
		
		
/*
 * GSTAT(gp, statbuf)
 *	struct gnode *gp;
 *	struct stat *statbuf;
 *
 * fill in various file stats into the buf statbuf on the file pointed
 * to by gp
 */

#define GSTAT(gp, statbuf)	\
	((OPS(gp)->go_stat)((gp), (statbuf)))

		
/*
 * GLOCK(gp)
 *	struct gnode *gp;
 *
 * lock the gnode gp
 */

#define GLOCK(gp)	\
		(OPS(gp)->go_lock ? OPS(gp)->go_lock(gp) : GNOFUNC)
		
		
/*
 * GUNLOCK(gp)
 *	struct gnode *gp;
 *
 * unlock the gnode gp
 */

#define GUNLOCK(gp)	\
		(OPS(gp)->go_unlock ? OPS(gp)->go_unlock(gp) : GNOFUNC)

		
/*
 * GUPDATE(gp, atime, mtime, waitfor, cred)
 *	struct gnode *gp;		
 *	struct timeval	*atime;
 *	struct timeval	*mtime;
 *	int	waitfor;
 *	struct ucred	*cred;
 *
 * update the gnode gp with the access and modify times atime and mtime.
 * wait for completion if waitfor set
 */

#define GUPDATE(gp, atime, mtime, waitfor, cred)	\
		(OPS(gp)->go_gupdat ? OPS(gp)->go_gupdat((gp), (atime), \
		(mtime), (waitfor), (cred)) : GNOFUNC)
		
		
/*
 * GOPEN(gp, ioflag)
 *	struct gnode *gp;
 *	int ioflag;
 *
 * attempt to open the file pointed to by gp with the flags ioflag
 */

#define GOPEN(gp, ioflag)	\
		(OPS(gp)->go_open)((gp), (ioflag))
		
		
/*
 * GCLOSE(gp, ioflag)
 *	struct gnode *gp;
 *	int ioflag;
 *
 * attempt to close the gnode gp
 */
		
#define GCLOSE(gp, ioflag)	\
		(OPS(gp)->go_close) ((gp), (ioflag))
		

/*
 * GSELECT(gp, rw, cred)
 *	struct gnode *gp;
 *	int rw;
 *	struct ucred *cred;
 *
 * perform a select on gp of type rw
 */
		
#define GSELECT(gp, rw, cred)		\
		(OPS(gp)->go_select ? OPS(gp)->go_select((gp), (rw), (cred)) \
		: GNOFUNC)
		
		
/*
 * GREADLINK(gp, uio)
 *	struct gnode *gp;
 *	struct uio *uio;
 *
 * read the symbolic link information of gp into uio
 */
		
#define GREADLINK(gp, uio)	\
		(OPS(gp)->go_readlink ? OPS(gp)->go_readlink((gp), (uio)) \
		: GNOFUNC)
		
		
/*
 * GGETFSDATA(mp)
 *	struct mount *mp;
 *
 * get the generic file system data for the file system mp
 */
		
#define GGETFSDATA(mp)	\
		(((mp)->m_ops->go_getfsdata) (mp))
	
	
/*
 * GFNCTL(gp, cmd, args, flag, cred)
 *	struct gnode *gp;
 *	int cmd;
 *	char *args;
 *	int flag;
 *	struct cred *ucred;
 *
 * perform some non-generic action on the gnode gp
 */
	
#define GFNCTL(gp, cmd, args, flag, cred)	\
		(OPS(gp)->go_fcntl ? OPS(gp)->go_fcntl((gp), (cmd), \
		(args), (flag), (cred)) : GNOFUNC)
		
		
/*
 * GIOCTL(gp, cmd, data, mode, ioflag, cred)
 *	struct gnode *gp;
 *	int cmd;
 *	char *data;
 *	int mode;
 *	int ioflag;
 *	struct ucred *cred;
 *
 * perform some device specific action on the device pointed to by gp
 */
		
#define GIOCTL(gp, cmd, data, mode, ioflag, cred)	\
		(OPS(gp)->go_ioctl ? OPS(gp)->go_ioctl((gp), (cmd), \
		(data), (mode), (ioflag), (cred)) : GNOFUNC)
		
	
		
/*
 * GBMAP(gp,vbn,rw,size,sync)
 *	struct gnode *gp;	gnode to map
 *	int vbn;		virtual block number
 *	int rw;			read or write operation, B_READ or B_WRITE
 *	int size;		size of request - used only on write
 *	int sync;		synchronous? (1=yes, 0=no) - used only
 *				on write
 *
 * interface to block mapping routine (mainly for virtual memory (vinifod))
 * but it is used in ufs_namei.c and ufs_gnodeops.c and ufs_subr.c
 * 
 * Return value is the actual starting disk block # (lbn or logical block #)
 * or -1 on some error with u.u_error set appropriately.
 */

#define GBMAP(gp,vbn,rw,size,sync)				\
	((OPS((gp))->go_bmap) ((gp),(vbn),(rw),(size),(sync)))
	

/*
 * GFREEGN(gp)
 *	struct gnode *gp;
 *
 * call the specific file system routine to release resources attached
 * to a gnode that will be used by another filesystem type
 */

#define GFREEGN(gp)					\
		(OPS(gp)->go_freegn ? OPS(gp)->go_freegn(gp) : GNOFUNC)
	
	
/* extra functions per mounted file system */
		
/*
 * BIOSTATEGY(bp)
 *	struct buf *bp;
 * GIOSTATEGY(gp)
 *	struct gnode *gp;
 *
 * interface to an I/O strategy routine (mainly for remote filesystems)
 * when I/O is done on the VBN rather than the LBN.
 */

#define BIOSTRATEGY(bp)				\
	(((bp)->b_gp) ? (((bp)->b_gp->g_mp->iostrat) (bp)) : GNOFUNC)
/* direct pointer to the strategy routine from the gnode */
#define GIOSTRATEGY(gp)	((gp)->g_mp->iostrat)
	
/*
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 */


		
struct	mount mount[NMOUNT];
struct	_sfs {
	struct	mount *(*function)();
};
struct	_sfs    mount_sfs[NUM_FS];
#define MOUNTFS(num)	mount_sfs[(num)].function
struct	mount	*mountfs();
struct	mount_ops	*ufs_ops;
struct	mount_ops	*fs_divorce_ops;
#endif KERNEL

/* FLAGS */

#define	M_RONLY		0x0001
#define M_MOD  		0x0002
#define M_QUOTA		0x0004
#define M_LOCAL		0x0008
#define M_NOEXEC	0x0010
#define M_NOSUID	0x0020
#define M_NODEV		0x0040
#define M_FORCE		0x0080
#define M_SYNC		0x0100
#define M_DONE		0x0200
#define M_NOCACHE	0x0400

#define	ISREADONLY(mp)	((mp)->m_flags & M_RONLY)
#define	ISLOCAL(mp)	((mp)->m_flags & M_LOCAL)
#define M_USRMNT	(M_NOSUID | M_NODEV)

/* MOUNT TIME OPTIONS */

#define MOUNT_NOEXEC	"noexec"
#define MOUNT_NOSUID	"nosuid"
#define MOUNT_NODEV	"nodev"
#define MOUNT_PGTHRESH	"pgthresh="
#define MOUNT_FORCE	"force"
#define MOUNT_SYNC	"sync"
#define MOUNT_NOCACHE	"nocache"
#define MOUNT_QUOTA	"quota"

#define GETMP(assigned, dev)						\
	{								\
		register struct mount *__mp__;				\
									\
		if((dev) == swapdev) {					\
			assigned = (struct mount *)MSWAPX;		\
		} else {						\
			assigned = NULL;				\
			for(__mp__ = mount; __mp__ < &mount[nmount]; __mp__++){\
				if((dev) == __mp__->m_dev) {		\
					assigned = __mp__;		\
					break;				\
				}					\
			}						\
		}							\
	}
	
extern int nmount;

#endif __MOUNT__
