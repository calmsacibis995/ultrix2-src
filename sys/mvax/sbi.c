
#ifndef lint
static char *sccsid = "@(#)sbi.c	1.7	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,85,86 by			*
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

/************************************************************************
 * Modification History
 *
 * 10-Jul-86   -- jaw	added adpt/nexus to ioctl
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 06-Jun-86 -- JAW  fix max uba test.....
 *
 * 09-Apr-86	Darrell Dunnuck
 *	Called badaddr via the macro BADADDR.
 *
 * 02-Apr-86	bjg
 *	Change handling of sbi_there to use all 32 bits.
 *
 * 9-Mar-86	darrell
 *	A pointer to cpusw is now passed, rather that using a global.
 *
 * 6-Mar-86	darrell
 *	Created this file to contain the probesbi and probeioa.
 *	Probesbi used to be probenexus in autoconf.c.  It was 
 *	made SBI specific and moved here.
 ************************************************************************/

#include "mba.h"
#include "uba.h"

#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/dk.h"
#include "../h/vm.h"
#include "../h/conf.h"
#include "../h/dmap.h"
#include "../h/reboot.h"

#include "../vax/cpu.h"
#include "../vax/mem.h"
#include "../vax/mtpr.h"
#include "../vax/ioa.h"
#include "../vax/nexus.h"
#include "../vax/scb.h"
#include "../vaxmba/mbareg.h"
#include "../vaxmba/mbavar.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../h/cpuconf.h"

extern	int	(*ubaintv[])();
extern	int	nNMBA;
extern	(*Mba_find)();
extern  int 	nNUBA;
/* save record of sbis present for sbi error logging for 780 and 8600 */
extern long sbi_there;	/* bits 0-15 for nexi,sbi0; 16-31 for nexi on sbi1*/
extern int ioanum;

/*
 * Probe nexus space, finding the interconnects
 * and setting up and probing mba's and uba's for devices.
 */
/*ARGSUSED*/
probesbi(cpup)
struct cpusw *cpup;
{
	register char  *nxv;
	char  *nxp;
	union nexcsr nexcsr;
	int base,i,nexnum;
	enum memtype memtype;

	(void) spl0();

	base=(ioanum * cpup->pc_nnexus);

