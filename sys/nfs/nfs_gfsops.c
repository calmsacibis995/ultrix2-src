#ifndef lint
static	char	*sccsid = "@(#)nfs_gfsops.c	1.23	(ULTRIX)	3/18/87";
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
 *	Portions of this software have been licensed to 
 *	Digital Equipment Company, Maynard, MA.
 *	Copyright (c) 1986 Sun Microsystems, Inc.  ALL RIGHTS RESERVED.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/user.h"
#include "../h/mount.h"
#include "../h/types.h"
#include "../h/fs_types.h"
#include "../h/trace.h"
#include "../h/uio.h"
#include "../h/file.h"
#include "../h/stat.h"
#include "../h/socketvar.h"
#include "../h/errno.h"
#include "../h/kernel.h"
#include "../h/trace.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../nfs/nfs.h"
#include "../h/mbuf.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/vnode.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif
#ifdef NFSDEBUG
extern int nfsdebug;
#endif

/*
 * GET_FILENAME leaves a pointer to the last component of a pathname
 * in cp.  It is used to patch up an awkwardness in the GFS mkdir
 * interface; GFS passes us the full pathname of the directory to
 * be created, but we must send only the last component over the wire.
 */
#define GET_FILENAME(pnamep, cp) \
	{ (cp) = &(pnamep)[strlen(pnamep)]; \
	  while (((cp) > (pnamep)) && (*(cp) != '/')) (cp)--; \
	  if (*(cp) == '/') (cp)++; \
	}

nfs_rwgp(gp, uiop, rw, ioflag, cred)
	register struct gnode *gp;
	register struct uio *uiop;
	enum uio_rw rw;
	int ioflag;
	struct ucred *cred;
{
	register int error = 0;
	register struct vnode *vp = (struct vnode *) gp;
	
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rwgp: %x %o %d %s %x %d\n",
	    vtomi(vp)->mi_addr.sin_addr.s_addr,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    rw == UIO_READ ? "READ" : "WRITE",
	    uiop->uio_iov->iov_base, uiop->uio_iov->iov_len);
#endif

	if ((gp->g_mode & GFMT) == GFDIR) {
		return(EISDIR);
	}

	if (rw == UIO_WRITE || (rw == UIO_READ && vtor(vp)->r_cred == NULL)) {
		crhold(cred);
		if (vtor(vp)->r_cred) {
			crfree(vtor(vp)->r_cred);
		}
		vtor(vp)->r_cred = cred;
	}

/*
	if ((ioflag & IO_APPEND) && rw == UIO_WRITE) {
		error = nfs_getattr(vp, cred);
		if (!error) {
			uiop->uio_offset = vp->g_size;
		}
	}
*/
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rwgp: vp 0x%x (%d) %s\n", vp, 
	((struct gnode *)vp)->g_number, (rw == UIO_WRITE) ? "WRITE" : "READ");
	dprint(nfsdebug, 4, "nfs_rwgp: base 0x%x len %d offset %d seg %s\n",
	uiop->uio_iov->iov_base, uiop->uio_iov->iov_len, uiop->uio_offset, 
	(uiop->uio_segflg == UIO_SYSSPACE) ? "SYSSPACE" : "USERSPACE");
#endif
	if (!error) {
		error = rwvp(vp, uiop, rw, ioflag);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rwgp returning %d\n", error);
#endif
	return (error);
}


nfs_select(gp, which, cred)
	struct gnode *gp;
	int	which;
	struct	ucred	*cred;
{
	return(EOPNOTSUPP);
}


nfs_glink(gp, ndp)
	register struct gnode *gp;
	register struct nameidata *ndp;
{
	u.u_error = nfs_link(gp, ndp->ni_pdir, ndp->ni_cp, u.u_cred);
	nfs_rele(gp);
	gput(ndp->ni_pdir);
	return(u.u_error);
}

int renamedebug = 0;

