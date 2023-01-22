
#ifndef lint
static	char	*sccsid = "@(#)cpuconf.c	1.12  (ULTRIX)        8/10/86";
#endif lint

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
 * 18-Apr-86 -- jaw	hooks for nmi faults and fixes to bierrors.
 *
 * 09-Apr-86 -- Darrell Dunnuck
 *	Changed references to badaddr to bbadaddr.
 *	Added badaddr routine for backware compatability.
 *
 * 02-Apr-86 -- jaw  add support for nautilus console and memory adapter
 *
 * 15-Mar-86	Darrell Dunnuck
 *	Initialize fields in cpusw that were moved here from the
 *	percpu struct.
 *
 * 12-Mar-86 -- bjg  Add sbierrlog routines for 780 and 8600
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 18-Feb-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *
 * 12-Feb-86	Darrell Dunnuck
 *	Defined entries into cpusw for the routines memerr,
 *	memenable, setcache, tocons, and the new routines
 *	cachenbl and nullcpu.
 *
 * 11-Dec-85 	Darrell Dunnuck
 *	Created this file to describe the cpusw structure.
 *
 **********************************************************************/

#include "../h/cpuconf.h"
#include "../machine/pte.h"
#include "../h/param.h"
#include "../vax/cpu.h"
#include "../vax/ioa.h"
#include "../vax/nexus.h"
#include "../vaxuba/ubareg.h"

int	nocpu();
int	nullcpu();
int 	bbadaddr();
int	readtodr();
int 	writetodr();

extern short nexty750[];
extern short nexty730[];
extern short nextyUVI[];
extern short *ioaaddr8600[];

#ifdef VAX8800
int  	ka8800nexaddr(); 
int	ka8800umaddr();
int	ka8800udevaddr();
int	ka8800machcheck();
int	ka8800memerr();
int	ka8800conf();
int 	ka8800badaddr();
int	ka8800tocons();
int	ka8800cachenbl();
int 	ka8800setcache();
int 	ka8800memenable();
int	ka8800readtodr();
int	ka8800writetodr();
#else
#define ka8800machcheck	nocpu
#define ka8800conf	nocpu
#define ka8800badaddr  	nocpu
#define	ka8800nexaddr	nocpu
#define	ka8800umaddr	nocpu
#define	ka8800udevaddr	nocpu
#define ka8800memerr 	nocpu
#define ka8800setcache	nocpu
#define ka8800memenable	nocpu
#define ka8800tocons	nocpu
#define ka8800cachenbl	nocpu
#define ka8800readtodr	nocpu
#define ka8800writetodr	nocpu
ka8800nmifault() { /*keep locore happy */}
nmifaultclear(){/* keep locore happy */}
ka8800requeue(){/* tty console driver */ }
ka8800startrx(){/* start rx50 */ }
rx8800_trans() {/* rx50 transmit routine */ }
#endif VAX8800

#ifdef VAX8600
int  	ka8600nexaddr(); 
int	ka8600memerr();
int	ka8600setcache();
int	ka8600memenable();
int	ka8600tocons();
int	ka8600cachenbl();
int	ka8600umaddr();
int	ka8600udevaddr();
int	ka8600machcheck();
int	ka8600logsbi();
int	ka8600conf();
#else
#define ka8600machcheck	nocpu
#define ka8600memerr 	nocpu
#define ka8600setcache	nocpu
#define ka8600memenable	nocpu
#define ka8600tocons	nocpu
#define ka8600configure	nocpu
#define ka8600cachenbl	nocpu
#define	ka8600nexaddr	nocpu
#define	ka8600umaddr	nocpu
#define	ka8600udevaddr	nocpu
#define ka8600logsbi	nocpu
#define	ka8600conf	nocpu
crlintr() { /*keep locore happy*/ }  
#endif VAX8600

