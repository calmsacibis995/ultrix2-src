#ifndef lint
static	char	*sccsid = "@(#)genassym.c	1.20	(ULTRIX)	2/13/87";
#endif lint

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
 ************************************************************************/

/*
 * genassym.c
 */

/*
 * Modification History:
 *
 * 13-Feb-87 -- Chase
 *	Increase size of mbuf map.  This change will consume an additional
 *	2k of memory on every system, but it should eliminate system panics
 *	caused by running out of mbuf map.
 *
 * 16-Jul-86 -- Todd M. Katz
 *	Add definition for CIADAP_SZ, the size of a CI adapter structure.
 *
 * 14-Apr-86 -- jaw
 *	remove MAXNUBA referances.....use NUBA only!
 *
 * 02-Apr-86 -- jrs
 *	Removed NQ stuff added in previous (defunct) scheduler
 *
 * 11-Mar-86 -- Larry
 *	NMBCLUSTERS is now a function of MAXUSERS
 *
 * 05-Mar-86 -- bglover
 *	Removed msgbuf from kernel; replaced with error log buffer
 *
 * 03-Mar-86 -- jrs
 *	Add in rpb offsets needed to fill in slave start code
 *
 * 05-Feb-86 -- jrs
 *	Add c_paddr cpudata structure offset
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 22-Jul-85 -- jrs
 *	Add cpudata structure offsets for multiprocessor work
 *
 * 31 Jun 85	darrell
 *	Added a lines to define UH_ZVCNT and UH_ZVFLG to make those
 *	elements of the uba_hd structure available to macro code (locore.s).
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 */

#define VAX8800 1
#define VAX8200 1
#define	VAX780	1
#define	VAX750	1
#define	VAX730	1

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/vmmeter.h"
#include "../h/vmparam.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/cmap.h"
#include "../h/map.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../vax/rpb.h"
#include "../h/mbuf.h"
#include "../vax/ioa.h"
#include "../vax/nexus.h"
#include "../vaxbi/buareg.h"
#include "../h/cpudata.h"
#include "../vaxci/ciadapter.h"
struct uba_hd uba_hd[1]; /* keep make happy */
int tty_ubinfo[1];  /* ditto */

