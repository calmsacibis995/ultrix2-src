
#ifndef lint
static char *sccsid = "@(#)bda.c	1.13	ULTRIX	10/3/86";
#endif lint

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

/* ------------------------------------------------------------------------
 * Modification History:
 *
 * 10-Jul-86   -- jaw	added adpt/nexus to ioctl
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 13-Mar-86 -- darrell
 *	Passing cpup into bdainit -- for consistency sake.
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 03-Feb-86 -- jaw  changes for new SCB vectoring.
 *
 * 11-Nov-85 -- jaw
 * 	physical address of BDA map registers incorrect.
 *
 * 11-Sep-85 -- jaw 
 *	cleaned up includes.
 *
 * 26-jul-85 -- jaw
 *	add code to allocate scb vectors.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 *
 * ------------------------------------------------------------------------
 */

#include "../h/types.h"
#include "../h/buf.h"
#include "../h/param.h"
#include "../h/vmmac.h"
#include "../h/kmalloc.h"

#include "../vax/scb.h"
#include "../vax/pte.h"
#include "../vaxuba/ubavar.h"

#include "../vaxbi/bireg.h"
#include "../vaxbi/bdareg.h"

extern int numuba;
extern int nbicpus;
extern int nbitypes;
extern struct bidata bidata[];

/*
 *
 * This routine set up the bda and then looks for its drivers.
 * First the interrupt vectors are set then the device is initialized
 *
 * Next the drivers for attached if they can be found
 *
 */
#define PHYS(addr)	((long) \
		((long)ptob(Sysmap[btop((long)(addr)&0x7fffffff)].pg_pfnum)  \
			| (long)( PGOFSET & (long)(addr) ) ) )

extern struct uba_driver uqdriver;
extern struct uba_ctlr *bifindum(); 


bdainit(nxv,nxp,cpup,binumber,binode)
struct bda_regs *nxv;
caddr_t	*nxp;
struct cpusw *cpup;
register int binumber;
register int binode;
{
	register char *puba;
	register char *vuba;
	register int savenumuba;
	register struct uba_ctlr *um;
	struct uba_driver *udp;
	int (**biprobe)();

	udp = &uqdriver;
	biprobe = &udp->ud_probe;
	if ((um = bifindum(binumber,binode,biprobe)) == 0)
		if ((um = bifindum(binumber,'?',biprobe))==0) 
			if ((um = bifindum('?','?',biprobe))==0) {
				binotconf(nxv,nxp,cpup,binumber,binode);
				return;
			}		

	um->um_addr = (caddr_t) 0xf2;
	savenumuba = numuba;
	numuba = um->um_ubanum;

	vuba = (char *) kmemall(2048,KM_CONTG);
	if (vuba == 0) {
		numuba = savenumuba;
		printf("kmemalloc returned 0 size \n");
		return;
	}

	puba = (char *) PHYS(vuba);
	puba = (char *) (((((long)puba)) & 0x7ffffe00)- 2048);
	vuba = (char *) ((((((long)vuba)) & 
				0x7ffffe00)- 2048) | 0x80000000);

	uba_hd[numuba].uh_vec = SCB_BI_ADDR(binumber);
	uba_hd[numuba].uh_lastiv= SCB_BI_LWOFFSET(binode,LEVEL15)+4;
	nxv->bda_biic.biic_int_ctrl = SCB_BI_LWOFFSET(binode,LEVEL15);

	uba_hd[numuba].uba_type = UBABDA;
	unifind(vuba,puba, nxv, nxp, 0, nxp, nxv, 0,binumber,binode);

	numuba=savenumuba;
	if (um->um_alive == 0) bi_init_failed(nxv,nxp,cpup,binumber,binode);

}
