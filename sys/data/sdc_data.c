/*
 * @(#)sdc_data.c	1.8	(ULTRIX)	1/9/87
 */
/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *   06-Jan-87 gmm (George Mathew)
 *	Included dkio.h and bbr.h to support ioctls for radisk
 *
 *   29-Sep-86 gmm (George Mathew)
 *	Changed sizes of structure/array definitions from NSD to NDRIVES
 *
 *  29-Sep-86  darrell (Darrell Dunnuck)
 *	Removed the definitions for vsbuf.  The space for vsbuf
 *	is now allocated in ../vaxuba/uba.c
 *
 *   3-Sep-86  gmm (George Mathew)
 *	Include file.h.
 *
 *  27-Aug-86  gmm (George Mathew)
 *	Corrected size of b partition on RD31 disk.
 *
 *  14-Aug-86  fred (Fred Canter)
 *	Changes slave names from sd to rd/rx.
 *
 *   5-Aug-86  gmm (George Mathew)
 *	Changes to allow sharing data buffer with TZK50 driver.
 *
 *   2-Jul-86  gmm (George Mathew)
 *	Added partial devioctl support and many driver improvements.
 *
 *  18-Jun-86  gmm (George Mathew)
 *	Created this data file for the VAXstar RD/RX disk driver.
 *
 **********************************************************************/

#include "rd.h"
#include "rx.h"
#if NRX > 0
#define	NSX	1
#else
#define	NSX	0
#endif
#define	NSD	NRD+NSX


#include "../vax/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/dk.h"
#include "../h/cmap.h"
#include "../h/dkbad.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/ioctl.h"
#include "../ufs/fs.h"
#include "../h/proc.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/devio.h"
#include "../h/file.h"
#include "../h/dkio.h"
#include "../h/bbr.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxuba/sdcreg.h"

#ifdef BINARY

extern	struct	uba_ctlr *sdminfo[];
extern	struct	uba_device *sddinfo[];

extern  struct	buf sdutab[];		/* Drive queue */
extern	struct	buf rsdbuf[];
extern	struct	pt sd_part[];		/* 001 Partition tbl for each pack */
extern	int	nNSDC;
extern	int	nNSD;
extern	struct  sdspace {
	char sd_pad[16384];
};

extern	struct size {
	daddr_t	nblocks;
	int	blkoffs;
} sd_rx50_sizes[],sd_rx33_sizes[],sd_rd31_sizes[],sd_rd32_sizes[],
  sd_rd53_sizes[],sd_rd54_sizes[];

extern struct sd_uib sd_uib[];

#else

struct	uba_ctlr *sdminfo[NSDC];
struct	uba_device *sddinfo[NDRIVES];

struct	buf sdutab[NDRIVES];
struct	buf rsdbuf[NDRIVES];
struct 	pt  sd_part[NDRIVES];		/*001 Partition tbl for each pack */
struct  sdspace {
	char sd_pad[16384];
};
struct sd_uib sd_uib[NDRIVES];

int	nNSDC = NSDC;
int	nNSD = NDRIVES;
/* THIS SHOULD BE READ OFF THE PACK, PER DRIVE */
struct size {
	daddr_t	nblocks;
	int	blkoffs;
} sd_rx50_sizes[8] ={
	-1,	0,		/* 0 thru 800 blocks */
	0,	0,		
	-1,	0,		
	0,	0,
	0,	0,
	0,	0,
	0,	0,		
	0,	0,
}, sd_rx33_sizes[8] ={
	-1,	0,		/* 0 thru 2400 blocks */
	0,	0,		
	-1,	0,		
	0,	0,
	0,	0,
	0,	0,
	0,	0,		
	0,	0,
}, sd_rd31_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	10024,	15884,		/* B=blk 15884 thru 25908 */
	-1,	0,		/* C=blk 0 thru 41560 */
	0,	0,
	0,	0,
	0,	0,
	-1,	25908,		/* G=blk 25908 thru end */
	0,	0,		
}, sd_rd32_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	15625,	15884,		/* B=blk 15884 thru 31508 */
	-1,	0,		/* C=blk 0 thru 83236 */
	25863,	31509,		/* D=blk 31509 thru 57371 */
	-1,	57372,		/* E=blk 57372 thru end */
	0,	0,
	-1,	31509,		/* G=blk 31509 thru end */
	0,	0,
}, sd_rd53_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	50714,	0,		/* E=blk 0 thru 50714 */
	-1,	50714,		/* F=blk 50714 thru end */
	-1,	49324,		/* G=blk 49324 thru end */
	-1,	15884,		/* H=blk 15884 thru end */
}, sd_rd54_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end 311200 */
	130938,	49324,		/* D=blk 49324 thru 180261 */
	-1,	180262,		/* E=blk 180262 thru end */
	0,	0,
	-1,	49324,		/* G=blk 49324 thru end */
	0,	0,
};
/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */

#endif

struct sd_st sd_st;
char	id_table[18][4];
