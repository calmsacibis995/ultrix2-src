#ifndef lint
static	char	*sccsid = "@(#)kern_sig.c	1.10	(ULTRIX)	3/31/87";
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

/* ------------------------------------------------------------------------
 * Modification History: /sys/sys/kern_sig.c
 *
 * 27 Mar-86 -- jaw 
 *	bug in ASMP code.  Jeff allowed the slave to execute "issig" which
 *	causes signals to be dropped.  This shows up in run adb on a
 *	program.  The fix is to switch to the master on entry into issig.
 *
 * 11 Sep 86 -- koehler 
 *	changed the namei interface
 *
 * 18 Mar 86 -- jrs
 *	Clean up cpu determination and preemption
 *
 * 16 Jul 85 -- jrs
 *	Add run queue locking and multicpu sched code
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code.
 *
 * 25 Oct 84 -- jrs
 *	Add changes for proc queues and
 *	security fix for non-read text segments
 *	Derived from 4.2BSD, labeled:
 *		kern_sig.c 6.3	84/05/22
 *
 * 10/1/85 -- Larry Cohen
 *	Add SIGWINCH signal to list of signals that dont go propogated.
 * -----------------------------------------------------------------------
 */

#include "../machine/reg.h"
#include "../machine/pte.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../h/timeb.h"
#include "../h/times.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/text.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/acct.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/interlock.h"
#include "../h/cpudata.h"
#include "../vax/mtpr.h"

#define	mask(s)	(1 << ((s)-1))
#define	cantmask	(mask(SIGKILL)|mask(SIGCONT)|mask(SIGSTOP))

sigvec()
{
	register struct a {
		int	signo;
		struct	sigvec *nsv;
		struct	sigvec *osv;
	} *uap = (struct a  *)u.u_ap;
	struct sigvec vec;
	register struct sigvec *sv;
	register int sig;

	sig = uap->signo;
	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP) {
		u.u_error = EINVAL;
		return;
	}
	sv = &vec;
	if (uap->osv) {
		sv->sv_handler = u.u_signal[sig];
		sv->sv_mask = u.u_sigmask[sig];
		sv->sv_onstack = (u.u_sigonstack & mask(sig)) != 0;
		u.u_error =
		    copyout((caddr_t)sv, (caddr_t)uap->osv, sizeof (vec));
		if (u.u_error)
			return;
	}
	if (uap->nsv) {
		u.u_error =
		    copyin((caddr_t)uap->nsv, (caddr_t)sv, sizeof (vec));
		if (u.u_error)
			return;
		if (sig == SIGCONT && sv->sv_handler == SIG_IGN) {
			u.u_error = EINVAL;
			return;
		}
		setsigvec(sig, sv);
	}
}

setsigvec(sig, sv)
	int sig;
	register struct sigvec *sv;
{
	register struct proc *p;
	register int bit;

	bit = mask(sig);
	p = u.u_procp;
	/*
	 * Change setting atomically.
	 */
	(void) spl6();
	u.u_signal[sig] = sv->sv_handler;
	u.u_sigmask[sig] = sv->sv_mask &~ cantmask;
	if (sv->sv_onstack)
		u.u_sigonstack |= bit;
	else
		u.u_sigonstack &= ~bit;
	if (sv->sv_handler == SIG_IGN) {
		p->p_sig &= ~bit;		/* never to be seen again */
		p->p_sigignore |= bit;
		p->p_sigcatch &= ~bit;
	} else {
		p->p_sigignore &= ~bit;
		if (sv->sv_handler == SIG_DFL)
			p->p_sigcatch &= ~bit;
		else
			p->p_sigcatch |= bit;
	}
	(void) spl0();
}

sigblock()
{
	struct a {
		int	sigmask;
	} *uap = (struct a *)u.u_ap;
	register struct proc *p = u.u_procp;

	(void) spl6();
	u.u_r.r_val1 = p->p_sigmask;
	p->p_sigmask |= uap->sigmask &~ cantmask;
	(void) spl0();
}

sigsetmask()
{
	struct a {
		int	sigmask;
	} *uap = (struct a *)u.u_ap;
	register struct proc *p = u.u_procp;

	(void) spl6();
	u.u_r.r_val1 = p->p_sigmask;
	p->p_sigmask = uap->sigmask &~ cantmask;
	(void) spl0();
}

