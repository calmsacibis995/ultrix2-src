/*
 * spt.s - system page table.
 */

/* @(#)spt.s	1.25	(ULTRIX)	2/13/87	*/

/************************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
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
 ************************************************************************
 * Modification History:
 *
 * 12-Feb-87 -- depp
 *	changed sizing of dmempt
 *
 * 13-Dec-86  -- Fred Canter and Ali Rafieymehr
 *	Changed sh and sg SYSMAPs respectively.
 *
 * 27-Aug-86  -- Fred Canter
 *	Removed unnecessary comments.
 *  5-Aug-86  -- Darrell Dunnuck
 *	Added mapping for buffer in the VAXstar TZK50 driver.
 *
 *  2-Jul-86  -- fred (Fred Canter)
 *	Added mapping for TEAMmate 8 line SLU registers.
 *
 * 18-Jun-86  -- fred (Fred Canter)
 *	Changed for VAXstar kernel support.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 15-Apr-86 -- afd
 *	Added QMEMmap and qmem for microVAX QBUS space.
 *	MicroVAX nexus space is faked by claming that a MicroVAX has
 *	32 nexi (in nexus.h) so that we get 512 ptes for nexus space for
 *	the QBUS.  This should be made QBUS dependant not cpu dependant.
 *
 * 14-Apr-86 -- jaw  remove MAXNUBA referances.....use NUBA only!
 *
 * 02-Apr-86 -- jaw  add support for nautilus console and memory adapter
 *
 * 10-mar-86 -- tresvik
 *	added support for SAS memory device
 *
 * 05 Mar 86 -- bjg
 *	Removed SYSMAP for msgbufmap; (replaced with error logger)
 *
 * 24 Feb 86 -- depp
 *	Added in kernel memory allocation map dmemptmap
 *
 * 04-feb-86 -- tresvik
 *	added VMB boot path support
 *
 * 04-feb-86 -- jaw  add mapping for 8800 i/o adpters.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 18-Jun-85 -- jaw
 *	Reserve some map space for Rx50 driver for VAX8200.
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 03 Nov 84 -- rjl
 *	MicroVAX-II needs to map 4 megabytes of space on the Q-BUS plus
 *	8k for the I/O page instead of 256k bytes on the UNIBUS. The extra
 *	map registers for the I/O page are necessary because it is not
 *	part of the memory space like UNIBUS adapters.
 *
 *	It also needs to map a 256k `nexus' space for the local registers.
 */
#include "uba.h"
#include "klesib.h"
#include "../machine/vmparam.h"

	.set	NBPG,512


/*
 * System page table
 */ 
#define	vaddr(x)	((((x)-_Sysmap)/4)*NBPG+0x80000000)
#define Clusterptes(x)	(((x)+(CLSIZE-1))&(~(CLSIZE-1)))
#define	SYSMAP(mname, vname, npte)			\
_/**/mname:	.globl	_/**/mname;		\
	.space	Clusterptes(npte)*4;				\
	.globl	_/**/vname;			\
	.set	_/**/vname,vaddr(_/**/mname)

	.data
	.align	2
	SYSMAP(Sysmap	,Sysbase	,SYSPTSIZE	)
	SYSMAP(UMBAbeg	,umbabeg	,0		)
/*
 * MAXNNEXUS is defined as 32 for MicroVAXen in nexus.h
 * so that we get 512 ptes for nexus space for MicroVAX.
 * This should be made QBUS dependant, not cpu dependant.
 * VAXstation 2000 and MicroVAX 2000 share these maps with Q-bus CPUs.
 */
	SYSMAP(Nexmap	,nexus		,16*MAXNNEXUS	)
	SYSMAP(UMEMmap	,umem		,512*NUBA	)
	SYSMAP(KBMEMmap	,kbmem		,512*NKLESIB	)
#ifdef MVAX
	SYSMAP(QMEMmap	,qmem		,(8192+16)	)
	SYSMAP(NMEMmap	,nmem		,256		)
	SYSMAP(sdbufmap	,SD_bufmap	,129		)
	SYSMAP(stbufmap	,ST_bufmap	,33		)
	SYSMAP(SGMEMmap	,sgmem		,192		)
	SYSMAP(sgbufmap	,SG_bufmap	,129		)
	SYSMAP(SHMEMmap	,shmem		,1		)
#else
	SYSMAP(QMEMmap	,qmem		,0		)
	SYSMAP(NMEMmap	,nmem		,0		)
	SYSMAP(sdbufmap	,SD_bufmap	,0		)
	SYSMAP(stbufmap	,ST_bufmap	,0		)
	SYSMAP(SGMEMmap	,sgmem		,0		)
	SYSMAP(sgbufmap	,SG_bufmap	,0		)
	SYSMAP(SHMEMmap	,shmem		,0		)
#endif MVAX
	SYSMAP(Ioamap	,ioa		,MAXNIOA	)
	SYSMAP(UMBAend	,umbaend	,0		)
	SYSMAP(Usrptmap	,usrpt		,USRPTSIZE	)
	SYSMAP(Forkmap	,forkutl	,UPAGES		)
	SYSMAP(Xswapmap	,xswaputl	,UPAGES		)
	SYSMAP(Xswap2map,xswap2utl	,UPAGES		)
	SYSMAP(Swapmap	,swaputl	,UPAGES		)
	SYSMAP(Pushmap	,pushutl	,UPAGES		)
	SYSMAP(Vfmap	,vfutl		,UPAGES		)
	SYSMAP(CMAP1	,CADDR1		,1		)
	SYSMAP(CMAP2	,CADDR2		,1		)
	SYSMAP(mcrmap	,mcr		,1		)
	SYSMAP(mmap	,vmmap		,1		)
	SYSMAP(camap	,cabase		,16*CLSIZE	)
	SYSMAP(ecamap	,calimit	,0		)
	SYSMAP(Mbmap	,mbutl		,NMBCLUSTERS*CLSIZE)
#ifdef MVAX
	SYSMAP(dmemptmap,dmempt		,MIN_DMEM_PAGES	)
#else 
	SYSMAP(dmemptmap,dmempt		,MAX_DMEM_PAGES )
#endif MVAX
	SYSMAP(vmbinfomap,vmb_info	,128		)
#ifdef VAX8200
	SYSMAP(V8200wmap ,v8200watch	,1		)
	SYSMAP(V8200pmap ,v8200port	,1		)
	SYSMAP(V8200rmap ,v8200rx50	,1		)
#endif
#if defined(VAX8200) || defined(VAX8800)
	SYSMAP(rxbufmap	,RX_bufmap	,129		)
#endif
#ifdef VAX8800
	SYSMAP(V8800mem	,v8800mem	,1		)
	SYSMAP(ADPT_loopback	,adpt_loopback	,4	)
	SYSMAP(Nbia	,nbia_addr 	,2		)

#endif VAX8800
#ifdef SAS
	SYSMAP(mdbufmap	,MD_bufmap	,129		)
#endif SAS

eSysmap:
	.globl	_Syssize
	.set	_Syssize,(eSysmap-_Sysmap)/4
	.text
	.align	1
	.globl _setptsize
	.globl	_sysptsize
_setptsize:
	.word 0
	movl	$SYSPTSIZE,_sysptsize
	ret