nfs_grename(gp, ssd, srcndp, tsd, targetndp, flag)
	register struct gnode *gp;
	struct gnode *ssd, *tsd;
	register struct	nameidata *srcndp, *targetndp;
{
	register struct gnode *tgp;
	register char *tcp, *scp;

	scp = srcndp->ni_cp;
	nfs_unlock(srcndp->ni_pdir);
	nfs_unlock(gp);

	if (!strcmp(scp, ".") || !strcmp(scp, "..")) {
		u.u_error = EINVAL;
		goto free_src;
	}

	tgp = GNAMEI(targetndp);
	if(u.u_error)
		goto free_src;
	tcp = targetndp->ni_cp;
	if(!strcmp(tcp, ".") || !strcmp(tcp, "..")) {
		u.u_error = EINVAL;
		goto free_targ;
	}

	if (gp->g_mp != targetndp->ni_pdir->g_mp) {
		u.u_error = EXDEV;
		goto free_targ;
	}

	if (ISREADONLY(gp->g_mp)) {
		u.u_error = EROFS;
		goto free_targ;
	}

#ifdef NFSDEBUG
if (renamedebug) {
	printf("nfs_grename: sndp 0x%x, tndp 0x%x\n", srcndp, targetndp);
	printf("nfs_grename: source 0x%x, target 0x%x\n", scp, tcp);
	printf("nfs_grename: source %s, target %s\n", scp, tcp);
	printf("nfs_grename: sp %d, tp %d\n", srcndp->ni_pdir->g_number,
			targetndp->ni_pdir->g_number);
}

	dprint(nfsdebug, 4, "nfs_grename: moving %s (pgp 0x%x) to %s (pdp 0x%x)\n",
	scp, srcndp->ni_pdir, tcp, targetndp->ni_pdir);
#endif
		
	u.u_error = nfs_rename(srcndp->ni_pdir, scp, targetndp->ni_pdir,
		tcp, u.u_cred);

free_targ:
	if(tgp)	gput(tgp);
	gput(targetndp->ni_pdir);

free_src:	
	nfs_rele(gp);
	nfs_rele(srcndp->ni_pdir);

	return(u.u_error);
}

struct gnode *
nfs_gmkdir(pgp, name, mode)
	register struct gnode *pgp;
	register char *name;
	register int	mode;
{
	struct vattr	va;
	struct gnode	*gp;
	register char	*cp;
	
	GET_FILENAME(name, cp);
	vattr_null(&va);
	va.va_type = VDIR;
	va.va_mode = (mode & 0777) & ~u.u_cmask;
	
	u.u_error = nfs_mkdir(pgp, cp, &va, &gp, u.u_cred);
	gput(pgp);
	return(u.u_error ? NULL : gp);
}


nfs_grmdir(gp, ndp)
	register struct gnode	*gp;
	register struct nameidata *ndp;
{
	if (gp == ndp->ni_pdir) {
		u.u_error = EINVAL;
		nfs_rele(gp);
		gput(ndp->ni_pdir);
		return(u.u_error);
	}
	if ((gp->g_mode & GFMT) != GFDIR)
		u.u_error = ENOTDIR;
	if (gp->g_dev != ndp->ni_pdir->g_dev)
		u.u_error = EBUSY;
	if (!u.u_error)
		u.u_error = nfs_rmdir(ndp->ni_pdir, ndp->ni_cp, u.u_cred);
	gput(gp);
	gput(ndp->ni_pdir);
	return(u.u_error);
}

nfs_umount(mp, force)
	register struct	mount	*mp;
	register int	force;		/* XXX ignored */
{
	register struct gnode *gp;
	register dev_t dev = mp->m_dev;
	register struct gnode *rgp = mp->m_rootgp;
	
	dnlc_purge();
	xumount(dev);
	gp = mp->m_gnodp;

#ifdef QUOTA
	if(gflush(dev, mp->m_qinod, rgp) < 0)
#else
	if(gflush(dev, rgp) < 0)
#endif
		return(EBUSY);

	nfs_rele(rgp);

#ifdef QUOTA
	closedq(mp);
	(void) gflush(dev, NULL, NULL);
#endif
	
	gp->g_flag &= ~GMOUNT;
	GRELE(gp);
	return(0);
}

nfs_sbupdate(mp)
	register struct mount *mp;
{
	return(0);
}


nfs_syncgp(gp, cred)
	register struct	gnode *gp;
	register struct	ucred	*cred;
{
	nfs_fsync(gp, cred);
	return(u.u_error);
}

nfs_getval(gp)
	register struct	gnode	*gp;
{
	return(u.u_error = nfs_getattr(gp, u.u_cred));
}


