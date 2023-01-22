#ifndef lint
static char *sccsid = "@(#)kern_exit.c	1.21	ULTRIX	3/3/87";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ------------------------------------------------------------------------
 * Modification History: /sys/sys/kern_exit.c
 *
 * 11 Sep 86 -- koehler 
 *	exit no longer uses a mbuf
 *
 * 17 Nov 86 -- depp
 *	Fixed "stale u-area" problem in vm_pt.c and vm_mem.c.  So, the routine
 *	"exit" no longer needs raised IPL when a process' "u" area is 
 *	deallocated.
 *
 * 11 Mar 86 -- robin
 *	Added code for user login limits that at&t require.
 *
 * 11 Mar 86 -- lp
 *	Changed order in which things get done on exit. Close down
 *	files before releasing vm (used by n-buffering).
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 24 Oct 84 -- jrs
 *	Update for linked proc lists
 *	Derived from 4.2BSD, labeled:
 *		kern_exit.c 6.3 84/06/10
 *
 * 4 April 85 -- Larry Cohen
 *	Changes to support open block in use capability  - 001
 *
 * 11 Mar 85 -- depp
 *	Added System V semaphore and shared memory support
 *
 *  4/13/85 - Larry Cohen
 *	call ioctl FIOCINUSE when closing inuse desriptor
 *
 *  05-May-85 - Larry Cohen
 *	loop up to u_omax instead of NOFILE
 *
 *  19 Jul 85 -- depp
 *	Removed calls to smexit, as this call is handled by vrelvm.
 *
 * 18 Sep 85 -- depp
 *	Added punlock call to unlock memory segments
 *
 * 23-Jul-85 -- jrs
 *	Add multicpu sched code
 *
 * -----------------------------------------------------------------------
 */

#include "../machine/reg.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/wait.h"
#include "../h/vm.h"
#include "../h/text.h"
#include "../h/file.h"
#ifdef notdef
#include "../h/mbuf.h"
#endif notdef
#include "../h/gnode.h"
#include "../h/ioctl.h"
#include "../h/cpudata.h"
#include "../h/interlock.h"
#include "../h/kmalloc.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

/*
 * Exit system call: pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
	int rv;
{
	register int i;
	register struct proc *p, *q, *nq;
	register int x;
	register struct gnode *gp;
#ifdef notdef
	struct mbuf *m = m_getclr(M_WAIT, MT_ZOMBIE);
#endif notdef
	caddr_t value;
	extern int mlil;

#ifdef PGINPROF
	vmsizmon();
#endif
	(void) punlock();	/* clean up any locked memory segments */
	p = u.u_procp;
/* login limit hack for at&t limits */
	if((p->p_flag & SLOGIN) != 0)
	{
		if (mlil > 0) mlil--;
		p->p_flag &= ~SLOGIN;
	}
/* end of login limit hack */
	p->p_flag &= ~SULOCK;
	p->p_flag |= SWEXIT;
	p->p_sigignore = ~0;
	p->p_cpticks = 0;
	p->p_pctcpu = 0;
	for (i = 0; i < NSIG; i++)
		u.u_signal[i] = SIG_IGN;
	untimeout(realitexpire, (caddr_t)p);

#ifdef GFSDEBUG
	if(GFS[7])
		cprintf("exit: pid %d gp 0x%x (%d) closef\n", p->p_pid,
		p->p_textp->x_gptr, p->p_textp->x_gptr->g_number);
#endif
	/* Close down any active files */
	
	for (i = 0; i <= u.u_omax; i++) {
		struct file *f;

		f = u.u_ofile[i];
		if (u.u_pofile[i] & UF_INUSE) {
			gp = (struct gnode *)f->f_data;
			if (gp && (gp->g_flag & GINUSE)) {
				gp->g_flag &= ~(GINUSE);
				(*f->f_ops->fo_ioctl)(f, FIOCINUSE, value);
				wakeup((caddr_t)&gp->g_flag);
			}
		}
		u.u_ofile[i] = NULL;
		u.u_pofile[i] = 0;
		closef(f);
	}
#ifdef GFSDEBUG
	if(GFS[7])
		cprintf("exit: get rid of rdir and cdir\n");
#endif
	gfs_lock(u.u_cdir);
	gput(u.u_cdir);
	if (u.u_rdir) {
		gfs_lock(u.u_rdir);
		gput(u.u_rdir);
	}
	/*
	 * Release virtual memory.  If we resulted from
	 * a vfork(), instead give the resources back to
	 * the parent.
	 */

#ifdef GFSDEBUG
	if(GFS[7])
		cprintf("exit: release vm\n");
