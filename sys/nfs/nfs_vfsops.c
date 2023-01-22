#ifndef lint
static	char	*sccsid = "@(#)nfs_vfsops.c	1.18	(ULTRIX)	3/3/87";
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
#include "../h/user.h"
#include "../h/uio.h"
#include "../h/fs_types.h"
#include "../h/mount.h"
#include "../h/socket.h"
#include "../h/fs_types.h"
#include "../h/ioctl.h"
#include "../net/if.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_gfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/vnode.h"


#ifdef NFSDEBUG
extern int nfsdebug;
#endif

extern int turn_on_sync;
struct vnode *makenfsnode();
int nfsmntno;

/*
 * nfs vfs operations.
 */
struct mount *nfs_mount();
int nfs_statfs();

int		nfs_umount(),	nfs_sbupdate();
struct gnode	*ufs_gget(),	*nfs_namei();
int		nfs_glink(),	nfs_unlink(),	nfs_grmdir();
struct gnode	*nfs_gmkdir();
struct gnode	*nfs_makenode();
int		nfs_grename(),	nfs_readdir();
int		nfs_rele();
struct gnode	*ufs_galloc();
int		nfs_syncgp(),	ufs_gfree(),	nfs_trunc();
int		nfs_rwgp(),	nfs_stat();
int		nfs_lock(),	nfs_unlock();
int		nfs_update(),	nfs_open();
int		nfs_close(),	nfs_getval();
int		nfs_select(),	nfs_greadlink(), nfs_gsymlink();
struct fs_data	*nfs_getfsdata();
int		ufs_fcntl(), nfs_strategy(), nfs_gbmap();
int		ufs_seek();

struct	mount_ops NFS_ops = {
	nfs_umount,
	nfs_sbupdate,
	0,		/* gget */
	nfs_namei,
	nfs_glink,
	nfs_unlink,
	nfs_gmkdir,
	nfs_grmdir,
	nfs_makenode,
	nfs_grename,
	nfs_readdir,
	nfs_rele,
	nfs_syncgp,
	nfs_trunc,
	nfs_getval,
	nfs_rwgp,
	0,		/* rlock */
	ufs_seek,	/* seek */
	nfs_stat,
	nfs_lock,
	nfs_unlock,
	nfs_update,
	nfs_open,
	nfs_close,
	nfs_select,
	nfs_greadlink,
	nfs_gsymlink,
	nfs_getfsdata,
	0,		/* fcntl */
	0,
	nfs_gbmap
};
struct  mount_ops  *nfs_ops = &NFS_ops;

/*
 * nfs_mount is called from the generic mount system call
 */

struct mount *
nfs_mount(special, path, flag, mp, ops)
caddr_t special;
caddr_t path;
int flag;
struct mount *mp;
struct nfs_gfs_mount *ops;
{

	struct vnode *rootvp = NULL;	/* root vnode */
	struct vfs *vfsp;		/* nfs vfs handle */
	struct mntinfo *mi;		/* mount info, pointed at by vfs */
	struct vattr va;		/* root vnode attributes */
	struct nfsfattr na;		/* root vnode attributes in nfs form */
	struct nfs_gfs_mount nfs_gfs_mount;
	struct nfs_gfs_mount *ngp = &nfs_gfs_mount;
	struct statfs sb;		/* server's file system stats */
	fhandle_t fh;
	int blocksize;
	struct arpreq arpcmd;
	int arperr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_mount: special=%s, path=%s, flag=%d\n",
		special, path, flag);
	dprint(nfsdebug, 4, "nfs_mount: mp = 0x%x, ops = 0x%x\n",
		mp, ops);