sigpause()
{
	struct a {
		int	sigmask;
	} *uap = (struct a *)u.u_ap;
	register struct proc *p = u.u_procp;

	/*
	 * When returning from sigpause, we want
	 * the old mask to be restored after the
	 * signal handler has finished.  Thus, we
	 * save it here and mark the proc structure
	 * to indicate this (should be in u.).
	 */
	u.u_oldmask = p->p_sigmask;
	p->p_flag |= SOMASK;
	p->p_sigmask = uap->sigmask &~ cantmask;
	for (;;)
		sleep((caddr_t)&u, PSLEP);
	/*NOTREACHED*/
}
#undef cantmask
#undef mask

sigstack()
{
	register struct a {
		struct	sigstack *nss;
		struct	sigstack *oss;
	} *uap = (struct a *)u.u_ap;
	struct sigstack ss;

	if (uap->oss) {
		u.u_error = copyout((caddr_t)&u.u_sigstack, (caddr_t)uap->oss, 
		    sizeof (struct sigstack));
		if (u.u_error)
			return;
	}
	if (uap->nss) {
		u.u_error =
		    copyin((caddr_t)uap->nss, (caddr_t)&ss, sizeof (ss));
		if (u.u_error == 0)
			u.u_sigstack = ss;
	}
}

/* KILL SHOULD BE UPDATED */

kill()
{
	register struct a {
		int	pid;
		int	signo;
	} *uap = (struct a *)u.u_ap;

	u.u_error = kill1(uap->signo < 0,
		uap->signo < 0 ? -uap->signo : uap->signo, uap->pid);
}

killpg()
{
	register struct a {
		int	pgrp;
		int	signo;
	} *uap = (struct a *)u.u_ap;

	u.u_error = kill1(1, uap->signo, uap->pgrp);
}

/* KILL CODE SHOULDNT KNOW ABOUT PROCESS INTERNALS !?! */

kill1(ispgrp, signo, who)
	int ispgrp, signo, who;
{
	register struct proc *p;
	int f, priv = 0;

	if (signo < 0 || signo > NSIG)
		return (EINVAL);
	if (who > 0 && !ispgrp) {
		p = pfind(who);
		if (p == 0)
			return (ESRCH);
		if (u.u_uid && u.u_uid != p->p_uid)
			return (EPERM);
		if (signo)
			psignal(p, signo);
		return (0);
	}
	if (who == -1 && u.u_uid == 0)
		priv++, who = 0, ispgrp = 1;	/* like sending to pgrp */
	else if (who == 0) {
		/*
		 * Zero process id means send to my process group.
		 */
		ispgrp = 1;
		who = u.u_procp->p_pgrp;
		if (who == 0)
			return (EINVAL);
	}
	for (f = 0, p = allproc; p != NULL; p = p->p_nxt) {
		if (!ispgrp) {
			if (p->p_pid != who)
				continue;
		} else if (p->p_pgrp != who && priv == 0 || p->p_ppid == 0 ||
		    (p->p_flag&SSYS) || (priv && p == u.u_procp))
			continue;
		if (u.u_uid != 0 && u.u_uid != p->p_uid &&
		    (signo != SIGCONT || !inferior(p)))
			continue;
		f++;
		if (signo)
			psignal(p, signo);
	}
	return (f == 0 ? ESRCH : 0);
}

