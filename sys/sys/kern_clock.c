#ifndef lint
static char *sccsid = "@(#)kern_clock.c	1.14	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
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
 * Modification History: /sys/sys/kern_clock.c
 *
 * 16-Jul-86 -- Todd M. Katz
 *	Add a kernel subroutine scheduler.  This mechanism consists of:
 *		1. ksched - A kernel subroutine scheduling routine.
 *		2. unksched - A kernel subroutine unscheduling routine.
 *		3. Changes to softclock().  When softclock() is invoked
 *		   all currently scheduled kernel subroutines are
 *		   asynchronously invoked prior to processing of due
 *		   periodic events on the timeout queue.
 *
 * 15-Jul-86 -- rich
 *	Added hooks for the adjusttime system call to hard clock and a
 * 	minor change to bumptime.
 *
 * 02-Apr-86 -- jrs
 *	Clean up so that single cpu tick updates run cleaner
 *
 * 18-Mar-86 -- jrs
 *	Clean up cpu determination and preemption
 *
 * 03-Mar-86 -- jrs
 *	Change gatherstats to record disk usage only when master cycles.
 *	cpu utilization is gathered as aggregate average of all processors.
 *	This allows for possible count of "missed" cycles externally.
 *
 * 23 Jul 85 -- jrs
 *	Add multicpu scheduling
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 16-Apr-85 -- lp
 *	Changed BUMPTIME to a macro. Added ipl 7 interrupt queues
 *	for interrupt burdened devices.
 *
 * 11 Mar 85 -- depp
 *	Added System V shared memory support.
 *
 * 24 Oct 84 -- jrs
 *	Added changes to limit soft clock calling
 *	Derived from 4.2BSD, labeled:
 *		kern_clock.c 6.7	84/05/22
 *
 * -----------------------------------------------------------------------
 */

#include "../machine/reg.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dk.h"
#include "../h/callout.h"
#include "../h/dir.h"
#include "../h/ksched.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/proc.h"
#include "../h/vm.h"
#include "../h/text.h"
#include "../h/cpudata.h"
#include "../h/interlock.h"

#ifdef vax
#include "../vax/mtpr.h"
#endif

#ifdef GPROF
#include "../h/gprof.h"
#endif

#include "../h/ipc.h"
#include "../h/shm.h"
extern struct sminfo sminfo;

/*
 * Bump a timeval by a small number of usec's. (macro)
 */

#define BUMPTIME(t, usec) {			\
	register struct timeval *tp = (t); 	\
	tp->tv_usec += (usec); 			\
	if (tp->tv_usec >= 1000000) { 		\
		tp->tv_usec -= 1000000; 	\
		(tp)->tv_sec++; 		\
	} 					\
}
	

/*
 * Clock handling routines.
 *
 * This code is written to operate with two timers which run
 * independently of each other. The main clock, running at hz
 * times per second, is used to do scheduling and timeout calculations.
 * The second timer does resource utilization estimation statistically
 * based on the state of the machine phz times a second. Both functions
 * can be performed by a single clock (ie hz == phz), however the
 * statistics will be much more prone to errors. Ideally a machine
 * would have separate clocks measuring time spent in user state, system
 * state, interrupt state, and idle state. These clocks would allow a non-
 * approximate measure of resource utilization.
 */

/*
 * TODO:
 *	time of day, system/user timing, timeouts, profiling on separate timers
 *	allocate more timeout table slots when table overflows.
 */

/*
 * The hz hardware interval timer.
 * We update the events relating to real time.
 * If this timer is also being used to gather statistics,
 * we run through the statistics gathering routine as well.
 */
