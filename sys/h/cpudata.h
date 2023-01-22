/*	@(#)cpudata.h	1.6	(ULTRIX)	12/16/86 */
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

/* ---------------------------------------------------------------------
 * Modification History 
 *
 * 11-DEC-86 -- jaw 	made MP variables externs so lint would work
 *	properly that include this file.
 *
 * 02-Apr-86 -- jrs
 *	Added bit defn for panicing
 *
 * 03-Mar-86 -- jrs
 *	Replaced unused c_panicstr element with c_cptime array to
 *	hold the per processor utilization summary.
 * 
 * 05-Feb-86 --jrs
 *	Added new c_paddr element for adb to latch onto
 *	Added translation buffer inval needed flag
 * 
 * 15 Jul 85 --jrs
 *	Created to hold per cpu related structures
 * ---------------------------------------------------------------------
 */

#ifndef LOCORE

#ifndef	CPUSTATES
#ifdef	KERNEL
#include "../h/dk.h"
#else	KERNEL
#include <sys/dk.h>
#endif	KERNEL
#endif	CPUSTATES

struct cpudata {
				/* c_paddr must be first element for adb!! */
	int	c_paddr;	/* physical address of pcb */
	int	c_state;	/* state of this processor */
	int	c_ident;	/* processor specific unique identifier */
	int	c_runrun;	/* this processor need to be rescheduled */
	int	c_noproc;	/* no assigned process */
	int	c_switch;	/* context switches done on this cpu */
	char	c_curpri;	/* priority of current running process */
	struct	proc *c_proc;	/* pointer to assigned proc */
	long	c_cptime[CPUSTATES];	/* usage time for each state */
};

extern struct cpudata cpudata[];

/*
 * global variables dealing with multi-processor level
 */

extern	int	maxcpu;		/* number of configured processors */
extern  int	activecpu;	/* number of active processors */
extern  int	mincurpri;	/* current minimum running priority */
extern  int	minpricpu;	/* cpu that min priority task is running on */
extern  char	*istack;	/* interrupt stacks for slave cpus */

#endif LOCORE

/*
 * state flags
 */

#define	CPU_RUN		0x00000001	/* processor is up and running */
#define	CPU_TBI		0x00000002	/* translation buffer needs inval */
#define	CPU_PANIC	0x00000004	/* this cpu has paniced */
