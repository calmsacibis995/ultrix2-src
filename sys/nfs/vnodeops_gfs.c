#ifndef lint
static	char	*sccsid = "@(#)vnodeops_gfs.c	1.18	(ULTRIX)	6/4/87";
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


/*
 * This file is an attempt to emulate the VFS operations done by the
 * NFS code with GFS operations. By convention, all gnodes are passed
 * in unlocked and remain unlocked, and gnodes returned are unlocked.
 */

#include "../h/types.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../ufs/fs.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/mount.h"
#include "../h/kernel.h"
#include "../h/uio.h"
#include "../h/buf.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/nfs.h"
#include "../nfs/vnode.h"
#include "../ufs/ufs_inode.h"

/*
 * Convert between vnode types and gnode formats
 */

enum vtype gftovt_tab[] = {
	VNON, VCHR, VDIR, VBLK, VREG, VLNK, VSOCK, VBAD
};
int vttogf_tab[] = {
	0, GFREG, GFDIR, GFBLK, GFCHR, GFLNK, GFSOCK, GFMT
};

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

int
vop_getattr(gp, vap, cred)
	struct gnode *gp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int ret;
	
#ifdef NFSDEBUG
	if(gp < &gnode[0] || gp >= &gnode[ngnode])
		panic("vop_getattr: gnode range");
	dprint(nfsdebug, 4, "vop_getattr: gp 0x%x ", gp);
	if(gp && gp->g_mp) {
		if(gp->g_mp->m_ops)
			dprint(nfsdebug, 4, " ops 0x%x\n", gp->g_mp->m_ops);
		else
			dprint(nfsdebug, 4, " ops not set\n");
	}
#endif

	ret = GGETVAL(gp);
	gfs_lock(gp);
	vap->va_type = IFTOVT(gp->g_mode);
	vap->va_mode = gp->g_mode;
	vap->va_uid = gp->g_uid;
	vap->va_gid = gp->g_gid;
	vap->va_fsid = gp->g_dev;
	vap->va_nodeid = gp->g_number;
	vap->va_nlink = gp->g_nlink;
	vap->va_size = gp->g_size;
	vap->va_atime = gp->g_atime;
	vap->va_mtime = gp->g_mtime;
	vap->va_ctime = gp->g_ctime;
	vap->va_rdev = gp->g_rdev;
	vap->va_blocks = gp->g_blocks;
	switch(gp->g_mode & GFMT) {

	case GFBLK:
		vap->va_blocksize = BLKDEV_IOSIZE;		
		break;

	case GFCHR:
		vap->va_blocksize = MAXBSIZE;
		break;

	default:
		vap->va_blocksize = gp->g_mp->m_fs_data->fd_bsize;
		break;
	}
	gfs_unlock(gp);
	return (0);
}