/*ARGSUSED*/
hardclock(pc, ps)
	caddr_t pc;
	int ps;
{
	register struct callout *p1;
	register struct proc *p;
	register int s, cpstate;
	int needsoft = 0;
	int cpndx;
	extern int tickdelta;
	extern long timedelta;

	/*
	 * Update real-time timeout queue.
	 * At front of queue are some number of events which are ``due''.
	 * The time to these is <= 0 and if negative represents the
	 * number of ticks which have passed since it was supposed to happen.
	 * The rest of the q elements (times > 0) are events yet to happen,
	 * where the time for each is given as a delta from the previous.
	 * Decrementing just the first of these serves to decrement the time
	 * to all events.
	 */

	cpndx = cpuindex();
	if (cpndx == 0) {
		p1 = calltodo.c_next;
		while (p1) {
			if (--p1->c_time > 0)
				break;
			needsoft++;
			if (p1->c_time == 0)
				break;
			p1 = p1->c_next;
		}
	}

	/*
	 * Charge the time out based on the mode the cpu is in.
	 * Here again we fudge for the lack of proper interval timers
	 * assuming that the current state has been around at least
	 * one tick.
	 */
	if (USERMODE(ps)) {
		if (u.u_prof.pr_scale)
			needsoft++;
		/*
		 * CPU was in user state.  Increment
		 * user time counter, and process process-virtual time
		 * interval timer.
		 */
		BUMPTIME(&u.u_ru.ru_utime, tick);
		if (timerisset(&u.u_timer[ITIMER_VIRTUAL].it_value) &&
		    itimerdecr(&u.u_timer[ITIMER_VIRTUAL], tick) == 0)
			psignal(u.u_procp, SIGVTALRM);
		if (u.u_procp->p_nice > NZERO)
			cpstate = CP_NICE;
		else
			cpstate = CP_USER;
	} else {
		/*
		 * CPU was in system state.  If profiling kernel
		 * increment a counter.  If no process is running
		 * then this is a system tick if we were running
		 * at a non-zero IPL (in a driver).  If a process is running,
		 * then we charge it with system time even if we were
		 * at a non-zero IPL, since the system often runs
		 * this way during processing of system calls.
		 * This is approximate, but the lack of true interval
		 * timers makes doing anything else difficult.
		 */
		cpstate = CP_SYS;
		if (cpudata[cpndx].c_noproc) {
			if (BASEPRI(ps))
				cpstate = CP_IDLE;
		} else {
			BUMPTIME(&u.u_ru.ru_stime, tick);
		}
	}

	/*
	 * If the cpu is currently scheduled to a process, then
	 * charge it with resource utilization for a tick, updating
	 * statistics which run in (user+system) virtual time,
	 * such as the cpu time limit and profiling timers.
	 * This assumes that the current process has been running
	 * the entire last tick.
	 */
	if (cpudata[cpndx].c_noproc == 0 && cpstate != CP_IDLE) {
		if ((u.u_ru.ru_utime.tv_sec+u.u_ru.ru_stime.tv_sec+1) >
		    u.u_rlimit[RLIMIT_CPU].rlim_cur) {
			psignal(u.u_procp, SIGXCPU);
			if (u.u_rlimit[RLIMIT_CPU].rlim_cur <
			    u.u_rlimit[RLIMIT_CPU].rlim_max)
				u.u_rlimit[RLIMIT_CPU].rlim_cur += 5;
		}
		if (timerisset(&u.u_timer[ITIMER_PROF].it_value) &&
		    itimerdecr(&u.u_timer[ITIMER_PROF], tick) == 0)
			psignal(u.u_procp, SIGPROF);
		s = u.u_procp->p_rssize;
		u.u_ru.ru_idrss += s; u.u_ru.ru_isrss += 0;	/* XXX */
		if (u.u_procp->p_textp) {
			register int xrss = u.u_procp->p_textp->x_rssize;

			s += xrss;
			u.u_ru.ru_ixrss += xrss;
		}

		/* begin SHMEM */
		if (u.u_procp->p_smbeg) {
			register struct smem *sp;
			register int i;

			for(i=0; i < sminfo.smseg; i++)
				if(sp = u.u_procp->p_sm[i].sm_p){
					s += sp->sm_rssize;
					u.u_ru.ru_ismrss +=
							sp->sm_rssize;
				}
		}
		/* end SHMEM */

		if (s > u.u_ru.ru_maxrss)
			u.u_ru.ru_maxrss = s;
	}

	/*
	 * We adjust the priority of the current process.
	 * The priority of a process gets worse as it accumulates
	 * CPU time.  The cpu usage estimator (p_cpu) is increased here
	 * and the formula for computing priorities (in kern_synch.c)
	 * will compute a different value each time the p_cpu increases
	 * by 4.  The cpu usage estimator ramps up quite quickly when
	 * the process is running (linearly), and decays away exponentially,
	 * at a rate which is proportionally slower when the system is
	 * busy.  The basic principal is that the system will 90% forget
	 * that a process used a lot of CPU time in 5*loadav seconds.
	 * This causes the system to favor processes which haven't run
	 * much recently, and to round-robin among other processes.
	 */
	if (!cpudata[cpndx].c_noproc) {
		p = u.u_procp;
		p->p_cpticks++;
		if (++p->p_cpu == 0)
			p->p_cpu--;
		if ((p->p_cpu&3) == 0) {
			(void) setpri(p);
			if (p->p_pri >= PUSER)
				p->p_pri = p->p_usrpri;
		}
	}

	/*
	 * If the alternate clock has not made itself known then
	 * we must gather the statistics.
	 */
	if (phz == 0)
		gatherstats(pc, ps);

	/*
	 * Increment the time-of-day, and schedule
	 * processing of the callouts at a very low cpu priority,
	 * so we don't keep the relatively high clock interrupt
	 * priority any longer than necessary.
	 */
	if (cpndx == 0) {
	    if(timedelta == 0) {
		BUMPTIME(&time, tick);
	    }
	    else {
		register delta;

		if (timedelta < 0) {
			delta = tick - tickdelta;
			timedelta += tickdelta;
		} else {
			delta = tick + tickdelta;
			timedelta -= tickdelta;
		}
		BUMPTIME(&time, delta);
	    }
	}
	
	if (needsoft) {
	      if (BASEPRI(ps)) {
			/*
			 * Save the overhead of a software interrupt;
			 * it will happen as soon as we return, so do it now.
			 */
			(void) splsoftclock();
			softclock(pc, ps);
		} else
			setsoftclock();
	}
}