#ifdef VAX8200
int  	ka8200nexaddr(); 
int	ka8200umaddr();
int	ka8200udevaddr();
int	ka8200machcheck();
int	ka8200conf();
int	ka8200memerr();
int	ka8200setcache();
int	ka8200memenable();
int	ka8200tocons();
int	ka8200cachenbl();
int	ka8200readtodr();
int	ka8200writetodr();
#else
#define ka8200conf	nocpu
#define	ka8200nexaddr	nocpu
#define	ka8200umaddr	nocpu
#define	ka8200udevaddr	nocpu
#define	ka8200machcheck nocpu
#define ka8200memerr 	nocpu
#define ka8200setcache	nocpu
#define ka8200memenable	nocpu
#define ka8200tocons	nocpu
#define ka8200cachenbl	nocpu
#define ka8200readtodr	nocpu
#define ka8200writetodr	nocpu
ka8200startrx() {/* rx50 start routine */ }
rx5_intr() { /* rx50 interrupt routine */ }
#endif

#ifdef VAX780
int  	ka780nexaddr(); 
int	ka780umaddr();
int	ka780udevaddr();
int	ka780machcheck();
int	ka780memerr();
int	ka780setcache();
int	ka780memenable();
int	ka780tocons();
int	ka780cachenbl();
int	ka780logsbi();
int	ka780conf();
#else
#define ka780machcheck	nocpu
#define ka780memerr 	nocpu
#define ka780setcache	nocpu
#define ka780memenable	nocpu
#define ka780tocons	nocpu
#define ka780configure	nocpu
#define ka780cachenbl	nocpu
#define	ka780nexaddr	nocpu
#define	ka780umaddr	nocpu
#define	ka780udevaddr	nocpu
#define ka780logsbi	nocpu
#define ka780conf	nocpu
#endif VAX780

#ifdef VAX750
int	ka750machcheck();
int	ka750memerr();
int	ka750setcache();
int	ka750memenable();
int	ka750tocons();
int	ka750cachenbl();
int  	ka750nexaddr(); 
int	ka750umaddr();
int	ka750udevaddr();
int	ka750conf();
#else
#define ka750machcheck	nocpu
#define ka750memerr 	nocpu
#define ka750setcache	nocpu
#define ka750memenable	nocpu
#define ka750tocons	nocpu
#define ka750configure	nocpu
#define ka750cachenbl	nocpu
#define	ka750nexaddr	nocpu
#define	ka750umaddr	nocpu
#define	ka750udevaddr	nocpu
#define	ka750conf	nocpu
#endif VAX750

#ifdef VAX730
int  	ka730nexaddr(); 
int	ka730umaddr();
int	ka730udevaddr();
int	ka730machcheck();
int	ka730memerr();
int	ka730memenable();
int	ka730tocons();
int	ka730conf();
#else
#define ka730machcheck	nocpu
#define	ka730nexaddr	nocpu
#define ka730memerr 	nocpu
#define ka730memenable	nocpu
#define ka730configure	nocpu
#define ka730tocons	nocpu
#define	ka730umaddr	nocpu
#define	ka730udevaddr	nocpu
#define	ka730conf	nocpu
#endif VAX730

#ifdef MVAX
int	ka610machcheck();
int	ka610setcache();
int	ka610cachenbl();
int	ka610tocons();
int  	ka610nexaddr(); 
int	ka610umaddr();
int	ka610udevaddr();
int	ka610conf();
int	ka630setcache();
int	ka630cachenbl();
int	ka630tocons();
int  	ka630nexaddr(); 
int	ka630umaddr();
int	ka630udevaddr();
int	ka630machcheck();
int	ka630readtodr();
int	ka630writetodr();
int	ka630conf();
#else
#define ka610machcheck	nocpu
#define	ka610setcache	nocpu
#define	ka610cachenbl	nocpu
#define	ka610tocons	nocpu
#define	ka610nexaddr	nocpu
#define	ka610umaddr	nocpu
#define	ka610udevaddr	nocpu
#define	ka610conf	nocpu
#define ka630machcheck	nocpu
#define	ka630setcache	nocpu
#define	ka630cachenbl	nocpu
#define	ka630tocons	nocpu
#define	ka630nexaddr	nocpu
#define	ka630umaddr	nocpu
#define	ka630udevaddr	nocpu
#define ka630readtodr	nocpu
#define ka630writetodr	nocpu
#define	ka630conf	nocpu
#endif MVAX

struct cpusw	cpusw[] =
{
	{ nocpu,		nocpu,		nocpu,		
	  nocpu,		nocpu,		nocpu,	
	  nocpu,		nocpu,		nocpu,
	  nocpu,		nocpu,		0,
	  0,			0,		0,
	  0,			0,		0,
	  0,			0				},/* 0 */

