/* @(#)autoconf_data.c	1.14	 (ULTRIX)	8/12/86 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *
 *			Modification History
 *
 * 12-Aug-86  -- prs	Removed isGENERIC option.
 *
 * 3-Aug-86   -- jaw 	allocate a uba struct for klesib
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 14-Apr-86 -- jaw
 *	remove MAXNUBA referances.....use NUBA only!
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 *	Stephen Reilly, 22-Mar-85
 *	Added new structures for the floating emulation stuff
 *
 ***********************************************************************/

#include "mba.h"
#include "uba.h"
#include "kdb.h"
#include "klesib.h"

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/dk.h"
#include "../h/vm.h"
#include "../h/conf.h"
#include "../h/dmap.h"
#include "../h/reboot.h"

#include "../vax/cpu.h"
#include "../vax/mem.h"
#include "../vax/mtpr.h"
#include "../vax/ioa.h"
#include "../vax/nexus.h"
#include "../vax/scb.h"
#include "../vaxmba/mbareg.h"
#include "../vaxmba/mbavar.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../h/cpuconf.h"
#ifdef	BINARY

extern	int	(*mbaintv[])();
extern	int	(*ubaintv[])();
extern	int	nNMBA;
extern	(*Mba_find)();
extern	int	nNUBA;

#else
/*
 * Addresses of the (locore) routines which bootstrap us from
 * hardware traps to C code.  Filled into the system control block
 * as necessary.
 */
#if NMBA > 0 
int	(*mbaintv[4])() =	{ Xmba0int, Xmba1int, Xmba2int, Xmba3int };
#endif NMBA 
#ifdef VAX8600
int	(*ubaintv[7])() =	{ Xua0int, Xua1int, Xua2int, Xua3int, Xua4int, Xua5int, Xua6int }; 
#else
#ifdef VAX780
int	(*ubaintv[4])() =	{ Xua0int, Xua1int, Xua2int, Xua3int };
#else VAX780
int	(*ubaintv[4])() = 	{ (int (*)()) -1, ( int(*)()) -1, ( int(*)()) -1, ( int(*)()) -1};
#endif VAX780
#endif VAX8600

/*
 * This allocates the space for the per-uba information,
 * such as buffered data path usage.
 */
struct	uba_hd uba_hd[NUBA+NKDB+NKLESIB];
#if NUBA > 0
int	tty_ubinfo[NUBA];
#else
int	tty_ubinfo[1];
#endif

int	nNMBA = NMBA;
#if NMBA > 0
extern	mbafind();
int	(*Mba_find)() = mbafind;
#else NMBA
int	(*Mba_find)() = (int (*)()) -1;
mbintr()	{/* Keep locore happy */ }
#endif NMBA

int	nNUBA = NUBA;

#ifdef	EMULFLT

asm(".globl	_vaxopcdec");
asm("_vaxopcdec:	.long	vax$opcdec");
asm(".globl	_vaxspecialhalt");
asm("_vaxspecialhalt: .long	vax$special_halt");
asm(".globl	_vaxspecialcont");
asm("_vaxspecialcont: .long	vax$special_cont");
asm(".globl	_vaxemulbegin");
asm("_vaxemulbegin:	.long	vax$emul_begin");
asm(".globl	_vaxemulend");
asm("_vaxemulend:	.long	vax$emul_end");
asm(".globl	_exeacviolat");
asm("_exeacviolat:	.long	exe$acviolat");

#else	EMULFLT

int (*vaxopcdec)() = 0;

#if MVAX
asm(".globl	_vaxspecialhalt");
asm("_vaxspecialhalt: .long	vax$special_halt");
asm(".globl	_vaxspecialcont");
asm("_vaxspecialcont: .long	vax$special_cont");
asm(".globl	_vaxemulbegin");
asm("_vaxemulbegin:	.long	vax$emul_begin");
asm(".globl	_vaxemulend");
asm("_vaxemulend:	.long	vax$emul_end");
asm(".globl	_exeacviolat");
asm("_exeacviolat:	.long	exe$acviolat");

#else MVAX

int (*vaxspecialhalt)() = 0;
int (*vaxspecialcont)() = 0;
int (*vaxemulbegin)() = 0;
int (*vaxemulend)() = 0;
int (*exeacviolat)() = 0;
#endif MVAX

#endif EMULFLT

#endif BINARY
