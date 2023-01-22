
/*
 *	@(#)lta_data.c	1.3	(ULTRIX)	5/21/86
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * lta_data.c
 */

#include "lta.h"
/*
 * LAT terminal driver data structures (service class 1)
 */

#include "bk.h"
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/devio.h"

/* Lta driver and data specific structure */
struct	lta_softc {
	long	sc_flags;		 /* Flags (one per line)	 */
	long	sc_category_flags;	 /* Category flags (one per line)*/
	u_long	sc_softcnt;		 /* Soft error count total	 */
	u_long	sc_hardcnt;		 /* Hard error count total	 */
	char	sc_device[DEV_SIZE];	 /* Device type string		 */
};

#ifdef BINARY
extern struct tty lata[];
extern struct lta_softc lta_softc[];
extern int nLAT1;

#else BINARY

#if NLTA == 1
#undef NLTA
#define NLTA 16
#endif

struct tty lata[NLTA];
struct lta_softc lta_softc[NLTA];

int nLAT1 = NLTA;

#endif BINARY