int	dk_ndrive = DK_NDRIVE;
/*
 * Gather statistics on resource utilization.
 *
 * We make a gross assumption: that the system has been in the
 * state it is in (user state, kernel state, interrupt state,
 * or idle state) for the entire last time interval, and
 * update statistics accordingly.
 */
/*ARGSUSED*/
gatherstats(pc, ps)
	caddr_t pc;
	int ps;
{
	int cpstate, s, cpndx;

	/*
	 * Determine what state the cpu is in.
	 */
	cpndx = cpuindex();
	if (USERMODE(ps)) {
		/*
		 * CPU was in user state.
		 */
		if (u.u_procp->p_nice > NZERO)
			cpstate = CP_NICE;
		else
			cpstate = CP_USER;
	} else {
		/*
		 * CPU was in system state.  If profiling kernel
		 * increment a counter.
		 */
		cpstate = CP_SYS;
		if (cpudata[cpndx].c_noproc && BASEPRI(ps))
			cpstate = CP_IDLE;
#ifdef GPROF
		s = pc - s_lowpc;
		if (profiling < 2 && s < s_textsize)
			kcount[s / (HISTFRACTION * sizeof (*kcount))]++;
#endif
	}
	/*
	 * We maintain statistics shown by user-level statistics
	 * programs:  the amount of time in each cpu state, and
	 * the amount of time each of DK_NDRIVE ``drives'' is busy.
	 */
	cpudata[cpndx].c_cptime[cpstate]++;
	if (activecpu == 1) {
		cp_time[cpstate]++;
	} else {
		lock(LOCK_CPTIME);
		cp_remaind[cpstate]++;
		if (cpndx == 0) {
			cpstate = CP_IDLE;
			for (s = 0; s < CPUSTATES; s++) {
				if (cp_remaind[s] >= activecpu) {
					cpstate = s;
					break;
				}
			}
			cp_time[cpstate]++;
			cp_remaind[cpstate] -= activecpu;
		}
		unlock(LOCK_CPTIME);
	}
	if (cpndx == 0) {
		for (s = 0; s < DK_NDRIVE; s++) {
			if (dk_busy&(1<<s)) {
				dk_time[s]++;
			}
		}
	}
}