	{ ka780machcheck,	ka780memerr,	ka780setcache,	
	  ka780memenable,	ka780tocons,	ka780conf,		
	  ka780cachenbl,	bbadaddr, 	ka780nexaddr, 
	  ka780umaddr,		ka780udevaddr,	readtodr,
	  writetodr,		ka780logsbi,	NNEX780,
	  NEXSIZE,		UMEMSIZE780,	1,
	  0,			0,		0,
	  0,			0				},/* 1 */

	{ ka750machcheck,	ka750memerr,	ka750setcache,	
	  ka750memenable,	ka750tocons,	ka750conf,		
	  ka750cachenbl,	bbadaddr, 	ka750nexaddr, 
	  ka750umaddr, 		ka750udevaddr,	readtodr,
	  writetodr,		nullcpu,	NNEX750,
	  NEXSIZE,		UMEMSIZE750,	0,
	  nexty750,		0,		0,
	  0,			0				},/* 2 */

	{ ka730machcheck,	ka730memerr,	nullcpu,		
	  ka730memenable,	ka730tocons,	ka730conf,	
	  nullcpu, 		bbadaddr, 	ka730nexaddr, 
	  ka730umaddr, 		ka730udevaddr,	readtodr,
	  writetodr,		nullcpu,	NNEX730,
	  NEXSIZE,		UMEMSIZE730,	0,
	  nexty730,		0,		0,
	  0,			0				},/* 3 */

	{ ka8600machcheck,	ka8600memerr,	ka8600setcache,	
	  ka8600memenable,	ka8600tocons,	ka8600conf,		
	  ka8600cachenbl,	bbadaddr, 	ka8600nexaddr, 
	  ka8600umaddr, 	ka8600udevaddr,	readtodr,
	  writetodr,		ka8600logsbi,	NNEX8600,
	  NEXSIZE,		UMEMSIZE8600,	1,	
	  0,			NIOA8600,	ioaaddr8600,	
	  IOAMAPSIZ,		0				},/* 4 */

	{ ka8200machcheck,	ka8200memerr,	ka8200setcache,	
	  ka8200memenable,	ka8200tocons,	ka8200conf,		
	  ka8200cachenbl,	bbadaddr, 	ka8200nexaddr, 
	  ka8200umaddr, 	ka8200udevaddr,	ka8200readtodr,
	  ka8200writetodr,	nullcpu,	NNEX8200,
	  NEXSIZE,		UMEMSIZE8200,	1,	
	  0,			0,		0,
	  0,			0				},/* 5 */

	{ ka8800machcheck,	ka8800memerr,	ka8800setcache,	
	  ka8800memenable,	ka8800tocons,	ka8800conf,		
	  ka8800cachenbl,	ka8800badaddr,	ka8800nexaddr, 
	  ka8800umaddr, 	ka8800udevaddr,	ka8800readtodr,
	  ka8800writetodr,	nullcpu,	NNEX8800,
	  NEXSIZE,		UMEMSIZE8800,	1,
	  0,			0,		0,
	  0,			0				},/* 6 */

	{ ka610machcheck,	nullcpu,	ka610setcache,	
	  nullcpu,		ka610tocons,	ka610conf,		
	  ka610cachenbl,	bbadaddr, 	ka610nexaddr, 
	  ka610umaddr, 		ka610udevaddr,	readtodr,
	  writetodr,		nullcpu,	NNEXUVI,
	  NEXSIZE,		QMEMSIZEUVI,	0,
	  nextyUVI,		0,		0,
	  0,			0				},/* 7 */

	{ ka630machcheck,	nullcpu,	ka630setcache,	
	  nullcpu,		ka630tocons,	ka630conf,		
	  ka630cachenbl,	bbadaddr, 	ka630nexaddr, 
	  ka630umaddr, 		ka630udevaddr,	ka630readtodr,
	  ka630writetodr,	nullcpu,	NNEXUVI,
	  QNEXSIZE,		QMEMSIZEUVI,	0,			
	  nextyUVI,		0,		0,
	  0,			0				},/* 8 */
};

nocpu()
{

	return (-1);

}

/*
 * null routine to pass back a success since this cpu type
 * doesn't need one of these routines.
 */

nullcpu()
{

	return(0);

}