int
vop_setattr(gp, vap, cred)
	register struct gnode *gp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int chtime = 0;
	int error = 0;
	struct timeval atime;
	struct timeval mtime;
	
	atime = time;
	mtime = time;
	
	/*
	 * cannot set these attributes
	 */
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_blocks != -1) ||
	    (vap->va_fsid != -1) || (vap->va_nodeid != -1)) {

#ifdef NFSDEBUG
		dprint(nfsdebug, 4, "vop_setattr: nlink %d blocksize %d rdev 0x%x blocks %d fsid %d nodeid %d type %d\n",
		vap->va_nlink, vap->va_blocksize, vap->va_rdev,
		vap->va_blocks, vap->va_fsid, vap->va_nodeid, vap->va_type);
#endif
		return (EINVAL);
	}

	gfs_lock(gp);

	/*
	 * Change file access modes. Must be owner or su.
	 */
	if (vap->va_mode != -1) {
		error = OWNER(gp, cred);
		if (error) {
			goto out;
		}
		gp->g_mode &= GFMT;
		gp->g_mode |= vap->va_mode & ~GFMT;
		if (cred->cr_uid != 0) {
			gp->g_mode &= ~GSVTX;	/* XXX Why? */
			if (!groupmember(gp->g_gid))
				gp->g_mode &= ~GSGID;
		}
		gp->g_flag |= GCHG;
		/* Remember to xrele text if su ever allowed over net. */
	}

	/*
	 * Change file ownership (must be su).
	 */
	if ( ((vap->va_uid != -1) && (vap->va_uid != gp->g_uid)) ||
	     ((vap->va_gid != -1) && (vap->va_gid != gp->g_gid)) )
	{
		if (!suser()) {
			error = u.u_error;
			goto out;
		}
		error = chown2(gp, vap->va_uid, vap->va_gid);
		if (error)
			goto out;
	}

	/*
	 * Truncate file. Must have write permission and not be a directory.
	 */
	if (vap->va_size != -1) {
		if ((gp->g_mode & GFMT) == GFDIR) {
			error = EISDIR;
			goto out;
		}
		if (access(gp, GWRITE)) {
			error = u.u_error;
			goto out;
		}
		(void)GTRUNC(gp, vap->va_size, cred);

	}
	/*
	 * Change file access or modified times.
	 */

	if (vap->va_atime.tv_sec != -1) {
		if (access(gp, GREAD) && access(gp, GWRITE)) {
			error = u.u_error;
			goto out;
		}
		atime.tv_sec = vap->va_atime.tv_sec;
		atime.tv_usec = vap->va_atime.tv_usec;
		chtime++;
	}
	if (vap->va_mtime.tv_sec != -1) {
		if (access(gp, GWRITE)) {
			error = u.u_error;
			goto out;
		}
		mtime.tv_sec = vap->va_mtime.tv_sec;
		mtime.tv_usec = vap->va_mtime.tv_usec;
		chtime++;
	}
	if (chtime) {
		gp->g_flag |= GACC|GUPD|GCHG;
		gp->g_ctime = time;
	}

out:
	(void) GUPDATE(gp, &atime, &mtime, 1, cred);
	gfs_unlock(gp);
	return (error);
}

int
vop_lookup(dgp, name, gpp, cred)
	struct gnode *dgp;
	char *name;
	struct gnode **gpp;
	struct ucred *cred;
{
	struct nameidata *ndp = &u.u_nd;

	ndp->ni_dirp = name;
	ndp->ni_nameiop = LOOKUP | NOMOUNT;
	u.u_cdir = dgp;
	if(*gpp = GNAMEI(ndp))
		gfs_unlock(*gpp);
	return (u.u_error);
}

int
vop_readlink(gp, uiop, cred)
	struct gnode *gp;
	struct uio *uiop;
	struct ucred *cred;
{
	gfs_lock(gp);
	(void) GREADLINK(gp, uiop);
	gfs_unlock(gp);
	return (u.u_error);
}

int
vop_access(gp, mode, cred)
	struct gnode *gp;
	int mode;
	struct ucred *cred;
{
	u.u_error = 0;
	access(gp, mode);
	return (u.u_error);
}

int
vop_rdwr(gp, uiop, rw, syncflg, cred)
	struct gnode *gp;
	struct uio *uiop;
	int rw;
	int syncflg;
	struct ucred *cred;
{
	int error;

	if ((gp->g_mode & GFMT) == GFREG) {
		gfs_lock(gp);
		error = GRWGP(gp, uiop, rw, syncflg, cred);
		gfs_unlock(gp);
	}
	else 
		panic("vop_rdwr: not a regular file");
	return (error);
}

int
vop_create(dgp, name, vap, exclusive, garbage, gpp, cred)
	struct gnode *dgp;
	register char *name;
	struct vattr *vap;
	int exclusive;
	int garbage;
	struct gnode **gpp;
	struct ucred *cred;
{
	register struct nameidata *ndp = &u.u_nd;
	register struct gnode *gp;
	register char *cp;
	
	/*
	 * Disallow filenames with slashes (or parity bits), since the GFS
	 * pathname parsing scheme assigns a special meaning to them.
	 * This is a problem: Sun's implementation allows clients to
	 * use whatever they want for pathname delimiters, whereas ours
	 * presupposes that "/" is a delimiter and cannot allow it
	 * to be used in a filename.  This may someday become an issue
	 * in a heterogeneous environment with Ultrix NFS servers.
	 */
	
