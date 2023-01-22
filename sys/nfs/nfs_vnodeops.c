#ifndef lint
static	char	*sccsid = "@(#)nfs_vnodeops.c	1.21	(ULTRIX)	3/18/87";
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
#include "../h/mount.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/buf.h"
#include "../h/kernel.h"
#include "../h/cmap.h"
#include "../h/proc.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/xdr.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/vnode.h"
#include "../h/fs_types.h"

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct vnode *makenfsnode();
struct vnode *dnlc_lookup();
char *newname();

#define check_stale_fh(errno, vp) if ((errno) == ESTALE) { dnlc_purge_vp(vp); }
#define	nfsattr_inval(vp)	(vtor(vp)->r_nfsattrtime.tv_sec = 0)

#define ISVDEV(t) ((t == VBLK) || (t == VCHR) || (t == VFIFO))

#define R_IN_FSYNC 0x40

/*
 * These are the vnode ops routines which implement the vnode interface to
 * the networked file system.  These routines just take their parameters,
 * make them look networkish by putting the right info into interface structs,
 * and then calling the appropriate remote routine(s) to do the work.
 *
 * Note on directory name lookup cacheing:  we desire that all operations
 * on a given client machine come out the same with or without the cache.
 * This is the same property we have with the disk buffer cache.  In order
 * to guarantee this, we serialize all operations on a given directory,
 * by using lock and unlock around rfscalls to the server.  This way,
 * we cannot get into races with ourself that would cause invalid information
 * in the cache.  Other clients (or the server itself) can cause our
 * cached information to become invalid, the same as with data buffers.
 * Also, if we do detect a stale fhandle, we purge the directory cache
 * relative to that vnode.  This way, the user won't get burned by the
 * cache repeatedly.
 */

int nfs_open();
int nfs_close();
int nfs_rdwr();
int nfs_ioctl();
int nfs_select();
int nfs_getattr();
int nfs_setattr();
int nfs_access();
int nfs_lookup();
int nfs_create();
int nfs_remove();
int nfs_link();
int nfs_rename();
int nfs_mkdir();
int nfs_rmdir();
int nfs_readdir();
int nfs_symlink();
int nfs_fsync();
int nfs_readlink();
int nfs_inactive();
int nfs_bmap();
int nfs_strategy();
int nfs_badop();

int
nfs_open(vp, flag)
	register struct	vnode *vp;
	int flag;
{
	register struct ucred *cred = u.u_cred;
	register int error;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_open %s %o %d flag %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    flag);
#endif
	/*
	 * refresh cached attributes
	 */
	nfsattr_inval(vp);
	error = nfs_getattr(vp, cred);
	if (!error) {
		vtor(vp)->r_flags |= ROPEN;
	}

	return (error);
}


nfs_close(vp, flag)
	register struct	vnode *vp;
	int	flag;
{
	register struct ucred *cred = u.u_cred;
	register struct rnode *rp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_close %s %o %d flag %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    flag);
#endif
	rp = vtor(vp);
	rp->r_flags &= ~ROPEN;

	/*
	 * If this is a close of a file open for writing or an unlinked
	 * open file or a file that has had an asynchronous write error,
	 * flush synchronously. This allows us to invalidate the file's
	 * buffers if there was a write error or the file was unlinked.
	 */
	if (flag & FWRITE || rp->r_unldvp != NULL ||
		rp->r_error) {
		(void) nfs_fsync(vp, cred);
	}
	if (rp->r_unldvp != NULL || rp->r_error) {
		binval((dev_t)(-1), (struct gnode *)vp);
		dnlc_purge_vp(vp);
	}

	return (flag & FWRITE? rp->r_error: 0);

}

int read_incore = 0;
int read_not_incore = 0;
int bread1 = 0;
int breada1 = 0;
int bread2 = 0;
int rfsread_count = 0;

int residdebug = 0;