nfs_update(gp, atime, mtime, wait, cred)
	register struct gnode *gp;
	register struct timeval	*atime, *mtime;
	register int	wait;
	register struct	ucred	*cred;
{
	struct vattr	va;	
	extern int shutting_down;
	int error;
	
	if(shutting_down)
		return(NULL);

	vattr_null(&va);

	if (gp->g_flag & GCHG) {
		if(gp->g_flag & GCID) {
			va.va_uid = gp->g_uid;
			va.va_gid = gp->g_gid;
		}
		if(gp->g_flag & GCMODE)
			va.va_mode = gp->g_mode;

		gp->g_flag &= ~(GCID | GCMODE | GCLINK | GCHG);
	}

	if (gp->g_flag & GACC) {
		gp->g_flag &= ~GACC;
		va.va_atime.tv_sec = atime->tv_sec;
		va.va_atime.tv_usec = atime->tv_usec;
	}
	if (gp->g_flag & GUPD) {
		gp->g_flag &= ~GUPD;
		va.va_mtime.tv_sec = mtime->tv_sec;
		va.va_mtime.tv_usec = mtime->tv_usec;
	}

	error = nfs_setattr(gp, &va, cred);

	/*
	 * Kludge to get the old attributes back if the setattr fails.
	 */
	if (error) nfs_getattr(gp, cred);
	return(error);
}

nfs_trunc(gp,newsize, cred)
	register struct gnode *gp;
	register unsigned newsize;
	register struct	ucred *cred;
{
	struct vattr	va;

	vattr_null(&va);
	va.va_size = newsize;
	return(u.u_error = nfs_setattr(gp, &va, cred));
}


nfs_greadlink(gp,uio)
	register struct	gnode *gp;
	register struct	uio	*uio;
{
	return(nfs_readlink(gp, uio, u.u_cred));
}


nfs_stat(gp,sb)
	register struct gnode *gp;
	register struct stat *sb;
{

	if(u.u_error = nfs_getattr(gp, u.u_cred)) {
		bzero(sb, sizeof(struct stat));
		return(u.u_error);
	}

	sb->st_dev = gp->g_mp->m_dev;
	sb->st_ino = gp->g_number;
	sb->st_mode = gp->g_mode;
	sb->st_nlink = gp->g_nlink;
	sb->st_uid = gp->g_uid;
	sb->st_gid = gp->g_gid;
	sb->st_rdev = (dev_t)gp->g_rdev;
	sb->st_size = gp->g_size;
	sb->st_atime = gp->g_atime.tv_sec;
	sb->st_spare1 = gp->g_atime.tv_usec;
	sb->st_mtime = gp->g_mtime.tv_sec;
	sb->st_spare2 = gp->g_mtime.tv_usec;
	sb->st_ctime = gp->g_ctime.tv_sec;
	sb->st_spare3 = gp->g_ctime.tv_usec;
	sb->st_blksize = gp->g_mp->m_fs_data->fd_otsize;
	sb->st_blocks = gp->g_blocks;
	sb->st_spare4[0] = sb->st_spare4[1] = 0;
	return (0);
}

nfs_lock(gp) 
	register struct gnode *gp;
{
	if (gp->g_count <= 0)
		panic("nfs_lock: unrefed gnode");
	while (gp->g_flag & GLOCKED) {
		gp->g_flag |= GWANT;
		sleep((caddr_t)gp, PINOD);
	}
	gp->g_flag |= GLOCKED;
}

nfs_unlock(gp) 
	register struct gnode *gp;
{
	if (!(gp->g_flag & GLOCKED))
		panic("nfs_unlock: locked gnode isn't");
	gp->g_flag &= ~GLOCKED;
	if (gp->g_flag&GWANT) {
		gp->g_flag &= ~GWANT;
		wakeup((caddr_t)gp);
	}
}

