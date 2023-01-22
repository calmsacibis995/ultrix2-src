
/*
 *	@(#)its_data.c	1.1	(ULTRIX)	1/29/87
 */

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
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
 * its_data.c
 *
 * Modification history
 *
 * ITEX/FG101 Series 100 frame buffer data file
 *
 *  2-Dec-86 - lp/rsp (Larry Palmer/Ricky Palmer)
 *
 *	Created prototype frame buffer driver.
 */

/*
 *	This  driver  provides user level programs with the ability to
 *	read  and  write  any  of  the	frame  buffer registers.  This
 *	capability  is	provided  via  the  ITSMAP  ioctl  request and
 *	subsequent  user  direct  access  to  the  registers and frame
 *	memory.   The  driver  initializes  the  scanner hardware in a
 *	default  state.   The same initialization is possible by using
 *	the  registers	directly.  Note that DMA is NOT possible since
 *	the board does not support local DMA transfers to another QBUS
 *	device.   The  documentation  provided	by  Imaging Technology
 *	should	be  consulted  for further details involving the frame
 *	buffer	hardware/software.   This  driver  is  provided  as  a
 *	service  to  Digital  Ultrix  customers  who  would  like  the
 *	capability to combine the frame buffer hardware with their GPX
 *	system	in  order  to  create  digital	images from a standard
 *	television  camera hookup.  The driver is provided "as-is" and
 *	is  NOT  supported  under any Digital agreements or contracts.
 *	Electronic   mail   concerning	the  driver  may  be  sent  to
 *	decvax!rsp  or	decvax!lp  but	there  is  no guarantee of any
 *	response or support.
 *
 *	This driver assumes the following configuration:
 *
 *		FG101 at 173000
 *		Memory Map @ 0x30200000
 *
 *	The availble ioctl requests for this driver are:
 *
 *	(1) ITSMAP - maps the frame buffer into user space.
 *	(2) ITSGRABIT - starts the frame buffer grabbing frames (30/sec).
 *	(3) ITSFREEZE - freezes a frame.
 *	(4) ITSZOOM - initiates a zoom;subsequent calls cycle through.
 *
 */

#include "its.h"

#include "../machine/pte.h"
#include "../machine/mtpr.h"

#include "../h/types.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/conf.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/bk.h"
#include "../h/clist.h"
#include "../h/file.h"
#include "../h/uio.h"
#include "../h/proc.h"
#include "../h/kernel.h"
#include "../ufs/fs.h"
#include "../h/ipc.h"
#include "../h/shm.h"

#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/itsreg.h"
#include "../vaxuba/itsioctl.h"

#ifdef BINARY

extern	struct	uba_device *itsdinfo[];
extern	struct its_softc {
	char	it_open;
	short	it_state;
	u_short it_cx, it_cy;
} its_softc[];
extern	struct buf ritsbuf[];
extern	int	numNITS;

#else BINARY

struct	uba_device *itsdinfo[NITS];
struct its_softc {
	char	it_open;
	short	it_state;
	u_short it_cx, it_cy;
} its_softc[NITS];
struct buf ritsbuf[NITS];
int	numNITS = NITS;

#endif BINARY