int
rwvp(vp, uio, rw, ioflag)
	register struct vnode *vp;
	register struct uio *uio;
	enum uio_rw rw;
	int ioflag;
{
	struct buf *bp;
	daddr_t bn;
	register int n, on;
	int size;
	int error = 0;
	daddr_t mapped_bn, mapped_rabn;
	int eof = 0;

	if (uio->uio_resid == 0) {
		return (0);
	}
	if (uio->uio_offset < 0 || (uio->uio_offset + uio->uio_resid) < 0) {
		return (EINVAL);
	}
	if (rw == UIO_WRITE && vp->v_type == VREG &&
	    uio->uio_offset + uio->uio_resid >
	      u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		return (EFBIG);
	}
	size = vtoblksz(vp);
	size &= ~(DEV_BSIZE - 1);
	if (size <= 0) {
		panic("rwvp: zero size");
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rwvp: starting io transfer\n");
#endif
	do {
		bn = uio->uio_offset / size;
		on = uio->uio_offset % size;
		n = MIN((unsigned)(size - on), uio->uio_resid);
		nfs_bmap(vp, bn, &mapped_bn);
		if (rw == UIO_READ) {
			int diff;

			if ((long) bn < 0) {
				bp = geteblk(size);
				bp->b_gp = (struct gnode *) vp;
				clrbuf(bp);
			} else {
				if (incore(vp->g_dev, mapped_bn, vp)) {

					/*
					 * get attributes to check whether in
					 * core data is stale
					 */

					++read_incore;
					(void) nfs_getattr(vp, u.u_cred);
				}
				else
					++read_not_incore;

				if (vp->g_lastr + 1 == bn) {
					nfs_bmap(vp, bn + 1, &mapped_rabn);
					++breada1;
					bp = breada(vp->g_dev, mapped_bn,
					size, mapped_rabn, size, vp);
				} else {
					++bread1;
					bp = bread(vp->g_dev, mapped_bn,
					size, vp);
				}
			}
			vp->g_lastr = bn;
			diff = vp->g_size - uio->uio_offset;
			if (diff < n) {
				if (diff <= 0) {
					brelse(bp);
					return(0);
				}
				n = diff;
				eof = 1;
			}
		} else {

			struct gnode *gp = (struct gnode *) (vp);

			if (vtor(vp)->r_error) {
				error = vtor(vp)->r_error;
				goto bad;
			}

			if (gp->g_textp) xuntext(gp->g_textp);
			if (n == size) {
				bp = getblk(vp->g_dev, mapped_bn, size, vp);
			} else {
				++bread2;
				bp = bread(vp->g_dev, mapped_bn, size, vp);
			}
		}
		if (bp->b_flags & B_ERROR) {
			error = geterror(bp);
			brelse(bp);
			goto bad;
		}
		u.u_error = uiomove(bp->b_un.b_addr+on, n, rw, uio);
		if (rw == UIO_READ) {
			brelse(bp);
		} else {
			/*
			 * g_size is the maximum number of bytes known
			 * to be in the file.
			 * Make sure it is at least as high as the last
			 * byte we just wrote into the buffer.
			 */
			if (vp->g_size < uio->uio_offset) {
				vp->g_size = uio->uio_offset;
			}
			vtor(vp)->r_flags |= RDIRTY;
			if (n + on == size) {
				bp->b_resid = 0;
				if (!(ioflag & IO_SYNC))
					bp->b_flags |= B_ASYNC;
				bwrite(bp);
			} else {
				/*
				 * b_resid is the number of bytes in the
				 * buffer that are NOT part of the file.
				 * The bp->b_resid field is the number of
				 * bytes in the buffer that are NOT part of
				 * the file. We first compute how many
				 * bytes beyond the end of the file the
				 * last byte in the buffer is (think
				 * about it). If the file continues past
				 * the end of the block then this value
				 * will be negative, and we set it to zero
				 * since all the bytes in the buffer are
				 * valid.  The result is used by nfswrite
				 * to decide how many bytes to send to the
				 * server.
				 */
				bp->b_resid = (size * (bn + 1)) - vp->g_size;
				if (bp->b_resid < 0)
					bp->b_resid = 0;
				if (ioflag & IO_SYNC)
					bwrite(bp);
				else
					bdwrite(bp, vp);
			}
		}
	} while (u.u_error == 0 && uio->uio_resid > 0 && !eof);

	if (rw == UIO_WRITE && uio->uio_resid && u.u_error == 0) {
		printf("rwvp: short write. resid %d vp %x bn %d\n",
		    uio->uio_resid, vp, bn);
	}

	if (error == 0)				/* XXX */
		error = u.u_error;		/* XXX */
bad:

	return (error);
}

/*
 * Write to a remote file.
 * Writes to remote server in largest size chunks that the server can
 * handle.  Write is synchronous from the client's point of view.
 */
nfswrite(vp, base, offset, count, cred)
	struct vnode *vp;
	caddr_t base;
	int offset;
	int count;
	struct ucred *cred;
{
	int error;
	struct nfswriteargs wa;
	struct nfsattrstat *ns;
	int tsize;

	if (cred == NULL)
		printf("nfswrite: NULL cred\n");

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfswrite %s %o %d off = %d, ct = %d, cred 0x%x\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    offset, count, cred);
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	do {
		tsize = min(vtomi(vp)->mi_stsize, count);
		wa.wa_data = base;
		wa.wa_fhandle = *vtofh(vp);
		wa.wa_begoff = offset;
		wa.wa_totcount = tsize;
		wa.wa_count = tsize;
		wa.wa_offset = offset;
		error = rfscall(vtomi(vp), RFS_WRITE, xdr_writeargs, (caddr_t)&wa,
			xdr_attrstat, (caddr_t)ns, cred);
		if (!error) {
			error = geterrno(ns->ns_status);
			check_stale_fh(error, vp);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "nfswrite: sent %d of %d, error %d\n",
		    tsize, count, error);
#endif
		count -= tsize;
		base += tsize;
		offset += tsize;
	} while (!error && count);

	if (!error) {
		nfs_attrcache(vp, &ns->ns_attr, NOFLUSH);
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
	switch (error) {
	case 0:
	case EDQUOT:
		break;

	case ENOSPC:
		printf("NFS write error, server %s,  remote file system full\n",
		   vtomi(vp)->mi_hostname );
		break;

	default:
	printf("NFS write error %d, server %s, fs(%d, %d), file %d\n",
		    error, vtomi(vp)->mi_hostname,
		    major(vtofh(vp)->fh_fsid),
		    minor(vtofh(vp)->fh_fsid),
		    vtofh(vp)->fh_fno);
		break;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfswrite: returning %d\n", error);
#endif

	return (error);
}

/*
 * Read from a remote file.
 * Reads data in largest chunks our interface can handle
 */
nfsread(vp, base, offset, count, residp, cred)
	struct vnode *vp;
	caddr_t base;
	int offset;
	int count;
	int *residp;
	struct ucred *cred;
{
	int error;
	struct nfsreadargs ra;
	struct nfsrdresult rr;
	register int tsize;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfsread %s %o %d offset = %d, totcount = %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    offset, count);
#endif NFSDEBUG
	do {
		tsize = min(vtomi(vp)->mi_tsize, count);
		rr.rr_data = base;
		ra.ra_fhandle = *vtofh(vp);
		ra.ra_offset = offset;
		ra.ra_totcount = tsize;
		ra.ra_count = tsize;
		++rfsread_count;
		error = rfscall(vtomi(vp), RFS_READ, xdr_readargs, (caddr_t)&ra,
			xdr_rdresult, (caddr_t)&rr, cred);
		if (!error) {
			error = geterrno(rr.rr_status);
			check_stale_fh(error, vp);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "nfsread: got %d of %d, error %d\n",
		    tsize, count, error);
#endif
		if (!error) {
			count -= rr.rr_count;
			base += rr.rr_count;
			offset += rr.rr_count;
		}
	} while (!error && count && rr.rr_count == tsize);

	*residp = count;

	if (!error) {
		nfs_attrcache(vp, &rr.rr_attr, SFLUSH);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfsread: returning %d, resid %d\n",
		error, *residp);
#endif
	return (error);
}

/*
 * Timeout values for attributes for
 * regular files, and for directories.
 */
int nfsac_regtimeo_min = 3;
int nfsac_regtimeo_max = 60;
int nfsac_dirtimeo_min = 30;
int nfsac_dirtimeo_max = 60;

nfs_attrcache(vp, na, fflag)
	struct vnode *vp;
	struct nfsfattr *na;
	enum staleflush fflag;
{
	register int delta;
	register struct rnode *rp = vtor(vp);
	int oldsize;

	/*
	 * check the new modify time against the old modify time
	 * to see if cached data is stale
	 */
	if (na->na_mtime.tv_sec != vp->g_mtime.tv_sec ||
	    na->na_mtime.tv_usec != vp->g_mtime.tv_usec) {

		/*
		 * The file has changed. If this was unexpected (SFLUSH)
		 * flush the delayed write blocks associated with this
		 * vnode from the buffer cache and mark the cached blocks
		 * on the free list as invalid. Also flush the page cache.
		 * If this is a text mark it invalid so that the next
		 * pagein from the file will fail. If the vnode is a
		 * directory, purge the directory name lookup cache.
		 */

		if (fflag == SFLUSH) {
			if ((vp->v_flag & VTEXT) == 0)
				mpurge(vp);
			binval((dev_t)(-1), (struct gnode *)vp);
		}

		if (vp->v_type == VDIR) {
			dnlc_purge_vp(vp);
		}
	}

	/*
	 * Copy the new attributes into the gnode and timestamp them.
	 * There is some weirdness with the gnode size here.  We must
	 * keep the old size iff the file is dirty and the old size is
	 * greater than the new size, since we may have writes buffered
	 * that will make the file grow.
	 */

	oldsize = vp->g_size;
	nattr_to_gattr(vp, na);
	if (oldsize > vp->g_size &&
	    ((rp->r_flags & RDIRTY) || (rp->r_flags & R_IN_FSYNC)))
		vp->g_size = oldsize;
	rp->r_nfsattrtime = time;

	/*
	 * Delta is the number of seconds that we will cache
	 * attributes of the file.  It is based on the number of seconds
	 * since the last change (i.e. files that changed recently
	 * are likely to change soon), but there is a minimum and
	 * a maximum for regular files and for directories.
	 */

	delta = (time.tv_sec - na->na_mtime.tv_sec) >> 4;
	if (vp->v_type == VDIR) {
		if (delta < nfsac_dirtimeo_min) {
			delta = nfsac_dirtimeo_min;
		} else if (delta > nfsac_dirtimeo_max) {
			delta = nfsac_dirtimeo_max;
		}
	} else {
		if (delta < nfsac_regtimeo_min) {
			delta = nfsac_regtimeo_min;
		} else if (delta > nfsac_regtimeo_max) {
			delta = nfsac_regtimeo_max;
		}
	}
	rp->r_nfsattrtime.tv_sec += delta;
}

int attr_cache = 1;

int
nfs_getattr(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	int error;
	struct nfsattrstat *ns;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_getattr %s %d %o\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
	dprint(nfsdebug, 4, "nfs_getattr rootvp 0x%x tsize %d\n",
	vtomi(vp)->mi_rootvp, vtomi(vp)->mi_tsize);
	if(vp->g_count == 0) {
		cprintf("nfs_getattr: zero count on 0x%x (%d)\n", vp, 
		vp->g_number);
		panic("nfs_getattr");
	}
#endif NFSDEBUG

	nfs_fsync(vp, cred);

if (attr_cache) {
	if((time.tv_sec < vtor(vp)->r_nfsattrtime.tv_sec) ||
	   ((time.tv_sec == vtor(vp)->r_nfsattrtime.tv_sec) && 
	    (time.tv_usec < vtor(vp)->r_nfsattrtime.tv_usec))) {

#ifdef NFSDEBUG
		dprint(nfsdebug, 5, "nfs_getattr: vp 0x%x (%d) attributes cache\n",
		vp, vp->g_number);
#endif
		/*
		 * Use cached attributes.
		 */
		return (0);
	}
}
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	error = rfscall(vtomi(vp), RFS_GETATTR, xdr_fhandle, (caddr_t)vtofh(vp),
	    xdr_attrstat, (caddr_t)ns, cred);
	if (!error) {
		error = geterrno(ns->ns_status);
		if (!error) {
			nfs_attrcache(vp, &ns->ns_attr, SFLUSH);
		}
		else {
			check_stale_fh(error, vp);
		}
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_getattr: returns %d\n", error);
#endif
	return (error);
}

int
nfs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;
	struct nfssaargs args;
	struct nfsattrstat *ns;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_setattr %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_blocks != -1) ||
	    (vap->va_ctime.tv_sec != -1) || (vap->va_ctime.tv_usec != -1)) {
		error = EINVAL;
	} else {

		nfs_fsync(vp, cred);
		if (vap->va_size != -1) {
			vp->g_size = vap->va_size;
		}
		vattr_to_sattr(vap, &args.saa_sa);
		args.saa_fh = *vtofh(vp);
		error = rfscall(vtomi(vp), RFS_SETATTR, xdr_saargs,
		    (caddr_t)&args, xdr_attrstat, (caddr_t)ns, cred);
		if (!error) {
			error = geterrno(ns->ns_status);
			if (!error) {
				nfs_attrcache(vp, &ns->ns_attr, SFLUSH);
			}
			else {
				check_stale_fh(error, vp);
			}
		}
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_setattr: returning %d\n", error);
#endif
	return (error);
}

