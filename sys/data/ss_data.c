
/*
 *	@(#)ss_data.c	1.6	(ULTRIX)	1/29/87
 */

/************************************************************************
 *									*
 *			Copyright (c) 1986, 1987 by			*
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
 * ss_data.c
 *
 * Modification history:
 *
 * 29-Jan-87 - Tim Burke
 *
 *	Added definition of ssdsr, a variable used to define the type of 
 *	modem control that is being followed.
 *
 *  30-Aug-86  -- fred (Fred Canter)
 *	Include mtpr.h (part of VAXstar crash dump fix).
 *
 *  27-Aug-86  -- fred (Fred Canter)
 *	Removed unnecessary comments.
 *
 *   5-Aug-86  -- fred (Fred Canter)
 *	Minor change for passing characters to bitmap driver (sm.c).
 *
 *  24-Jul-86  -- tim  (Tim Burke)
 *	Added ssmodem and sstimestamp which are used in the tracking
 *	of modem signals for DEC Standard 52.
 *
 *   2-Jul-86  -- fred (Fred Canter)
 *	Cleaned out DZ32 code and converted from 8 lines per
 *	unit to 4 lines per unit.
 *
 *  18-Jun-86  -- fred (Fred Canter)
 *	Created this data file for VAXstar serial line unit driver.
 *	Derived from dz_data.c.
 *
 */

#include	"ss.h"

#include "../machine/pte.h"

#include "bk.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/conf.h"
#include "../h/bk.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/devio.h"
#include "../h/cpuconf.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxuba/pdma.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ssreg.h"
#include "../vaxuba/smreg.h"

#ifdef BINARY

extern	struct	uba_device *ssinfo[];
extern	struct	ss_softc ss_softc[];
extern	struct	tty ss_tty[];
extern	int	ss_cnt;

/*
* Software copy of ssbrk since it isn't readable
*/
extern	char	ss_brk[];
extern	char	sssoftCAR[];
extern	char	ssdefaultCAR[];

/*
* Pdma structures for fast output code
*/
extern	struct	pdma sspdma[];

extern	int	sschars[];		/* recent input count */
extern	int	ssrate[];		/* smoothed input count */

extern	int	nNSS;
extern	int	nNSSLINE;
extern u_char	ssmodem;
extern struct timeval sstimestamp;
extern	int	ssdsr;

#else BINARY

#define NSSLINE 	(4*NSS)
struct	uba_device *ssinfo[NSS];
struct	ss_softc ss_softc[NSS];
struct	tty ss_tty[NSSLINE];
int	ss_cnt = { NSSLINE };

/*
 * Pdma structures for fast output code
 */
struct	pdma sspdma[NSSLINE];


/*
 * Software copy of ssbrk since it isn't readable
 */
char	ss_brk[NSS];
char	sssoftCAR[NSS];
char	ssdefaultCAR[NSS];
int	ssrate[NSS];
int	sschars[NSS];

int	nNSS = NSS;
int	nNSSLINE = NSSLINE;
u_char	ssmodem;		/* keeps track of modem state */
struct timeval sstimestamp;	/* Record length of carrier drop */

#ifdef NODSR
int ssdsr = 0;				/*"0"=Ignore DSR*/
#else NODSR
int ssdsr = 1;				/*"1"=follow DECSTD52 */
#endif NODSR

#endif BINARY

