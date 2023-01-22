#ifndef lint
static char *sccsid = "@(#)kern_synch.c	1.13	ULTRIX	1/29/87";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
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
 * Modification History: /sys/sys/kern_synch.c
 *
 * 21 Jan 87 -- jaw
 *	performance fixes to syscall.
 *
 * 11-Sep-86 -- koehler
 *	added debug code to check for missed wakeups
 *
 * 15-Apr-86 -- jf
 *	Add support for system process sleep and wakeup
 *
 * 02-Apr-86 -- jrs
 *	Clean up low pri cpu determination and improve single cpu case
 *
 * 18 Mar 86 -- jrs
 *	Clean up cpu determination and preemption
 *
 * 12-Feb-86 -- pmk 
 *	Change spl4 to spl1 in sleep rundown check.
 *	Move rundown check in sleep to before panic check.
 *
 * 16 Jul 85 -- jrs
 *	Add run queue locking and multicpu sched code
 *
 * 20-Jan-86 -- pmk
 *	Add check in sleep for rundown - if set don't swtch
 *	fix recursive panic
 *
 * 25 Oct 84 -- jrs
 *	Add changes for proc queue lists
 *	Derived from 4.2BSD, labeled:
 *		kern_synch.c 6.2	84/05/22
 *
 * -----------------------------------------------------------------------
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/sysproc.h"
#include "../h/file.h"
#include "../h/gnode.h"
#include "../h/vm.h"
#include "../h/kernel.h"
#include "../h/buf.h"
#include "../h/interlock.h"
#include "../h/cpudata.h"

#ifdef vax
#include "../vax/mtpr.h"	/* XXX */
#endif
extern int runrun;
extern int extracpu;

/*
 * Force switch among equal priority processes every 100ms.
 */
roundrobin()
{
	int cpindex, cpident;
	if (extracpu) {
		cpident = cpuindex();
		for (cpindex = 0; cpindex < activecpu; cpindex++) {
			cpudata[cpindex].c_runrun++;
			if (cpindex != cpident) 
				intrcpu(cpindex);
			else aston();
		}
	} else {
		runrun++;
		aston();
	}
	timeout(roundrobin, (caddr_t)0, hz / 10);
}

/* constants to digital decay and forget 90% of usage in 5*loadav time */
#undef ave
#define	ave(a,b) ((int)(((int)(a*b))/(b+1)))
int	nrscale = 2;
double	ccpu = 0.95122942450071400909;		/* exp(-1/20) */

/*
 * Recompute process priorities, once a second
 */
schedcpu()
{
	register double ccpu1 = (1.0 - ccpu) / (double)hz;
	register struct proc *p;
	register int s, a;

	wakeup((caddr_t)&lbolt);
	for (p = allproc; p != NULL; p = p->p_nxt) {
		if (p->p_time != 127)
			p->p_time++;
		if (p->p_stat==SSLEEP || p->p_stat==SSTOP)
			if (p->p_slptime != 127)
				p->p_slptime++;
		if (p->p_flag&SLOAD)
			p->p_pctcpu = ccpu * p->p_pctcpu + ccpu1 * p->p_cpticks;
		p->p_cpticks = 0;
		a = ave((p->p_cpu & 0377), avenrun[0]*nrscale) +
		     p->p_nice - NZERO;
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		p->p_cpu = a;
		(void) setpri(p);
		s = spl6();	/* prevent state changes */
		if (p->p_pri >= PUSER) {
#define	PPQ	(128 / NQS)

			lock(LOCK_RQ);
			if (p->p_stat == SRUN && (p->p_flag & SLOAD) &&
				(p->p_pri / PPQ) != (p->p_usrpri / PPQ)) {
				for (a = 0; a < activecpu; a++) {
					if (p == cpudata[a].c_proc &&
						cpudata[a].c_noproc == 0) {
						p->p_pri = p->p_usrpri;
						break;
					}
				}
				if (a >= activecpu) {
					remrq(p);
					p->p_pri = p->p_usrpri;
					setrq(p);
				}
			} else {
				p->p_pri = p->p_usrpri;
			}
			unlock(LOCK_RQ);
		}
		splx(s);
	}
	vmmeter();
	if (runin!=0) {
		runin = 0;
		wakeup((caddr_t)&runin);
	}
	if (bclnlist != NULL)
		wakeup((caddr_t)&proc[2]);
	timeout(schedcpu, (caddr_t)0, hz);
}

#define SQSIZE 0100	/* Must be power of 2 */
#define HASH(x)	(( (int) x >> 5) & (SQSIZE-1))
struct proc *slpque[SQSIZE];

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
	caddr_t chan;
	int pri;
{
	register struct proc *rp, **hp;
	register s,k;

	if (cusysproc) {
	    sleepsysproc(chan);
	    return;
	}
	rp = u.u_procp;
	s = spl6();
	if (rundown != 0) {		/* set in boot - don't swtch */
	    k = spl1();
	    splx(k);
	    goto out;
	}
	if (chan==0 || rp->p_stat != SRUN || rp->p_rlink)
		panic("sleep");
	rp->p_wchan = chan;
	rp->p_slptime = 0;
	rp->p_pri = pri;
	hp = &slpque[HASH(chan)];
	rp->p_link = *hp;
	*hp = rp;
	if (pri > PZERO) {
		if (ISSIG(rp)) {
			if (rp->p_wchan)
				unsleep(rp);
			rp->p_stat = SRUN;
			(void) spl0();
			goto psig;
		}
		if (rp->p_wchan == 0)
			goto out;
		rp->p_stat = SSLEEP;
		u.u_ru.ru_nvcsw++;
		lock(LOCK_RQ);
		swtch();
		(void) spl0();
		if (ISSIG(rp))
			goto psig;
	} else {
		rp->p_stat = SSLEEP;
		u.u_ru.ru_nvcsw++;
		lock(LOCK_RQ);
		swtch();
		(void) spl0();
	}
	if (extracpu == 0) {
		mincurpri = rp->p_usrpri;
	} else {
		cpudata[u.u_pcb.pcb_cpundx].c_curpri = rp->p_usrpri;
		if ((u.u_pcb.pcb_cpundx == minpricpu
				&& rp->p_usrpri != mincurpri)
				|| rp->p_usrpri > mincurpri) {
			minpri();
		}
	}
out:
	splx(s);
	return;

	/*
	 * If priority was low (>PZERO) and
	 * there has been a signal, execute non-local goto through
	 * u.u_qsave, aborting the system call in progress (see trap.c)
	 * (or finishing a tsleep, see below)
	 */
psig:
	longjmp(&u.u_qsave);
	/*NOTREACHED*/
}