int
nfs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	int *gp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_access %s %o %d mode %d uid %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    mode, cred->cr_uid);
#endif NFSDEBUG

	u.u_error = nfs_getattr(vp, cred);
	if (u.u_error) {
		return (u.u_error);
	}
	/*
	 * If you're the super-user, you always get access.
	 * Actually that's not true; you almost never get access.
	 * Due to a protocol bug we don't have enough information to
	 * decide at this point, so we have to try anyway.
	 */
	if (cred->cr_uid == 0)
		return (0);		/* <=== BUG */
	/*
	 * Access check is based on only
	 * one of owner, group, public.
	 * If not owner, then check group.
	 * If not a member of the group, then
	 * check public access.
	 */
	if (cred->cr_uid != vp->g_uid) {
		mode >>= 3;
		if (cred->cr_gid == vp->g_gid)
			goto found;
		gp = cred->cr_groups;
		for (; gp < &cred->cr_groups[NGROUPS] && *gp != NOGROUP; gp++)
			if (vp->g_gid == *gp)
				goto found;
		mode >>= 3;
	}
found:
	if ((vp->g_mode & mode) == mode) {
		return (0);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_access: returning %d\n", u.u_error);
#endif NFSDEBUG
	u.u_error = EACCES;
	return (EACCES);
}

