#ifndef lint
static	char	*sccsid = "@(#)kern_exec.c	1.17	(ULTRIX)	1/29/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
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
 *
 *   Modification history:
 *
 * 29 Jan 87
 *	Add new arg to bdwrite() calls.
 *
 * 15 Dec 86 -- depp
 *	Fixed problem with error return from rdwri (in getxfile()).
 *
 * 11 Sep 86 -- koehler
 *	gfs namei interface change
 *
 * 02 Apr 86 -- depp
 *	Integrated two performance changes into execve.  From 4.3UCB, 
 *	arguments are now copied by strings rather than by character, and
 *	the a.out header information (struct exec) is now placed on the 
 *	stack, rather than in the process' "u"-area.  From SUN, we
 *	now will fully read in small 0413 executables, rather than
 *	demand paging them in.
 *
 * 12-Feb-86 -- jrs
 *	Added calls to tbsync() to control mp translation buffer
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 14 Oct 85 -- Reilly
 *	Modified user.h
 *
 * 18 Sep 85 -- depp
 *	Added punlock call to clear memory locks before text detach (execve)
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code.
 *
 * 11 Mar 85 -- depp
 *	Added in System V shared memory support
 *
 *  05-May-85 - Larry Cohen
 *	loop up to u_omax instead of NOFILE
 *
 *  19 Jul 85 -- depp
 *	Removed call to smexec as this call is made in vrelvm as smclean
 *
 */

#include "../machine/reg.h"
#include "../machine/pte.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/gnode.h"
#include "../h/mount.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/text.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/acct.h"
#include "../h/exec.h"
#include "../h/kmalloc.h"

#ifdef vax
#include "../vax/mtpr.h"
#endif

#ifdef GFSDEBUG
extern short GFS[];
#endif

 
/* SETREGS currently only sets the PC, but in the future ...? */
#define SETREGS(x)	(u.u_ar0[PC] = (x) + 2)

/*
 * text+data (not bss) smaller than fd_pgthresh will be read in (if there
 * is enough free memory), even though the file is 0413
 */

#define IS_SPAGI(ep,gp,ts)	\
	(((ep)->a_magic == 0413) && \
	(((ts) + clrnd(btoc((ep)->a_data))) > \
	MIN(freemem,(gp)->g_mp->m_fs_data->fd_pgthresh)))

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

execv()
{
	((struct execa *)u.u_ap)->envp = NULL;
	execve();
}

execve()
{
	register nc;
	register char *cp;
	register struct buf *bp = 0;
	register struct execa *uap;
	register struct proc *p = u.u_procp;
	int na, ne, ucp, ap, len, cc;
	int indir, uid, gid;
	char *sharg;
	struct gnode *gp;
	swblk_t bno = 0;
	char cfname[MAXCOMLEN + 1];
	char cfarg[SHSIZE];
	union {
		char	ex_shell[SHSIZE];  /* #! and name of interpreter */
		struct	exec ex_exec;
	} exdata;
	register struct nameidata *ndp = &u.u_nd;
	char *fname = ((struct execa *) u.u_ap)->fname;
	int resid, error, s;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	
	
	if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
		return;
	}
 	if(u.u_error = copyinstr(fname, ndp->ni_dirp, MAXPATHLEN, (u_int *)
	0)) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}

	if ((gp = gfs_namei(ndp)) == NULL) {
		km_free(ndp->ni_dirp, MAXPATHLEN);
		return;
	}
	
	km_free(ndp->ni_dirp, MAXPATHLEN);
	if((gp->g_mp->m_flags & M_NOEXEC) && u.u_uid) {
		u.u_error = EROFS;
		goto bad;
	}

	if((gp->g_mp->m_flags & M_NOSUID) && (gp->g_mode & (GSUID | GSGID))
	&& u.u_uid) {
		u.u_error = EROFS;
		goto bad;
	}
	
#ifdef GFSDEBUG
	if(GFS[7])
		cprintf("execve: gp 0x%x (%d)\n", gp, gp->g_number);
#endif
	bno = 0;
	bp = 0;
	indir = 0;
	uid = u.u_uid;
	gid = u.u_gid;
	if (gp->g_mode & GSUID)
		uid = gp->g_uid;
	if (gp->g_mode & GSGID)
		gid = gp->g_gid;

