/*	@(#)uda_data.c	1.38	(ULTRIX)	3/27/87	*/

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 * uda_data.c
 *
 * Modification history
 *
 * MSCP/RA??/RC25/RD??/RRD??/RX?? data file
 *
 * 14-Jan-86	rr	Increased number of command packets
 *
 * 14-Jan-86	Robin   Added rrd50 and rx35
 *
 * 7-Jul-86   -- jaw 	fix so UDA with no drives attached builds.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 22-Oct-84 - Stephen Reilly
 *
 *	Added the pt structure to handle different partitions per pack. -001
 *
 * 13-Mar-85 - jaw
 *
 *	Add support for 8200 and bua.
 *
 *  4-Apr-85 - Stephen Reilly
 *
 *	Added support to handle 32 logical devices and the maximum slave
 *	number for mscp devices. -002
 *
 * 19-Jun-85 - jaw
 *
 *	VAX8200 name change.
 *
 * 20-Jun-85 - Stephen Reilly
 *
 *	The partition table, based on media type, is now determine through
 *	and intialized array instead of a switch statement.
 *
 * 18-Nov-85 - Larry Palmer
 *
 *	Added ud_burst so that a site with uda's on reasonably empty
 *	unibuses can get hog mode of uda50. This is a kludge way of
 *	setting it and perhaps it should be done as a flag on the
 *	controller line in its conf file.
 *
 *  4-Feb-86 - jaw
 *
 *	Got rid of biic.h.
 *
 *  1-Apr-86 - robin
 *
 *	Added rd54 data.
 *
 * 16-Apr-86 - Ricky Palmer
 *
 *	Added include of devio.h. V2.0
 *
 *  13-May-86 - Robin
 *
 *	added ra82 data.
 *
 *  10-Jun-86 - map
 *	
 *	Added definition of rctsearch structure for BBR.
 *	Added include of bbr.h
 *
 *  13-june-86 - Robin
 *
 *	added ra90 data.
 *
 *  8-Aug-86 - Robin
 *
 *	Fixed the ra60 partitions as they went beyond the end of the disk.
 *
 *  15-Aug-86 - prs
 *
 *	Added the declaration for udad[NUQ]. The dump routine in uda.c depends
 *	on this structure being sized properly.
 *
 */

#include "ra.h"
#include "uq.h"

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/dk.h"
#include "../h/cmap.h"
#include "../h/uio.h"
#include "../h/ioctl.h"
#include "../ufs/fs.h"
#include "../h/dkio.h"
#include "../h/devio.h"

#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vax/mtpr.h"
#include "../vaxbi/buareg.h"

#define TENSEC	(1000)

#define NRSPL2	3		/* log2 number of response packets */
#define NCMDL2	4		/* log2 number of command packets */
#define NRSP	(1<<NRSPL2)
#define NCMD	(1<<NCMDL2)

#define NDPUQ	4		/* 002 Number of Drives per uqssp controller */

#include "../vaxuba/udareg.h"
#include "../vax/mscp.h"
#include "../h/bbr.h"

/* MSCP protocol storage area */
struct uda {
	struct udaca	uda_ca; 	/* Communications area		*/
	struct mscp	uda_rsp[NRSP];	/* Response packets		*/
	struct mscp	uda_cmd[NCMD];	/* Command packets		*/
};

#ifdef	BINARY

extern struct uda_softc uda_softc[];
extern struct uda uda[];

/* THIS SHOULD BE READ OFF THE PACK, PER DRIVE */
extern struct size {
	daddr_t nblocks;
	daddr_t blkoffs;
}  rd51_sizes[], rd52_sizes[], rd53_sizes[], rd54_sizes[], rc25_sizes[], 
   rx50_sizes[], rx33_sizes[], ra60_sizes[], ra70_sizes[], ra80_sizes[], 
   ra81_sizes[], ra82_sizes[], ra90_sizes[], rd31_sizes[], rd32_sizes[],
   rrd50_sizes[],rx35_sizes[];
/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */

extern struct  uba_ctlr *udminfo[];
extern	struct	uba_device *uddinfo[];
extern	struct	buf rudbuf[];
extern	struct	buf udutab[];		/* Drive queue */
extern struct  buf udwtab[];		   /* I/O wait queue, per controller */
extern int     udamicro[];	   /* to store microcode level */
extern struct ra_info ra_info[];
extern struct	rct_search rctsearch[];
extern struct pt ra_part[];	/* 001 Partition per pack */
/*
 *			002
 *	A translation of slave number to uba_device structure
 */