/*
 * Remove a process from its wait queue
 */
unsleep(p)
	register struct proc *p;
{
	register struct proc **hp;
	register s;

	s = spl6();
	if (p->p_wchan) {
		hp = &slpque[HASH(p->p_wchan)];
		while (*hp != p)
			hp = &(*hp)->p_link;
		*hp = p->p_link;
		p->p_wchan = 0;
	}
	splx(s);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	register struct proc *p, **q, **h;
	int s;

	s = spl6();
	h = &slpque[HASH(chan)];
restart:
	for (q = h; p = *q; ) {
		if (p->p_rlink || p->p_stat != SSLEEP && p->p_stat != SSTOP)
			panic("wakeup");
		if (p->p_wchan==chan) {
			p->p_wchan = 0;
			*q = p->p_link;
			p->p_slptime = 0;
			if (p->p_stat == SSLEEP) {
				/* OPTIMIZED INLINE EXPANSION OF setrun(p) */
				p->p_stat = SRUN;
				if (p->p_flag & SLOAD) {
					lock(LOCK_RQ);
					setrq(p);
					unlock(LOCK_RQ);
				}
				/*
				 * Since curpri is a usrpri,
				 * p->p_pri is always better than curpri.
				 */
				if (extracpu) {
					cpudata[cpuindex()].c_runrun++;
				} else {
					runrun ++;		
				}
				aston();
				if ((p->p_flag&SLOAD) == 0) {
					if (runout != 0) {
						runout = 0;
						wakeup((caddr_t)&runout);
					}
					wantin++;
				}
				/* END INLINE EXPANSION */
				goto restart;
			}
		} else
			q = &p->p_link;
	}
	wakeupsysproc(chan);
#ifdef GFSDEBUG
	for(p = proc; p < &proc[nproc]; p++)
		if((p->p_stat & SSLEEP) && (p->p_wchan == chan)) {
			printf("wakeup: missed wakeup chan 0x%x p 0x%x\n",
			chan, p);
			panic("wakeup");
		}
#endif
	splx(s);
}

/*
 * Initialize the (doubly-linked) run queues
 * to be empty.
 */
rqinit()
{
	register int i;

	for (i = 0; i < NQS; i++)
		qs[i].ph_link = qs[i].ph_rlink = (struct proc *)&qs[i];
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
	register struct proc *p;
{
	register int s;

	s = spl6();
	switch (p->p_stat) {

	case 0:
	case SWAIT:
	case SRUN:
	case SZOMB:
	default:
		panic("setrun");

	case SSTOP:
	case SSLEEP:
		unsleep(p);		/* e.g. when sending signals */
		break;

	case SIDL:
		break;
	}
	p->p_stat = SRUN;
	if (p->p_flag & SLOAD) {
		lock(LOCK_RQ);
		setrq(p);
		unlock(LOCK_RQ);
	}
	splx(s);
	if (p->p_pri < mincurpri) {
		if (extracpu== 0) {
			runrun++;
			aston();
		} else {
			cpudata[minpricpu].c_runrun++;
			if (minpricpu != cpuindex()) {
				intrcpu(minpricpu);
			} else {
				aston();
			}
		}
	}
	if ((p->p_flag&SLOAD) == 0) {
		if (runout != 0) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		wantin++;
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */
setpri(pp)
	register struct proc *pp;
{
	register int p;
	int cpident;

	p = (pp->p_cpu & 0377)/4;
	p += PUSER + 2*(pp->p_nice - NZERO);
	if (pp->p_rssize > pp->p_maxrss && freemem < desfree)
		p += 2*4;	/* effectively, nice(4) */
	if (p > 127)
		p = 127;
	cpident = cpuindex();
	if (p < mincurpri && pp != cpudata[cpident].c_proc) {
		if (extracpu==0 ) {
			runrun++;
			aston();
		} else {
			cpudata[minpricpu].c_runrun++;
			if (minpricpu != cpident) {
				intrcpu(minpricpu);
			} else {
				aston();
			}
		}
	}
	pp->p_usrpri = p;
	return (p);
}

/*
 * find and remember current lowest running process
 */

minpri()
{
	register int index;
	register int lowpri;
	register int lowcpu;
	
	if (extracpu ==0) return;
		
	lowpri = -1;
	for (index = 0; index < activecpu; index++) {
		if (cpudata[index].c_noproc != 0) {
			lowpri = 127;
			lowcpu = index;
			break;
		}
		if (cpudata[index].c_curpri > lowpri) {
			lowpri = cpudata[index].c_curpri;
			lowcpu = index;
		}
	}
	mincurpri = lowpri;
	minpricpu = lowcpu;
}