	cp = name;
	while(*cp)
		if((*cp == '/') || (*cp & 0200)) {
			u.u_error = EINVAL;
			return(u.u_error);
		}
		else
			cp++;
	
	ndp->ni_dirp = name;
	ndp->ni_nameiop = CREATE | LOCKPARENT | NOMOUNT;
	u.u_cdir = dgp;
	*gpp = GNAMEI(ndp);
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "vop_create: pdp 0x%x (%d) flags 0%o\n",
	ndp->ni_pdir, ndp->ni_pdir != (struct gnode *) 0xc0000000 ?
	ndp->ni_pdir->g_number : -1, ndp->ni_pdir != (struct gnode *)
	0xc0000000 ? ndp->ni_pdir->g_flag : -1);
#endif
	if (*gpp == NULL) {
		if (u.u_error) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 4, "vop_create: 1 u.u_error %d\n",
			u.u_error);
#endif
			return (u.u_error);
		}
 		*gpp = GMAKNODE((vap->va_mode & 07777 & (~GSVTX)) | GFREG, ndp);
		if (*gpp == NULL)
			return (u.u_error);
		if ((vap->va_size != 0) && (vap->va_size != -1)) {
			u.u_error = EINVAL;		/* ??? */
			goto bad;
		}
	} else {
		if (exclusive) {
			u.u_error = EEXIST;
			goto bad;
		}
		else {
			gp = *gpp;
			if (access(gp, GWRITE))
				goto bad;
			if ((gp->g_mode&GFMT) == GFDIR) {
				u.u_error = EISDIR;
				goto bad;
			}
			if (vap->va_size == 0) {
				if (GTRUNC(gp, (u_long)0, u.u_cred) == GNOFUNC) {
					u.u_error == EOPNOTSUPP;
					goto bad;
				}
				if (u.u_error)
					goto bad;
			}
			gput(dgp);
		}
	}
	gfs_unlock(*gpp);
	return(u.u_error);
bad:
	gput(dgp);
	gput(*gpp);
	*gpp = NULL;
	return (u.u_error);
}

int
vop_remove(pgp, name, cred)
	struct gnode *pgp;
	char *name;
	struct ucred *cred;
{
	struct nameidata *ndp = &u.u_nd;
	struct gnode *gp;
	int ret;
	
	ndp->ni_nameiop = DELETE | LOCKPARENT | NOMOUNT;
	ndp->ni_dirp = name;
	u.u_cdir = pgp;
	gp = GNAMEI(ndp);
	if (gp == NULL) {
		return (u.u_error);
	}

	/* Can't unlink directories -- use vop_rmdir. */
	if ((gp->g_mode & GFMT) == GFDIR) {
		u.u_error = EISDIR;
		goto out;
	}

	if (gp->g_dev != pgp->g_dev) {
		u.u_error = EBUSY;
		goto out;
	}
	if (gp->g_flag & GTEXT)
		xrele(gp);
	ret = GUNLINK(gp, ndp);
out:
	if(gp == ndp->ni_pdir) 
		panic("vop_remove: tried to unlink a dir");
	else
		gput(gp);
	gput(pgp);
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "vop_remove error %d\n", u.u_error);
#endif
	return (u.u_error);
}

int
vop_link(sgp, tdgp, name, cred)
	struct gnode *sgp;
	struct gnode *tdgp;
	char *name;
	struct ucred *cred;
{
	register struct nameidata *ndp = &u.u_nd;
	struct gnode *gp;
	
	gfs_lock(sgp);
	sgp->g_nlink++;
	sgp->g_flag |= GCHG;
	(void) GUPDATE(sgp, &time, &time, 1, cred);
	gfs_unlock(sgp);

	ndp->ni_nameiop = CREATE | NOMOUNT;
	ndp->ni_dirp = name;
	u.u_cdir = tdgp;
	gp = GNAMEI(ndp);
	if (u.u_error) {
		(void) GRELE(sgp);
		return(u.u_error);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "vop_link: sgp 0x%x (%d) flagx 0%o, gp->pdir 0x%x (%d) flags 0%o\n",
	sgp, sgp->g_number, sgp->g_flag, tdgp, tdgp->g_number, tdgp->g_flag);
#endif