int
nfs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	int error;
	struct nfsrdlnres rl;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_readlink %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif NFSDEBUG

	if(vp->v_type != VLNK)
		return (ENXIO);
	rl.rl_data = (char *)kmem_alloc((u_int)NFS_MAXPATHLEN);
	error =
	    rfscall(vtomi(vp), RFS_READLINK, xdr_fhandle, (caddr_t)vtofh(vp),
	       xdr_rdlnres, (caddr_t)&rl, cred);
	if (!error) {
		error = geterrno(rl.rl_status);
		if (!error) {
			error = uiomove(rl.rl_data, (int)rl.rl_count,
			    UIO_READ, uiop);
		}
		else {
			check_stale_fh(error, vp);
		}
	}
	kmem_free((caddr_t)rl.rl_data, (u_int)NFS_MAXPATHLEN);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_readlink: returning %d\n", error);
#endif NFSDEBUG
	return (error);
}

/*ARGSUSED*/
int
nfs_fsync(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	register struct rnode *rp;
	register int offset, blksize;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_fsync %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
#endif
	rp = vtor(vp);
	if (rp->r_flags & RDIRTY) {
		rp->r_flags |= R_IN_FSYNC;
		rp->r_flags &= ~RDIRTY;	
		bflushgp(vp->g_dev, vp); /* start delayed writes */
		blksize = vtoblksz(vp);
		for (offset = 0; offset < vp->g_size; offset += blksize) {
			blkflush(vp->g_dev, (daddr_t)(offset / DEV_BSIZE),
				 (long)blksize, vp);
		}
		rp->r_flags &= ~R_IN_FSYNC;
	}

	return (rp->r_error);
}

/*
 * Make an NFS gnode inactive.
 * Weirdness: if the file was removed while it was open it got
 * renamed (by nfs_remove) instead.  Here we remove the renamed
 * file.  Note: the gnode must be in a consistent state when this
 * routine is called, since we may block in rfscall.
 */
/*ARGSUSED*/
int
nfs_inactive(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_inactive %s %o %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno);
	dprint(nfsdebug, 4, "nfs_inactive: gp 0x%x (%d)\n", vp,
		 vp->g_number);
