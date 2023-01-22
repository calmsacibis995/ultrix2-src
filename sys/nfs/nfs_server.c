#ifndef lint
static	char	*sccsid = "@(#)nfs_server.c	1.12	(ULTRIX)	3/3/87";
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
#include "../h/buf.h"
#include "../h/uio.h"
#include "../h/file.h"
#include "../h/socketvar.h"
#include "../h/socket.h"
#include "../h/errno.h"
#include "../h/mbuf.h"
#include "../h/fs_types.h"
#include "../h/kernel.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/auth.h"
#include "../rpc/auth_unix.h"
#include "../rpc/svc.h"
#include "../rpc/xdr.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/vnode.h"

extern char *rfsnames[];

int dup_writes = 0;
int dup_creates = 0;
int dup_removes = 0;
int dup_links = 0;
int dup_mkdirs = 0;
int dup_rmdirs = 0;
int dup_renames = 0;
int dup_setattrs = 0;
int throwaways = 0;

/*
 * rpc service program version range supported
 */
#define	VERSIONMIN	2
#define	VERSIONMAX	2

/*
 * Returns true iff filesystem for a given fsid is exported read-only
 *
 * define rdonly(vp)	(((vp)->v_vfsp->vfs_flag & VFS_EXPORTED) && \
 *			 ((vp)->v_vfsp->vfs_exflags & EX_RDONLY))
 *
 */

#define rdonly(vp) ISREADONLY((vp)->g_mp)

struct vnode	*fhtovp();
struct file	*getsock();
void		svcerr_progvers();
void		rfs_dispatch();

#ifdef GFSDEBUG
extern short *GFS;
#endif

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct {
	int	ncalls;		/* number of calls received */
	int	nbadcalls;	/* calls that failed */
	int	reqs[32];	/* count for each request */
} svstat;

int soreserve_size = 18000;

/*
 * NFS Server system call.
 * Does all of the work of running a NFS server.
 * sock is the fd of an open UDP socket.
 */
nfs_svc()
{
	struct a {
		int     sock;
	} *uap = (struct a *)u.u_ap;
	struct gnode	*rdir;
	struct gnode	*cdir;
	struct socket   *so;
	struct file	*fp;
	SVCXPRT *xprt;
	u_long	vers;
	int	error;
 
	fp = getsock(uap->sock);
	if (fp == 0) {
		u.u_error = EBADF;
		return;
	}
	so = (struct socket *)fp->f_data;
	 
	/*
	 *	Allocate extra space for this socket, to minimize
	 *	lost requests for NFS.  We don't want everyone to do
	 *	this, so do it here, rather than in udp_usrreq().
	 */

	error = soreserve(so, soreserve_size, 
		soreserve_size + 2 *(sizeof(struct sockaddr)));
	if (error)	{
		u.u_error = error;
		return;
	}

	/*
	 * Be sure that rdir (the server's root vnode) is set.
	 * Save the current directory and restore it again when
	 * the call terminates.  rfs_lookup uses u.u_cdir for lookupname.
	 */
	rdir = u.u_rdir;
	cdir = u.u_cdir;
	if (rdir == (struct gnode *)0) {
		u.u_rdir = u.u_cdir;
	}
	xprt = svckudp_create(so, NFS_PORT);
	for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) {
		(void) svc_register(xprt, NFS_PROGRAM, vers, rfs_dispatch,
		    FALSE);
	}
	if (setjmp(&u.u_qsave)) {
		for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) {
			svc_unregister(NFS_PROGRAM, vers);
		}
		SVC_DESTROY(xprt);
		u.u_error = EINTR;
	} else {
		svc_run(xprt);  /* never returns */
	}
	u.u_rdir = rdir;
	u.u_cdir = cdir;
}


/*
 * Get file handle system call.
 * Takes open file descriptor and returns a file handle for it.
 */
nfs_getfh()
{
	register struct a {
		int	fdes;
		fhandle_t	*fhp;
	} *uap = (struct a*)u.u_ap;
	register struct file *fp;
	fhandle_t fh;
	struct vnode *vp;

	if (!suser()) {
		return;
	}
	fp = getf(uap->fdes);
	if (fp == NULL) {
		return;
	}
	vp = (struct vnode *)fp->f_data;
	u.u_error = makefh(&fh, vp);
	if (!u.u_error) {
		u.u_error =
		    copyout((caddr_t)&fh, (caddr_t)uap->fhp, sizeof(fh));
	}
	return;
}

	
/*
 * These are the interface routines for the server side of the
 * Networked File System.  See the NFS protocol specification
 * for a description of this interface.
 */


/*
 * Get file attributes.
 * Returns the current attributes of the file with the given fhandle.
 */
int
rfs_getattr(fhp, ns)
	fhandle_t *fhp;
	register struct nfsattrstat *ns;
{
	register int error = 0;
	register struct vnode *vp;
	struct vattr va;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_getattr fh %o %d\n",
	    fhp->fh_fsid, fhp->fh_fno);