#ifdef GFSDEBUG
	if(GFS[7])
		cprintf("execve: uid (%d) u.u_uid %d cdir 0x%x (%d) count %d\n",
		uid, u.u_uid, u.u_cdir, u.u_cdir->g_number, u.u_cdir->g_count);
#endif
  again:
	if (access(gp, GEXEC))
		goto bad;
	if ((u.u_procp->p_flag&STRC) && access(gp, GREAD))
		goto bad;
	if ((gp->g_mode & GFMT) != GFREG ||
	   (gp->g_mode & (GEXEC|(GEXEC>>3)|(GEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}

	/*
	 * Read in first few bytes of file for segment sizes, magic number:
	 *	407 = plain executable
	 *	410 = RO text
	 *	413 = demand paged RO text
	 * Also an ASCII line beginning with #! is
	 * the file name of a ``shell'' and arguments may be prepended
	 * to the argument list if given here.
	 *
	 * SHELL NAMES ARE LIMITED IN LENGTH.
	 *
	 * ONLY ONE ARGUMENT MAY BE PASSED TO THE SHELL FROM
	 * THE ASCII LINE.
	 */
	exdata.ex_shell[0] = '\0';	/* for zero length files */
	u.u_error = rdwri(UIO_READ, gp, (caddr_t)&exdata, sizeof (exdata),
	    0, 1, &resid);
	if (u.u_error)
		goto bad;
#ifndef lint
	if (resid > sizeof(exdata) - sizeof(exdata.ex_exec) &&
	    exdata.ex_shell[0] != '#') {
		u.u_error = ENOEXEC;
		goto bad;
	}
#endif
	switch (exdata.ex_exec.a_magic) {

	case 0407:
		exdata.ex_exec.a_data += exdata.ex_exec.a_text;
		exdata.ex_exec.a_text = 0;
		break;

	case 0413:
	case 0410:
		if (exdata.ex_exec.a_text == 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		break;

	default:
		if (exdata.ex_shell[0] != '#' ||
		    exdata.ex_shell[1] != '!' ||
		    indir) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &exdata.ex_shell[2];		/* skip "#!" */
		while (cp < &exdata.ex_shell[SHSIZE]) {
			if (*cp == '\t')
				*cp = ' ';
			else if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			cp++;
		}
		if (*cp != '\0') {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &exdata.ex_shell[2];
		while (*cp == ' ')
			cp++;
		ndp->ni_dirp = cp;
		while (*cp && *cp != ' ')
			cp++;
		cfarg[0] = '\0';
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp)
				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
		}
		indir = 1;
		gput(gp);
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		gp = gfs_namei(ndp);
		if (gp == NULL)
			return;
#ifdef GFSDEBUG
		if(GFS[7])
			cprintf("execve: gp 2 0x%x (%d)\n", gp, gp->g_number);
#endif
		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)cfname,
		    MAXCOMLEN);
		cfname[MAXCOMLEN] = '\0';
		goto again;
	}

	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	cc = 0;
	uap = (struct execa *)u.u_ap;
	bno = rmalloc(argmap, (long)ctod(clrnd((int)btoc(NCARGS))));
	if (bno == 0) {
		swkill(p, "exec: no swap space");
		goto bad;
	}
	if (bno % CLSIZE)
		panic("execa rmalloc");
	/*
	 * Copy arguments into file in argdev area.
	 */
	if (uap->argp) for (;;) {
		ap = NULL;
		sharg = NULL;
		if (indir && na == 0) {
			sharg = cfname;
			ap = (int)sharg;
			uap->argp++;		/* ignore argv[0] */
		} else if (indir && (na == 1 && cfarg[0])) {
			sharg = cfarg;
			ap = (int)sharg;
		} else if (indir && (na == 1 || na == 2 && cfarg[0]))
			ap = (int)uap->fname;
		else if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap == NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) != NULL)
				uap->envp++, ne++;
		}
		if (ap == NULL)
			break;
		na++;
		if (ap == -1) {
			u.u_error = EFAULT;
			break;
		}
		do {
			if (cc <= 0) {
				/*
				 * We depend on NCARGS being a multiple of
				 * CLSIZE*NBPG.  This way we need only check
				 * overflow before each buffer allocation.
				 */
				if (nc >= NCARGS-1) {
					error = E2BIG;
					break;
				}
				if (bp)
					bdwrite(bp, (struct gnode *)0);
				cc = CLSIZE*NBPG;
				bp = getblk(argdev, bno + ctod(nc/NBPG), cc,gp);
				cp = bp->b_un.b_addr;
			}
			if (sharg) {
				error = copystr(sharg, cp, cc, &len);
				sharg += len;
			} else {
				error = copyinstr((caddr_t)ap, cp, cc, &len);
				ap += len;
			}
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENOENT);
		if (error) {
			u.u_error = error;
			if (bp)
				brelse(bp);
			bp = 0;
			goto badarg;
		}
	}
	if (bp)
		bdwrite(bp, (struct gnode *)0);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(gp, &exdata.ex_exec, nc + (na+4)*NBPW, uid, gid);
	if (u.u_error) {
badarg:
		for (cc = 0; cc < nc; cc += CLSIZE*NBPG) {
			bp = baddr(argdev, bno + ctod(cc/NBPG), CLSIZE*NBPG,gp);
			if (bp) {
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				brelse(bp);
				bp = 0;
			}
		}
		goto bad;
	}

	/*
	 * Copy back arglist.
	 */
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	cc = 0;
	for (;;) {
		ap += NBPW;
		if (na == ne) {
			(void) suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);
		do {
			if (cc <= 0) {
				if (bp)
					brelse(bp);
				cc = CLSIZE*NBPG;
				bp = bread(argdev, bno + ctod(nc / NBPG),cc,gp);
				bp->b_flags |= B_AGE;		/* throw away */
				bp->b_flags &= ~B_DELWRI;	/* cancel io */
				cp = bp->b_un.b_addr;

			}
			error = copyoutstr(cp, (caddr_t)ucp, cc, &len);
			ucp += len;
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENOENT);
		if (error == EFAULT)
			panic("exec: EFAULT");
	}
	(void) suword((caddr_t)ap, 0);

	/*
	 * Reset caught signals.  Held signals
	 * remain held through p_sigmask.
	 */
	while (u.u_procp->p_sigcatch) {
		s = spl6();
		nc = ffs(u.u_procp->p_sigcatch);
		u.u_procp->p_sigcatch &= ~(1 << (nc - 1));
		u.u_signal[nc] = SIG_DFL;
		splx(s);
	}
