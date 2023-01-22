
/*
 *	@(#)tmscp_data.c	1.12	(ULTRIX)	4/16/86
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
 * tmscp_data.c
 *
 * Modification history:
 *
 * TMSCP data file
 *
 * 18-Jul-85 - afd
 *
 *	Moved per unit fields out of softc & into tms_info, and added to
 *	tms_info: resid, endcode, flags, & clserex for status command.
 *
 * 18-Oct-85 - afd
 *
 *	Added the field "tms_fmtmenu" to the "tms_info" struct.  This
 *	holds the unit's format (density) menu.
 *
 *  5-Feb-85 - jaw
 *
 *	Removed biic.h.
 *
 *  8-Feb-86 - ricky palmer
 *
 *	Added "dis_eot_tms[]" array for eot code and merged lastiow into
 *	tms_info:tms_flags field. V2.0
 *
 * 19-Mar-86 - ricky palmer
 *
 *	Added "devio.h" to include list. V2.0
 *	Added "softcnt", "hardcnt", and "category_flags" to tms_info
 *	structure. V2.0
 *
 * 10-Feb-87 - pmk
 *	Added include tmscp.h and if NTMS 0 init structures to 1.
 */

#include "tms.h"
#include "tmscp.h"

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
#include "../h/ioctl.h"
#include "../h/mtio.h"
#include "../h/cmap.h"
#include "../h/uio.h"
#include "../h/dkio.h"
#include "../h/devio.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"

#define TENSEC	(1000)

#define NRSPL2	3			/* Log2 number of resp. pckts.	*/
#define NCMDL2	3			/* Log2 number of command packs.*/
#define NRSP	(1<<NRSPL2)
#define NCMD	(1<<NCMDL2)

#include "../vaxuba/tmscpreg.h"
#include "../vax/tmscp.h"

/* TMSCP protocol storage area */
struct tmscp {
	struct tmscpca	tmscp_ca;	/* Communications area		*/
	struct mscp	tmscp_rsp[NRSP];	/* Response packets	*/
	struct mscp	tmscp_cmd[NCMD];	/* Command packets	*/
};

#ifdef	BINARY

extern struct uba_ctlr *tmscpminfo[];
extern struct uba_device *tmsdinfo[];
extern struct uba_device *tmscpip[][1];
extern struct buf rtmsbuf[];		/* Raw i/o buffer		*/
extern struct buf ctmscpbuf[];		/* Internal cmd ioctl buffer	*/
extern struct buf tmsutab[];		/* Drive queue			*/
extern struct buf tmscpwtab[];		/* I/O wait queue, per ctlr.	*/
extern struct tmscp_softc tmscp_softc[];/* Software state, per ctlr.	*/
extern struct tmscp tmscp[];		/* TMSCP protocol storage area	*/
extern struct tms_info tms_info[];	/* Software state, per device	*/
extern int    tmscpmicro[];		/* To store microcode level	*/
extern short  utoctlr[];		/* Filled in by slave routine	*/

extern int nNTMS;			/* Number of tape devices	*/
extern int nNTMSCP;			/* Number of controllers	*/
extern char dis_eot_tms[];		/* Disable eot code array	*/

#else not BINARY

struct uba_ctlr *tmscpminfo[NTMSCP];
struct uba_device *tmscpip[NTMSCP][1];
struct buf ctmscpbuf[NTMSCP];		/* Internal cmd ioctl buffer	*/
struct buf tmscpwtab[NTMSCP];		/* I/O wait queue, per ctlr.	*/
struct tmscp_softc tmscp_softc[NTMSCP]; /* Software state per ctlr.	*/
struct tmscp tmscp[NTMSCP];		/* TMSCP protocol storage area	*/
int    tmscpmicro[NTMSCP];		/* To store microcode level	*/

#if NTMS > 0
struct uba_device *tmsdinfo[NTMS];
struct buf rtmsbuf[NTMS];		/* Raw i/o buffer		*/
struct buf tmsutab[NTMS];		/* Drive queue			*/
struct tms_info tms_info[NTMS]; 	/* Software state, per device	*/
short  utoctlr[NTMS];			/* Filled in by slave routine	*/
char   dis_eot_tms[NTMS];		/* Disable eot code array	*/
#else
struct uba_device *tmsdinfo[1];
struct buf rtmsbuf[1];			/* Raw i/o buffer		*/
struct buf tmsutab[1];			/* Drive queue			*/
struct tms_info tms_info[1]; 		/* Software state, per device	*/
short  utoctlr[1];			/* Filled in by slave routine	*/
char   dis_eot_tms[1];			/* Disable eot code array	*/
#endif

int nNTMS = NTMS;			/* Number of tape devices	*/
int nNTMSCP = NTMSCP;			/* Number of controllers	*/

#endif BINARY