	if (gp) {
		gput(gp);
		(void) GRELE(sgp);
		return (EEXIST);
	}

	if(sgp->g_mp != tdgp->g_mp) {
		u.u_error = EXDEV;
		gput(tdgp);
		(void) GRELE(sgp);
		return(u.u_error);
	}
	(void) GLINK(sgp, ndp);
	return (u.u_error);
}

int
vop_rename(sdp, sname, tdp, tname, cred)
	struct gnode *sdp;
	char *sname;
	struct gnode *tdp;
	char *tname;
	struct ucred *cred;
{
	struct nameidata tnd;
	struct nameidata *sndp = &u.u_nd;
	struct gnode *sgp;
	int ret;

	tnd.ni_nameiop = CREATE | LOCKPARENT | NOMOUNT;
	tnd.ni_segflg = UIO_SYSSPACE;
	tnd.ni_dirp = tname;

	sndp->ni_nameiop = DELETE | LOCKPARENT | NOMOUNT;
	sndp->ni_dirp = sname;
	u.u_cdir = sdp;
	sgp = GNAMEI(sndp);
	if (sgp == NULL) 
		return (u.u_error);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "vop_rename: sgp 0x%x, (%d)\n", sgp, sgp->g_number);
#endif
	u.u_cdir = tdp;
	ret = GRENAMEG(sgp, sdp, sndp, tdp, &tnd, NOMOUNT);
	return (u.u_error);
}

int
vop_symlink(pgp, fname, vap, tname, cred)
	struct gnode *pgp;
	char *fname;
	struct vattr *vap;	/* NOT DEALT WITH */
	char *tname;
	struct ucred *cred;
{
	register struct nameidata *ndp = &u.u_nd;
	struct 	uio auio;
	struct iovec aiov;
	register struct gnode *gp;
	
	ndp->ni_nameiop = CREATE | LOCKPARENT | NOMOUNT;
	ndp->ni_dirp = fname;
	u.u_cdir = pgp;
	gp = GNAMEI(ndp);
	if (gp) {
		gput(gp);
		gput(ndp->ni_pdir);
		return (EEXIST);
	}
	if (u.u_error)
		return (u.u_error);
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "vop_symlink: pdir 0x%x\n", ndp->ni_pdir);
#endif
	if(GSYMLINK(ndp, tname) == GNOFUNC)
		u.u_error = EOPNOTSUPP;
	return (u.u_error);
}

int
vop_mkdir(pgp, name, vap, gpp, cred)
	struct gnode *pgp;
	char *name;
	struct vattr *vap;
	struct gnode **gpp;
	struct ucred *cred;
{
	register struct nameidata *ndp = &u.u_nd;

	ndp->ni_nameiop = CREATE | LOCKPARENT | NOMOUNT;
	ndp->ni_dirp = name;
	u.u_cdir = pgp;
	*gpp = GNAMEI(ndp);
	if (u.u_error) {
		return (u.u_error);
	}
	if (*gpp != NULL) {
		gput(pgp);
		gput(*gpp);
		return (EEXIST);
	}
	*gpp = GMKDIR(ndp->ni_pdir, name, vap->va_mode);
#ifdef NFSDEBUG 
	dprint(nfsdebug, 4, "vop_mkdir: gp 0x%x \n", *gpp);
#endif
	return (u.u_error);	
}

int
vop_rmdir(pgp, name, cred)
	struct gnode *pgp;
	char *name;
	struct ucred *cred;
{
	register struct nameidata *ndp = &u.u_nd;
	struct gnode *gp;
	int ret;
	
	ndp->ni_nameiop = DELETE | LOCKPARENT | NOMOUNT;
	ndp->ni_dirp = name;
	u.u_cdir = pgp;
	gp = GNAMEI(ndp);
	if (gp == NULL) {
		return (u.u_error);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "vop_rmdir: gp 0x%x (%d), pgp 0x%x (%d) lock %d\n",
	gp, gp->g_number, ndp->ni_pdir, ndp->ni_pdir->g_number,
	ndp->ni_pdir->g_flag & GLOCKED);
#endif
	ret = GRMDIR(gp, ndp);
	return (u.u_error);
}