#endif
	/*
	 * Copy NFS mount arguments out of userspace
	 */
	u.u_error = copyin((caddr_t)ops, (caddr_t)ngp, sizeof(nfs_gfs_mount));
	if (u.u_error)
		goto error;

	/*
	 * Set up data structures in fsdata spare area
	 */
	mi = MP_TO_MIP(mp);
	vfsp = MP_TO_VFSP(mp);
	mp->m_bufp = (struct buf *) (NODEV);	/* to reserve this slot */
	mp->m_dev = getpdev();			/* pseudo-device number */
	mp->iostrat = nfs_strategy;		/* set it immediately */
	mp->m_ops = nfs_ops;
	mp->m_fstype = GT_NFS;
	mp->m_flags = (flag ? M_RONLY : 0);
	mp->m_flags |= (ngp->gfs_flags &
			(M_NOEXEC | M_NOSUID | M_NODEV | M_FORCE | M_SYNC));

	if (ngp->flags & NFSMNT_PGTHRESH) {
		int pg_thresh = ngp->pg_thresh * 1024;
		mp->m_fs_data->fd_pgthresh =
			clrnd(btoc((pg_thresh > MINPGTHRESH) ?
				pg_thresh : MINPGTHRESH));
	} else {
		mp->m_fs_data->fd_pgthresh = clrnd(btoc(MINPGTHRESH * 8));
	}

	if (turn_on_sync == 0)
		mp->m_flags &= ~M_SYNC;


	u.u_error = copyin((caddr_t)ngp->fh, (caddr_t)&fh, sizeof(fh));
	if (u.u_error)
		goto error;
	u.u_error = copyin((caddr_t)ngp->addr, (caddr_t)(&mi->mi_addr), sizeof(mi->mi_addr));
	if (u.u_error)
		goto error;
	if (!(ngp->flags & NFSMNT_HOSTNAME))		/* XXX */
		addr_to_str(&(mi->mi_addr), mi->mi_hostname);
	else {
		u.u_error = copyinstr(ngp->hostname, mi->mi_hostname,
			HOSTNAMESZ, (caddr_t)0);
		if (u.u_error)
			goto error;
	}

	if (ngp->optstr) {
		u.u_error = copyinstr(ngp->optstr, mi->mi_optstr,
			MNTMAXSTR, (caddr_t)0);
		if (u.u_error)
			goto error;
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
		"nfs_mount: saddr 0x%x, fh dev 0x%x gno %d\n",
		&(mi->mi_addr), fh.fh_fsid, fh.fh_fno);
#endif

	/*
	 * Create a mount record
	 */
	mi->mi_refct = 0;
	mi->mi_stsize = 0;
	mi->mi_hard = ((ngp->flags & NFSMNT_SOFT) == 0);
	mi->mi_int = ((ngp->flags & NFSMNT_INT) == NFSMNT_INT);
	if (ngp->flags & NFSMNT_RETRANS) {
		mi->mi_retrans = ngp->retrans;
		if (ngp->retrans < 0) {
			u.u_error = EINVAL;
			goto error;
		}
	} else {
		mi->mi_retrans = NFS_RETRIES;
	}
	if (ngp->flags & NFSMNT_TIMEO) {
		mi->mi_timeo = ngp->timeo;
		if (ngp->timeo <= 0) {
			u.u_error = EINVAL;
			goto error;
		}
	} else {
		mi->mi_timeo = NFS_TIMEO;
	}
	mi->mi_mntno = nfsmntno++;
	mi->mi_printed = 0;
	mi->mi_stats = 0;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_mount: mi_addr 0x%x port %d addr 0x%x\n",
		&mi->mi_addr, mi->mi_addr.sin_port, (int)mi->mi_addr.sin_addr.s_addr);

