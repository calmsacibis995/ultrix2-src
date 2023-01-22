#ifndef lint
static	char	*sccsid = "@(#)nfs_subr.c	1.17	(ULTRIX)	3/3/87";
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
#include "../h/kernel.h"
#include "../h/buf.h"
#include "../h/proc.h"
#include "../h/fs_types.h"
#include "../h/mount.h"
#include "../h/gnode.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/uio.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../netinet/in.h"
#include "../net/if.h"
#include "../nfs/nfs.h"
#include "../nfs/nfs_clnt.h"
#include "../nfs/vfs.h"
#include "../nfs/vnode.h"

#ifdef NFSDEBUG
int nfsdebug = 2;
#endif

int do_uprintf = 0;
extern struct mount_ops *nfs_ops;
struct gnode *rfind();
extern int cachedebug;

/*
 * Client side utilities
 */

/*
 * client side statistics
 */
struct {
	int	nclsleeps;		/* client handle waits */
	int	nclgets;		/* client handle gets */
	int	ncalls;			/* client requests */
	int	nbadcalls;		/* rpc failures */
	int	reqs[32];		/* count of each request */
} clstat;

#define MAXCLIENTS	6
struct chtab {
	int	ch_timesused;
	bool_t	ch_inuse;
	CLIENT	*ch_client;
} chtable[MAXCLIENTS];

int	clwanted = 0;

struct chtab *
clget(mi, cred)
	struct mntinfo *mi;
	struct ucred *cred;
{
	register struct chtab *ch;
	int retrans;

	/*
	 * If soft mount and server is down just try once
	 */
	if (!mi->mi_hard && mi->mi_down) {
		retrans = 1;
	} else {
		retrans = mi->mi_retrans;
	}

	/*
	 * Find an unused handle or create one if not at limit yet.
	 */
	for (;;) {
		clstat.nclgets++;
		for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
			if (!ch->ch_inuse) {
				ch->ch_inuse = TRUE;
				if (ch->ch_client == NULL) {
					ch->ch_client =
					    clntkudp_create(&mi->mi_addr,
					    NFS_PROGRAM, NFS_VERSION,
					    retrans, cred);
					if (ch->ch_client == NULL)
						panic("clget: null client");

				} else {
					clntkudp_init(ch->ch_client,
					    &mi->mi_addr, retrans, cred);
				}
				ch->ch_timesused++;
				return (ch);
			}
		}
		/*
		 * If we got here there are no available handles
		 */
		clwanted++;
		clstat.nclsleeps++;
		(void) sleep((caddr_t)chtable, PRIBIO);
	}
}

clfree(ch)
	struct chtab *ch;
{
	ch->ch_inuse = FALSE;
	if (clwanted) {
		clwanted = 0;
		wakeup((caddr_t)chtable);
	}
}

char *rpcstatnames[] = {
	"Success", "Can't encode arguments", "Can't decode result",
	"Unable to send", "Unable to receive", "Timed out",
	"Incompatible versions of RPC", "Authentication error",
	"Program unavailable", "Program/version mismatch",
	"Procedure unavailable", "Server can't decode arguments",
	"Remote system error", "Unknown host", "Port mapper failure",
	"Program not registered", "Failed (unspecified error)",
	"Interrupted"
	};

char *rfsnames[] = {
	"null", "getattr", "setattr", "unused", "lookup", "readlink", "read",
	"unused", "write", "create", "remove", "rename", "link", "symlink",
	"mkdir", "rmdir", "readdir", "fsstat" };

/*
 * Back off for retransmission timeout, MAXTIMO is in 10ths of a sec
 */
#define MAXTIMO	300
#define backoff(tim)	((((tim) << 2) > MAXTIMO) ? MAXTIMO : ((tim) << 2))

int
rfscall(mi, which, xdrargs, argsp, xdrres, resp, cred)
	register struct mntinfo *mi;
	int	 which;
	xdrproc_t xdrargs;
	caddr_t	argsp;
	xdrproc_t xdrres;
	caddr_t	resp;
	struct ucred *cred;
{
	struct chtab *cht;
	CLIENT *client;
	register enum clnt_stat status;
	struct rpc_err rpcerr;
	struct timeval wait;
	struct ucred *newcred;
	int timeo;
	int user_told;
	bool_t tryagain;

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "rfscall: %x, %d, %x, %x, %x, %x\n",
	    mi, which, xdrargs, argsp, xdrres, resp);
#endif

	clstat.ncalls++;
	clstat.reqs[which]++;

	rpcerr.re_errno = 0;
	newcred = NULL;
	timeo = mi->mi_timeo;
	user_told = 0;