nfs_rele(gp)
	register struct gnode *gp;
{
	struct vnode *vp;

#ifdef NFSDEBUG
	int *foo;
	dprint(nfsdebug, 4, "nfs_rele: gp 0x%x (%d) count %d ", gp,
		gp->g_number, gp->g_count - 1);
	dprint(nfsdebug, 4, "called by 0x%x\n", *(&foo + 5));
#endif
	if(gp->g_count <= 0)
		panic("nfs_rele: zero count");
	if(gp->g_count == 1)
		nfs_inactive(gp, u.u_cred);

	/*
	 * Check the count again before freeing up the gnode, in case
	 * another reference has been created (we may block in the
	 * nfs_inactive call).
	 * We have to worry about this because the gnode is still in the
	 * cache during the period that we are making it inactive.
	 */

	if (--gp->g_count == 0) {
		vp = (struct vnode *)gp;
		((struct mntinfo *)vp->v_vfsp->vfs_data)->mi_refct--;
		if (vtor(vp)->r_cred) {
			crfree(vtor(vp)->r_cred);
			vtor(vp)->r_cred = NULL;
		}
		freegnode(gp);
	}
}

nfs_unlink(gp, ndp)
	register struct gnode *gp;
	register struct nameidata *ndp;
{
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_unlink: trying to remove '%s' from gp 0x%x (%d)\n",
	ndp->ni_cp, ndp->ni_pdir, ndp->ni_pdir->g_number);
#endif
	u.u_error = nfs_remove(ndp->ni_pdir, gp, ndp->ni_cp, u.u_cred);
	return(u.u_error);
}


struct fs_data *
nfs_getfsdata(mp)
	register struct mount *mp;
{
	register struct fs_data *fsdata = mp->m_fs_data;
	struct statfs statfs;

#ifdef GFSDEBUG
	if(GFS[14])
		printf("nfs_getfsdata: mp 0x%x\n", mp);
#endif
	u.u_error = nfs_statfs(mp, &statfs);

	if (!u.u_error) {
		fsdata->fd_fstype = GT_NFS;
		fsdata->fd_gtot = 0;
		fsdata->fd_gfree = 0;
		fsdata->fd_btot = statfs.f_blocks;
		fsdata->fd_bfree = statfs.f_bfree;
		fsdata->fd_bfreen = statfs.f_bavail;
		fsdata->fd_mtsize =
			fsdata->fd_otsize =
				MP_TO_MIP(mp)->mi_stsize;
	}
	
	return(fsdata);
}


nfs_gsymlink(ndp, to)
	register struct nameidata *ndp;
	register char *to;
{
	struct vattr va;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_symlink: creating a link with name '%s' parent 0x%x (%d)\n",
	ndp->ni_cp, ndp->ni_pdir, ndp->ni_pdir->g_number);
#endif

	vattr_null(&va);
	va.va_mode = 0777;
	nfs_unlock(ndp->ni_pdir);
	u.u_error = nfs_symlink(ndp->ni_pdir, ndp->ni_cp, &va, to, u.u_cred);
	nfs_rele(ndp->ni_pdir);
	return(u.u_error);
}


struct gnode *
nfs_makenode(mode, ndp)
	register int mode;
	register struct nameidata *ndp;
{
	struct vattr _va;
	register struct vattr *va = &_va;
	struct gnode *vpp;
	extern struct timeval time;
	
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_makenode: trying to make '%s' mode 0%o\n",
	ndp->ni_cp, mode);
#endif
	/*
	 * There are assumptions in the underlying code that only
	 * regular files may be made.  If (when) this is no longer
	 * true, va_type and va_rdev will be forced to change.
	 */

	if ((mode & GFMT) != GFREG) {
		u.u_error = EINVAL;
		goto bad;
	}

	vattr_null(va);
	va->va_type = VREG;	
	va->va_mode = (mode & ~u.u_cmask) & 0xffff; 
	va->va_size = 0;

	u.u_error = nfs_create(ndp->ni_pdir, ndp->ni_cp, va, NONEXCL, mode, &vpp, u.u_cred);

bad:
	gput(ndp->ni_pdir);
	if (u.u_error)
		return(NULL);
	else {
		nfs_lock(vpp);
		return(vpp);
	}
}

int nfsnameidebug = 0;

/*
 * The routine nfs_namei performs pathname lookup in remote filesystems,
 * with side effects.
 *
 * Unlike the ufs namei, we leave a pointer to the last pathname component
 * in ndp->ni_cp.  Other NFS code depends on this.
 */ 

