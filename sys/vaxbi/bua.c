#ifndef lint
static char *sccsid = "@(#)bua.c	1.16	ULTRIX	12/16/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985,1986 by			*
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

/* ------------------------------------------------------------------------
 * Modification History:
 *
 * 	10-Jul-86   -- jaw	added adpt/nexus to ioctl
 *
 * 	13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 	5-Jun-86   -- jaw 	changes to config.
 *
 *	09-Mar-86  darrell -- pointer to cpusw structure now passed in.
 *		No longer use a global reference.
 *
 * 	18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 	04-feb-86 -- jaw  get rid of biic.h.
 *
 *	03-Feb-86 -- jaw  changes to SCB vectoring...
 *
 *	11-Sep-85 -- jaw  put in offical name of BUA.
 *
 *	08-Aug-85 -- lp   fixed where bua_vec points for BLA case
 *
 *	05-Aug-85 -- lp   add mask to BLA check & move HIGH arb code
 *			  to bua case only.
 *
 *	26-jul-85 -- jaw  add vector allocation code.
 *
 *	19-Jun-85 -- jaw  VAX8200 name change.
 *
 *	05 Jun 85 -- jaw  clean up and move buainit from bua.c file.
 *
 *	20 Mar 85 -- jaw  add support for VAX 8200.
 *
 * ------------------------------------------------------------------------
 */
#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/cpuconf.h"

#include "../vax/cpu.h"
#include "../vax/mem.h"
#include "../vax/mtpr.h"
#include "../vax/clock.h"
#include "../vax/nexus.h"
#include "../vax/scb.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"

extern int nNUBA;
extern int numuba;


bbuainit(bua,physbua,cpup,binumber,binode)
register struct bua_regs *bua;
register char *physbua;
struct	cpusw *cpup;
int binumber;
int binode;
{
	int i;

	/* set offset in SCB */
	uba_hd[numuba].uh_lastiv = NBPG;

	if(numuba >= nNUBA) {
		binotconf(bua,physbua,cpup,binumber,binode);
		return;
	}

	printf("uba%x at vaxbi%x node %x \n", numuba,
		binumber,binode);

	/* bua must be at HIGH arb....or else */
	bua->bua_biic.biic_ctrl |= BICTRL_HIARB;
	bua->bua_vec = SCB_UNIBUS_PAGEOFFSET(numuba);

	uba_hd[numuba].uba_type = UBABUA;
	
	DELAY(5000000);  /* delay 5 seconds for the UNIBUS to settle */

	unifind(bua,physbua,umem[numuba],cpup->v_umaddr(binumber,binode),
		cpup->pc_umsize,cpup->v_udevaddr(binumber,binode),
		UMEMmap[numuba],cpup->pc_haveubasr,binumber,binode);
	numuba++;	
	bua->bua_ctrl |= BUACR_BUAEIE;

}

buainit(bua)
	register struct bua_regs *bua;
{
	long tbua_vec;
	long tbi_dest;
	long i;

	tbi_dest = bua->bua_biic.biic_int_dst;
	tbua_vec = bua->bua_vec;

	if (bisst(&(bua->bua_biic.biic_ctrl))) {
		if (((short)bua->bua_biic.biic_typ) == BI_BUA)
			printf (" DWBUA reset failed \n");
		else
			printf (" KLESI-B reset failed \n");
	}
	DELAY(5000000);  /* delay 5 seconds for the UNIBUS to settle */

	bua->bua_ctrl |= BUACR_BUAEIE;
	bua->bua_biic.biic_int_dst = tbi_dest;
	bua->bua_vec = tbua_vec;
}