	nxv = (char *)nexus + (ioanum * cpup->pc_nnexus * cpup->pc_nexsize);
	for( nexnum= 0; nexnum < (cpup->pc_nnexus); 
		nexnum++, nxv += cpup->pc_nexsize) {

		nxp = (char *)cpup->v_nexaddr(ioanum,nexnum);
		/*
		 * Map the nexus.
		 */
		nxaccess (nxp, Nexmap[base+nexnum], cpup->pc_nexsize);
		/*
		 * See if there is anything there.
		 */
		if (BADADDR((caddr_t) nxv, 4))
			continue;

		sbi_there |= 1<< ((ioanum * cpup->pc_nnexus) + nexnum);

		/*
		 * Get the nexus type from the nexty table in a configuration
		 * register is not present.
		 */
		if (cpup->pc_nextype && cpup->pc_nextype[nexnum] != NEX_ANY)
			nexcsr.nex_csr = cpup->pc_nextype[nexnum];
		else
			nexcsr = ((struct nexus *)nxv)->nexcsr;
		if (nexcsr.nex_csr & NEX_APD)
			continue;
		/*
		 * Process each nexus type (or pseudo nexus type)
		 */
		switch (nexcsr.nex_type) {

			case NEX_MBA: 
				printf ("mba%d at address 0x%x\n", nummba, nxp);
				if (nummba >= nNMBA) {
					printf ("%d mba's", nummba);
					goto unconfig;
				}
#if NMBA > 0
				if (nNMBA > 0) {
					(*Mba_find)(nxv,nxp,nexnum,ioanum);
					nummba++;
				}
#endif
				break;

			case NEX_UBA0: 
			case NEX_UBA1: 
			case NEX_UBA2: 
			case NEX_UBA3: 
				printf ("uba%d at address 0x%x\n", numuba, nxp);
				/* first check that enough UBA structures allocated */	
				if (numuba >= nNUBA) {
					printf("Only %d UBA's configured -- extra UBA ignored\n", nNUBA);
					break;
				}
				if (cpu == VAX_780 || cpu == VAX_8600) {
					setscbnex (ubaintv[numuba],nexnum);
				}
				/*
				 * In the following calculation of ``i''
				 * the unibus adapter number is bumped
				 * by 4 for each io adapter so as to
				 * properly index the address tables for
				 * Unibus address space.  This is only
				 * used with the VAX 8600 processor at
				 * this time
				 */
				i = (nexcsr.nex_type - NEX_UBA0);
				switch(cpu) {
				case VAX_780:
				case VAX_8600:
					uba_hd[numuba].uba_type = UBA780;
					break;
				case VAX_730:
					uba_hd[numuba].uba_type = UBA730;
					break;
				case VAX_750:
					uba_hd[numuba].uba_type = UBA750;
					break;
				default:
					uba_hd[numuba].uba_type = 0;
					break;
				}
				unifind (nxv, nxp, umem[numuba], 
					cpup->v_umaddr(ioanum,i),
					cpup->pc_umsize, 
					cpup->v_udevaddr(ioanum,i),
					UMEMmap[numuba], cpup->pc_haveubasr,ioanum,nexnum);
				if (uba_hd[numuba].uba_type & UBA780)
					((struct uba_regs      *) nxv) -> uba_cr =
						UBACR_IFS | UBACR_BRIE |
						UBACR_USEFIE | UBACR_SUEFIE |
						(((struct uba_regs     *) nxv) -> uba_cr & 0x7c000000);
				numuba++;
				break;

			case NEX_DR32: 
				/*
				 * There can be more than one... are there
				 * other codes ???
				 */
				printf ("dr32");
				goto unsupp;

			/* 
			 * MS750 and MS730 are assigned "invented" adapter
			 * types in 'cpudata.c' and are defined in 'nexus.h'
			 */

			case NEX_MEM750: 
				printf ("mcr%d (MS750) at address 0x%x\n", nmcr, nxp);
				if (nmcr >= 1) {
					printf ("2 mcr's");
					goto unsupp;
				}
				mcrdata[nmcr].memtype = MEMTYPE_MS750;
				mcrdata[nmcr++].mcraddr = (struct mcr  *) nxv;
				break;

			case NEX_MEM730: 
				printf ("mcr%d (MS730) at address 0x%x\n", nmcr, nxp);
				if (nmcr >= 1) {
					printf ("2 mcr's");
					goto unsupp;
				}
				mcrdata[nmcr].memtype = MEMTYPE_MS730;
				mcrdata[nmcr++].mcraddr = (struct mcr  *) nxv;
				break;

			case NEX_MPM0: 
			case NEX_MPM1: 
			case NEX_MPM2: 
			case NEX_MPM3: 
				printf ("mpm");
				goto unsupp;

			case NEX_CI: 
				printf ("ci");
				goto unsupp;

			default: 
				switch (nexcsr.nex_type & 0x60) {
					/* 
					 * Fall through to here to see if the
					 * nexus is MS780C or MS780E.
					 */

					case NEX_MS780C: 
						printf ("mcr%d (MS780-C) at address 0x%x, ", nmcr, nxp);
						if (((nexcsr.nex_type & 0x18) == 0)
						    || ((nexcsr.nex_type & 0x18) == 3)) {
							printf ("memory arrays misconfigured\n");
							continue;
						}
						else {
							printf ("%dKbytes, ",
							    (((nexcsr.nex_csr >> 9) & 0x3f) + 1) * 64);
						}
						printf ("%s\n", (nexcsr.nex_type & 1)
							    ? "interleaved" : "noninterleaved");
						if (nmcr >= 4) {
							printf ("5 mcr's");
							goto unsupp;
						}
						mcrdata[nmcr].memtype = MEMTYPE_MS780C;
						mcrdata[nmcr++].mcraddr = (struct mcr  *) nxv;
						break;

					case NEX_MS780E: 
						printf ("mcr%d (MS780-E) at address 0x%x, ", nmcr, nxp);

						if (((nexcsr.nex_type & 0x18) == 0)
								|| ((nexcsr.nex_type & 0x18) == 3)) {
							printf ("memory arrays misconfigured\n");
							continue;
						}
						else {
							printf ("%dMbytes, ", ((nexcsr.nex_csr >> 9) & 0x7f) + 1);
						}
						switch (nexcsr.nex_type & 0x7) {
							case 0: 
								printf ("noninterleaved (lower ctlr)\n");
								break;
							case 1: 
								printf ("external interleave (lower ctlr)\n");
								break;
							case 2: 
								printf ("noninterleaved (upper ctlr)\n");
								break;
							case 3: 
								printf ("external interleave (upper ctlr)\n");
								break;
							case 4: 
								printf ("internal interleave\n");
								break;
							default: 
								printf ("bad interleave mode\n");
						}
						if (nmcr >= 4) {
							printf ("5 mcr's");
							goto unsupp;
						}
						mcrdata[nmcr].memtype = MEMTYPE_MS780E;
						mcrdata[nmcr++].mcraddr = (struct mcr  *) nxv;
						break;

					default: 
						printf ("nexus type %x", nexcsr.nex_type);
						goto unsupp;
				}
		}
		continue;
unsupp: 
		printf (" unsupported at address 0x%x\n", nxp);
		continue;
unconfig: 
		printf (" not configured\n");
		continue;
	}
}

/*
 * Probe for IO adapters.
 * Call appropriate probe routines based on the typ of adapter found.
 */
/*ARGSUSED*/
probeioa(cpup)
struct cpusw *cpup;
{
	register char  *ioav;
	struct sbia_regs *sbiv;
	char  *ioap;
	union ioacsr ioacsr;

	ioanum = 0, ioav = (char *)ioa;
	for( ; ioanum < cpup->pc_nioa; ioanum++, ioav += cpup->pc_ioasize) {
		ioap = (char *)cpup->pc_ioabase[ ioanum ];
		/*
		 * Map the IO adapter.  Use 'nxaccess' to set up the mapping
		 */
		nxaccess (ioap, Ioamap[ioanum], cpup->pc_ioasize);
		/*
		 * See if there is anything there.
		 */
		if (BADADDR((caddr_t) ioav, 4))
			continue;
		/*
		 * Get the IO adapter type from the configuration register
		 */
		ioacsr = ((struct ioa *)ioav)->ioacsr;
		/*
		 * Process each IO adapter type 
		 */
		switch (ioacsr.ioa_type & IOA_TYPMSK) {

		case IOA_SBIA: 
			printf ("IO adapter %d at address 0x%x is an SBI adapter\n", ioanum, ioap);
			probesbi(cpup,ioanum);	/* find the nexus adapters */
			sbiv = (struct sbia_regs *)ioav;
			sbiv->sbi_errsum = 0xffffffff;	/* Clear errs */
			sbiv->sbi_error	= 0x1000;	/* One more */
			sbiv->sbi_fltsts = 0xc0000;	/* Enable SBI Flts */
			break;

		default: 
			printf ("IO adapter %d at address 0x%x is unsupported (type = 0x%x)",
				ioanum, ioap, ioacsr.ioa_type);
			break;
		}
		continue;
	}
}