/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 */
gsignal(pgrp, sig)
	register int pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for (p = allproc; p != NULL; p = p->p_nxt)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
	register struct proc *p;
	register int sig;
{
	register int s;
	register int (*action)();
	int sigmask;
	int cpindex, cpident;

	if ((unsigned)sig >= NSIG)
		return;
	sigmask = 1 << (sig-1);

	/*
	 * If proc is traced, always give parent a chance.
	 */
	if (p->p_flag & STRC)
		action = SIG_DFL;
	else {
		/*
		 * If the signal is being ignored,
		 * then we forget about it immediately.
		 */
		if (p->p_sigignore & sigmask)
			return;
		if (p->p_sigmask & sigmask)
			action = SIG_HOLD;
		else if (p->p_sigcatch & sigmask)
			action = SIG_CATCH;
		else
			action = SIG_DFL;
	}
#define mask(sig)	(1<<(sig-1))
#define	stops	(mask(SIGSTOP)|mask(SIGTSTP)|mask(SIGTTIN)|mask(SIGTTOU))
	if (sig) {
		p->p_sig |= sigmask;
		switch (sig) {

		case SIGTERM:
			if ((p->p_flag&STRC) || action != SIG_DFL)
				break;
			/* fall into ... */

		case SIGKILL:
			if (p->p_nice > NZERO)
				p->p_nice = NZERO;
			break;

		case SIGCONT:
			p->p_sig &= ~stops;
			break;

		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			p->p_sig &= ~mask(SIGCONT);
			break;
		}
	}
#undef mask
#undef stops
	/*
	 * Defer further processing for signals which are held.
	 */
	if (action == SIG_HOLD)
		return;
	s = spl6();
	switch (p->p_stat) {

	case SSLEEP:
		/*
		 * If process is sleeping at negative priority
		 * we can't interrupt the sleep... the signal will
		 * be noticed when the process returns through
		 * trap() or syscall().
		 */
		if (p->p_pri <= PZERO)
			goto out;
		/*
		 * Process is sleeping and traced... make it runnable
		 * so it can discover the signal in issig() and stop
		 * for the parent.
		 */
		if (p->p_flag&STRC)
			goto run;
		switch (sig) {

		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			/*
			 * These are the signals which by default
			 * stop a process.
			 */
			if (action != SIG_DFL)
				goto run;
			/*
			 * Don't clog system with children of init
			 * stopped from the keyboard.
			 */
			if (sig != SIGSTOP && p->p_pptr == &proc[1]) {
				psignal(p, SIGKILL);
				p->p_sig &= ~sigmask;
				splx(s);
				return;
			}
			/*
			 * If a child in vfork(), stopping could
			 * cause deadlock.
			 */
			if (p->p_flag&SVFORK)
				goto out;
			p->p_sig &= ~sigmask;
			p->p_cursig = sig;
			stop(p);
			goto out;

		case SIGIO:
		case SIGURG:
		case SIGCHLD:
		case SIGWINCH:
			/*
			 * These signals are special in that they
			 * don't get propogated... if the process
			 * isn't interested, forget it.
			 */
			if (action != SIG_DFL)
				goto run;
			p->p_sig &= ~sigmask;		/* take it away */
			goto out;

		default:
			/*
			 * All other signals cause the process to run
			 */
			goto run;
		}
		/*NOTREACHED*/

	case SSTOP:
		/*
		 * If traced process is already stopped,
		 * then no further action is necessary.
		 */
		if (p->p_flag&STRC)
			goto out;
		switch (sig) {

		case SIGKILL:
			/*
			 * Kill signal always sets processes running.
			 */
			goto run;

		case SIGCONT:
			/*
			 * If the process catches SIGCONT, let it handle
			 * the signal itself.  If it isn't waiting on
			 * an event, then it goes back to run state.
			 * Otherwise, process goes back to sleep state.
			 */
			if (action != SIG_DFL || p->p_wchan == 0)
				goto run;
			p->p_stat = SSLEEP;
			goto out;

		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			/*
			 * Already stopped, don't need to stop again.
			 * (If we did the shell could get confused.)
			 */
			p->p_sig &= ~sigmask;		/* take it away */
			goto out;

		default:
			/*
			 * If process is sleeping interruptibly, then
			 * unstick it so that when it is continued
			 * it can look at the signal.
			 * But don't setrun the process as its not to
			 * be unstopped by the signal alone.
			 */
			if (p->p_wchan && p->p_pri > PZERO)
				unsleep(p);
			goto out;
		}
		/*NOTREACHED*/

	default:
		/*
		 * SRUN, SIDL, SZOMB do nothing with the signal,
		 * other than kicking ourselves if we are running.
		 * It will either never be noticed, or noticed very soon.
		 */
		cpident = cpuindex();
		lock(LOCK_RQ);
		for (cpindex = 0; cpindex < activecpu; cpindex++) {
			if (p == cpudata[cpindex].c_proc &&
					!cpudata[cpindex].c_noproc) {
				if (cpident == cpindex) {
					aston();
				} else {
					intrcpu(cpindex);
				}
				break;
			}
		}
		unlock(LOCK_RQ);
		goto out;
	}
	/*NOTREACHED*/
run:
	/*
	 * Raise priority to at least PUSER.
	 */
	if (p->p_pri > PUSER)
		if (p->p_stat == SRUN && (p->p_flag & SLOAD)) {
			lock(LOCK_RQ);
			for (cpindex = 0; cpindex < activecpu; cpindex++) {
				if (p == cpudata[cpindex].c_proc &&
					cpudata[cpindex].c_noproc == 0) {
					p->p_pri = PUSER;
					break;
				}
			}
			if (cpindex >= activecpu) {
				remrq(p);
				p->p_pri = PUSER;
				setrq(p);
			}
			unlock(LOCK_RQ);
		} else
			p->p_pri = PUSER;
	setrun(p);
out:
	splx(s);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * The signal to process is put in p_cursig.
 * This is asked at least once each time a process enters the
 * system (though this can usually be done without actually
 * calling issig by checking the pending signal masks.)
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
extern int extracpu;
issig()
{
	register struct proc *p;
	register int sig;
	int sigbits, sigmask;
	int s;

	p = u.u_procp;
	if (extracpu) {
		p->p_flag |= SMASTER;
		if (cpuindex() != 0) {
			s = spl6();
			lock(LOCK_RQ);
			setrq(p);
			u.u_ru.ru_nivcsw++;
			swtch();
			(void) splx(s);
		}
	}
	for (;;) {
		sigbits = p->p_sig &~ p->p_sigmask;
		if ((p->p_flag&STRC) == 0)
			sigbits &= ~p->p_sigignore;
		if (p->p_flag&SVFORK)
#define bit(a) (1<<(a-1))
			sigbits &= ~(bit(SIGSTOP)|bit(SIGTSTP)|bit(SIGTTIN)|bit(SIGTTOU));
		if (sigbits == 0)
			break;
		sig = ffs(sigbits);
		sigmask = 1 << (sig-1);
		p->p_sig &= ~sigmask;		/* take the signal! */
		p->p_cursig = sig;
		if (p->p_flag&STRC && (p->p_flag&SVFORK) == 0) {
			/*
			 * If traced, always stop, and stay
			 * stopped until released by the parent.
			 */
			do {
				stop(p);
				s = spl6();
				lock(LOCK_RQ);
				swtch();
				splx(s);
			} while (!procxmt() && p->p_flag&STRC);

			/*
			 * If the traced bit got turned off,
			 * then put the signal taken above back into p_sig
			 * and go back up to the top to rescan signals.
			 * This ensures that p_sig* and u_signal are consistent.
			 */
			if ((p->p_flag&STRC) == 0) {
				p->p_sig |= sigmask;
				continue;
			}

			/*
			 * If parent wants us to take the signal,
			 * then it will leave it in p->p_cursig;
			 * otherwise we just look for signals again.
			 */
			sig = p->p_cursig;
			if (sig == 0)
				continue;

			/*
			 * If signal is being masked put it back
			 * into p_sig and look for other signals.
			 */
			sigmask = 1 << (sig-1);
			if (p->p_sigmask & sigmask) {
				p->p_sig |= sigmask;
				continue;
			}
		}
		switch (u.u_signal[sig]) {

		case SIG_DFL:
			/*
			 * Don't take default actions on system processes.
			 */
			if (p->p_ppid == 0)
				break;
			switch (sig) {

			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU:
				/*
				 * Children of init aren't allowed to stop
				 * on signals from the keyboard.
				 */
				if (p->p_pptr == &proc[1]) {
					psignal(p, SIGKILL);
					continue;
				}
				/* fall into ... */

			case SIGSTOP:
				if (p->p_flag&STRC)
					continue;
				stop(p);
				s = spl6();
				lock(LOCK_RQ);
				swtch();
				splx(s);
				continue;

			case SIGCONT:
			case SIGCHLD:
			case SIGURG:
			case SIGIO:
			case SIGWINCH:
				/*
				 * These signals are normally not
				 * sent if the action is the default.
				 */
				continue;		/* == ignore */

			default:
				goto send;
			}
			/*NOTREACHED*/

		case SIG_HOLD:
		case SIG_IGN:
			/*
			 * Masking above should prevent us
			 * ever trying to take action on a held
			 * or ignored signal, unless process is traced.
			 */
			if ((p->p_flag&STRC) == 0)
				printf("issig\n");
			continue;

		default:
			/*
			 * This signal has an action, let
			 * psig process it.
			 */
			goto send;
		}
		/*NOTREACHED*/
	}
	/*
	 * Didn't find a signal to send.
	 */
	p->p_cursig = 0;
	return (0);

send:
	/*
	 * Let psig process the signal.
	 */
	return (sig);
}

/*
 * Put the argument process into the stopped
 * state and notify the parent via wakeup and/or signal.
 */
stop(p)
	register struct proc *p;
{

	p->p_stat = SSTOP;
	p->p_flag &= ~SWTED;
	wakeup((caddr_t)p->p_pptr);
	/*
	 * Avoid sending signal to parent if process is traced
	 */
	if (p->p_flag&STRC)
		return;
	psignal(p->p_pptr, SIGCHLD);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 * The signal bit has already been cleared by issig,
 * and the current signal number stored in p->p_cursig.
 */
psig()
{
	register struct proc *p = u.u_procp;
	register int sig = p->p_cursig;
	int sigmask = 1 << (sig - 1), returnmask;
	register int (*action)();

	if (sig == 0)
		panic("psig");
	action = u.u_signal[sig];
	if (action != SIG_DFL) {
		if (action == SIG_IGN || (p->p_sigmask & sigmask))
			panic("psig action");
		u.u_error = 0;
		/*
		 * Set the new mask value and also defer further
		 * occurences of this signal (unless we're simulating
		 * the old signal facilities). 
		 *
		 * Special case: user has done a sigpause.  Here the
		 * current mask is not of interest, but rather the
		 * mask from before the sigpause is what we want restored
		 * after the signal processing is completed.
		 */
		(void) spl6();
		if (p->p_flag & SOUSIG) {
			if (sig != SIGILL && sig != SIGTRAP) {
				u.u_signal[sig] = SIG_DFL;
				p->p_sigcatch &= ~sigmask;
			}
			sigmask = 0;
		}
		if (p->p_flag & SOMASK) {
			returnmask = u.u_oldmask;
			p->p_flag &= ~SOMASK;
		} else
			returnmask = p->p_sigmask;
		p->p_sigmask |= u.u_sigmask[sig] | sigmask;
		(void) spl0();
		u.u_ru.ru_nsignals++;
		sendsig(action, sig, returnmask);
		p->p_cursig = 0;
		return;
	}
	u.u_acflag |= AXSIG;
	switch (sig) {

	case SIGILL:
	case SIGIOT:
	case SIGBUS:
	case SIGQUIT:
	case SIGTRAP:
	case SIGEMT:
	case SIGFPE:
	case SIGSEGV:
	case SIGSYS:
		u.u_arg[0] = sig;
		if (core())
			sig += 0200;
	}
	exit(sig);
}

/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 * It writes UPAGES block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
core()
{
	register struct gnode *gp;
	register struct nameidata *ndp = &u.u_nd;

	if (u.u_uid != u.u_ruid || u.u_gid != u.u_rgid)
		return (0);
	/* try not to dump core for files user does not have
	   read access to the executable */
	if (u.u_procp->p_textp) {
		if (gp = u.u_procp->p_textp->x_gptr) {
			if (access(gp, GREAD)) {
				return(0);
			}
		}
	}
	if (ctob(UPAGES+u.u_dsize+u.u_ssize) >=
	    u.u_rlimit[RLIMIT_CORE].rlim_cur)
		return (0);
	u.u_error = 0;
	ndp->ni_nameiop = CREATE | FOLLOW;
	ndp->ni_dirp = "core";
	if((gp = gfs_namei(ndp)) == NULL) {
		if (u.u_error)
			return (0);
		gp = GMAKNODE(0644 | GFREG, ndp);
		if (gp==NULL)
			return (0);
	}
	if (access(gp, GWRITE) ||
	   (gp->g_mode & GFMT) != GFREG ||
	   gp->g_nlink != 1) {
		u.u_error = EFAULT;
		goto out;
	}
	(void)GTRUNC(gp, (u_long)0, u.u_cred);
	u.u_acflag |= ACORE;
	u.u_error = rdwri(UIO_WRITE, gp, (caddr_t)&u, ctob(UPAGES), 0, 1,
	(int *)0);
	if (u.u_error == 0)
		u.u_error = rdwri(UIO_WRITE, gp,
		(caddr_t)ctob(dptov(u.u_procp, 0)), ctob(u.u_dsize),
		ctob(UPAGES), 0, (int *)0);
	if (u.u_error == 0)
		u.u_error = rdwri(UIO_WRITE, gp,
		(caddr_t)ctob(sptov(u.u_procp, u.u_ssize - 1)),
		    ctob(u.u_ssize),
		    ctob(UPAGES)+ctob(u.u_dsize), 0, (int *)0);
	gp->g_flag |= GMOD;
	(void) GUPDATE(gp, &time, &time, 0, u.u_cred);
out:
	gput(gp);
	return (u.u_error == 0);
}
