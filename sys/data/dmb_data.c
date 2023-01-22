
/*
 *	@(#)dmb_data.c	1.3	(ULTRIX)	1/29/87
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
 * dmb_data.c
 *
 * Modification history
 *
 * DMB32 data file
 *
 * 29-Jan-87 - Tim Burke
 *
 *	Added definition of dmbdsr, a variable used to define the type of 
 *	modem control that is being followed.
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added "devio.h" to include list. V2.0
 *
 */

#include	"dmb.h"

#include "../machine/pte.h"

#include "bk.h"
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/devio.h"

#include "../vax/scb.h"
#include "../vax/mtpr.h"
#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxbi/dmbreg.h"

struct	uba_device *dmbinfo[NDMB];

#ifdef BINARY

extern	struct	uba_device *dmbinfo[];
extern	struct	dmb_softc dmb_softc[];

struct	tty dmb_tty[];
char	dmb_dma[];
char	dmbsoftCAR[];
char	dmbdefaultCAR[];
extern	u_char dmbmodem[];
extern	struct timeval dmbtimestamp[];
extern	u_short dmb_numchars[];

extern	int	nNDMB;
extern	int	ndmb;
extern	int	dmbdsr;

struct dmbl_softc dmbl_softc[];

#else BINARY

struct	dmb_softc dmb_softc[NDMB];
struct	tty dmb_tty[NDMB*8];
char	dmb_dma[NDMB*8];
char	dmbsoftCAR[NDMB];
char	dmbdefaultCAR[NDMB];
struct dmbl_softc dmbl_softc[NDMB];
u_char dmbmodem[NDMB*8];
struct timeval dmbtimestamp[NDMB*8];
u_short dmb_numchars[NDMB*8];

int	nNDMB = NDMB;
int	ndmb = NDMB*8;

#ifdef NODSR
int dmbdsr = 0;		/* A "0" here means ignore DSR */
#else NODSR
int dmbdsr = 1;		/* A "1" here means follow DS52 DSR signals */
#endif NODSR

#endif BINARY