struct gnode *
nfs_namei(ndp)
	register struct nameidata *ndp;
{
	struct gnode *gpp, *gp;
	register char *slash, *ptr, *cp, *ncp;
	int lockparent, flag, i;
	char name[NFS_MAXNAMLEN+1];
	struct uio _auio;
	register struct uio *auio = &_auio;
	struct iovec _aiov;
	register struct iovec *aiov = &_aiov;
	struct mount *mpp;
	
#ifdef NFSDEBUG
	if (nfsnameidebug)
	printf("Entering nfs_namei to look up %s\n", ndp->ni_cp);
#endif

	flag = ndp->ni_nameiop & (LOOKUP | CREATE | DELETE);
	lockparent = ndp->ni_nameiop & LOCKPARENT;
	gp = ndp->ni_pdir;
	nfs_unlock(gp);

#ifdef NFSDEBUG
	if (gp == NULL) panic("nfs_namei: no parent");
	if ((flag != LOOKUP) && (flag != CREATE) && (flag != DELETE))
		panic("nfs_namei: bad flags");
#endif NFSDEBUG
	
	cp = ndp->ni_cp;
	while (*cp == '/')
		cp++;

	while(*cp) {

		/*
		 * Find the next pathname component, move it into name[].
		 */

		for (i = 0; cp[i] != 0 && cp[i] != '/'; i++) {
			if (i == NFS_MAXNAMLEN)
				u.u_error = ENAMETOOLONG;
			if (cp[i] & 0200)
				u.u_error = EINVAL;
			if (u.u_error) {
				nfs_rele(gp);
				return(NULL);
			}
			name[i] = cp[i];
		}

		name[i] = '\0';
		ncp = cp + i;

		/*
		 * If we're at the root of a filesystem and the next
		 * component is ".." then we just bounce back to caller.
		 */

		if((name[0] == '.') && (name[1] == '.') && !name[2]
		&& (gp == gp->g_mp->m_rootgp)) {
			nfs_rele(gp);
			gp = gp->g_mp->m_gnodp;
			gfs_lock(gp);
			gp->g_count++;
			gp->g_flag |= GINCOMPLETE;
			ndp->ni_pdir = gp;
			ndp->ni_cp = cp;
			return(gp);
		}
		
		/*
		 * Now look up the current pathname component.
		 */

#ifdef NFSDEBUG
		if (nfsnameidebug)
		printf("nfs_namei: about to do an nfs_lookup for \"%s\"\n",
			name);
#endif NFSDEBUG

		u.u_error = nfs_lookup(gp, name, &gpp, u.u_cred);

#ifdef NFSDEBUG
if (nfsnameidebug) {
	if (u.u_error)
		printf ("nfs_namei: nfs_lookup error return %d\n", u.u_error);
	if (gpp)
	printf("nfs_namei: nfs_lookup returned #%d\n", (gpp)->g_number);
	else printf("nfs_namei: nfs_lookup returned null\n");
}
#endif NFSDEBUG

		/*
		 * Take appropriate action if the lookup fails.
		 * Negate the error if the file doesn't exist and
		 * the operation is create.
		 */

		if (u.u_error) {
			if ((flag == CREATE) && (cp[i] == '\0') &&
			(u.u_error == ENOENT) && !access(gp, GWRITE)) {
				u.u_error = 0;
				nfs_lock(gp);
				ndp->ni_pdir = gp;
				ndp->ni_cp = cp;
			}
			else {
				nfs_rele(gp);
			}
			return(NULL);
		}

		/*
		 * The lookup has succeeded.  If we've hit a mount point
		 * traverse it if it's type NFS, else bounce back to
		 * caller.
		 *
		 * XXX Should check that unmount is not in progress on fs.
		 */

		if (gpp->g_flag & GMOUNT) {
			mpp = gpp->g_mpp;
			if (mpp->m_fstype != GT_NFS) {
				nfs_rele(gp);
				nfs_rele(gpp);
				gp = mpp->m_rootgp;
				gfs_lock(gp);
				gp->g_count++;
				ndp->ni_cp = ncp;
				ndp->ni_pdir = gp;
				gp->g_flag |= GINCOMPLETE;
				return(gp);
			}
			else {
				nfs_rele(gp);
				gp = gpp;
				gpp = mpp->m_rootgp;
				gpp->g_count++;
			}
		}

		/*
		 * Check for symbolic links
		 */
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: check for slink mode 0x%x GFLNK 0x%x flag 0x%x\n",
	gpp->g_mode&GFMT, GFLNK, ndp->ni_nameiop&FOLLOW);
#endif NFSDEBUG	
		if ((gpp->g_mode & GFMT) == GFLNK  &&
			((ndp->ni_nameiop & FOLLOW) || *ncp == '/')) {
			u_int pathlen = strlen(ncp) + 1;

			if (gpp->g_size + pathlen >= MAXPATHLEN - 1) {
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: slink name too long\n");
#endif NFSDEBUG
				u.u_error = ENAMETOOLONG;
				nfs_rele(gp);
				nfs_rele(gpp);
				return (NULL);
			}

			if (++ndp->ni_slcnt > MAXSYMLINKS) {
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: too many slinks\n");
#endif NFSDEBUG
				u.u_error = ELOOP;
				nfs_rele(gp);
				nfs_rele(gpp);
				return (NULL);
			}

			aiov->iov_base = ndp->ni_dirp;
			aiov->iov_len = auio->uio_resid = gpp->g_size;
			auio->uio_iov = aiov;
			auio->uio_iovcnt = 1;
			auio->uio_segflg = UIO_SYSSPACE;
			auio->uio_offset = 0;

			ovbcopy(ncp, ndp->ni_dirp + gpp->g_size, pathlen);
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: call nfs_readlink\n");
#endif NFSDEBUG

			u.u_error = nfs_readlink((struct vnode *)gpp,
				auio, u.u_cred);
			if (u.u_error) {
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: nfs_readlink returned error\n");
#endif NFSDEBUG
				nfs_rele(gp);
				nfs_rele(gpp);
				return (NULL);
			}

			cp = ndp->ni_dirp;
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: pathname is now \"%s\"\n", cp);
#endif NFSDEBUG
			nfs_rele(gpp);
			if (*cp == '/') {
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: slink name starts with slash\n");
#endif NFSDEBUG
				nfs_rele(gp);
				while (*cp == '/')
					cp++;
				if ((gp = u.u_rdir) == NULL)
					gp = rootdir;
				nfs_lock(gp);
				gp->g_count++;
				ndp->ni_cp = cp;
				ndp->ni_pdir = gp;
				gp->g_flag |= GINCOMPLETE;
				return(gp);
			}
			else {
#ifdef NFSDEBUG
if (nfsnameidebug)
printf("nfs_namei: slink name does not start with slash\n");
#endif NFSDEBUG
				continue;
			}
		}

		/*
		 * The lookup has succeeded and we haven't hit a mount
		 * point.  Handle the case where this is the last component
		 * of the pathname.
		 */

		while (*ncp == '/')
			ncp++;
		if (*ncp == '\0') {
			if (ndp->ni_nameiop & NOCACHE)
				dnlc_purge_vp(gpp);
			if ((flag == DELETE) && access(gp, GWRITE))  {
				nfs_rele(gp);
				nfs_rele(gpp);
				return(NULL);
			}
			ndp->ni_cp = cp;
			if (lockparent) {
				if (gp != gpp)
					nfs_lock(gp);
				ndp->ni_pdir = gp;
			}
			else {
				nfs_rele(gp);
			}
			nfs_lock(gpp);
			gpp->g_flag &= ~GINCOMPLETE;
			return(gpp);
		}

		/*
		 * The lookup has succeeded, but this is not the last
		 * component of the pathname.
		 */

		nfs_rele(gp);
		gp = gpp;
		cp = ncp;
	}

	/*
	 * "Temporary" hack for handling null pathnames, which denote the
	 * starting directory by convention.  If the path is null and the
	 * operation is a create or delete we return EISDIR for hysterical
	 * reasons.  Note: we can also get here if the pathname resolves
	 * to an NFS mount point.
	 */
	if (flag != LOOKUP) {
		if (gp == gp->g_mp->m_rootgp) {
			if (flag == CREATE)
				u.u_error = EEXIST;
			else
				u.u_error = EBUSY;
		} else
			u.u_error = EISDIR;
		nfs_rele(gp);
		return(NULL);
	}

	nfs_lock(gp);
	gp->g_flag &= ~GINCOMPLETE;
	return(gp);
}