extern struct uda_device {
	struct uda_drive {
		short	uda_slave;
		struct uba_device *uda_ui;
	} ra[NDPUQ];
} udip[];
extern struct rapt radfpt[];

extern	int	nNRA;
extern int     nNUQ;

extern short ud_burst[];

extern struct uda udad[];

#else BINARY

struct uda_softc uda_softc[NUQ];
struct uda uda[NUQ];

/* THIS SHOULD BE READ OFF THE PACK, PER DRIVE */
struct size {
	daddr_t nblocks;
	daddr_t blkoffs;
} rd31_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	10024,	15884,		/* B=blk 15884 thru 25908 */
	-1,	0,		/* C=blk 0 thru 41560 */
	0,	0,		
	0,	0,	
	0,	0,
	-1,	25908,		/* G=blk 25908 thru end */
	0,	0
}, rd32_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	15625,	15884,		/* B=blk 15884 thru 31508 */
	-1,	0,		/* C=blk 0 thru 83236 */
	25863,	31509,		/* D=blk 31509 thru 57371 */
	-1,	57372,		/* E=blk 57372 thru end */
	0,	0,
	-1,	31509,		/* G=blk 31509 thru end */
	0,	0
}, rd51_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	5716,	15884,		/* B=blk 15884 thru 21599 */
	-1,	0,		/* C=blk 0 thru 21599 */
	0,	0,		
	0,	0,	
	0,	0,
	0,	0,		
	0,	0	
}, rd52_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	9766,	15884,		/* B=blk 15884 thru 25649 */
	-1,	0,		/* C=blk 0 thru end 60480 */
	0,	0,
	50714,	0,		/* E=blk 0 thru 50714 */
	-1,	50714,		/* F=blk 50714 thru end */
	-1,	25650,		/* G=blk 25650 thru end */
	-1,	15884		/* H=blk 15884 thru end */
}, rd53_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end 60480 */
	0,	0,		
	50714,	0,		/* E=blk 0 thru 50714 */
	-1,	50714,		/* F=blk 50714 thru end */
	-1,	49324,		/* G=blk 49324 thru end */
	-1,	15884		/* H=blk 15884 thru end */
}, rd54_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end 311200 */
	130938,	49324,		/* D=blk 49324 thru 180261 */
	-1,	180262,		/* E=blk 180262 thru end   */
	0,	0,
	-1,	49324,		/* G=blk 49324 thru end */
	0,	0
}, rc25_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	10032,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	0,	0,
	0,	0,
	-1,	25916,		/* G=blk 49324 thru 131403 */
	0,	0
}, rx33_sizes[8] = {
	-1,	0,		/* 0 thru 800 or 2400 handles both rx50 & rx33*/
	0,	0,
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0
}, rx35_sizes[8] = {
	-1,	0,		/* 0 end	*/
	0,	0,
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0
}, rx50_sizes[8] = {
	-1,	0,		/* 0 thru 800 */
	0,	0,
	-1,	0,		/* C=blk 0 thru end */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0
}, rrd50_sizes[8] ={		/* values aren't correct, can't write it anyway*/
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end */
	122993,	131404, 	/* D=blk 131404 thru 254396 */
	122993,	254397, 	/* E=blk 254397 thru 377389 */
	-1,	377390, 	/* F=blk 377390 thru end */
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404		/* H=blk 131404 thru end */
}, ra60_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end (400175) */
	89591,	131404, 	/* D=blk 131404 thru 220994 */
	89590,	220995, 	/* E=blk 220995 thru 310584 */
	-1,	310585, 	/* F=blk 310585 thru end */
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404		/* H=blk 131404 thru end */
}, ra70_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end (547041) */
	138545,	131404,	 	/* D=blk 131404 thru 269948 */
	138545,	269949,	 	/* E=blk 269949 thru 408493 */
	-1,	408494,		/* F=blk 408494 thru end */
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404		/* H=blk 131404 thru end */
}, ra80_sizes[8] = {
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end (237212)*/
	35260,	131404, 	/* D=blk 131404 thru 166663 */
	35260,	166664, 	/* E=blk 166664 thru 201923 */
	-1,	201924, 	/* F=blk 201924 thru end */
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404		/* H=blk 131404 thru end */
}, ra81_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	33440,	15884,		/* B=blk 15884 thru 49323 */
	-1,	0,		/* C=blk 0 thru end (500383)*/
	122993,	131404, 	/* D=blk 131404 thru 254396 */
	122993,	254397, 	/* E=blk 254397 thru 377389 */
	-1,	377390, 	/* F=blk 377390 thru end */
	82080,	49324,		/* G=blk 49324 thru 131403 */
	-1,	131404		/* H=blk 131404 thru end */
}, ra82_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	66690,	15884,		/* B=blk 15884 thru 82573 */
	-1,	0,		/* C=blk 0 thru end */
	220096,	82574,	 	/* D=blk 82574 thru 302669 */
	219735,	302670,	 	/* E=blk 302670 thru 522404 */
	437760,	522405, 	/* F=blk 522405 thru 960164 */
	877591,	82574,		/* G=blk 82574 thru 960164 */
	-1,	960165		/* H=blk 960165 thru end */
}, ra90_sizes[8] ={
	15884,	0,		/* A=blk 0 thru 15883 */
	127072,	15884,		/* B=blk 15884 thru 142955 */
	-1,	0,		/* C=blk 0 thru end */
	420197,	142956,	 	/* D=blk 142956 thru 563152 */
	420197,	563153,	 	/* E=blk 563153 thru 983349 */
	840393,	983350, 	/* F=blk 983350 thru 1823742 */
	1680787,142956,		/* G=blk 142956 thru 1823742 */
	-1,	1823743		/* H=blk 1823743 thru end */
};
/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */

struct	uba_ctlr *udminfo[NUQ];
struct	buf udwtab[NUQ];		/* I/O wait queue, per controller */

#if NRA > 0
struct	uba_device *uddinfo[NRA];
struct	buf rudbuf[NRA];
struct	buf udutab[NRA];
struct	ra_info  ra_info[NRA];
struct	rct_search rctsearch[NRA];
struct	pt  ra_part[NRA];		/* 001 Partition per pack */
#else
struct	uba_device *uddinfo[1];
struct	buf rudbuf[1];
struct	buf udutab[1];
struct	ra_info  ra_info[1];
struct	rct_search rctsearch[1];
struct	pt  ra_part[1];		/* 001 Partition per pack */
#endif

/*
 *			002
 *	A translation of slave number to uba_device structure
 */
struct uda_device {
	struct uda_drive {
		short	uda_slave;
		struct uba_device *uda_ui;
	} ra[NDPUQ];
} udip[NUQ];

struct rapt radfpt[] = {
	0x20643019, rc25_sizes,		/* RC25	(fixed)disk drive	*/
	0x20643319, rc25_sizes,		/* RC25	(removable)disk drive	*/
	0x25658021, rx33_sizes,		/* RC33	disk drive		*/
	0x6d658023, rx35_sizes,		/* RX35	disk drive		*/
	0x25658032, rx50_sizes,		/* RX50	disk drive		*/
	0x25652232, rrd50_sizes,	/* RRD50 disk drive		*/
	0x22a4103c, ra60_sizes,		/* RA60	disk drive		*/
	0x25641046, ra70_sizes,		/* RA70 disk drive		*/
	0x25641050, ra80_sizes,		/* RA80	disk drive		*/
	0x25641051, ra81_sizes,		/* RA81 disk drive		*/
	0x25641052, ra82_sizes,		/* RA82	disk drive		*/
	0x2564105a, ra90_sizes,		/* RA90	disk drive		*/
	0x2564401f, rd31_sizes,		/* RD31	disk drive		*/
	0x25644020, rd32_sizes,		/* RD32	disk drive		*/
	0x25644033, rd51_sizes,		/* RD51	disk drive		*/
	0x25644034, rd52_sizes,		/* RD52 disk drive		*/
	0x25644035, rd53_sizes,		/* RD53 disk drive		*/
	0x25644036, rd54_sizes,		/* RD54 disk drive		*/
	0, 0
};

int	nNRA = NRA;
int	nNUQ = NUQ;
int	udamicro[NUQ]; 	/* to store microcode level */

/* Uda burst rates per uda50 ctrl */
short ud_burst[] = { 3, 3, 3, 3, 3, 3, 3, 3 };

struct uda udad[NUQ];

#endif BINARY