#endif NFSDEBUG
	if (vtor(vp)->r_unlname != NULL) {
		setdiropargs(&da, vtor(vp)->r_unlname, vtor(vp)->r_unldvp);
		error = rfscall(vtomi(vtor(vp)->r_unldvp), RFS_REMOVE,
		    xdr_diropargs, (caddr_t)&da,
		    xdr_enum, (caddr_t)&status, vtor(vp)->r_unlcred);
		if (!error) {
			error = geterrno(status);
		}

#ifdef NFSDEBUG
		dprint(nfsdebug, 4, "nfs_inactive: vp 0x%x (%d) free unlname 0x%x\n",
		vp, vp->g_number, vtor(vp)->r_unlname);
#endif NFSDEBUG

		VN_RELE(vtor(vp)->r_unldvp);
		kmem_free((caddr_t)vtor(vp)->r_unlname, (u_int)NFS_MAXNAMLEN);
		crfree(vtor(vp)->r_unlcred);

		vtor(vp)->r_unldvp = NULL;
		vtor(vp)->r_unlname = NULL;
		vtor(vp)->r_unlcred = NULL;
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_inactive done\n");
#endif
	return (0);
}

extern int cachedebug;

/*
 * Remote file system operations having to do with directory manipulation.
 */

nfs_lookup(dvp, nm, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	struct nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_lookup %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno,
	    nm);
#endif NFSDEBUG

#ifdef NFSDEBUG
	if(dvp->g_mp->m_fstype != GT_NFS) {
		cprintf("nfs_lookup: dvp 0x%x (%d)\n", dvp, dvp->g_number);
		panic("remote directory isn't");
	}
#endif
	/*
	 * Before checking dnlc, call getattr to be
	 * sure directory hasn't changed.  getattr
	 * will purge dnlc if a change has occurred.
	 */
	if (error = nfs_getattr(dvp, cred)) {
		*vpp = (struct vnode *)0;
		return (error);
	}
	*vpp = (struct vnode *) dnlc_lookup(dvp, nm, cred);
	if (*vpp) {
if (cachedebug) {
printf("\nnfs_lookup: found gnode #%d in dnlc cache\n",(*vpp)->g_number);
printf("nfs_lookup: it had a use count of %d\n", (*vpp)->g_count);
}

#ifdef NFSDEBUG
		if((*vpp)->g_count == 0)
			panic("found free gnode in dnlc cache");
#endif NFSDEBUG
		VN_HOLD(*vpp);
		return (0);
	}
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&da, nm, dvp);
	error = rfscall(vtomi(dvp), RFS_LOOKUP, xdr_diropargs, (caddr_t)&da,
	    xdr_diropres, (caddr_t)dr, cred);
	if (!error) {
		error = geterrno(dr->dr_status);
		check_stale_fh(error, dvp);
	}
	
	if (!error) {
		*vpp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, dvp->v_vfsp,
		((struct gnode *) dvp)->g_mp);
		if(*vpp == NULL)
			error = u.u_error;
		else if ((enum vtype) dr->dr_attr.na_type == VDIR)
			dnlc_enter(dvp, nm, *vpp, cred);
	} else {
		*vpp = (struct vnode *)0;
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
	return (error);
}

/*ARGSUSED*/
nfs_create(dvp, nm, va, exclusive, mode, vpp, cred)
	struct vnode *dvp;
	char *nm;
	struct vattr *va;
	enum vcexcl exclusive;
	int mode;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "nfs_create: in dvp size %d\n", dvp->g_size);
	dprint(nfsdebug, 4, "nfs_create %s %o %d '%s' excl=%d, mode=%o\n",
		vtomi(dvp)->mi_hostname, vtofh(dvp)->fh_fsid,
		vtofh(dvp)->fh_fno, nm, exclusive, mode);
#endif NFSDEBUG

	if (exclusive == EXCL) {
		/*
		 * This is buggy: there is a race between the lookup and the
		 * create.  We should send the exclusive flag over the wire.
		 */
		error = nfs_lookup(dvp, nm, vpp, cred);
		if (error != ENOENT) {
			if(*vpp)
				VN_RELE(*vpp);
/*			VN_RELE(dvp);		*/
			return (error ? error : EEXIST);
		}
		
	}
	*vpp = (struct vnode *)0;

	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	vattr_to_sattr(va, &args.ca_sa);
	error = rfscall(vtomi(dvp), RFS_CREATE, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(dr->dr_status);
		if (error) {
			check_stale_fh(error, dvp);
		}
		else {
			*vpp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr,
			dvp->v_vfsp, ((struct gnode *)dvp)->g_mp);
#ifdef NFSDEBUG
			dprint(nfsdebug, 4, "nfs_create: *vpp 0x%x ret\n", *vpp);
#endif
			if(*vpp != NULL) {
				((struct gnode *)*vpp)->g_size = 0;
			} else
				error = u.u_error;
		}
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(struct nfsdiropres));
#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "nfs_create: out dvp size %d\n", dvp->g_size);
	dprint(nfsdebug, 5, "nfs_create returning error %d *vpp 0x%x\n", error,
	*vpp);
#endif
	return (error);
}

/*
 * Weirdness: if the vnode to be removed is open
 * we rename it instead of removing it and nfs_inactive
 * will remove the new name.
 */
nfs_remove(dvp, vp, nm, cred)
	struct vnode *dvp;
	struct vnode *vp;
	char *nm;
	struct ucred *cred;
{
	int error;
	struct nfsdiropargs da;
	enum nfsstat status;
	char *tmpname;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_remove %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname, vtofh(dvp)->fh_fsid,
	    vtofh(dvp)->fh_fno, nm);