#endif
	vp = fhtovp(fhp);
	if (vp == NULL) {
		ns->ns_status = NFSERR_STALE;
		return(ESTALE);
	}
	error = VOP_GETATTR(vp, &va, u.u_cred);
	if (!error) {
		vattr_to_nattr(&va, &ns->ns_attr);
	}
	ns->ns_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_getattr: returning %d\n", error);
#endif
	return(0);
}

/*
 * Set file attributes.
 * Sets the attributes of the file with the given fhandle.  Returns
 * the new attributes.
 */
int
rfs_setattr(args, ns, req)
	struct nfssaargs *args;
	register struct nfsattrstat *ns;
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *vp;
	struct vattr va;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_setattr fh %o %d\n",
	    args->saa_fh.fh_fsid, args->saa_fh.fh_fno);
#endif
	vp = fhtovp(&args->saa_fh);
	if (vp == NULL) {
		ns->ns_status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == vp->g_ctime.tv_sec &&
			duptime.tv_usec == vp->g_ctime.tv_usec)
				++dup_setattrs;
		else {
			sattr_to_vattr(&args->saa_sa, &va);
			error = VOP_SETATTR(vp, &va, u.u_cred);
		}

		if (!error) {
			error = VOP_GETATTR(vp, &va, u.u_cred);
			if (!error) {
				vattr_to_nattr(&va, &ns->ns_attr);
			}
		}

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, vp->g_ctime, DUP_DONE);
		}
	}
	ns->ns_status = puterrno(error);
	VN_RELE(vp);
	return (0);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_setattr: returning %d\n", error);
#endif
}

/*
 * Directory lookup.
 * Returns an fhandle and file attributes for file name in a directory.
 */
int
rfs_lookup(da, dr)
	struct nfsdiropargs *da;
	register struct  nfsdiropres *dr;
{
	register int error = 0;
	register struct vnode *dvp;
	struct vnode *vp;
	struct vattr va;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_lookup %s fh %o %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif
	dvp = fhtovp(&da->da_fhandle);
	if (dvp == NULL) {
		dr->dr_status = NFSERR_STALE;
		return(ESTALE);
	}

	/*
	 * do lookup.
	 */
	error = VOP_LOOKUP(dvp, da->da_name, &vp, u.u_cred);
	if (error) {
		vp = (struct vnode *)0;
	} else {
		error = VOP_GETATTR(vp, &va, u.u_cred);
		if (!error) {
			vattr_to_nattr(&va, &dr->dr_attr);
			error = makefh(&dr->dr_fhandle, vp);
		}
	}
	dr->dr_status = puterrno(error);
	if (vp) {
		VN_RELE(vp);
	}
	VN_RELE(dvp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_lookup: returning %d\n", error);
#endif
	return(0);
}

/*
 * Read symbolic link.
 * Returns the string in the symbolic link at the given fhandle.
 */
int
rfs_readlink(fhp, rl)
	fhandle_t *fhp;
	register struct nfsrdlnres *rl;
{
	register int error = 0;
	struct iovec iov;
	struct uio uio;
	struct vnode *vp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_readlink fh %o %d\n",
	    fhp->fh_fsid, fhp->fh_fno);
#endif
	vp = fhtovp(fhp);
	if (vp == NULL) {
		rl->rl_status = NFSERR_STALE;
		return(ESTALE);
	}

	/*
	 * Allocate data for pathname.  This will be freed by rfs_rlfree.
	 */
	rl->rl_data = (char *)kmem_alloc((u_int)MAXPATHLEN);

	/*
	 * Set up io vector to read sym link data
	 */
	iov.iov_base = rl->rl_data;
	iov.iov_len = NFS_MAXPATHLEN;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_offset = 0;
	uio.uio_resid = NFS_MAXPATHLEN;

	/*
	 * read link
	 */
	error = VOP_READLINK(vp, &uio, u.u_cred);

	/*
	 * Clean up
	 */
	if (error) {	
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN);
		rl->rl_count = 0;
		rl->rl_data = NULL;
	} else {
		rl->rl_count = NFS_MAXPATHLEN - uio.uio_resid;
	}
	rl->rl_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_readlink: returning '%s' %d\n",
	    rl->rl_data, error);
#endif
	return(0);
}

/*
 * Free data allocated by rfs_readlink
 */
rfs_rlfree(rl)
	struct nfsrdlnres *rl;
{
	if (rl->rl_data) {
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN); 
	}
}

/*
 * Read data.
 * Returns some data read from the file at the given fhandle.
 */

int
rfs_read(ra, rr)
	struct nfsreadargs *ra;
	register struct nfsrdresult *rr;
{
	register int error = 0;
	struct vnode *vp;
	struct iovec iov;
	struct uio uio;
	int offset, fsbsize;
	struct buf *bp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_read %d, offset %d from fh %o %d\n",
		ra->ra_count, ra->ra_offset,
		ra->ra_fhandle.fh_fsid, ra->ra_fhandle.fh_fno);
