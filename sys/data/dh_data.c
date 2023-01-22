/*
 * @(#)dh_data.c	1.6	(ULTRIX)	4/14/86
 */
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

/*
 * 14-Apr-86 -- jaw
 *	remove MAXNUBA referances.....use NUBA only!
 */
#include	"dh.h"
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

#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/dhreg.h"
#include "../vaxuba/dmreg.h"

#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"

#ifdef BINARY

extern	struct	uba_device *dhinfo[];
extern	short	dhsar[];			/* software copy of last bar */
extern	short	dhsoftCAR[];
extern	short	dhdefaultCAR[];
extern	int	dhchars[];			/* recent input count */
extern	int	dhrate[];			/* smoothed input count */

extern	struct	tty dh11[];
extern	int	ndh11;
extern	struct	uba_device *dminfo[];

extern	int	nNDH;
extern  int 	cbase[]; 

#else
int	cbase[NUBA];			/* base address in unibus map */

struct	uba_device *dhinfo[NDH];
struct	uba_device *dminfo[NDH];
short	dhsar[NDH];			/* software copy of last bar */
short	dhsoftCAR[NDH];
short	dhdefaultCAR[NDH];
int	dhrate[NDH];
int	dhchars[NDH];

struct	tty dh11[NDH*16];
int	ndh11	= NDH*16;
int	nNDH	= NDH;
#endif