#endif NFSDEBUG

	status = NFS_OK;
	if ((vtor(vp)->r_flags & ROPEN) && vtor(vp)->r_unlname == NULL) {
		tmpname = newname(nm);
		dnlc_purge_vp(vp);
		error = nfs_rename(dvp, nm, dvp, tmpname, cred);
		if (error) {
			kmem_free((caddr_t)tmpname,
			    (u_int)NFS_MAXNAMLEN);
		} else {
			VN_HOLD(dvp);
			vtor(vp)->r_unldvp = dvp;
			vtor(vp)->r_unlname = tmpname;
#ifdef NFSDEBUG
			dprint(nfsdebug, 4, "nfs_remove: vp 0x%x (%d) allocate unlname 0x%x\n",
			vp, vp->g_number, vtor(vp)->r_unlname);
#endif
			if (vtor(vp)->r_unlcred != NULL) {
				crfree(vtor(vp)->r_unlcred);
			}
			crhold(cred);
			vtor(vp)->r_unlcred = cred;
		}
	} else {
		setdiropargs(&da, nm, dvp);
		error = rfscall(
		    vtomi(dvp), RFS_REMOVE, xdr_diropargs, (caddr_t)&da,
		    xdr_enum, (caddr_t)&status, cred);
		nfsattr_inval(dvp);	/* mod time changed */
		nfsattr_inval(vp);	/* link count changed */
		check_stale_fh(error ? error : geterrno(error), dvp);

	}
#ifdef notdef
	bflush(vp);
#endif notdef

	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_remove: returning %d\n", error);
#endif
	return (error);
}

nfs_link(vp, tdvp, tnm, cred)
	struct vnode *vp;
	struct vnode *tdvp;
	char *tnm;
	struct ucred *cred;
{
	int error;
	struct nfslinkargs args;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_link from %s %o %d to %s %o %d '%s'\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno,
	    vtomi(tdvp)->mi_hostname,
	    vtofh(tdvp)->fh_fsid, vtofh(tdvp)->fh_fno,
	    tnm);