/*
 * Software priority level clock interrupt.
 * Run periodic events from timeout queue.
 * Asynchronously invoke all current scheduled kernel subroutines.
 */
/*ARGSUSED*/
softclock(pc, ps)
	caddr_t pc;
	int ps;
{

	if (cpuindex() == 0) {

		/* Asynchronously invoke each scheduled kernel subroutine
		 * with the specified arguement and at the specified IPL.
		 */
		for (;;) {
			register struct kschedblk *p1;
			register caddr_t arg;
			register void (*func)();
			register int s, ipl;

			if ((p1 = kschedq) == 0)
			    break;
			arg = p1->arg; func = p1->func; ipl = p1->ipl;
			kschedq = p1->next;
			s = splx(ipl);
			(void)(*func)(arg, p1);
			splx(s);
		}
		for (;;) {
			register struct callout *p1;
			register caddr_t arg;
			register int (*func)();
			register int a, s;

			s = spl7();
			if ((p1 = calltodo.c_next) == 0 || p1->c_time > 0) {
				splx(s);
				break;
			}
			arg = p1->c_arg; func = p1->c_func; a = p1->c_time;
			calltodo.c_next = p1->c_next;
			p1->c_next = callfree;
			callfree = p1;
			splx(s);
			(*func)(arg, a);
		}
	}
	/*
	 * If trapped user-mode and profiling, give it
	 * a profiling tick.
	 */
	if (USERMODE(ps)) {
		register struct proc *p = u.u_procp;

		if (u.u_prof.pr_scale) {
			p->p_flag |= SOWEUPC;
			aston();
		}
		/*
		 * Check to see if process has accumulated
		 * more than 10 minutes of user time.  If so
		 * reduce priority to give others a chance.
		 */
		if (p->p_uid && p->p_nice == NZERO &&
		    u.u_ru.ru_utime.tv_sec > 10 * 60) {
			p->p_nice = NZERO+4;
			(void) setpri(p);
			p->p_pri = p->p_usrpri;
		}
	}
}

/*
 * Bump a timeval by a small number of usec's.
 */
bumptime(tp, usec)
	register struct timeval *tp;
	int usec;
{

	tp->tv_usec += usec;
	if (tp->tv_usec >= 1000000) {
		tp->tv_usec -= 1000000;
		tp->tv_sec++;
	}
}

/* Schedule a kernel subroutine.
 *
 * The supplied and initialized kernel subroutine control block is placed onto
 * the head of the list of such control blocks and a software clock interrupt
 * is requested.  When IPL drops and the posted interrupt occurs, the routine
 * softclock() checks this list and asynchronous invokes each scheduled kernel
 * subroutine.
 *
 * RESTRICTIONS:
 *	1. Both the caller's IPL and the scheduled subroutine's IPL must be
 *	   equal to or greater than the soft clock IPL.
 *	2. The caller must not be dependent upon the order in which scheduled
 *	   subroutines are asynchronously invoked.
 *	3. The frequency of kernel subroutine schedulings must remain low.  The
 *	   current scheduling mechanism is not set up for frequent schedulings.
 *	   For example, no attempt is made to determine whether a soft clock
 *	   interrupt is already outstanding.  Instead, one is always requested.
 *	   This mechanism should be changed if kernel subroutine scheduling
 *	   becomes a frequent event.
 */
void ksched(f)
    register struct kschedblk *f;
{
    f->next = kschedq;
    kschedq = f;
    setsoftclock();
}

/* Unschedule a kernel subroutine.
 *
 * The specified kernel subroutine control block is removed from the
 * appropriate list of such control blocks.  No action is taken if the control
 * block is not found on the list.
 */
void unksched(f)
    register struct kschedblk *f;
{
    if (kschedq == f)
	kschedq = f->next;
    else {
	register struct kschedblk *p;

	p = kschedq;
	while( p != 0)
	    if (p->next == f) {
		p->next = f->next;
		break;
	    }
	    else
		p = p->next;
    }
}

/*
 * Arrange that (*fun)(arg) is called in t/hz seconds.
 */
timeout(fun, arg, t)
	int (*fun)();
	caddr_t arg;
	register int t;
{
	register struct callout *p1, *p2, *pnew;
	register int s = spl7();

	if (t == 0)
		t = 1;
	pnew = callfree;
	if (pnew == NULL)
		panic("timeout table overflow");
	callfree = pnew->c_next;
	pnew->c_arg = arg;
	pnew->c_func = fun;
	for (p1 = &calltodo; (p2 = p1->c_next) && p2->c_time < t; p1 = p2)
		if (p2->c_time > 0)
			t -= p2->c_time;
	p1->c_next = pnew;
	pnew->c_next = p2;
	pnew->c_time = t;
	if (p2)
		p2->c_time -= t;
	splx(s);
}

/*
 * untimeout is called to remove a function timeout call
 * from the callout structure.
 */
untimeout(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;

	s = spl7();
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			break;
		}
	}
	splx(s);
}

/*
 * Compute number of hz until specified time.
 * Used to compute third argument to timeout() from an
 * absolute time.
 */
hzto(tv)
	struct timeval *tv;
{
	register long ticks;
	register long sec;
	int s = spl7();

	/*
	 * If number of milliseconds will fit in 32 bit arithmetic,
	 * then compute number of milliseconds to time and scale to
	 * ticks.  Otherwise just compute number of hz in time, rounding
	 * times greater than representible to maximum value.
	 *
	 * Delta times less than 25 days can be computed ``exactly''.
	 * Maximum value for any timeout in 10ms ticks is 250 days.
	 */
	sec = tv->tv_sec - time.tv_sec;
	if (sec <= 0x7fffffff / 1000 - 1000)
		ticks = ((tv->tv_sec - time.tv_sec) * 1000 +
			(tv->tv_usec - time.tv_usec) / 1000) / (tick / 1000);
	else if (sec <= 0x7fffffff / hz)
		ticks = sec * hz;
	else
		ticks = 0x7fffffff;
	splx(s);
	return (ticks);
}

profil()
{
	register struct a {
		short	*bufbase;
		unsigned bufsize;
		unsigned pcoffset;
		unsigned pcscale;
	} *uap = (struct a *)u.u_ap;
	register struct uprof *upp = &u.u_prof;

	upp->pr_base = uap->bufbase;
	upp->pr_size = uap->bufsize;
	upp->pr_off = uap->pcoffset;
	upp->pr_scale = uap->pcscale;
}

opause()
{

	for (;;)
		sleep((caddr_t)&u, PSLEP);
}

/*
 * Arrange that (*fun)(args) is called on @ ipl 7.
 *   (called on as end of interrupt routine)
 */
chrqueue(fun, arg1, arg2)
	register int (*fun)();
	register int arg1;
	register int arg2;
{
	register struct chrout *pnew;
	int s = spl6();

	pnew = chrfree;
	chrfree = pnew->c_next;
	if(chrfree == chrcur)
		panic("Character queue overflow");
	pnew->c_arg = arg1;
	pnew->d_arg = arg2;
	pnew->c_func = fun;
	setintqueue();	/* Schedule interrupt queue at IPL 7 */
	splx(s);
}

intqueue()
{
		if(chrcur != chrfree)
		for(;;) {
			register struct chrout *c1;
			register int s;
			s = spl7();
			c1 = chrcur;
			if (c1 == chrfree) {
				splx(s);
				break;
			}
			chrcur = c1->c_next;
			splx(s);
			(*c1->c_func)(c1->c_arg, c1->d_arg);
		}
}