#endif
	rr->rr_data = NULL;
	rr->rr_count = 0;
	vp = fhtovp(&ra->ra_fhandle);
	if (vp == NULL) {
		rr->rr_status = NFSERR_STALE;
		return(ESTALE);
	}
	/*
	 * This is a kludge to allow reading of files created
	 * with no read permission.  The owner of the file
	 * is always allowed to read it.
	 */
	if (u.u_uid != vp->g_uid) {
		error = VOP_ACCESS(vp, VREAD, u.u_cred);
		if (error) {
			/*
			 * Exec is the same as read over the net because
			 * of demand loading.
			 */
			error = VOP_ACCESS(vp, VEXEC, u.u_cred);
		}
		if (error) {
			goto bad;
		}
	}

	/*
	 * Check whether we can do this with bread, which would
	 * save the copy through the uio.
	 */
	fsbsize = ((struct gnode *)vp)->g_mp->m_fs_data->fd_otsize;
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_read: fsbsize = %d\n", fsbsize);
#endif
	offset = ra->ra_offset % fsbsize;
	if (offset + ra->ra_count <= fsbsize) {
		if (ra->ra_offset >= vp->g_size) {
			rr->rr_count = 0;
			gattr_to_nattr(vp, &rr->rr_attr);
			rr->rr_attr.na_blocksize = fsbsize;
			error = 0;
			goto done;
		}
#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
		"rfs_read: call vop_bread, vp = 0x%x\n", vp);
#endif
		error = VOP_BREAD(vp, ra->ra_offset / fsbsize, &bp);
		if (error == 0) {
			rr->rr_data = bp->b_un.b_addr + offset;
			rr->rr_count = min(
			    (u_int)(vp->g_size - ra->ra_offset),
			    (u_int)ra->ra_count);
			rr->rr_bp = bp;
			rr->rr_vp = vp;
			VN_HOLD(vp);
			gattr_to_nattr(vp, &rr->rr_attr);
			rr->rr_attr.na_blocksize = fsbsize;
			goto done;
		} else {
			printf("nfs read: failed, errno %d\n", error);
		}
	}

	rr->rr_bp = (struct buf *) 0;
			
	/*
	 * Allocate space for data.  This will be freed by xdr_rdresult
	 * when it is called with x_op = XDR_FREE.
	 */
	rr->rr_bufallocaddr = rr->rr_data =
		(char *)kmem_alloc((u_int)ra->ra_count);
	rr->rr_bufallocsize = ra->ra_count;

	/*
	 * Set up io vector
	 */
	iov.iov_base = rr->rr_data;
	iov.iov_len = ra->ra_count;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_offset = ra->ra_offset;
	uio.uio_resid = ra->ra_count;
	/*
	 * for now we assume no append mode and ignore
	 * totcount (read ahead)
	 */

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_read: call vop_rdwr\n");
#endif
	error = VOP_RDWR(vp, &uio, UIO_READ, IO_SYNC, u.u_cred);
	if (error) {
		goto bad;
	}
	gattr_to_nattr(vp, &rr->rr_attr);
	rr->rr_attr.na_blocksize = fsbsize;
	rr->rr_count = ra->ra_count - uio.uio_resid;
bad:
	if (error && rr->rr_data != NULL) {
		kmem_free(rr->rr_bufallocaddr, rr->rr_bufallocsize);
		rr->rr_data = NULL;
		rr->rr_count = 0;
	}
done:
	rr->rr_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_read returning %d, count = %d\n",
	    error, rr->rr_count);
#endif
	return(0);
}

/*
 * Free data allocated by rfs_read.
 */
rfs_rdfree(rr)
	struct nfsrdresult *rr;
{
	if (rr->rr_bp == 0 && rr->rr_data) {
		kmem_free(rr->rr_bufallocaddr, rr->rr_bufallocsize);
	}
}

/*
 * Write data to file.
 * Returns attributes of a file after writing some data to it.
 */
int
rfs_write(wa, ns, req)
	struct nfswriteargs *wa;
	struct nfsattrstat *ns;
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *vp;
	struct vattr va;
	struct iovec iov;
	struct uio uio;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_write: fh %o %d, count %d offset %d\n",
		wa->wa_fhandle.fh_fsid, wa->wa_fhandle.fh_fno,
		wa->wa_count, wa->wa_offset);
#endif

	vp = fhtovp(&wa->wa_fhandle);
	if (vp == NULL) {
		ns->ns_status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == vp->g_mtime.tv_sec &&
			duptime.tv_usec == vp->g_mtime.tv_usec) {
			++dup_writes;
		}
		else {
			if (u.u_uid != vp->g_uid) {
				/*
				 * This is a kludge to allow writes of
				 * files created with read only permission.
				 * The owner of the file
				 * is always allowed to write it.
				 */
				error = VOP_ACCESS(vp, VWRITE, u.u_cred);
			}
			if (!error) {
				iov.iov_base = wa->wa_data;
				iov.iov_len = wa->wa_count;
				uio.uio_iov = &iov;
				uio.uio_iovcnt = 1;
				uio.uio_segflg = UIO_SYSSPACE;
				uio.uio_offset = wa->wa_offset;
				uio.uio_resid = wa->wa_count;
				/*
				 * for now we assume no append mode
				 */
				error = VOP_RDWR(vp, &uio, UIO_WRITE,
					IO_SYNC, u.u_cred);
			}
		}

		if (!error) {
			/*
			 * Get attributes again so we send the latest mod
			 * time to the client side for his cache.
			 */
			error = VOP_GETATTR(vp, &va, u.u_cred);
		}

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			vattr_to_nattr(&va, &ns->ns_attr);
			svckudp_dupsave(req, vp->g_mtime, DUP_DONE);
		}
	}