#endif
	if ((p->p_flag & SVFORK) == 0)
		vrelvm();
	else {
		p->p_flag &= ~SVFORK;
		wakeup((caddr_t)p);
		while ((p->p_flag & SVFDONE) == 0)
			sleep((caddr_t)p, PZERO - 1);
		p->p_flag &= ~SVFDONE;
	}
	p->p_flag &= ~STRC;
	u.u_rlimit[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;

	semexit();	/* clean up any outstanding semaphores */

	acct();
#ifdef QUOTA
	qclean();
#endif
	crfree(u.u_cred);
	vrelpt(u.u_procp);
	vrelu(u.u_procp, 0);
	if (*p->p_prev = p->p_nxt)		/* off allproc queue */
		p->p_nxt->p_prev = p->p_prev;
	if (p->p_nxt = zombproc)		/* onto zombproc */
		p->p_nxt->p_prev = &p->p_nxt;
	p->p_prev = &zombproc;
	zombproc = p;
	multprog--;
	p->p_stat = SZOMB;
	cpudata[u.u_pcb.pcb_cpundx].c_noproc = 1;
	i = PIDHASH(p->p_pid);
	x = p - proc;
	if (pidhash[i] == x)
		pidhash[i] = p->p_idhash;
	else {
		for (i = pidhash[i]; i != 0; i = proc[i].p_idhash)
			if (proc[i].p_idhash == x) {
				proc[i].p_idhash = p->p_idhash;
				goto done;
			}
		panic("exit");
	}
	if (p->p_pid == 1)
		panic("init died");
done:
	p->p_xstat = rv;
#ifdef notdef
	if (m == 0)
		mprintf("exit: m_getclr, no mbufs available");
	else {
		p->p_ru = mtod(m, struct rusage *);
#else NEW
		p->p_ru = (struct rusage *)
			km_alloc(sizeof(struct rusage),KM_CLRSG|KM_SLEEP);
#endif notdef
		*p->p_ru = u.u_ru;
		ruadd(p->p_ru, &u.u_cru);
#ifdef notdef
	}
#endif notdef
	if (p->p_cptr)		/* only need this if any child is S_ZOMB */
		wakeup((caddr_t)&proc[1]);
	for (q = p->p_cptr; q != NULL; q = nq) {
		nq = q->p_osptr;
		if (nq != NULL)
			nq->p_ysptr = NULL;
		if (proc[1].p_cptr)
			proc[1].p_cptr->p_ysptr = q;
		q->p_osptr = proc[1].p_cptr;
		q->p_ysptr = NULL;
		proc[1].p_cptr = q;

		q->p_pptr = &proc[1];
		q->p_ppid = 1;
		/*
		 * Traced processes are killed
		 * since their existence means someone is screwing up.
		 * Stopped processes are sent a hangup and a continue.
		 * This is designed to be ``safe'' for setuid
		 * processes since they must be willing to tolerate
		 * hangups anyways.
		 */
		if (q->p_flag&STRC) {
			q->p_flag &= ~STRC;
			psignal(q, SIGKILL);
		} else if (q->p_stat == SSTOP) {
			psignal(q, SIGHUP);
			psignal(q, SIGCONT);
		}
		/*
		 * Protect this process from future
		 * tty signals, clear TSTP/TTIN/TTOU if pending.
		 */
		(void) spgrp(q, -1);
	}
	psignal(p->p_pptr, SIGCHLD);
	wakeup((caddr_t)p->p_pptr);
	(void) spl6();
	lock(LOCK_RQ);
	swtch();
}

wait()
{
	struct rusage ru, *rup;

	if ((u.u_ar0[PS] & PSL_ALLCC) != PSL_ALLCC) {
		u.u_error = wait1(0, (struct rusage *)0);
		return;
	}
	rup = (struct rusage *)u.u_ar0[R1];
	u.u_error = wait1(u.u_ar0[R0], &ru);
	if (u.u_error)
		return;
	if (rup != (struct rusage *)0)
		u.u_error = copyout((caddr_t)&ru, (caddr_t)rup,
		    sizeof (struct rusage));
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait1(options, ru)
	register int options;
	struct rusage *ru;
{
	register f;
	register struct proc *p, *q;

	f = 0;
loop:
	q = u.u_procp;
	for (p = q->p_cptr; p; p = p->p_osptr) {
		f++;
		if (p->p_stat == SZOMB) {
			u.u_r.r_val1 = p->p_pid;
			u.u_r.r_val2 = p->p_xstat;
			p->p_xstat = 0;
			if (p->p_ru) { /* add check for valid mbuf */
				if (ru)
					*ru = *p->p_ru;
				ruadd(&u.u_cru, p->p_ru);
#ifdef notdef
				(void) m_free(dtom(p->p_ru));
#else NEW
				(void) km_free(p->p_ru,sizeof(struct rusage));
#endif notdef
			}
			p->p_ru = 0;
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			if (*p->p_prev = p->p_nxt)	/* off zombproc */
				p->p_nxt->p_prev = p->p_prev;
			p->p_nxt = freeproc;		/* onto freeproc */
			freeproc = p;
			if (q = p->p_ysptr)
				q->p_osptr = p->p_osptr;
			if (q = p->p_osptr)
				q->p_ysptr = p->p_ysptr;
			if ((q = p->p_pptr)->p_cptr == p)
				q->p_cptr = p->p_osptr;
			p->p_pptr = 0;
			p->p_ysptr = 0;
			p->p_osptr = 0;
			p->p_cptr = 0;
			p->p_sig = 0;
			p->p_sigcatch = 0;
			p->p_sigignore = 0;
			p->p_sigmask = 0;
			p->p_pgrp = 0;
			p->p_flag = 0;
			p->p_wchan = 0;
			p->p_cursig = 0;
			return (0);
		}
		if (p->p_stat == SSTOP && (p->p_flag&SWTED)==0 &&
		    (p->p_flag&STRC || options&WUNTRACED)) {
			p->p_flag |= SWTED;
			u.u_r.r_val1 = p->p_pid;
			u.u_r.r_val2 = (p->p_cursig<<8) | WSTOPPED;
			return (0);
		}
	}
	if (f == 0)
		return (ECHILD);
	if (options&WNOHANG) {
		u.u_r.r_val1 = 0;
		return (0);
	}
	if ((u.u_procp->p_flag&SOUSIG) == 0 && setjmp(&u.u_qsave)) {
		u.u_eosys = RESTARTSYS;
		return (0);
	}
	sleep((caddr_t)u.u_procp, PWAIT);
	goto loop;
}
