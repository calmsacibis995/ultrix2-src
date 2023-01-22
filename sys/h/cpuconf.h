/*
 *@(#)cpuconf.h	1.8	(ULTRIX)	8/10/86
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984,85,86 by			*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/***********************************************************************
 *
 * Modification History:
 *
 * 06-Aug-86 -- jaw	fixed baddaddr to work on running system.
 *
 * 16-Apr-86	Darrell Dunnuck
 *	Defined the macro BADADDR.
 *
 * 14-Mar-86	Darrell Dunnuck
 *	Removed percpu structure and put needed elements of that 
 *	struct into cpusw.
 *
 * 12-Mar-86 -- bjg  Add sbierr routine to cpusw
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 18-Feb-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 12-Feb-85	Darrell Dunnuck
 *	added cachenbl to cpusw
 *
 * 12-Dec-85 	Darrell Dunnuck
 *	Created this file to as part of machdep restructuring.
 *
 **********************************************************************/

/*
 * Macros for entering the cpusw 
 */
#define BADADDR(addr,len) (badaddr(addr, len))

/*
 * The cpu switch is defined here.  Each entry is the only
 * line between the main unix code and the cpu dependent
 * code.  The initialization of the cpu switch is in the
 * file cpuconf.c.
 */

struct cpusw
{
	int	(*v_machcheck)();
	int	(*v_memerr)();
	int	(*v_setcache)();
	int	(*v_memenable)();
	int	(*v_tocons)();
	int	(*v_configure)();
	int	(*v_cachenbl)();
	int	(*v_badaddr)();
	int	(*v_nexaddr)();
	int	(*v_umaddr)();
	int	(*v_udevaddr)();
	int	(*v_readtodr)();
	int	(*v_writetodr)();
	int	(*v_sbierr)();
	short	pc_nnexus;		/* number of nexus slots */
	int	pc_nexsize;		/* size of a nexus */
	int	pc_umsize;		/* unibus memory size */
	short	pc_haveubasr;		/* have uba status register */
	short	*pc_nextype;		/* adapter types if no conf. reg. */
	short	pc_nioa;		/* number of IO adapters (8600) */
	short	**pc_ioabase;		/* physical base of IO adapters space */
	int	pc_ioasize;		/* size of an IO adapter */
	int	flags;			/* cpusw flags */
};

#ifdef KERNEL
struct	cpusw cpusw[];
#endif