retry:
	cht = clget(mi, cred);
	client = cht->ch_client;

	/*
	 * If hard mounted fs, retry call forever unless hard error occurs
	 */
	do {
		tryagain = FALSE;

		wait.tv_sec = timeo / 10;
		wait.tv_usec = 100000 * (timeo % 10);
		status = CLNT_CALL(client, which, xdrargs, argsp,
		    xdrres, resp, wait);
		switch (status) {
		case RPC_SUCCESS:
			break;

		/*
		 * Unrecoverable errors: give up immediately
		 */
		case RPC_AUTHERROR:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_VERSMISMATCH:
		case RPC_PROGVERSMISMATCH:
		case RPC_CANTDECODEARGS:
			break;

		default:
			if (mi->mi_hard) {
				if (mi->mi_int && interrupted()) {
					status = RPC_INTR;
					rpcerr.re_status = RPC_SYSTEMERROR;
					rpcerr.re_errno = EINTR;
					tryagain = FALSE;
					break;
				} else {
					tryagain = TRUE;
					timeo = backoff(timeo);
					if (!mi->mi_printed) {
						mi->mi_printed = 1;
	mprintf("NFS server %s not responding, still trying\n", mi->mi_hostname);
					}
					if (!user_told && u.u_ttyp) {
						user_told = 1;
	uprintf("NFS server %s not responding, still trying\n", mi->mi_hostname);
					}
				}
			}
		}
	} while (tryagain);

	if (status != RPC_SUCCESS) {
		if (status != RPC_INTR) {
			CLNT_GETERR(client, &rpcerr);
		}
		if (u.u_error == 0)
			u.u_error = EIO;
		clstat.nbadcalls++;
		if (!(which == RFS_STATFS && mi->mi_stats)) { 
			mi->mi_down = 1;
			mprintf("NFS %s failed for server %s: %s\n",
				rfsnames[which], mi->mi_hostname,
				rpcstatnames[(int)status]);
		}
		if (u.u_ttyp) {
			if (do_uprintf)
			uprintf("NFS %s failed for server %s: %s\n",
				rfsnames[which], mi->mi_hostname,
				rpcstatnames[(int)status]);
		}
	} else if (resp && *(int *)resp == EACCES &&
	    newcred == NULL && cred->cr_uid == 0 && cred->cr_ruid != 0) {
		/*
		 * Boy is this a kludge!  If the reply status is EACCES
		 * it may be because we are root (no root net access).
		 * Check the real uid, if it isn't root make that
		 * the uid instead and retry the call.
		 */
		newcred = crdup(cred);
		cred = newcred;
		cred->cr_uid = cred->cr_ruid;
		clfree(cht);
		goto retry;
	} else if (mi->mi_hard) {
		if (mi->mi_printed) {
			mprintf("NFS server %s ok\n", mi->mi_hostname);
			mi->mi_printed = 0;
		}
		if (user_told) {
			uprintf("NFS server %s ok\n", mi->mi_hostname);
		}
	} else {
		mi->mi_down = 0;
	}

	clfree(cht);
#ifdef NFSDEBUG
	dprint(nfsdebug, 7, "rfscall: returning %d\n", rpcerr.re_errno);
#endif
	if (newcred) {
		crfree(newcred);
	}

	return (rpcerr.re_errno);
}

/*
 * Check if this process got an interrupt from the keyboard while sleeping
 */
int
interrupted() 
{
	register struct proc *p = u.u_procp;
	register int s, smask, intr;

#define bit(a) 	(1<<(a-1))

	s = spl6();
	smask = p->p_sigmask;
	p->p_sigmask |= 
		~(bit(SIGHUP) | bit(SIGINT) | bit(SIGQUIT) | bit(SIGTERM));

	if (ISSIG(p)) {
		intr = TRUE;
	} else {
		intr = FALSE;
	}
	p->p_sigmask = smask;
	(void) splx(s);

	return(intr);
}

/*
 * Set vattr structure to a null value.
 */
void
vattr_null(vap)
	struct vattr *vap;
{
	register int n;
	register char *cp;

	n = sizeof(struct vattr);
	cp = (char *)vap;
	while (n--) {
		*cp++ = -1;
	}
}

int attrdebug = 0;

vattr_to_sattr(vap, sa)
	register struct vattr *vap;
	register struct nfssattr *sa;
{
	sa->sa_mode = vap->va_mode;
	sa->sa_uid = vap->va_uid;
	sa->sa_gid = vap->va_gid;
	sa->sa_size = vap->va_size;
	sa->sa_atime  = vap->va_atime;
	sa->sa_mtime  = vap->va_mtime;
if (attrdebug) {
	printf("vattr_to_sattr: atime: %d, %d    mtime: %d, %d\n",
		sa->sa_atime.tv_sec, sa->sa_atime.tv_usec,
		sa->sa_mtime.tv_sec, sa->sa_mtime.tv_usec);
}

}