#ifdef NFSDEBUG
	if (error) {
		printf("nfs write: failed, errno %d fh %o %d\n",
		    error, wa->wa_fhandle.fh_fsid, wa->wa_fhandle.fh_fno);
	}
#endif NFSDEBUG

	ns->ns_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_write: returning %d\n", error);
#endif
	return(0);
}

/*
 * Create a file.
 * Creates a file with given attributes and returns those attributes
 * and an fhandle for the new file.
 */
int
rfs_create(args, dr, req)
	struct nfscreatargs *args;
	struct  nfsdiropres *dr;
	struct svc_req *req;
{
	register int error = 0;
	struct vattr va;
	struct vnode *vp;
	register struct vnode *dvp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_create: %s dfh %o %d\n",
	    args->ca_da.da_name, args->ca_da.da_fhandle.fh_fsid,
	    args->ca_da.da_fhandle.fh_fno);
#endif
	sattr_to_vattr(&args->ca_sa, &va);
	va.va_type = VREG;
	/*
	 * XXX Should get exclusive flag and use it.
	 */
	dvp = fhtovp(&args->ca_da.da_fhandle);
	if (dvp == NULL) {
		dr->dr_status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(dvp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == dvp->g_ctime.tv_sec &&
			duptime.tv_usec == dvp->g_ctime.tv_usec) {
			++dup_creates;
			error = VOP_LOOKUP(dvp, args->ca_da.da_name,
				&vp, u.u_cred);
		}
		else {
			error = VOP_CREATE(dvp, args->ca_da.da_name,
				&va, NONEXCL, VWRITE, &vp, u.u_cred);
		}

		if (!error) {
			error = VOP_GETATTR(vp, &va, u.u_cred);
			if (!error) {
				vattr_to_nattr(&va, &dr->dr_attr);
				error = makefh(&dr->dr_fhandle, vp);
			}
			VN_RELE(vp);
		}

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, dvp->g_ctime, DUP_DONE);
		}
	}
	dr->dr_status = puterrno(error);
	VN_RELE(dvp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_create: returning %d\n", error);
#endif
	return(0);
}

/*
 * Remove a file.
 * Remove named file from parent directory.
 */
int
rfs_remove(da, status, req)
	struct nfsdiropargs *da;
	enum nfsstat *status;
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *vp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_remove %s dfh %o %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif
	vp = fhtovp(&da->da_fhandle);
	if (vp == NULL) {
		*status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == vp->g_ctime.tv_sec &&
			duptime.tv_usec == vp->g_ctime.tv_usec)
				++dup_removes;
		else
			error = VOP_REMOVE(vp, da->da_name, u.u_cred);

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, vp->g_ctime, DUP_DONE);
		}
	}
	*status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_remove: %s returning %d\n",
	    da->da_name, error);
#endif
	return(0);
}

/*
 * rename a file
 * Give a file (from) a new name (to).
 */
int
rfs_rename(args, status, req)
	struct nfsrnmargs *args;
	enum nfsstat *status; 
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *fromvp;
	register struct vnode *tovp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_rename %s ffh %o %d -> %s tfh %o %d\n",
	    args->rna_from.da_name,
	    args->rna_from.da_fhandle.fh_fsid,
	    args->rna_from.da_fhandle.fh_fno,
	    args->rna_to.da_name,
	    args->rna_to.da_fhandle.fh_fsid,
	    args->rna_to.da_fhandle.fh_fno);
#endif
	fromvp = fhtovp(&args->rna_from.da_fhandle);
	if (fromvp == NULL) {
		*status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(fromvp)) {
		error = EROFS;
		goto fromerr;
	}
	tovp = fhtovp(&args->rna_to.da_fhandle);
	if (tovp == NULL) {
		*status = NFSERR_STALE;
		VN_RELE(fromvp);
		return(ESTALE);
	}
	if (rdonly(tovp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == fromvp->g_ctime.tv_sec &&
			duptime.tv_usec == fromvp->g_ctime.tv_usec)
			++dup_renames;
		else
			error = VOP_RENAME(fromvp, args->rna_from.da_name,
			    tovp, args->rna_to.da_name, u.u_cred);

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, fromvp->g_ctime, DUP_DONE);
		}
	}
	VN_RELE(tovp);
fromerr:
	VN_RELE(fromvp);
	*status = puterrno(error); 

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_rename: returning %d\n", error);
#endif
	return(0);
} 