#endif NFSDEBUG
	args.la_from = *vtofh(vp);
	setdiropargs(&args.la_to, tnm, tdvp);
	error = rfscall(vtomi(vp), RFS_LINK, xdr_linkargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(tdvp);	/* mod time changed */
	nfsattr_inval(vp);	/* link count changed */
	if (!error) {
		error = geterrno(status);
		check_stale_fh(error, vp);
		check_stale_fh(error, tdvp);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_link returning %d\n", error);
#endif
	return (error);
}

nfs_rename(odvp, onm, ndvp, nnm, cred)
	struct vnode *odvp;
	char *onm;
	struct vnode *ndvp;
	char *nnm;
	struct ucred *cred;
{
	int error;
	enum nfsstat status;
	struct nfsrnmargs args;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rename from %s %o %d '%s' to %s %o %d '%s'\n",
	    vtomi(odvp)->mi_hostname,
	    vtofh(odvp)->fh_fsid, vtofh(odvp)->fh_fno, onm,
	    vtomi(ndvp)->mi_hostname,
	    vtofh(ndvp)->fh_fsid, vtofh(ndvp)->fh_fno, nnm);
#endif NFSDEBUG
	if (!bcmp(onm, ".", 2) || !bcmp(onm, "..", 3) 
		|| !bcmp(nnm, ".", 3) || !bcmp(nnm, "..", 3)) {
		error = EINVAL;
	} else {
		dnlc_remove(odvp, onm);
		dnlc_remove(ndvp, nnm);
		setdiropargs(&args.rna_from, onm, odvp);
		setdiropargs(&args.rna_to, nnm, ndvp);
		error = rfscall(vtomi(odvp), RFS_RENAME,
		    xdr_rnmargs, (caddr_t)&args,
		    xdr_enum, (caddr_t)&status, cred);
		nfsattr_inval(odvp);	/* mod time changed */
		nfsattr_inval(ndvp);	/* mod time changed */
		if (!error) {
			error = geterrno(status);
			check_stale_fh(error, odvp);
			check_stale_fh(error, ndvp);
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rename returning %d\n", error);
#endif
	return (error);
}

nfs_mkdir(dvp, nm, va, vpp, cred)
	struct vnode *dvp;
	char *nm;
	register struct vattr *va;
	struct vnode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug,3, "nfs_mkdir, in dvp size = %d\n", dvp->g_size);

	dprint(nfsdebug, 4, "nfs_mkdir %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, nm);
#endif
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	vattr_to_sattr(va, &args.ca_sa);
	error = rfscall(vtomi(dvp), RFS_MKDIR, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	nfsattr_inval(dvp);	/* mod time changed */
	if (!error) {
		error = geterrno(dr->dr_status);
		check_stale_fh(error, dvp);
	}
	if (!error) {
		*vpp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, dvp->v_vfsp,
		((struct gnode *) dvp)->g_mp);
		if(*vpp == NULL)
			error = u.u_error;
	} else {
		*vpp = (struct vnode *)0;
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug,3, "nfs_mkdir, out dvp size = %d\n", dvp->g_size);
	dprint(nfsdebug, 5, "nfs_mkdir returning %d\n", error);
#endif
	return (error);
}

nfs_rmdir(dvp, nm, cred)
	struct vnode *dvp;
	char *nm;
	struct ucred *cred;
{
	int error;
	enum nfsstat status;
	struct nfsdiropargs da;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rmdir %s %o %d '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, nm);
#endif NFSDEBUG
	setdiropargs(&da, nm, dvp);
	dnlc_purge_vp(dvp);
	error = rfscall(vtomi(dvp), RFS_RMDIR, xdr_diropargs, (caddr_t)&da,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(status);
		check_stale_fh(error, dvp);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rmdir returning %d\n", error);
#endif
	return (error);
}

nfs_symlink(dvp, lnm, tva, tnm, cred)
	struct vnode *dvp;
	char *lnm;
	struct vattr *tva;
	char *tnm;
	struct ucred *cred;
{
	int error;
	struct nfsslargs args;
	enum nfsstat status;

#ifdef NFSDEBUG
dprint(nfsdebug, 4, "nfs_symlink %s %o %d '%s' to '%s'\n",
	    vtomi(dvp)->mi_hostname,
	    vtofh(dvp)->fh_fsid, vtofh(dvp)->fh_fno, lnm, tnm);
#endif NFSDEBUG
	setdiropargs(&args.sla_from, lnm, dvp);
	vattr_to_sattr(tva, &args.sla_sa);
	args.sla_tnm = tnm;
	error = rfscall(vtomi(dvp), RFS_SYMLINK, xdr_slargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(status);
		check_stale_fh(error, dvp);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_sysmlink: returning %d\n", error);
#endif
	return (error);
}

/*
 * Read directory entries.
 * There are some weird things to look out for here.  The uio_offset
 * field is either 0 or it is the offset returned from a previous
 * readdir.  It is an opaque value used by the server to find the
 * correct directory block to read.  The byte count must be at least
 * vtoblksz(vp) bytes.  The count field is the number of blocks to
 * read on the server.  This is advisory only, the server may return
 * only one block's worth of entries.  Entries may be compressed on
 * the server.
 */
nfs_readdir(vp, uiop, cred)
	struct vnode *vp;
	register struct uio *uiop;
	struct ucred *cred;
{
	register int error = 0;
	register struct iovec *iovp;
	register unsigned count;
	struct nfsrddirargs rda;
	struct nfsrddirres  rd;
	register struct rnode *rp;

	rp = vtor(vp);
	if ((rp->r_flags & REOF) && (vp->g_size == (u_long)uiop->uio_offset)) {
		return (0);
        }
	iovp = uiop->uio_iov;
	count = iovp->iov_len;
#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "nfs_readdir %s %o %d count %d offset %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno, count, uiop->uio_offset);
#endif
	/*
	 * XXX We should do some kind of test for count >= DEV_BSIZE
	 */
	if (uiop->uio_iovcnt != 1) {
		return (EINVAL);
	}
	count = MIN(count, vtomi(vp)->mi_tsize);
	rda.rda_count = count;
	rda.rda_offset = uiop->uio_offset;
	rda.rda_fh = *vtofh(vp);
	rd.rd_size = count;
	rd.rd_entries = (struct direct *)kmem_alloc((u_int)count);

	error = rfscall(vtomi(vp), RFS_READDIR, xdr_rddirargs, (caddr_t)&rda,
	    xdr_getrddirres, (caddr_t)&rd, cred);
	if (!error) {
		error = geterrno(rd.rd_status);
		check_stale_fh(error, vp);
	}
	if (!error) {
		/*
		 * move dir entries to user land
		 */
		if (rd.rd_size) {
			error = uiomove((caddr_t)rd.rd_entries,
			    (int)rd.rd_size, UIO_READ, uiop);
			rda.rda_offset = rd.rd_offset;
			uiop->uio_offset = rd.rd_offset;
		}
		if (rd.rd_eof) {
			rp->r_flags |= REOF;
			vp->g_size = uiop->uio_offset;
		}
	}
	kmem_free((caddr_t)rd.rd_entries, (u_int)count);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_readdir: returning %d resid %d, offset %d\n",
	    error, uiop->uio_resid, uiop->uio_offset);
#endif
	return (error);
}

/*
 * GFS operation for getting block maps
 */

nfs_gbmap(vp, vbn, rw, size, sync)
	register struct vnode *vp;	/* gnode */
	register daddr_t vbn;		/* virtual block */
	int rw, size, sync;		/* ignore for nfs */
{
	daddr_t lbn;
	nfs_bmap(vp, vbn, &lbn);
	return((int)lbn);
}

/*
 * Convert from file system blocks to device blocks
 */
int
nfs_bmap(vp, bn, bnp)
	struct vnode *vp;	/* file's vnode */
	daddr_t bn;		/* fs block number */
	daddr_t *bnp;		/* RETURN device block number */
{
	int bsize;		/* server's block size in bytes */

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_bmap %s %o %d blk %d\n",
	    vtomi(vp)->mi_hostname,
	    vtofh(vp)->fh_fsid, vtofh(vp)->fh_fno, bn);
#endif NFSDEBUG
	if (bnp) {
		bsize = vtoblksz(vp);
		*bnp = bn * (bsize / DEV_BSIZE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_bmap: vp 0x%x (%d) -> *vpp 0x%x\n",
	vp, vp->g_number, vp);
	dprint(nfsdebug, 4, "nfs_bmap: bn %d -> *bnp %d\n", bn, 
	bnp ? *bnp : -1);
#endif
	return (0);
}

struct buf *async_bufhead;
int async_daemon_count;

#ifdef vax
#include "../h/vm.h"
#include "../h/map.h"
#include "../machine/pte.h"
#endif

int
nfs_strategy(bp)
	register struct buf *bp;
{
	register struct buf *bp1;
	register struct vnode *vp = (struct vnode *) bp->b_gp;
	
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_strategy: bp %x vp 0x%x (%d) addr 0x%x\n",
		 bp, vp, vp?vp->g_number:-1, bp->b_un.b_addr);
#endif
	if (bp->b_flags & (B_DIRTY|B_UAREA|B_PAGET))
		panic("nfs_strategy: swapping to nfs");

	/*
	 * If there was an asynchronous write error on this gnode
	 * then we just return the old error code. This continues
	 * until the gnode goes away (zero ref count). We do this because
	 * there can be many procs writing this gnode.
	 */
	if (vtor(vp)->r_error) {
		bp->b_error = vtor(vp)->r_error;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

#ifdef vax
	if (bp->b_flags & B_PHYS) {
		register int npte;
		register int n;
		register long a;
		register struct pte *pte, *kpte;
		caddr_t va;
		int o;
		caddr_t saddr;

		/*
		 * Buffer's data is in userland, or in some other
		 * currently inaccessable place. We get a hunk of
		 * kernel address space and map it in.
		 */
		o = (int)bp->b_un.b_addr & PGOFSET;
		npte = btoc(bp->b_bcount + o);
		while ((a = rmalloc(kernelmap, (long)clrnd(npte))) == NULL) {
			kmapwnt++;
			sleep((caddr_t)kernelmap, PSWP+4);
		}
		kpte = &Usrptmap[a];
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		for (n = npte; n--; kpte++, pte++)
			*(int *)kpte = PG_NOACC | (*(int *)pte & PG_PFNUM);
		va = (caddr_t)kmxtob(a);
		vmaccess(&Usrptmap[a], va, npte);
		saddr = bp->b_un.b_addr;
		bp->b_un.b_addr = va + o;
		/*
		 * do the io
		 */
		do_bio(bp, vp);
		/*
		 * Release kernel maps
		 */
		bp->b_un.b_addr = saddr;
		kpte = &Usrptmap[a];
		for (n = npte; n-- ; kpte++)
			*(int *)kpte = PG_NOACC;
		rmfree(kernelmap, (long)clrnd(npte), a);
	} else
#endif

	if (async_daemon_count && bp->b_flags & B_ASYNC) {
		if (async_bufhead) {
			bp1 = async_bufhead;
			while (bp1->b_actf) {
				bp1 = bp1->b_actf;
			}
			bp1->b_actf = bp;
		} else {
			async_bufhead = bp;
		}
		bp->b_actf = NULL;
		bp->b_gp->g_count++;
		wakeup((caddr_t) &async_bufhead);
	} else {
		do_bio(bp, vp);
	}
}

nfs_biod()
{
	register struct buf *bp;

	/*
	 * First release resoruces.
	 */
/*	if ((u.u_procp->p_flag & SVFORK) == 0) {
		vrelvm();
	}
*/	if (setjmp(&u.u_qsave)) {
		async_daemon_count--;
		exit(0);
	}
	for (;;) {
		async_daemon_count++;
		while (async_bufhead == NULL) {
			sleep((caddr_t)&async_bufhead, PZERO + 1);
		}
		async_daemon_count--;
		bp = async_bufhead;
		async_bufhead = bp->b_actf;
		do_bio(bp, (struct vnode *)bp->b_gp);
		nfs_rele(bp->b_gp);

	}
}

do_bio(bp, vp)
	register struct buf *bp;
	register struct vnode *vp;
{
	register int error;
	int resid;

#ifdef NFSDEBUG
	dprint(nfsdebug, 10,
	    "do_bio: addr %x, blk %d, offset %d, size %d, B_READ %d\n",
	    bp->b_un.b_addr, bp->b_blkno, bp->b_blkno * DEV_BSIZE,
	    bp->b_bcount, bp->b_flags & B_READ);
#endif
	if ((bp->b_flags & B_READ) == B_READ) {
		error = nfsread(vp, bp->b_un.b_addr,
			bp->b_blkno * DEV_BSIZE, (int)bp->b_bcount,
			&resid, vtor(vp)->r_cred);
		if (resid) {
			bzero(bp->b_un.b_addr + bp->b_bcount - resid,
				(u_int)resid);
		}
	} else {
		register struct rnode *rp;

		/*
		 * If the write fails and it was asynchronous
		 * all future writes will get an error.
		 */
		rp = vtor(vp);
		if ((error = rp->r_error) == 0) {
			error = nfswrite(vp, bp->b_un.b_addr,
				bp->b_blkno * DEV_BSIZE,
				bp->b_bcount - bp->b_resid,
				rp->r_cred);
			if ((bp->b_flags & B_ASYNC) && error)
				rp->r_error = error;
		}
	}
	if (error) {
		bp->b_error = error;
		bp->b_flags |= B_ERROR;
	}
	else {
		bp->b_flags &= ~B_AGE;
	}

	iodone(bp);

}

int
nfs_badop()
{
	panic("nfs_badop");
}
