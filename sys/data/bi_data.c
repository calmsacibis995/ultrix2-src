
#ifndef lint
static char *sccsid = "@(#)bi_data.c	1.9	ULTRIX	10/3/86";
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
 * 	06-Aug-86 -- jaw  added nVAXBI varible.
 *
 * 	5-Jun-86   -- jaw 	changes to config.
 *
 *	09-Apr-86 -- lp   Added AIE
 *
 * 	05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 	18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 *	11-Mar-86 -- afd   Add DMB32 (BICOMBO)
 *
 * 	04-feb-86 -- jaw  get rid of biic.h.
 *
 *	03-feb-86 -- jaw   added bidata to hold info about each bi.
 *
 *	11-Sep-85 -- jaw   put in offical BI device names.
 *
 * 	19-Jun-85 -- jaw   VAX8200 name change.
 *
 *	05 Jun 85 -- jaw   support for BDA and BLA added.
 *
 * 	20 Mar 85 -- jaw   add support for VAX 8200
 *
 *
 * ------------------------------------------------------------------------
 */

#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../vax/cpu.h"
#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"
#include "../vax/ka8200.h"
#include "../vaxuba/ubavar.h"


int binotconf();

nobireset()
{
	/* no bi reset routine */

}
#ifdef VAX8800
int nbiinit();
#else
#define nbiinit binotconf
#endif


#if defined(VAX8200)
int ka820init();
int	bimeminit();
#else
#define ka820init binotconf
#define bimeminit binotconf
#endif

#include "vaxbi.h"
#if NVAXBI > 0
int	bbuainit();
#else
#define bbuainit binotconf
#define buainit	binotconf
#endif

#include "kdb.h"
#if NKDB > 0
int	bdainit();
#else
#define bdainit	binotconf
#endif

#include "dmb.h"
#if NDMB > 0
int	dmbinit();
#else
#define dmbinit binotconf
#endif

#include "klesib.h"
#if NKLESIB > 0
int	blainit();
#else
#define blainit binotconf
#endif

#include "bvpni.h"
#if NBVPNI > 0
int bvpniprobe(), bvpniattach();
struct uba_device *tmpni_info[NBVPNI];
u_short ni_std[] = { 0 };
struct uba_driver bvpnidriver = 
	{ bvpniprobe, 0, bvpniattach, 0, ni_std, "ni_ethernet", 
		tmpni_info, "ni", 0};
#else
#define bvpniprobe binotconf
#endif

int nNVAXBI = NVAXBI;

#if NVAXBI > 0
int (*buaprobes[])() = {  bbuainit,0};
int (*blaprobes[])() = {  blainit,0};
int (*ka820probes[])() = {  ka820init,0};
int (*bimemprobes[])() = {  bimeminit,0};
int (*bdaprobes[])() = {  bdainit,0};
int (*nbiprobes[])() = {  nbiinit,0};
int (*dmbprobes[])() = {  dmbinit,0};
int (*aieprobes[])() = {  bvpniprobe,0};


struct bisw bisw [] =
{

	{ BI_BUA, 	"uba",		buaprobes ,	nobireset,	
	  (BIF_ADAPTER|BIF_SST|BIF_SET_HEIE)},

	{ BI_KA820,	"ka820",	ka820probes,	nobireset,	
	  (BIF_NOCONF|BIF_SET_HEIE)},

	{ BI_MEM1,	"ms820",	bimemprobes,	nobireset,	
	  (BIF_NOCONF|BIF_SET_HEIE)},

	{ BI_BDA,	"kdb",		bdaprobes,	nobireset,	
	  (BIF_ADAPTER|BIF_SET_HEIE)},

	{ BI_BLA,	"klesib",	blaprobes,	nobireset,
	  (BIF_ADAPTER|BIF_SST|BIF_SET_HEIE)},

	{ BI_NBI,	"nbib",		nbiprobes,	nobireset,
	  (BIF_NOCONF|BIF_SET_HEIE)},

	{ BI_COMB,	"dmb",		dmbprobes, 	nobireset,	
	  (BIF_DEVICE|BIF_SST|BIF_SET_HEIE)},

	{ BI_AIE,	"aie",		aieprobes,	nobireset,	
	  (BIF_DEVICE|BIF_SST)},

	{ 0 }
};
int nbitypes = sizeof (bisw) / sizeof (bisw[0]);

struct bidata bidata[NVAXBI];
#endif


bi_init_failed(nxv,nxp,cpup,binumber,binode) 
struct bi_nodespace *nxv;
char *nxp;
struct cpusw *cpup;
int 	binumber;
int 	binode;
{
	struct bisw *pbisw;	
	pbisw = bidata[binumber].bierr[binode].pbisw;

	printf("%s at vaxbi%x node %x failed to initialize!\n",
		pbisw->bi_name,binumber,binode);

}

binotconf(nxv,nxp,cpup,binumber,binode,bitype) 
struct bi_nodespace *nxv;
char *nxp;
struct cpusw *cpup;
int 	binumber;
int 	binode;
{
	struct bisw *pbisw;	

	pbisw = bidata[binumber].bierr[binode].pbisw;

	printf("%s at vaxbi%x node %x option not configured!\n",
		pbisw->bi_name,binumber,binode);
		
}
