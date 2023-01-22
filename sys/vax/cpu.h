/* 
 * @(#)cpu.h	1.17 	Ultrix 8/10/86
 */

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
 * Modification History:
 *
 *  5-Aug-86 -- fred (Fred Canter)
 *	Changed VS_TEAMMATE to VS_MULTU to match VS410 spec.
 *
 * 28-Jul-86 -- bglover
 *	Added defines for cpu_subtype for Scorpio and Nautilus
 *
 *  2-Jul-86 -- fred (Fred Canter)
 *	Changed VS_JMPWX to VS_TEAMMATE (ID VAXstar vs TEAMmate CPU).
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 03-Mar-86 -- darrell
 *	Removed the percpu structure - moved some of the fields in
 *	percpu to cpusw.
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *
 * 28 Jan 86 -- darrell
 *	The generic machine check stack frame structure is now 
 *	defined here, as part of the machine dependent code 
 *	restructuring.
 *
 *  3 Jul 85 -- jrs
 *   	add support for vax 8800.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 *  20 Mar 85 -- jaw
 *   	add support for vax 8200.
 *
 *  4 Nov 84 -- rjl
 *	Generalized the percpu structure to handle the MicroVAX-II's treatment
 *	of the local register space (nexus) and the q-bus.
 *
 * 19 Aug 84 -- rjl
 *	Added support for MicroVAX-II
 *
 * 30 Nov 83 --tresvik
 *	Split the cp_eco field into eco and subeco levels
 *	for the 11/780.  Removed cp_hrev for the 11/730 as per CSSE
 *	the SRM (reserved field).
 *
 *  6 Oct 83 --jmcg
 *	Had to make a comment out of sccsid.
 *
 *  5 Oct 83 --jmcg
 *	Added structs and defines for VAX 730 and MicroVAX.
 *
 *  3 Oct 83 --jmcg
 *	Derived from 4.2BSD pre-release, labeled:
 *		cpu.h	6.1	83/07/29
 *	It must be RE-MASTERED when 4.2BSD final release arrives.
 * ------------------------------------------------------------------------
 */

#ifndef LOCORE
/*
 * Cpu identification, from SID (and SID extension) register.
 */
union cpusid {
	int	cpusid;
	struct cpuany {
		u_int	:24,
			cp_type:8;
	} cpuany;
	struct cpu8800 {
		u_int	cp_sno:16,
			cp_eco:7,
			cp_lr:1,
			cp_type:8;
	} cpu8800;
	struct cpu8600 {
		u_int	cp_sno:12,		/* serial number */
			cp_plant:4,		/* plant number */
			cp_eco:8,		/* eco level */
			cp_type:8;		/* VAX_8600 */
	} cpu8600;
	struct cpu8200 {
		u_int	cp_urev:8,
			cp_secp:1,
			cp_patch:10,
			cp_hrev:5,
			cp_type:8;
	} cpu8200;
	struct cpu780 {
		u_int	cp_sno:12,		/* serial number */
			cp_plant:3,		/* plant number */
			cp_subeco:4,		/* sub-system-rev */
			cp_eco:5,		/* eco level */
			cp_type:8;		/* VAX_780 */
	} cpu780;
	struct cpu750 {
		u_int	cp_hrev:8,		/* hardware rev level */
			cp_urev:8,		/* ucode rev level */
			:8,
			cp_type:8;		/* VAX_750 */
	} cpu750;
	struct cpu730 {
		u_int	:8,			/* reserved */
			cp_urev:8,		/* ucode rev level */
			:8,			/* reserved */
			cp_type:8;		/* VAX_730 */
	} cpu730;
	struct cpuMVI {				/* MicroVAX I */
		u_int	cp_hrev:8,		/* hardware rev level */
			cp_urev:8,		/* ucode rev level */
			:8,
			cp_type:8;		/* MVAX_I */
	} cpuMVI;
	struct cpuMVII {			/* MicroVAX II */
		u_int	cp_hrev:8,		/* hardware rev level */
			cp_urev:8,		/* ucode rev level */
			:8,
			cp_type:8;		/* MVAX_II */
	} cpuMVII;
	/* note: structs for 750, 730, and MicroVAX_I have the same layout */
	/* MicroVAX-chip-based systems will require additional logic to
	 * distinguish between implementations.
	 */
};
#endif
#define	VAX_780		1
#define	VAX_750		2
#define	VAX_730		3
#define VAX_8600	4
#define VAX_8200	5
#define VAX_8800	6
#define MVAX_I		7
#define MVAX_II		8
#define	VAX_MAX		8

/*
 * SID extension register and CPU sub-types,
 * needed for VAXstar support.
 */

#define	SID_EXT		0x20040004	/* I/O space phys addr of SID ext reg */
#define	ST_MVAXII	0x1		/* Micro/VAX-II sub-type */
#define	ST_VAXSTAR	0x4		/* VAXstar sub-type */
#define ST_8200		0x5		/* Single CPU Scorpio */
#define ST_8300		0x6		/* Dual CPU Scorpio */
#define ST_8400		0x7		/* 3-CPU Scorpio (non-supp.) */
#define ST_8500		0x8		/* Single CPU Naut. (slow) */
#define ST_8550		0x9		/* Single CPU Naut.(fast/non-expand) */
#define ST_8700		0xa		/* Single CPU Naut. (fast/expand) */
#define ST_8800		0xb		/* Dual CPU Naut. (fast) */
		
/*
 * VAXstar I/O reset and configuration
 * and test register definitions.
 */

#define	VS_IORESET	0x20020000	/* VAXstar I/O reset register (w/o) */
#define	VS_CFGTST	0x20020000	/* VAXstar config & test reg (r/o) */
#define	VS_MTYPEMASK	0x7		/* Memory option type (mask) */
#define	VS_MTYPE_1MB	0x1		/* 1MB memory board */
#define	VS_MTYPE_2MB	0x2		/* 2MB memory board */
#define	VS_MTYPE_4MB	0x3		/* 4MB memory board */
#define	VS_VIDOPT	0x8		/* Video option present (color) */
#define	VS_CURTEST	0x10		/* Monochrome video present */
#define	VS_L3CON	0x20		/* SLU line 3 is the console */
#define	VS_NETOPT	0x40		/* Network option present */
#define	VS_MULTU	0x80		/* Set=MICROVAX 1800, clear=VAXstar */

#ifndef LOCORE
#ifdef KERNEL
int	cpu;
int	cpu_subtype;
int	vs_cfgtst;	/* VAXstar configuration and test register */
#endif KERNEL

/*
 * Generic Machinecheck Stack Frame
 */
struct mcframe	{
	int	mc_bcnt;		/* byte count */
	int	mc_summary;		/* summary parameter */
};
#endif LOCORE