main(argc, argv)
char *argv[];
{
	register struct proc *p = (struct proc *)0;
	register struct uba_regs *uba = (struct uba_regs *)0;
	register struct uba_hd *uh = (struct uba_hd *)0;
	register struct vmmeter *vm = (struct vmmeter *)0;
	register struct user *up = (struct user *)0;
	register struct rusage *rup = (struct rusage *)0;
	struct rpb *rp = (struct rpb *)0;
	struct text *tp = (struct text *)0;
	struct cpudata *cpd = (struct cpudata *)0;
	int maxusers = 10;
	int physmem_est = 2; /* estimate of physical mem. in megabytes */
	int factor; /* inflate ptes by this factor because of phys. mem */
	int nmbclusters; 


	printf("#ifdef LOCORE\n");
	printf("#define\tP_LINK %d\n", &p->p_link);
	printf("#define\tP_RLINK %d\n", &p->p_rlink);
	printf("#define\tP_XLINK %d\n", &p->p_xlink);
	printf("#define\tP_ADDR %d\n", &p->p_addr);
	printf("#define\tP_PRI %d\n", &p->p_pri);
	printf("#define\tP_STAT %d\n", &p->p_stat);
	printf("#define\tP_WCHAN %d\n", &p->p_wchan);
	printf("#define\tP_TSIZE %d\n", &p->p_tsize);
	printf("#define\tP_SSIZE %d\n", &p->p_ssize);
	printf("#define\tP_P0BR %d\n", &p->p_p0br);
	printf("#define\tP_SZPT %d\n", &p->p_szpt);
	printf("#define\tP_TEXTP %d\n", &p->p_textp);
	printf("#define\tP_FLAG %d\n", &p->p_flag);
	printf("#define\tSSLEEP %d\n", SSLEEP);
	printf("#define\tSRUN %d\n", SRUN);
	printf("#define\tSMASTER %d\n", SMASTER);
	printf("#define\tUBA_BRRVR %d\n", uba->uba_brrvr);
	printf("#define\tUH_UBA %d\n", &uh->uh_uba);
	printf("#define\tUH_VEC %d\n", &uh->uh_vec);
	printf("#define\tUH_SIZE %d\n", sizeof (struct uba_hd));
	printf("#define\tUH_ZVCNT %d\n", &uh->uh_zvcnt);
	printf("#define\tUH_ZVFLG %d\n", &uh->uh_zvflg);
	printf("#define\tRP_FLAG %d\n", &rp->rp_flag);
	printf("#define\tRP_BUGCHK %d\n", &rp->bugchk);
	printf("#define\tRP_WAIT %d\n", rp->wait);
	printf("#define\tX_CADDR %d\n", &tp->x_caddr);
	printf("#define\tV_SWTCH %d\n", &vm->v_swtch);
	printf("#define\tV_TRAP %d\n", &vm->v_trap);
	printf("#define\tV_SYSCALL %d\n", &vm->v_syscall);
	printf("#define\tV_INTR %d\n", &vm->v_intr);
	printf("#define\tV_PDMA %d\n", &vm->v_pdma);
	printf("#define\tV_FAULTS %d\n", &vm->v_faults);
	printf("#define\tV_PGREC %d\n", &vm->v_pgrec);
	printf("#define\tV_FASTPGREC %d\n", &vm->v_fastpgrec);
	printf("#define\tC_PADDR %d\n", &cpd->c_paddr);
	printf("#define\tC_STATE %d\n", &cpd->c_state);
	printf("#define\tC_IDENT %d\n", &cpd->c_ident);
	printf("#define\tC_RUNRUN %d\n", &cpd->c_runrun);
	printf("#define\tC_CURPRI %d\n", &cpd->c_curpri);
	printf("#define\tC_NOPROC %d\n", &cpd->c_noproc);
	printf("#define\tC_PROC %d\n", &cpd->c_proc);
	printf("#define\tC_SWITCH %d\n", &cpd->c_switch);
	printf("#define\tC_SIZE %d\n", sizeof (struct cpudata));
	printf("#define\tUPAGES %d\n", UPAGES);
	printf("#define\tNISP %d\n", NISP);
	printf("#define\tCLSIZE %d\n", CLSIZE);
	printf("#define\tCIADAP_SZ %d\n", sizeof(CIADAP));
	maxusers=atoi(argv[1]);
	physmem_est=atoi(argv[2]);
	if (physmem_est < 2)
		physmem_est = 2;
	else if (physmem_est > 128)
		physmem_est = 128;
	/*
	 * the number of system page table entries increases with maxusers and
	 * physical memory 
	 */
	factor = (physmem_est / 2) + (maxusers / 5);
	printf("/*maxusers=%d, SYSPTSIZE=(20+maxusers+%d)*NPTEPG)*/ \n", maxusers, factor);
	printf("#define\tSYSPTSIZE %d\n", (20+maxusers+factor)*NPTEPG);
	printf("#define\tUSRPTSIZE %d\n", USRPTSIZE);
	nmbclusters = NMBCLUSTERS * (maxusers/32 + 2);
	printf("#define\tNMBCLUSTERS %d\n", nmbclusters);
	printf("#define\tU_PROCP %d\n", &up->u_procp);
	printf("#define\tU_RU %d\n", &up->u_ru);
	printf("#define\tRU_MINFLT %d\n", &rup->ru_minflt);
	printf("#define\tMAXNIOA %d\n", MAXNIOA);
	printf("#define\tMAXNNEXUS %d\n", MAXNNEXUS);
	printf("#else\n");
	printf("asm(\".set\tU_ARG,%d\");\n", up->u_arg);
	printf("asm(\".set\tU_QSAVE,%d\");\n", up->u_qsave);
	printf("#endif\n");
}

