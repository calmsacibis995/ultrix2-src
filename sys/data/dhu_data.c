
/*
 *	@(#)dhu_data.c	1.7	(ULTRIX)	1/29/87
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986, 1987 by		*
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
 * dhu_data.c
 *
 * Modification history
 *
 * DHU11/DHV11 data file
 *
 * 16-Jan-86 - Larry Cohen
 *
 *	Add full DEC standard 52 support.
 *
 * 14-Apr-86 - jaw
 *
 *	Remove MAXNUBA referances.....use NUBA only!
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added "devio.h" to include list. V2.0
 *
 * 29-Jan-87 - Tim Burke
 *
 *	Added definition of dhudsr, a variable used to define the type of 
 *	modem control that is being followed.
 *
 */

#include	"dhu.h"
#include	"uba.h"

#include "../machine/pte.h"

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
#include "../h/kernel.h"
#include "../h/devio.h"

#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/dhureg.h"

#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"

#ifdef BINARY

extern	struct	uba_device *dhuinfo[];
extern	struct	dhu_softc dhu_softc[];
extern	short	dhusoftCAR[];
extern	short	dhudefaultCAR[];

extern	struct	tty dhu11[];
extern	u_char	 dhumodem[];
extern	struct timeval	dhutimestamp[];
extern	int	ndhu11;

extern	int	nNDHU;
extern	int	cbase[];
extern	int	dhudsr;

#else BINARY

int	cbase[NUBA];		/* base address in unibus map */
struct	uba_device *dhuinfo[NDHU];
struct	dhu_softc dhu_softc[NDHU];
short	dhusoftCAR[NDHU];
short	dhudefaultCAR[NDHU];

struct	tty dhu11[NDHU*16];   /* one tty structure per line */
u_char	 dhumodem[NDHU*16];   /* to keep track of modem state */
struct	timeval dhutimestamp[NDHU*16]; /* to keep track of CD transient drops */
int	ndhu11	= NDHU*16;   /* total number of dhu lines   */
int	nNDHU	= NDHU;     /* total number of dhu modules */

#ifdef NODSR
int dhudsr = 0;			/* A "0" here means ignore DSR */
#else NODSR
int dhudsr = 1;			/* A "1" here means drop line if DSR drops */
#endif NODSR

#endif BINARY