#endif

	/*
	 * For now we just support AF_INET
	 */
	if (mi->mi_addr.sin_family != AF_INET) {
		u.u_error = EPFNOSUPPORT;
		goto error;
	}

	/*
	 * Make a vfs struct for nfs.  We do this here instead of below
	 * because rootvp needs a vfs before we can do a getattr on it.
	 */
	VFS_INIT(vfsp, (caddr_t)mi);

	/*
	 * Make the root vnode
	 */
	rootvp = makenfsnode(&fh, (struct nfsfattr *) 0, vfsp, mp);
	if(rootvp == NULL) {
		goto error;
	}
	if (rootvp->v_flag & VROOT) {
		u.u_error = EBUSY;
		goto error;
	}

	/*
	 * Get real attributes of the root vnode, and remake it.
	 * While we're at it, get the transfer size for this filesystem.
	 */
	u.u_error = nfs_makeroot(&fh, &rootvp, &blocksize);
	if (u.u_error) {
		goto error;
	}

	rootvp->v_flag |= VROOT;
	mp->m_rootgp = (struct gnode *) rootvp;
	mi->mi_rootvp = rootvp;

	if(u.u_uid) {
		((struct gnode *) rootvp)->g_uid = u.u_uid;
		((struct gnode *) rootvp)->g_flag |= GCHG;
	}

	/*
	 * Get server's filesystem stats.  Use these to set transfer
	 * sizes, filesystem block size, and read-only.
	 */
	mi->mi_tsize = min(NFS_MAXDATA, nfstsize());
	if (ngp->flags & NFSMNT_RSIZE) {
		if (ngp->rsize <= 0) {
			u.u_error = EINVAL;
			goto error;
		}
		mi->mi_tsize = MIN(mi->mi_tsize, ngp->rsize);
	}

	if (ngp->flags & NFSMNT_WSIZE) {
		if (ngp->wsize <= 0) {
			u.u_error = EINVAL;
			goto error;
		}
		mi->mi_stsize = ngp->wsize; /* touched by nfs_getfsdata */
	}

	(void) nfs_getfsdata(mp);
	if (u.u_error)
		goto error;

#ifdef NFSDEBUG
        dprint(nfsdebug, 10,
		"nfs_mount: vfs %x: vnodecov = %x,	data = %x\n",
		vfsp, vfsp->vfs_vnodecovered, vfsp->vfs_data);
	dprint(nfsdebug, 10, "rootvp %x: vfs %x\n",
		rootvp, rootvp->v_vfsp);
	dprint(nfsdebug, 4,
	    "nfs_mount: hard %d timeo %d retries %d wsize %d rsize %d\n",
	    mi->mi_hard, mi->mi_timeo, mi->mi_retrans, mi->mi_stsize,
	    mi->mi_tsize);
	dprint(nfsdebug, 4, "nfs_mount: mp->m_flags = 0x%x\n",
		mp->m_flags);
	dprint(nfsdebug, 4, "nfs_mount: fd_otsize = %d, fd_mtsize=%d\n",
		mp->m_fs_data->fd_otsize, mp->m_fs_data->fd_mtsize);
#endif

	/*
	 * Should set read only here!
	 */

	/*
	 * Set filesystem block size to at least CLBYTES and at most MAXBSIZE
	 */

	mi->mi_bsize = MAX(CLBYTES, blocksize);
	mi->mi_bsize = MIN(mi->mi_bsize, MAXBSIZE);
	vfsp->vfs_bsize = mi->mi_bsize;
	mp->m_bsize = mi->mi_bsize;

	/*
	 * Now that we have set up communication with the server,
	 * make his arp table entry permanent. We do this to avoid
	 * nasty arpresolve timeouts on down servers after a
	 * temporary entry has been flushed. These nasty timeouts
	 * are seen in clntkudp_callit where we sleep on an mbuf
	 * that is being held in an arp trash area for cleanup.
	 */
	bzero((caddr_t)&arpcmd, sizeof(struct arpreq));
	bcopy((caddr_t)&mi->mi_addr, (caddr_t)&arpcmd.arp_pa,
		sizeof(struct sockaddr));
	arpcmd.arp_ha.sa_family = AF_UNSPEC;
	if (arperr = arpioctl(SIOCGARP, (caddr_t)&arpcmd))
		printf("nfs_mount: bad get arp, err %d\n", arperr);
	else {
		arpcmd.arp_flags |= ATF_PERM;
		if (arperr = arpioctl(SIOCSARP,	(caddr_t)&arpcmd))
			printf("nfs_mount: bad set arp, err %d\n", arperr);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 4,
	    "nfs_mount: vfs_bsize %d\n",vfsp->vfs_bsize);
#endif

	dnlc_purge();
	return(mp);
error:
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_mount: returning error, %d\n",
		u.u_error);
#endif
	if (u.u_error) {
		if (rootvp) {
			VN_RELE(rootvp);
		}
	}
	return((struct mount *)NULL);
}

#ifdef notneeded
/*
 * Called by vfs_mountroot when nfs is going to be mounted as root
 */
nfs_mountroot()
{

	return(EOPNOTSUPP);
}
#endif

/*
 * Get file system statistics.
 */