/*
 * Link to a file.
 * Create a file (to) which is a hard link to the given file (from).
 */
int
rfs_link(args, status, req)
	struct nfslinkargs *args;
	enum nfsstat *status;  
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *fromvp;
	register struct vnode *tovp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_link ffh %o %d -> %s tfh %o %d\n",
	    args->la_from.fh_fsid, args->la_from.fh_fno,
	    args->la_to.da_name,
	    args->la_to.da_fhandle.fh_fsid, args->la_to.da_fhandle.fh_fno);
#endif
	fromvp = fhtovp(&args->la_from);
	if (fromvp == NULL) {
		*status = NFSERR_STALE;
		return(ESTALE);
	}
	tovp = fhtovp(&args->la_to.da_fhandle);
	if (tovp == NULL) {
		*status = NFSERR_STALE;
		VN_RELE(fromvp);
		return(ESTALE);
	}
	if (rdonly(tovp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == fromvp->g_ctime.tv_sec &&
			duptime.tv_usec == fromvp->g_ctime.tv_usec) {
			++dup_links;
			VN_RELE(fromvp);
		}
		else
			error = VOP_LINK(fromvp, tovp, args->la_to.da_name, u.u_cred);

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, fromvp->g_ctime, DUP_DONE);
		}
	}
	*status = puterrno(error);

#ifdef notdef		/* HACK -- remove for GFS interface */
	VN_RELE(fromvp);
#endif

	VN_RELE(tovp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_link: returning %d\n", error);
#endif
	return(0);
} 
 
/*
 * Symbolicly link to a file.
 * Create a file (to) with the given attributes which is a symbolic link
 * to the given path name (to).
 */
int
rfs_symlink(args, status, req)
	struct nfsslargs *args;
	enum nfsstat *status;   
	struct svc_req *req;
{		  
	register int error = 0;
	struct vattr va;
	register struct vnode *vp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_symlink %s ffh %o %d -> %s\n",
	    args->sla_from.da_name,
	    args->sla_from.da_fhandle.fh_fsid,
	    args->sla_from.da_fhandle.fh_fno,
	    args->sla_tnm);
#endif
	sattr_to_vattr(&args->sla_sa, &va);
	va.va_type = VLNK;
	vp = fhtovp(&args->sla_from.da_fhandle);
	if (vp == NULL) {
		*status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		error = VOP_SYMLINK(vp, args->sla_from.da_name,
		    &va, args->sla_tnm, u.u_cred);
	}
	*status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_symlink: returning %d\n", error);
#endif
	return(0);
}  
  
/*
 * Make a directory.
 * Create a directory with the given name, parent directory, and attributes.
 * Returns a file handle and attributes for the new directory.
 */
int
rfs_mkdir(args, dr, req)
	struct nfscreatargs *args;
	struct  nfsdiropres *dr;
	struct svc_req *req;
{
	register int error = 0;
	struct vattr va;
	struct vnode *dvp;
	register struct vnode *vp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_mkdir %s fh %o %d\n",
	    args->ca_da.da_name, args->ca_da.da_fhandle.fh_fsid,
	    args->ca_da.da_fhandle.fh_fno);
#endif
	sattr_to_vattr(&args->ca_sa, &va);
	va.va_type = VDIR;
	/*
	 * Should get exclusive flag and pass it on here
	 */
	vp = fhtovp(&args->ca_da.da_fhandle);
	if (vp == NULL) {
		dr->dr_status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == vp->g_ctime.tv_sec &&
			duptime.tv_usec == vp->g_ctime.tv_usec) {
			++dup_mkdirs;
			error = VOP_LOOKUP(vp, args->ca_da.da_name, &dvp,
				u.u_cred);
			if (!error) {
				error = VOP_GETATTR(dvp, &va, u.u_cred);
			}
		}
		else {
			error = VOP_MKDIR(vp, args->ca_da.da_name, &va,
				&dvp, u.u_cred);
		}
		if (!error) {
			vattr_to_nattr(&va, &dr->dr_attr);
			error = makefh(&dr->dr_fhandle, dvp);
			VN_RELE(dvp);
		}

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, vp->g_ctime, DUP_DONE);
		}

	}
	dr->dr_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_mkdir: returning %d\n", error);
#endif
	return(0);
}

/*
 * Remove a directory.
 * Remove the given directory name from the given parent directory.
 */
int
rfs_rmdir(da, status, req)
	struct nfsdiropargs *da;
	enum nfsstat *status;
	struct svc_req *req;
{
	register int error = 0;
	register struct vnode *vp;
	struct timeval duptime;
	int dupmark;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_rmdir %s fh %o %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif

	vp = fhtovp(&da->da_fhandle);
	if (vp == NULL) {
		*status = NFSERR_STALE;
		return(ESTALE);
	}
	if (rdonly(vp)) {
		error = EROFS;
	} else {
		if (svckudp_dup(req, &duptime, &dupmark) &&
			dupmark == DUP_DONE &&
			duptime.tv_sec == vp->g_ctime.tv_sec &&
			duptime.tv_usec == vp->g_ctime.tv_usec) {
				++dup_rmdirs;
		}
		else {
			error = VOP_RMDIR(vp, da->da_name, u.u_cred);
		}

		if (error) {
			svckudp_dupsave(req, time, DUP_FAIL);
		} else {
			svckudp_dupsave(req, vp->g_ctime, DUP_DONE);
		}
	}
	*status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rmdir returning %d\n", error);
#endif
	return(0);
}

int
rfs_readdir(rda, rd)
	struct nfsrddirargs *rda;
	register struct nfsrddirres  *rd;
{
	register int error = 0;
	u_long offset;
	u_long skipped;
	struct iovec iov;
	struct uio uio;
	register struct vnode *vp;
	struct direct *dp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "rfs_readdir fh %o %d count %d\n",
	    rda->rda_fh.fh_fsid, rda->rda_fh.fh_fno, rda->rda_count);
#endif
	vp = fhtovp(&rda->rda_fh);
	if (vp == NULL) {
		rd->rd_status = NFSERR_STALE;
		return(ESTALE);
	}
	/*
	 * check cd access to dir.  we have to do this here because
	 * the opendir doesn't go over the wire.
	 */
	error = VOP_ACCESS(vp, VEXEC, u.u_cred);
	if (error) {
		goto bad;
	}

	/*
	 * Allocate data for entries.  This will be freed by rfs_rdfree.
	 */
	rd->rd_bufallocaddr = (char *)kmem_alloc((u_int)rda->rda_count);
	rd->rd_entries = (struct direct *)rd->rd_bufallocaddr;

nxtblk:

	rd->rd_bufallocsize = rd->rd_bufsize = rda->rda_count;
	rd->rd_offset = offset = rda->rda_offset & ~(DIRBLKSIZ -1);

	/*
	 * Set up io vector to read directory data
	 */
	iov.iov_base = (caddr_t)rd->rd_entries;
	iov.iov_len = rda->rda_count;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_offset = offset;
	uio.uio_resid = rda->rda_count;

	/*
	 * read directory
	 */
	error = VOP_READDIR(vp, &uio, u.u_cred);

	/*
	 * Clean up
	 */
	if (error) {	
		rd->rd_size = 0;
		goto bad;
	}

	/*
	 * set size and eof
	 */
	if (uio.uio_resid) {
		rd->rd_size = rda->rda_count - uio.uio_resid;
		rd->rd_eof = TRUE;
	} else {
		rd->rd_size = rda->rda_count;
		rd->rd_eof = FALSE;
	}

	/*
	 * if client request was in the middle of a block
	 * or block begins with null entries skip entries
	 * til we are on a valid entry >= client's requested
	 * offset.
	 */
	dp = rd->rd_entries;
	skipped = 0;
	while ((skipped < rd->rd_size) &&
	    ((offset + dp->d_reclen <= rda->rda_offset) || (dp->d_ino == 0))) {
		skipped += dp->d_reclen;
		offset += dp->d_reclen;
		dp = (struct direct *)((int)dp + dp->d_reclen);
	}
	/*
	 * Reset entries pointer and free space we are skipping
	 */
	if (skipped) {
		rd->rd_size -= skipped;
		rd->rd_bufsize -= skipped;
		rd->rd_offset = offset;
		rd->rd_entries = (struct direct *)
		    ((int)rd->rd_entries + skipped);
		if (rd->rd_size == 0 && !rd->rd_eof) {
			/*
			 * we have skipped a whole block, reset offset
			 * and read another block (unless eof)
			 */
			rda->rda_offset = rd->rd_offset;
			goto nxtblk;
		}
	}
bad:
	rd->rd_status = puterrno(error);
	VN_RELE(vp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_readdir: returning %d\n", error);
#endif
	return(0);
}

rfs_rddirfree(rd)
	struct nfsrddirres *rd;
{
	if (rd->rd_bufallocaddr)
		kmem_free(rd->rd_bufallocaddr, rd->rd_bufallocsize);
}

