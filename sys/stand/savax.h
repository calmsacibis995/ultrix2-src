
/*	@(#)savax.h	1.5	(ULTRIX)	4/18/86 */
/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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
 * Standalone definitions peculiar to vaxen
 * The mba devices in the standalone system are addressed as 
 *	xx(unit,section)
 * where unit is
 *	8*mbanum+drive
 * The mbadrv macro gives the address of the device registers
 * for the specified unit; the mbamba macro gives the address of the
 * mba registers themselves.
 *
 * The uba devices are also addressed by giving, as unit,
 *	8*ubanum+drive
 * The ubamem macro converts a specified unibus address (ala pdp-11)
 * into a unibus memory address space address.
 */
/*
 * Modification History
 *
 * 14-Mar-85 --tresvik
 * 	Modified MAXNMBA to 8 for VAX8600
 */

int	cpu;		/* see <sys/cpu.h> */

#define	MAXNMBA	8
#define MAXNUBA 8
struct	mba_regs **mbaddr;
int	mbaact;
caddr_t	*devaddr;
struct	uba_regs **ubaddr;

#define	UNITTOMBA(unit)		((unit)>>3)
#define	UNITTODRIVE(unit)	((unit)&07)

#define	mbamba(unit)		(mbaddr[UNITTOMBA(unit)])
#define	mbadrv(unit) 		(&mbamba(unit)->mba_drv[UNITTODRIVE(unit)])

#define	UNITTOUBA(unit)		((unit)>>3)
#define	ubauba(unit)		(ubaddr[UNITTOUBA(unit)])
#define	ubamem(unit, off)	((devaddr[UNITTOUBA(unit)]+ubdevreg(off)))

#define	PHYSUBA0	0x20006000
#define	PHYSMBA0	0x20010000
#define	PHYSMBA1	0x20012000
#define	PHYSUMEM	0x2013e000

#define NRL	1
#define NHP	1
#define NHT	1
#define NRB	1
#define NRK	1
#define NTE	1
#define NTS	1
#define NTJ	1
#define NTMS	1
#define NUP	1
#define NRA	1