int
nfs_statfs(mp, sbp)
register struct mount *mp;
struct statfs *sbp;
{
	struct nfsstatfs fs;
	struct mntinfo *mi;
	fhandle_t *fh;
	struct mntinfo nmi;

	mi = MP_TO_MIP(mp);

	/* HACK a local mntinfo structure to avoid hard mounted */
	/* down server problems. Make sure NOT to do this on */
	/* the first call (from nfs_mount) */
	nmi = *mi;
	if (mi->mi_stats) {
		nmi.mi_hard = 0;
		nmi.mi_retrans = 1;
	}

	fh = vtofh(mi->mi_rootvp);

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_statfs fh %o %d\n", fh->fh_fsid, fh->fh_fno);
#endif
	u.u_error = rfscall(&nmi, RFS_STATFS, xdr_fhandle,
	    (caddr_t)fh, xdr_statfs, (caddr_t)&fs, u.u_cred);
	if (u.u_error) {
		register struct fs_data *fsd = mp->m_fs_data;
		if (u.u_error == ETIMEDOUT) {
			u.u_error = 0;
			sbp->f_bsize = fsd->fd_bsize;
			sbp->f_blocks = fsd->fd_btot;
			sbp->f_bfree = fsd->fd_bfree;
			sbp->f_bavail = fsd->fd_bfreen;
		}
	} else {
		if (!(u.u_error = geterrno(fs.fs_status))) {
			if (mi->mi_stsize) {
				mi->mi_stsize = min(mi->mi_stsize, fs.fs_tsize);
			} else {
				mi->mi_stsize = fs.fs_tsize;
			}
			sbp->f_bsize = fs.fsstat_bsize;
			sbp->f_blocks = fs.fs_blocks;
			sbp->f_bfree = fs.fs_bfree;
			sbp->f_bavail = fs.fs_bavail;
			/*
			 * XXX This is wrong - should be a real fsid
			 */
			bcopy((caddr_t)&fh->fh_fsid, (caddr_t)sbp->f_fsid,
			    sizeof(fsid_t));
			mi->mi_stats = 1;
		}
	}
	
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_statfs returning %d\n", u.u_error);
	dprint(nfsdebug, 5, "fs_tsize = %d, fsstat_bsize = %d, fs_blocks = %d\n",
		fs.fs_tsize, fs.fsstat_bsize, fs.fs_blocks);
	dprint(nfsdebug, 5, "fs_bfree = %d, fs_bavail = %d\n",
		fs.fs_bfree, fs.fs_bavail);

#endif
	return (u.u_error);
}

static char *
itoa(n, str)
	u_short n;
	char *str;
{
	char prbuf[11];
	register char *cp;

	cp = prbuf;
	do {
		*cp++ = "0123456789"[n%10];
		n /= 10;
	} while (n);
	do {
		*str++ = *--cp;
	} while (cp > prbuf);
	return (str);
}

/*
 * Convert a INET address into a string for printing
 */
addr_to_str(addr, str)
	struct sockaddr_in *addr;
	char *str;
{
	str = itoa(addr->sin_addr.s_net, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_host, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_lh, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_impno, str);
	*str = '\0';
}

int
nfs_makeroot(fh, vpp, blocksizep)
	fhandle_t *fh;
	struct vnode **vpp;
	int *blocksizep;
{
	struct nfsattrstat *ns;
	int error = 0;
	struct mount *mp;
	struct vfs *vfsp;
	struct vnode *vp;
	
	vp = *vpp;
	mp = (*vpp)->g_mp;
	vfsp = (*vpp)->v_vfsp;

	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	error = rfscall(vtomi(*vpp), RFS_GETATTR, xdr_fhandle, (caddr_t)vtofh(vp),
	    xdr_attrstat, (caddr_t)ns, u.u_cred);
	if (!error) {
		if (error = geterrno(ns->ns_status))
		goto out;
	}
	nfs_rele(*vpp);
	*vpp = makenfsnode(fh, &(ns->ns_attr), vfsp, mp);
	if(*vpp != NULL)
		*blocksizep = ns->ns_attr.na_blocksize;
	else
		error = u.u_error;
out:
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
	return(error);
}