setdiropargs(da, nm, dvp)
	struct nfsdiropargs *da;
	char *nm;
	struct vnode *dvp;
{

	da->da_fhandle = *vtofh(dvp);
	da->da_name = nm;
}

/*
 * Return a gnode for the given fhandle.  If no gnode exists for this
 * fhandle create one and put it in the gnode table.  If a gnode is found,
 * return it with its reference count incremented.  KLUDGE: the GFS buffer
 * hashing scheme will not work unless we reuse the same gnode slot when
 * we reopen a given file.  For this reason we leave gnodes on their
 * hash chain when we free them, so rfind can find them and reclaim them.
 * We must take care when reinitializing reclaimed gnodes; some parts must
 * be reinitialized and others must be kept from the last invocation (such
 * as the modify time).
 */
struct vnode *
makenfsnode(fh, attr, vfsp, mp)
	fhandle_t *fh;
	struct nfsfattr *attr;
	struct vfs *vfsp;
	struct mount *mp;
{
	char newnode = 0;
	register struct gnode *gp;
	register struct vnode *vp;
	gno_t	gno;

	gno = (attr) ? attr->na_nodeid : fh->fh_fno;
	if ((gp = rfind(mp->m_dev, gno, fh)) == NULL) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 4, "makenfsnode: mp 0x%x\n", mp);
#endif
		if((gp = getegnode(GNOHASH(mp->m_dev, gno),
			mp->m_dev, gno)) == NULL)
			return(NULL);
		gp->g_count = 1;
		newnode++;
	}

	/*
	 * Initialize the gnode if this is the first reference (the gnode
	 * is either new or reclaimed).
	 */

	if (gp->g_count == 1) {
		gp->g_mp = mp;
		gp->g_number = gno;
		vp = (struct vnode *) gp;
		vtor(vp)->r_fh = *fh;
		if (attr)
			vp->v_type = (enum vtype)attr->na_type;
		vp->v_vfsp = vfsp;
		((struct mntinfo *)(vfsp->vfs_data))->mi_refct++;

/*		if(gp->g_flag & GLOCKED) {
			gp->g_flag &= ~GLOCKED;
			gp->g_flag |= VROOT;
		}
*/
	}
	if (cachedebug) {
		if (vtor((struct vnode *)gp)->r_fh.fh_fgen != fh->fh_fgen)
			printf("makenfsnode: stale gnode in cache\n");
	}
	if (attr) {
		nfs_attrcache(gp, attr, (newnode? NOFLUSH: SFLUSH));
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "makenfsnode: returning gp 0x%x, #%d\n",
		 gp, gp->g_number);
#endif
	return ((struct vnode *)gp);
}

int cachedebug = 0;
 
struct gnode *
rfind(fsid, gno, fh)
	register int gno;
	register int fsid;
	fhandle_t *fh;
{
	register struct ghead *ih;
	register struct gnode *gp;
	register struct gnode *gq;
	 
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfind: fsid 0x%x gnumber %d\n", fsid, gno);
#endif
	ih = &ghead[GNOHASH(fsid, gno)];
	for (gp = ih->gh_chain[0]; gp != (struct gnode *)ih; gp = gp->g_forw) {
		if (gno == gp->g_number && fsid == gp->g_dev) {

			if (bcmp(vtofh((struct vnode *)gp), fh, sizeof(*fh))) {
if (cachedebug) printf("rfind: rejected stale gnode, #%d, 0x%x\n",
gp->g_number, gp);
				continue;
			}
			if (cachedebug) {
				printf("\nrfind: found gnode #%d, 0x%x\n", gp->g_number, gp);
				printf("rfind: it has a use count of %d\n", gp->g_count);
			}
#ifdef NFSDEBUG
			dprint(nfsdebug, 4, "rfind: gp 0x%x (%d) cache match\n",
			gp, gp->g_number);
#endif NFSDEBUG

			if (gp->g_count == 0) {
				gremque(gp);
				bzero(vtor((struct vnode *)gp), sizeof(struct rnode));
			}
			gp->g_count++;
			return(gp);
		}
	}
	return(NULL);
}

#define	PREFIXLEN	4
static char prefix[PREFIXLEN+1] = ".nfs";

