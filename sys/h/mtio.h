
/*
 *	@(#)mtio.h	1.10	(ULTRIX)	1/29/87
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1987 by			*
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

/*
 * mtio.h	6.1	07/29/83
 *
 * Modification history
 *
 * Common structures and definitions for mag tape drivers and ioctl
 *
 *  6-Dec-84 -	afd
 *
 *	Derived from 4.2BSD labeled: mtio.h	6.1	83/07/29.
 *	Deleted 2 Sun definitions and added tmscp definition.
 *
 * 18-Jul-85 -	afd
 *
 *	Added MTCACHE & MTNOCACHE for tmscp units.
 *
 * 31-Jul-85 -	ricky palmer
 *
 *	Added mt_softstat field and defines.
 *
 * 23-Jan-86 -	ricky palmer
 *
 *	Added common tape driver definitions.
 *
 *  4-Mar-86 - ricky palmer
 *
 *	Moved mt_softstat functionality and defines to "devio.h". V2.0
 *	Also moved ioctl defines to "ioctl.h". V2.0
 *
 * 17-Mar-86 - ricky palmer
 *
 *	Changed UNIT, SEL, and density tape definitions back to "old"
 *	value equivalents so that minor bits are compatible with pre-2.0
 *	minor numbers (this is a real kludge that Berkeley started in
 *	the first place with a POOR design).
 *
 *   5-Aug-86 - darrell (Darrell Dunnuck)
 *
 *	Added a define for VAXstar TZK50 driver.
 *
 *  29-Jan-87 - rsp (Ricky Palmer)
 *
 *	Fixed the UNIT macro to correctly handle more than four drives.
 *
 */

/* Structure for MTIOCTOP ioctl - mag tape operation command */
struct	mtop	{
	short	mt_op;			/* Operations defined below	*/
	daddr_t mt_count;		/* How many of them		*/
};

/* Structure for MTIOCGET ioctl - mag tape get status command */
struct	mtget	{
	short	mt_type;		/* Type of device defined below */
	short	mt_dsreg;		/* ``drive status'' register	*/
	short	mt_erreg;		/* ``error'' register		*/
	short	mt_resid;		/* Residual count		*/
};

/* Basic definitions common to various tape drivers */
#define b_repcnt	b_bcount		/* Repeat count 	*/
#define b_command	b_resid 		/* Command value	*/
#define SSEEK		0x01			/* Seeking		*/
#define SIO		0x02			/* Doing sequential i/o */
#define SCOM		0x03			/* Sending control cmd. */
#define SREW		0x04			/* Sending drive rewnd. */
#define SERASE		0x05			/* Erase interrec. gap	*/
#define SERASED 	0x06			/* Erased interrec. gap */
#define MASKREG(r)	((r) & 0xffff)		/* Register mask	*/
#define UNIT(dev)	((minor(dev)&0xe0)>>3)| \
			(minor(dev)&0x03) 	/* Tape unit number	*/
#define SEL(dev)	(minor(dev)&0x001c)	/* Tape select number	*/
#define INF		(daddr_t)1000000L	/* Invalid block number */
#define DISEOT		0x01			/* Disable EOT code	*/
#define DBSIZE		0x20			/* Dump blocksize (32)	*/
#define PHYS(a,b)	((b)((int)(a)&0x7fffffff)) /* Physical dump dev.*/
#define MTLR		0x00		/* Low density/Rewind (0)	*/
#define MTMR		0x10		/* Medium density/Rewind (16)	*/
#define MTHR		0x08		/* High density/Rewind (8)	*/
#define MTLN		0x04		/* Low density/Norewind (4)	*/
#define MTMN		0x14		/* Medium density/Norewind (20) */
#define MTHN		0x0c		/* High density/Norewind (12)	*/
#define MTX0		0x00		/* eXperimental 0		*/
#define MTX1		0x01		/* eXperimental 1		*/

/* Tape operation definitions for operation word (mt_op) */
#define MTWEOF		0x00		/* Write end-of-file record	*/
#define MTFSF		0x01		/* Forward space file		*/
#define MTBSF		0x02		/* Backward space file		*/
#define MTFSR		0x03		/* Forward space record 	*/
#define MTBSR		0x04		/* Backward space record	*/
#define MTREW		0x05		/* Rewind			*/
#define MTOFFL		0x06		/* Rewind and unload tape	*/
#define MTNOP		0x07		/* No operation 		*/
#define MTCACHE 	0x08		/* Enable tmscp caching 	*/
#define MTNOCACHE	0x09		/* Disable tmscp caching	*/
#define MTCSE		0x0a		/* Clear serious exception	*/
#define MTCLX		0x0b		/* Clear hard/soft-ware problem */
#define MTCLS		0x0c		/* Clear subsystem		*/
#define MTENAEOT	0x0d		/* Enable default eot code	*/
#define MTDISEOT	0x0e		/* Disable default eot code	*/

/* Get status definitions for device type word (mt_type) */
#define MT_ISTS 	0x01		/* ts11/ts05/tu80		*/
#define MT_ISHT 	0x02		/* tm03/te16/tu45/tu77		*/
#define MT_ISTM 	0x03		/* tm11/te10			*/
#define MT_ISMT 	0x04		/* tm78/tu78			*/
#define MT_ISUT 	0x05		/* tu45 			*/
#define MT_ISTMSCP	0x06		/* All tmscp tape drives	*/
#define MT_ISST		0x07		/* TZK50 on vaxstar		*/

/* Default tape device definitions for programs */
#ifndef KERNEL
#define DEFTAPE_RL	"/dev/rmt0l"	/* 1st tape, rewind, low dens.	*/
#define DEFTAPE_RM	"/dev/rmt0m"	/* 1st tape, rewind, med. dens. */
#define DEFTAPE_RH	"/dev/rmt0h"	/* 1st tape, rewind, high dens. */
#define DEFTAPE_NL	"/dev/nrmt0l"	/* 1st tape, norew., low dens.	*/
#define DEFTAPE_NM	"/dev/nrmt0m"	/* 1st tape, norew., med. dens. */
#define DEFTAPE_NH	"/dev/nrmt0h"	/* 1st tape, norew., high dens. */
#endif KERNEL