int
vop_readdir(gp, uiop, cred)
	struct gnode *gp;
	struct uio *uiop;
	struct ucred *cred;
{
	if (access(gp, GREAD)) {
		u.u_error = EPERM;
		return(EPERM);			/* XXX */
	}
	(void) GGETDIRENTS(gp, uiop, cred);
	return (u.u_error);
}

int
vop_brelse(gp, bp)
	struct gnode *gp;
	struct buf *bp;
{
	bp->b_resid = 0;
	brelse(bp);
}

int
vop_bread(gp, lbn, bpp)
	struct gnode *gp;
	daddr_t lbn;
	struct buf **bpp;
{
	register struct buf *bp;
	register daddr_t bn;
	register int size;

	size = blksize(FS(gp), gp, lbn);
	bn = GBMAP(gp, lbn, B_READ, 0, 0);

#ifdef NFSDEBUG
	dprint(nfsdebug, 10,
		"vop_bread: lbn = %d, size = %d, bn = %d, lastr = %d\n",
		lbn, size, bn, gp->g_lastr);
#endif

	if ((long)bn < 0) {
		bp = geteblk(size);
		clrbuf(bp);
	} else if (gp->g_lastr + 1 == lbn) {

#ifdef NFSDEBUG
	dprint(nfsdebug, 10,
		"vop_bread: call breada(), rablock = %d, rasize = %d\n",
		rablock, rasize);
#endif

		bp = breada(gp->g_dev, bn, size, rablock, rasize,
			(struct gnode *) 0);
	} else {

#ifdef NFSDEBUG
	dprint(nfsdebug, 10,
		"vop_bread: call bread()\n");
#endif

		bp = bread(gp->g_dev, bn, size, (struct gnode *) 0);
	}
	gp->g_lastr = lbn;
	gp->g_flag |= GACC;
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return (EIO);
	} else {
		*bpp = bp;
		return (0);
	}
}

/*
 * Perform chown operation on gnode gp.  This is similar to chown1()
 * in gfs_syscalls.c, but we leave some preliminary error checking and
 * the GUPDATE operation for the caller to do.  This routine should not
 * exist; please merge these someday.
 */

struct dquot *inoquota();

chown2(gp, uid, gid)
	register struct gnode *gp;
	register int uid, gid;
{
#ifdef QUOTA
	register long change;
#endif

	if (uid == -1)
		uid = gp->g_uid;
	if (gid == -1)
		gid = gp->g_gid;

#ifdef QUOTA
	if (gp->g_uid == uid)
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

#ifdef QUOTA
	gp->g_dquot = inoquota(gp);
	(void) chkdq(gp, change, 1);
	(void) chkiq(gp->g_dev, (struct gnode *)NULL, uid, 1);
	return (u.u_error);	
#else
	return(0);
#endif
}

gattr_to_nattr(gp, na)
	register struct gnode *gp;
	register struct nfsfattr *na;
{
	na->na_type = (enum nfsftype)IFTOVT(gp->g_mode);
	na->na_mode = gp->g_mode;
	na->na_uid = gp->g_uid;
	na->na_gid = gp->g_gid;
	na->na_fsid = gp->g_dev;
	na->na_nodeid = gp->g_number;
	na->na_nlink = gp->g_nlink;
	na->na_size = gp->g_size;
	na->na_atime = gp->g_atime;
	na->na_mtime = gp->g_mtime;
	na->na_ctime = gp->g_ctime;
	na->na_rdev = gp->g_rdev;
	na->na_blocks = gp->g_blocks;
/*	na->na_blocksize =  KLUDGE: CALLER MUST DO THIS */
}

nattr_to_gattr(gp, na)
	register struct gnode *gp;
	register struct nfsfattr *na;
{
	gp->g_mode = na->na_mode;
	gp->g_uid = na->na_uid;
	gp->g_gid = na->na_gid;
	gp->g_nlink = na->na_nlink; 
	gp->g_size = na->na_size; 
	gp->g_atime = na->na_atime;
	gp->g_mtime = na->na_mtime;
	gp->g_ctime = na->na_ctime; 
	gp->g_blocks = na->na_blocks;
}