rfs_statfs(fh, fs)
	fhandle_t *fh;
	register struct nfsstatfs *fs;
{
	register struct gnode *gp;
	struct fs_data *fsd;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_statfs fh %o %d\n", fh->fh_fsid, fh->fh_fno);
#endif

	gp = (struct gnode *)fhtovp(fh);
	if (gp == NULL) {
		fs->fs_status = NFSERR_STALE;
		return(ESTALE);
	}

	fsd = gp->g_mp->m_fs_data;
	fs->fs_tsize = nfstsize();
	fs->fsstat_bsize = FSDUNIT;
	fs->fs_blocks = fsd->fd_btot;
	fs->fs_bfree = fsd->fd_bfree;
	fs->fs_bavail = fsd->fd_bfreen;
	GRELE(gp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_statfs returning 0\n");
	dprint(nfsdebug, 5, "fs_tsize = %d, fsstat_bsize = %d, fs_blocks = %d\n",
		fs->fs_tsize, fs->fsstat_bsize, fs->fs_blocks);
	dprint(nfsdebug, 5, "fs_bfree = %d, fs_bavail = %d\n",
		fs->fs_bfree, fs->fs_bavail);
#endif
	return(0);
}

/*ARGSUSED*/
rfs_null(argp, resp)
	caddr_t *argp;
	caddr_t *resp;
{
	/* do nothing */
	return (0);
}

/*ARGSUSED*/
rfs_error(argp, resp)
	caddr_t *argp;
	caddr_t *resp;
{
	return (EOPNOTSUPP);
}

int
nullfree()
{
}

/*
 * rfs dispatch table
 * Indexed by version,proc
 */

struct rfsdisp {
	int	  (*dis_proc)();	/* proc to call */
	xdrproc_t dis_xdrargs;		/* xdr routine to get args */
	int	  dis_argsz;		/* sizeof args */
	xdrproc_t dis_xdrres;		/* xdr routine to put results */
	int	  dis_ressz;		/* size of results */
	int	  (*dis_resfree)();	/* frees space allocated by proc */
} rfsdisptab[][RFS_NPROC]  = {
	{
	/*
	 * VERSION 2
	 * Changed rddirres to have eof at end instead of beginning
	 */
	/* RFS_NULL = 0 */
	{rfs_null, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_GETATTR = 1 */
	{rfs_getattr, xdr_fhandle, sizeof(fhandle_t),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_SETATTR = 2 */
	{rfs_setattr, xdr_saargs, sizeof(struct nfssaargs),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_ROOT = 3 *** NO LONGER SUPPORTED *** */
	{rfs_error, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_LOOKUP = 4 */
	{rfs_lookup, xdr_diropargs, sizeof(struct nfsdiropargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_READLINK = 5 */
	{rfs_readlink, xdr_fhandle, sizeof(fhandle_t),
	    xdr_rdlnres, sizeof(struct nfsrdlnres), rfs_rlfree},
	/* RFS_READ = 6 */
	{rfs_read, xdr_readargs, sizeof(struct nfsreadargs),
	    xdr_rdresult, sizeof(struct nfsrdresult), rfs_rdfree},
	/* RFS_WRITECACHE = 7 *** NO LONGER SUPPORTED *** */
	{rfs_error, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_WRITE = 8 */
	{rfs_write, xdr_writeargs, sizeof(struct nfswriteargs),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_CREATE = 9 */
	{rfs_create, xdr_creatargs, sizeof(struct nfscreatargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_REMOVE = 10 */
	{rfs_remove, xdr_diropargs, sizeof(struct nfsdiropargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_RENAME = 11 */
	{rfs_rename, xdr_rnmargs, sizeof(struct nfsrnmargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_LINK = 12 */
	{rfs_link, xdr_linkargs, sizeof(struct nfslinkargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_SYMLINK = 13 */
	{rfs_symlink, xdr_slargs, sizeof(struct nfsslargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_MKDIR = 14 */
	{rfs_mkdir, xdr_creatargs, sizeof(struct nfscreatargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_RMDIR = 15 */
	{rfs_rmdir, xdr_diropargs, sizeof(struct nfsdiropargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_READDIR = 16 */
	{rfs_readdir, xdr_rddirargs, sizeof(struct nfsrddirargs),
	    xdr_putrddirres, sizeof(struct nfsrddirres), rfs_rddirfree},
	/* RFS_STATFS = 17 */
	{rfs_statfs, xdr_fhandle, sizeof(fhandle_t),
	    xdr_statfs, sizeof(struct nfsstatfs), nullfree},
	}
};

struct rfsspace {
	struct rfsspace *rs_next;
	caddr_t		rs_dummy;
};

struct rfsspace *rfsfreesp = NULL;

int rfssize = 0;

caddr_t
rfsget()
{
	int i;
	struct rfsdisp *dis;
	caddr_t ret;

	if (rfssize == 0) {
		for (i = 0; i < 1 + VERSIONMAX - VERSIONMIN; i++) {
			for (dis = &rfsdisptab[i][0];
			     dis < &rfsdisptab[i][RFS_NPROC];
			     dis++) {
				rfssize = MAX(rfssize, dis->dis_argsz);
				rfssize = MAX(rfssize, dis->dis_ressz);
			}
		}
	}

	if (rfsfreesp) {
		ret = (caddr_t)rfsfreesp;
		rfsfreesp = rfsfreesp->rs_next;
	} else {
		ret = (caddr_t)kmem_alloc((u_int)rfssize);
	}
	return (ret);
}

rfsput(rs)
	struct rfsspace *rs;
{
	rs->rs_next = rfsfreesp;
	rfsfreesp = rs;
}


int nobody = -2;

/*
 * If nfs_portmon is set, then clients are required to use
 * privileged ports (ports < IPPORT_RESERVED) in order to get NFS services.
 */
int nfs_portmon = 0;

void
rfs_dispatch(req, xprt)
	register struct svc_req *req;
	register SVCXPRT *xprt;
{
	register struct rfsdisp *disp;
	register int error = 0;
	register caddr_t *args = NULL;
	register caddr_t *res = NULL;
	int which;
	int vers;
	struct authunix_parms *aup;
	int *gp;
	struct ucred *tmpcr;
	struct ucred *newcr = NULL;
	int dup_xid = 0;

	svstat.ncalls++;

	if (svckudp_dupbusy(req)) {
		dup_xid = 1;
		throwaways++;
		goto done;
	}

	which = req->rq_proc;
	if (which < 0 || which >= RFS_NPROC) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2,
		    "rfs_dispatch: bad proc %d\n", which);
#endif
		svcerr_noproc(req->rq_xprt);
		error++;
		goto done;
	}
	vers = req->rq_vers;
	if (vers < VERSIONMIN || vers > VERSIONMAX) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2,
		    "rfs_dispatch: bad vers %d low %d high %d\n",
		    vers, VERSIONMIN, VERSIONMAX);
#endif
		svcerr_progvers(req->rq_xprt, (u_long)VERSIONMIN,
		    (u_long)VERSIONMAX);
		error++;
		goto done;
	}
	vers -= VERSIONMIN;
	disp = &rfsdisptab[vers][which];

	/*
	 * Clean up as if a system call just started
	 */
	u.u_error = 0;

	/*
	 * Allocate args struct and deserialize into it.
	 */
	args = (caddr_t *)rfsget();
	bzero((caddr_t)args, rfssize);
	if ( ! SVC_GETARGS(xprt, disp->dis_xdrargs, args)) {
		svcerr_decode(xprt);
		error++;
		goto done;
	}

	/*
	 * Check for unix style credentials
	 */
	if (req->rq_cred.oa_flavor != AUTH_UNIX && which != RFS_NULL) {
		svcerr_weakauth(xprt);
		error++;
		goto done;
	}

	if (nfs_portmon) {
		/*
		* Check for privileged port number
		*/
       	static count = 0;
		if (ntohs(xprt->xp_raddr.sin_port) >= IPPORT_RESERVED) {
			svcerr_weakauth(xprt);
			if (count == 0) {
				printf("NFS request from unprivileged port, ");
				printf("source IP address = %u.%u.%u.%u\n",
					xprt->xp_raddr.sin_addr.s_net,
					xprt->xp_raddr.sin_addr.s_host,
					xprt->xp_raddr.sin_addr.s_lh,
					xprt->xp_raddr.sin_addr.s_impno);
			}
			count++;
			count %= 256;
			error++;
			goto done;
		}
	}


	/*
	 * Set uid, gid, and gids to auth params
	 */
	if (which != RFS_NULL) {
		aup = (struct authunix_parms *)req->rq_clntcred;
		newcr = crget();
		if (aup->aup_uid == 0) {
			/*
			 * root over the net becomes other on the server (uid -2)
			 */
			newcr->cr_uid = nobody;
		} else {
			newcr->cr_uid = aup->aup_uid;
		}
		newcr->cr_gid = aup->aup_gid;
		bcopy((caddr_t)aup->aup_gids, (caddr_t)newcr->cr_groups,
		    aup->aup_len * sizeof(newcr->cr_groups[0]));
		for (gp = &newcr->cr_groups[aup->aup_len];
		     gp < &newcr->cr_groups[NGROUPS];
		     gp++) {
			*gp = NOGROUP;
		}
		tmpcr = u.u_cred;
		u.u_cred = newcr;
	}

	/*
	 * Allocate results struct.
	 */
	res = (caddr_t *)rfsget();
	bzero((caddr_t)res, rfssize);

	svstat.reqs[which]++;

	/*
	 * Call service routine with arg struct and results struct
	 */
	if ((*disp->dis_proc)(args, res, req) == ESTALE)
		rfs_stalefh(xprt, which);

done:
	/*
	 * Free arguments struct
	 */
	if (!SVC_FREEARGS(xprt, disp->dis_xdrargs, args) ) {
		error++;
	}
	if (args != NULL) {
		rfsput((struct rfsspace *)args);
	}

	/*
	 * Serialize and send results struct
	 */
	if (!error && !dup_xid) {
		if (!svc_sendreply(xprt, disp->dis_xdrres, (caddr_t)res)) {
			error++;
		}
	}

	/*
	 * Free results struct
	 */
	if (res != NULL) {
		if ( disp->dis_resfree != nullfree ) {
			(*disp->dis_resfree)(res);
		}
		rfsput((struct rfsspace *)res);
	}
	/*
	 * restore original credentials
	 */
	if (newcr) {
		u.u_cred = tmpcr;
		crfree(newcr);
	}
	svstat.nbadcalls += error;
}

rfs_stalefh(xprt, which)
	SVCXPRT *xprt;
	int	which;
{

	printf(" op: %s, client address = %u.%u.%u.%u\n",
		rfsnames[which],
		xprt->xp_raddr.sin_addr.s_net,
		xprt->xp_raddr.sin_addr.s_host,
		xprt->xp_raddr.sin_addr.s_lh,
		xprt->xp_raddr.sin_addr.s_impno);

}