#ifdef notdef
	/*
	 * Reset stack state to the user stack.
	 * Clear set of signals caught on the signal stack.
	 */
	u.u_onstack = 0;
	u.u_sigsp = 0;
	u.u_sigonstack = 0;
#endif notdef

	for (nc = u.u_omax; nc >= 0; --nc) {
		if (u.u_pofile[nc] & UF_EXCLOSE) {
			closef(u.u_ofile[nc]);
			u.u_ofile[nc] = NULL;
			u.u_pofile[nc] = 0;
		}
		u.u_pofile[nc] &= ~UF_MAPPED;
	}
#ifdef notdef
	while (u.u_omax >= 0 && u.u_ofile[u.u_omax] == NULL)
		u.u_omax--;
#endif notdef
	SETREGS(exdata.ex_exec.a_entry);
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	if (indir)
		bcopy((caddr_t)cfname, (caddr_t)u.u_comm, MAXCOMLEN);
	else {
		if (ndp->ni_dent.d_namlen > MAXCOMLEN)
			ndp->ni_dent.d_namlen = MAXCOMLEN;
		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)u.u_comm,
		    (unsigned)(ndp->ni_dent.d_namlen + 1));
	}
bad:
	if (bp)
		brelse(bp);
	if (bno)
		rmfree(argmap, (long)ctod(clrnd((int) btoc(NCARGS))), bno);
	if (gp) {
		gput(gp);
	}
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(gp, ep, nargc, uid, gid)
	register struct gnode *gp;
	register struct exec *ep;
	int nargc, uid, gid;
{
	register struct proc *p = u.u_procp;
	size_t ts, ds, ss;
	int pagi = 0;
	struct text *xp;
	
	if (gp->g_flag & GXMOD) {			/* XXX */
		u.u_error = ETXTBSY;
		goto bad;
	}
	if (ep->a_text != 0 && (gp->g_flag&GTEXT) == 0 && gp->g_count != 1) {
		/* check to see if someone has a descriptor for write */
		register struct file *fp;

		for (fp = file; fp < fileNFILE; fp++) {
			if (fp->f_type == DTYPE_INODE &&
			    fp->f_count > 0 &&
			    (struct gnode *)fp->f_data == gp &&
			    (fp->f_flag&FWRITE)) {
				u.u_error = ETXTBSY;
				goto bad;
			}
		}
	}

	/*
	 * Compute text and data sizes and make sure not too large.
	 * NB - Check data and bss separately as they may overflow 
	 * when summed together.
	 */
	ts = clrnd(btoc(ep->a_text));
	ds = clrnd(btoc(ep->a_data + ep->a_bss));
	ss = clrnd(SSIZE + btoc(nargc));
	if (chksize((unsigned)ts, (unsigned)ds, (unsigned)ss)) {
		goto bad;
	}
	
	/*
	 * Make sure enough space to start process.
	 */
	u.u_cdmap = zdmap;
	u.u_csmap = zdmap;
	if (swpexpand(ds, ss, &u.u_cdmap, &u.u_csmap) == NULL) {
		goto bad;
	}
	
	/*
	 * At this point, committed to the new image!
	 * Release virtual memory resources of old process, and
	 * initialize the virtual memory of the new process.
	 * If we resulted from vfork(), instead wakeup our
	 * parent who will set SVFDONE when he has taken back
	 * our resources.
	 */
	/* BUT FIRST, clear any memory locks */
	(void) punlock();

	if ((p->p_flag & SVFORK) == 0)
		vrelvm();
	else {
		p->p_flag &= ~SVFORK;
		p->p_flag |= SKEEP;
		wakeup((caddr_t)p);
		while ((p->p_flag & SVFDONE) == 0)
			sleep((caddr_t)p, PZERO - 1);
		p->p_flag &= ~(SVFDONE|SKEEP);
	}

	/*
	 * If page currently in use or reclaimable, 
	 * Then set pagi according to it's current use,
	 * Else determine whether to demand page this process
	 */
	if ((gp->g_flag & GTEXT) && (xp = gp->g_textp)) {
		if (xp->x_flag & XPAGI)
			pagi = SPAGI;
	} else if (IS_SPAGI(ep,gp,ts))
		pagi = SPAGI;

	p->p_flag &= ~(SPAGI|SSEQL|SUANOM|SOUSIG);
	p->p_flag |= pagi;
	u.u_dmap = u.u_cdmap;
	u.u_smap = u.u_csmap;
	vgetvm(ts, ds, ss);

	if (pagi == 0)
		u.u_error =
		    rdwri(UIO_READ, gp, (char *)ctob(dptov(p, 0)),
			(int)ep->a_data,
			(int)((ep->a_magic == 0413 ?
			    CLBYTES : sizeof(struct exec)) + ep->a_text),
			0, (int *)0);
	if(u.u_error) {
		swkill(p, "exec: error reading data area");
		goto bad;
	}

	if(xalloc(gp, ep, pagi) == NULL) {
		goto bad;
	}
	
	if (pagi && p->p_textp)
		vinifod((struct fpte *)dptopte(p, 0), PG_FTEXT, gp,
		    (long)(1 + ts/CLSIZE), (int)btoc(ep->a_data));

#ifdef vax
	/* THIS SHOULD BE DONE AT A LOWER LEVEL, IF AT ALL */
	mtpr(TBIA, 0);
#endif
	tbsync();

	if (u.u_error) {
		swkill(p, "exec: I/O error mapping pages");
		goto bad;
	}
	
	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((p->p_flag&STRC)==0) {
		if(uid != u.u_uid || gid != u.u_gid)
			u.u_cred = crcopy(u.u_cred);
		u.u_uid = uid;
		p->p_uid = uid;
		u.u_gid = gid;
	} else
		psignal(p, SIGTRAP);
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_prof.pr_scale = 0;
bad:
	return;
}
