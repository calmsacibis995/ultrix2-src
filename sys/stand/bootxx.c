/************************************************************************
 *									*
 *			Copyright (c) 1983, 1986 by			*
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

# ifndef lint
static char *sccsid = "@(#)bootxx.c	1.7	(ULTRIX)	3/23/86";
# endif not lint

/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/bootxx.c
 *
 * 15-Jul-85 --map
 *	fix defintion of saveCSR - make it external
 *
 * 26-Jun-85 --map
 *	change definition of saveCSR to fix loading problem
 *
 * 04-Apr-85 --map
 *	added saveCSR - used for saving boot device CSR
 *
 * 29-Nov-84 --tresvik
 *	changed call to _exit to a call to exit.  exit is defined locally
 *
 *  9 Jul 84 -- rjl
 *	The inclusion of a font table and the added code prohibits the usage 
 *	of the qvss as the system console during the secondary boot phase. 
 *	Therefore all printf's in the secondary boot stage have been eliminated
 *	during this phase.
 *
 * 29 Dec 83 --jmcg
 *	Added support for non-zero unit.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		bootxx.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/vm.h"
#include <a.out.h>
#include "saio.h"
#include "../h/reboot.h"

char bootprog[] = "xx(0,0)boot";
extern long saveCSR;

/*
 * Boot program... arguments passed in r9,r10 and r11
 * are passed through to the full boot program.
 */

main()
{
	register howto, devtype, bootcsr;	/* howto=r11, devtype=r10, bootcsr=r9 */
	int io;
	int	unit;

#ifdef lint
	howto = 0; devtype = 0; bootcsr = 0;
#endif
	saveCSR = bootcsr;			/* salt away boot device CSR for later use */
	unit = (devtype & 0xffff ) >> 8;
	if( unit < 10) 
		bootprog[3] = "0123456789"[unit];
#ifndef MVAX
	printf("loading %s", bootprog);
#endif MVAX
	io = open(bootprog, 0);
	if (io >= 0)
		copyunix(howto, devtype, bootcsr, io);
#ifndef MVAX
	printf("boot failed");
#endif MVAX
	exit();
}

/*ARGSUSED*/
copyunix(howto, devtype, bootcsr, io)
	register howto, devtype, bootcsr, io;	/* howto=r11, devtype=r10, bootcsr=r9  */
{
	struct exec x;
	register int i;
	char *addr;

	i = read(io, (char *)&x, sizeof x);
	if (i != sizeof x ||
	    (x.a_magic != 0407 && x.a_magic != 0413 && x.a_magic != 0410))
		_stop("Bad format\n");
	if ((x.a_magic == 0413 || x.a_magic == 0410) &&
	    lseek(io, 0x400, 0) == -1)
		goto shread;
	if (read(io, (char *)0, x.a_text) != x.a_text)
		goto shread;
	addr = (char *)x.a_text;
	if (x.a_magic == 0413 || x.a_magic == 0410)
		while ((int)addr & CLOFSET)
			*addr++ = 0;
	if (read(io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	x.a_bss += 128*512;	/* slop */
	for (i = 0; i < x.a_bss; i++)
		*addr++ = 0;
	x.a_entry &= 0x7fffffff;
	(*((int (*)()) x.a_entry))();
	exit();
shread:
	_stop("Short read\n");
}