char *
newname(s)
	char *s;
{
	char *news;
	register char *s1, *s2;
	int id;

	id = time.tv_usec;
	news = (char *)kmem_alloc((u_int)NFS_MAXNAMLEN);
	for (s1 = news, s2 = prefix; s2 < &prefix[PREFIXLEN]; ) {
		*s1++ = *s2++;
	}
	while (id) {
		*s1++ = "0123456789ABCDEF"[id & 0x0f];
		id = id >> 4;
	}
	*s1 = '\0';
	return (news);
}


/*
 * Server side utilities
 */

vattr_to_nattr(vap, na)
	register struct vattr *vap;
	register struct nfsfattr *na;
{

	na->na_type = (enum nfsftype)vap->va_type;
	na->na_mode = vap->va_mode;
	na->na_uid = vap->va_uid;
	na->na_gid = vap->va_gid;
	na->na_fsid = vap->va_fsid;
	na->na_nodeid = vap->va_nodeid;
	na->na_nlink = vap->va_nlink;
	na->na_size = vap->va_size;
	na->na_atime = vap->va_atime;
	na->na_mtime = vap->va_mtime;
	na->na_ctime = vap->va_ctime;
	na->na_rdev = vap->va_rdev;
	na->na_blocks = vap->va_blocks;
	na->na_blocksize = vap->va_blocksize;
}

sattr_to_vattr(sa, vap)
	register struct nfssattr *sa;
	register struct vattr *vap;
{
	vattr_null(vap);
	vap->va_mode = sa->sa_mode;
	vap->va_uid = sa->sa_uid;
	vap->va_gid = sa->sa_gid;
	vap->va_size = sa->sa_size;
	vap->va_atime = sa->sa_atime;
	vap->va_mtime = sa->sa_mtime;
}

/*
 * Make an fhandle from a ufs gnode
 */
makefh(fh, gp)
	register fhandle_t *fh;
	struct gnode *gp;
{
	if (gp->g_mp->m_ops == nfs_ops)
		return(EREMOTE);
	bzero((caddr_t)fh, NFS_FHSIZE);
	fh->fh_fsid = gp->g_dev;
	fh->fh_fno = gp->g_number;
	fh->fh_fgen = gp->g_gennum;
	return (0);
}

int stalefh_count = 0;
extern int nobody;

/*
 * Convert an fhandle into a vnode.
 * Uses the inode number in the fhandle (fh_fno) to get the locked inode.
 * The inode is unlocked and used to get the vnode.
 * WARNING: users of this routine must do a VN_RELE on the vnode when they
 * are done with it.
 */
struct vnode *
fhtovp(fh)
	fhandle_t *fh;
{

	struct mount *mp;
	struct vnode *vp;
	extern struct mount *getmp();

	GETMP(mp, fh->fh_fsid);
	if (mp == NULL) {
		printf("NFS server: stale fhandle, fs(%d,%d) file %d,",
			major(fh->fh_fsid), minor(fh->fh_fsid), 
			fh->fh_fno);
		return (NULL);
	}
	vp = (struct vnode *) GGET(fh->fh_fsid, mp, fh->fh_fno, NOMOUNT);
	if (vp == NULL) {
	printf("NFS server: fhtovp, fs(%d, %d) file %d, couldn't gget,",
			major(fh->fh_fsid), minor(fh->fh_fsid), 
			fh->fh_fno);
		return (NULL);
	}

	gfs_unlock(vp);
	if (vp->g_gennum != fh->fh_fgen || vp->g_nlink <= 0) {
		printf("NFS server: stale fhandle, fs(%d,%d) file %d,",
			major(fh->fh_fsid), minor(fh->fh_fsid), 
			fh->fh_fno);
		stalefh_count++;		
		if(vp->g_count == 1) {
			vp->g_count = 0;
			freegnode(vp);
		} else
			(void) GRELE(vp);
		return (NULL);
	}

	return (vp);
}

/*
 * General utilities
 */

/*
 * Returns the prefered transfer size in bytes based on
 * what network interfaces are available.
 */
nfstsize()
{
	return (8192);
}

#ifdef NFSDEBUG
/*
 * Utilities used by both client and server
 * Standard levels:
 * 0) no debugging
 * 1) hard failures
 * 2) soft failures
 * 3) current test software
 * 4) main procedure entry points
 * 5) main procedure exit points
 * 6) utility procedure entry points
 * 7) utility procedure exit points
 * 8) obscure procedure entry points
 * 9) obscure procedure exit points
 * 10) random stuff
 * 11) all <= 1
 * 12) all <= 2
 * 13) all <= 3
 * ...
 */

/*VARARGS2*/
dprint(var, level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	int var;
	int level;
	char *str;
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	if (var == level || (var > 10 && (var - 10) >= level))
		printf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif
