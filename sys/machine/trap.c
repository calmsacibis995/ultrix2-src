#ifndef lint
static char *sccsid = "@(#)trap.c	1.14	ULTRIX	1/29/87";
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
 * Modification History: /sys/vax/trap.c
 *
 * 21 Jan 87 -- jaw
 *	performance fixes to syscall.
 *
 * 02-Apr-86 -- jrs
 *	Clean up for better perfomance in single cpu case.
 *	Allow some syscalls to be mp.
 *
 * 18 Mar 86 -- jrs
 *	Change calls to cpuindex/cpuident
 *
 * 24 Feb 86 -- depp
 *	Added code to SIGBUS to pass virtual address to process
 *
 * 16 Jul 85 -- jrs
 *	Add run queue locking and multicpu sched mods
 *
 * 20 Jan 86 -- pmk
 *	Added binary error logging for traps
 *
 * 14 Oct 85 -- reilly
 *	Modified user.h
 *
 * 29 Oct 84 -- jrs
 *	Fix carry bit clear for compat mode problem
 *	Derived from 4.2BSD, labeled:
 *		trap.c 6.2	84/06/10
 *
 * -----------------------------------------------------------------------
 */

#include "../machine/psl.h"
#include "../machine/reg.h"
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "assym.s"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../vax/trap.h"
#include "../h/acct.h"
#include "../h/kernel.h"
#include "../h/errlog.h"
#include "../h/interlock.h"
#include "../h/cpudata.h"
#include "../vax/mtpr.h"
#include "../h/systrace.h"

#define	USER	040		/* user-mode flag added to type */
extern int runrun;

extern int extracpu;

struct	sysent	sysent[];
int	nsysent;
#ifdef SYS_TRACE
int	traceopens = 0;
#endif SYS_TRACE

char	*trap_type[] = {
	"Reserved addressing mode",
	"Privileged instruction",
	"Reserved operand",
	"Breakpoint",
	"Xfc trap",
	"Syscall trap",
	"Arithmetic fault",
	"Ast trap",
	"Segmentation fault",
	"Protection fault",
	"Trace trap",
	"Compatibility mode trap",
#ifdef notdef
	"Page fault",
	"Page table fault",
#endif
};
#define	TRAP_TYPES	(sizeof trap_type / sizeof trap_type[0])

/*
 * Called from the trap handler when a processor trap occurs.
 */
/*ARGSUSED*/
trap(sp, type, code, pc, psl)
	int sp, type;
	unsigned code;
	int pc, psl;
{
	register int *locr0 = ((int *)&psl)-PS;
	register int i;
	register struct proc *p;
	register int cpndx;
	register int s;
	register struct el_rec *elrp;
	struct timeval syst;

	syst = u.u_ru.ru_stime;
	if (USERMODE(locr0[PS])) {
		type |= USER;
		u.u_ar0 = locr0;
		cpndx = u.u_pcb.pcb_cpundx;
	} else {
		if (extracpu) cpndx = cpuindex();
		else cpndx=0;
	}

	switch (type) {

	default:
		elrp = ealloc(EL_EXPTFLTSIZE,EL_PRISEVERE);
		if (elrp != 0) {
		    LSUBID(elrp,ELCT_EXPTFLT,(type + 1),EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
		    elrp->el_body.elexptflt.exptflt_va = code;
		    elrp->el_body.elexptflt.exptflt_pc = pc;
		    elrp->el_body.elexptflt.exptflt_psl = psl;
		    EVALID(elrp);
		}
		cprintf("trap type %d, code = %x, pc = %x\n", type, code, pc);
		type &= ~USER;
		if ((unsigned)type < TRAP_TYPES)
			panic(trap_type[type]);
		panic("trap");

	case T_PROTFLT+USER:	/* protection fault */
		i = SIGBUS;
		u.u_code = code;
		break;

	case T_PRIVINFLT+USER:	/* privileged instruction fault */
	case T_RESADFLT+USER:	/* reserved addressing fault */
	case T_RESOPFLT+USER:	/* resereved operand fault */
		u.u_code = type &~ USER;
		i = SIGILL;
		break;

	case T_ASTFLT+USER:
		astoff();
		if ((u.u_procp->p_flag & SOWEUPC) && u.u_prof.pr_scale) {
			addupc(pc, &u.u_prof, 1);
			u.u_procp->p_flag &= ~SOWEUPC;
		}
		goto out;

	case T_ARITHTRAP+USER:
		u.u_code = code;
		i = SIGFPE;
		break;

	/*
	 * If the user SP is above the stack segment,
	 * grow the stack automatically.
	 */
	case T_SEGFLT+USER:
		if (extracpu) {
			u.u_procp->p_flag |= SMASTER;
			if (cpndx != 0) {
				s = spl6();
				lock(LOCK_RQ);
				setrq(u.u_procp);
				u.u_ru.ru_nivcsw++;
				swtch();
				(void) splx(s);
			}
		}
		if (grow((unsigned)locr0[SP]) || grow(code))
			goto out;
		u.u_code = code;
		i = SIGSEGV;
		break;

	case T_TABLEFLT:	/* allow page table faults in kernel mode */
	case T_TABLEFLT+USER:   /* page table fault */
		panic("ptable fault");

	case T_PAGEFLT:		/* allow page faults in kernel mode */
	case T_PAGEFLT+USER:	/* page fault */

		if (extracpu) {
			u.u_procp->p_flag |= SMASTER;
			if (cpndx != 0) {
				s = spl6();
				lock(LOCK_RQ);
				setrq(u.u_procp);
				u.u_ru.ru_nivcsw++;
				swtch();
				(void) splx(s);
			}
		}
		i = u.u_error;
		pagein(code, 0);
		u.u_error = i;
		if (type == T_PAGEFLT)
			return;
		goto out;

	case T_BPTFLT+USER:	/* bpt instruction fault */
	case T_TRCTRAP+USER:	/* trace trap */
		locr0[PS] &= ~PSL_T;
		i = SIGTRAP;
		break;

	case T_XFCFLT+USER:	/* xfc instruction fault */
		i = SIGEMT;
		break;

	case T_COMPATFLT+USER:	/* compatibility mode fault */
		u.u_acflag |= ACOMPAT;
		u.u_code = code;
		i = SIGILL;
		break;
	}
	if (extracpu) {
		u.u_procp->p_flag |= SMASTER;
		if (cpndx != 0) {
			s = spl6();
			lock(LOCK_RQ);
			setrq(u.u_procp);
			u.u_ru.ru_nivcsw++;
			swtch();
			(void) splx(s);
		}
	}
	psignal(u.u_procp, i);
out:
	p = u.u_procp;
	if (p->p_cursig || ISSIG(p)) {
		if (extracpu) {
			u.u_procp->p_flag |= SMASTER;
			if (cpndx != 0) {
				s = spl6();
				lock(LOCK_RQ);
				setrq(u.u_procp);
				u.u_ru.ru_nivcsw++;
				swtch();
				(void) splx(s);
			}
		}
		psig();
	}
	p->p_pri = p->p_usrpri;	
	if (extracpu == 0) {
		if (runrun) {
			(void) spl6();
			setrq(p);
			u.u_ru.ru_nivcsw++;
			swtch();
		}
		mincurpri = p->p_pri;
	} else {
		p->p_flag &= ~SMASTER;
		if (cpudata[u.u_pcb.pcb_cpundx].c_runrun) {
		/*
		 * Since we are u.u_procp, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 */
			(void) spl6();
			lock(LOCK_RQ);
			setrq(p);
			u.u_ru.ru_nivcsw++;
			swtch();
		}
		cpudata[u.u_pcb.pcb_cpundx].c_curpri = p->p_pri;
		if ((cpndx == minpricpu && p->p_pri != mincurpri)
				|| p->p_pri > mincurpri) {
			minpri();
		}
	}
	if (u.u_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &u.u_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(locr0[PC], &u.u_prof, ticks);
	}
}

/*
 * Called from the trap handler when a system call occurs
 */
/*ARGSUSED*/
syscall(sp, type, code, pc, psl)
	unsigned code;
{
	register int *locr0 = ((int *)&psl)-PS;
	register caddr_t params;		/* known to be r10 below */
	register int i;				/* known to be r9 below */
	register struct sysent *callp;
	register struct proc *p;
	register struct nameidata *ndp = &u.u_nd;
	int opc;
	int s;
	struct timeval syst;

	syst = u.u_ru.ru_stime;
	if (!USERMODE(locr0[PS]))
		panic("syscall");
	u.u_ar0 = locr0;
	if (code == 139) {	/* getdprop */	/* XXX */
		sigcleanup();			/* XXX */
		goto done;			/* XXX */
	}
	params = (caddr_t)locr0[AP] + NBPW;
	u.u_error = 0;
	opc = pc - 2;
	if (code > 63)
		opc -= 2;
	callp = (code >= nsysent) ? &sysent[63] : &sysent[code];
	if (callp == sysent) {
		i = fuword(params);
		params += NBPW;
		callp = ((unsigned)i >= nsysent) ? &sysent[63] : &sysent[i];
	}
	if (i = callp->sy_narg * sizeof (int)) {
#ifndef lint
		asm("prober $3,r9,(r10)");		/* GROT */
		asm("bnequ ok");			/* GROT */
		u.u_error = EFAULT;			/* GROT */
		goto bad;				/* GROT */
ok:
asm("ok:");						/* GROT */
		asm("movc3 r9,(r10),_u+U_ARG");		/* GROT */
#else
		bcopy(params, (caddr_t)u.u_arg, (u_int)i);
#endif
	}
	u.u_ap = u.u_arg;
	ndp->ni_dirp = (caddr_t)u.u_arg[0];
	u.u_r.r_val1 = 0;
	u.u_r.r_val2 = locr0[R1];
	if (setjmp(&u.u_qsave)) {
		if (u.u_error == 0 && u.u_eosys == JUSTRETURN)
			u.u_error = EINTR;
	} else {
		u.u_eosys = JUSTRETURN;
		if (extracpu) {
			if (callp->sy_mpsafe == 0) {
				u.u_procp->p_flag |= SMASTER;
				if (u.u_pcb.pcb_cpundx != 0) {
					s = spl6();
					lock(LOCK_RQ);
					setrq(u.u_procp);
					u.u_ru.ru_nivcsw++;
					swtch();
					(void) splx(s);
				}
			}	
		}
#ifdef SYS_TRACE
 		/* trace it just before we do it! (only if open) */
 		if (traceopens) {
			syscall_trace(code,callp->sy_narg,BEFORE);
			(*(callp->sy_call))();
			syscall_trace(code,callp->sy_narg,AFTER);
		} else 
#endif SYS_TRACE
			(*(callp->sy_call))();
	}
	if (u.u_eosys == RESTARTSYS)
		pc = opc;
#ifdef notdef
	else if (u.u_eosys == SIMULATERTI)
		dorti();
#endif
	else if (u.u_error) {
#ifndef lint
bad:
#endif
		locr0[R0] = u.u_error;
		locr0[PS] |= PSL_C;	/* carry bit */
	} else {
		locr0[R0] = u.u_r.r_val1;
		locr0[R1] = u.u_r.r_val2;
		locr0[PS] &= ~PSL_C;
	}
done:
	p = u.u_procp;
	if (p->p_cursig || ISSIG(p)) {
		if (extracpu) {
			u.u_procp->p_flag |= SMASTER;
			if (u.u_pcb.pcb_cpundx != 0) {
				s = spl6();
				lock(LOCK_RQ);
				setrq(u.u_procp);
				u.u_ru.ru_nivcsw++;
				swtch();
				(void) splx(s);
			}
		}
		psig();
	}
	p->p_pri = p->p_usrpri;
	if(extracpu==0) {
		if (runrun) {
			(void) spl6();
			setrq(p);
			u.u_ru.ru_nivcsw++;
			swtch();
		}
		mincurpri = p->p_pri;
	} else {
		p->p_flag &= ~SMASTER;
		if (cpudata[u.u_pcb.pcb_cpundx].c_runrun) {
			/*
			 * Since we are u.u_procp, clock will normally just change
			 * our priority without moving us from one queue to another
			 * (since the running process is not on a queue.)
			 * If that happened after we setrq ourselves but before we
			 * swtch()'ed, we might not be on the queue indicated by
			 * our priority.
			 */
			(void) spl6();
			lock(LOCK_RQ);
			setrq(p);
			u.u_ru.ru_nivcsw++;
			swtch();
		}
		cpudata[u.u_pcb.pcb_cpundx].c_curpri = p->p_pri;
		if ((u.u_pcb.pcb_cpundx == minpricpu && p->p_pri != mincurpri)
				|| p->p_pri > mincurpri) {
			minpri();
		}
	}
	if (u.u_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &u.u_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(locr0[PC], &u.u_prof, ticks);
	}
}

/*
 * nonexistent system call-- signal process (may want to handle it)
 * flag error if process won't see signal immediately
 * Q: should we do that all the time ??
 */
nosys()
{
	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
		u.u_error = EINVAL;
	psignal(u.u_procp, SIGSYS);
}
