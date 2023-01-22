# ifndef lint
static	char	*sccsid = "@(#)autoconf.c	1.22	2/19/86";
# endif not lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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

/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/autoconf.c
 *
 * 18-feb-86 -- jaw  change of nexus addresses.
 *
 * 03-OCT-85 -- jaw
 *	MVAXII boot hang fix.
 *
 * 19-Sep-85 -- jaw
 *	fix so that copy could go through autoconf twice for 8200.
 *
 * 30-Aug-85 -- jaw
 * 	fix up a case where the 8200 boot would fail because of BI
 *	error interrupt.
 *
 * 23-Aug-85 -- map
 *	Only setup saveCSR on first BDA encountered.
 *	Add BLA to case statement. Same as BUA.
 *
 * 13-Aug-85 -- rjl
 *	Added configuration support for multiple virtual consoles.
 *	This allows either a qdss of a qvss.
 *
 * 15-Jul-85 -- map
 *	now setup saveCSR with BDA address always.
 *	make saveCSR defined extern
 *
 * 24-Jun-85 -- map
 *	add initialization of can_bda variable
 *
 * 19-Jun-85 -- jaw
 *	make it so rx50 boot can get vmunix off ra disk.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 18-Jun-85 -- jaw
 *	Fixed so rabads would load....can_bda was problem.
 *
 * 20-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 14-Mar-85 -tresvik
 *	Redefined configuration of UBA and MBA for VAX8600
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 22 Aug 84 -- rjl
 *	Added support for MicroVAX-II. Initialize and enable
 *	the q-bus map.
 *
 *  7 Jul 84 -- rjl
 *	Add support for QVSS as system console
 *
 * 29 Dec 83 --jmcg
 *	Allow conditional compilation based on CPUs defined.
 *	Added cases for MicroVAX_1.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		autoconf.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../machine/pte.h"

#include "../h/param.h"

#include "../vax/cpu.h"
#include "../vax/nexus.h"
#include "../vaxuba/ubareg.h"
#include "../vaxmba/mbareg.h"
#include "../vax/mtpr.h"
#include "../vaxbi/buareg.h"

#include "savax.h"

#if defined(VAX8600) 
#define	UTRA(j,i) ((struct uba_regs *)(NEX8600(j,i)))
#define	UMAA(j,i)	((caddr_t)UDEVADDR8600(j,i))
#define	MTRA(j,i)	((struct mba_regs *)(NEX8600(j,i)))

struct	uba_regs *ubaddr8600[] = { 
	UTRA(0,3), UTRA(0,4), UTRA(0,5), UTRA(0,6),
	UTRA(1,3), UTRA(1,4), UTRA(1,5), UTRA(1,6)
};
caddr_t	udevaddr8600[] = {
	UMAA(0,0), UMAA(0,1), UMAA(0,2), UMAA(0,3),
	UMAA(1,0), UMAA(1,1), UMAA(1,2), UMAA(1,3)
};
struct	mba_regs *mbaddr8600[] = { 
	MTRA(0,8), MTRA(0,9), MTRA(0,10), MTRA(0,11),
	MTRA(1,8), MTRA(1,9), MTRA(1,10), MTRA(1,11)
};

#undef	UTRA
#undef	UTRB
#undef	UMAA
#undef	UMAB
#undef	MTRA
#undef	MTRB
#endif VAX8600

#if defined(VAX8200)
struct	uba_regs *ubaddr8200[16];
caddr_t	udevaddr8200[16];
#endif


#if defined(VAX780)
#define	UTR(i)	((struct uba_regs *)(NEX780(i)))
#define	UMA(i)	((caddr_t)UDEVADDR780(i))
#define	MTR(i)	((struct mba_regs *)(NEX780(i)))

struct	uba_regs *ubaddr780[] = { UTR(3), UTR(4), UTR(5), UTR(6) };
caddr_t	udevaddr780[] = { UMA(0), UMA(1), UMA(2), UMA(3) };
struct	mba_regs *mbaddr780[] = { MTR(8), MTR(9), MTR(10), MTR(11) };

#undef	UTR
#undef	UMA
#undef	MTR
#endif

#if defined(VAX750)
#define	UTR(i)	((struct uba_regs *)(NEX750(i)))
#define	UMA(i)	((caddr_t)UDEVADDR750(i))
#define	MTR(i)	((struct mba_regs *)(NEX750(i)))

struct	uba_regs *ubaddr750[] = { UTR(8), UTR(9) };
caddr_t	udevaddr750[] = { UMA(0), UMA(1) };
struct	mba_regs *mbaddr750[] = { MTR(4), MTR(5), MTR(6), MTR(7) };

#undef	UTR
#undef	UMA
#undef	MTR
#endif

#if defined(VAX730)
#define	UTR(i)	((struct uba_regs *)(NEX730(i)))
#define	UMA	((caddr_t)UDEVADDR730)

struct	uba_regs *ubaddr730[] = { UTR(3) };
caddr_t	udevaddr730[] = { UMA };

#undef	UTR
#undef	UMA
#endif

#ifdef MVAX
caddr_t	udevaddrmvax[] = { ( (caddr_t)QDEVADDRUVI) };
struct  uba_regs *ubaddrUVII[] = &((struct qb_regs *)NEXUVII)->qb_uba.uba;
/*
 * Base address of msvlp 512kb memory board csr's (MicroVAX-I)
 */
#define QMEMCSRBASE ((ushort *)0x20001440)
#define QMEMCSREND ((ushort *)0x2000145e)
/*
 * Interprocessor door bell register.
 */
#define QIPCR ((ushort *)0x20001f40)
/*
 * Q-Bus map register base.
 */
#define QMAPBASE ((unsigned int *)0x20088000)
/*
 * Memory system error and control register.
 */
#define QMSERR ((unsigned int *)0x20080004)

#define QMAPVALID 0x80000000
#define LMEAE 0x20

#ifdef VCONS
/*
 * Virtual console configuration tables.
 */
extern qv_init(),qd_init();

int (*vcons_init[])() = {
	qd_init,
	qv_init,
	0
};
int (*v_getc)()=0,
    (*v_putc)()=0;
#endif VCONS

#endif MVAX

int nmba = 0;
int nuba = 0;
char can_bda = 0;
extern long saveCSR;

configure()
{
#ifdef MVAX
	register howto, devtype;		/* r11, r10 */
#endif MVAX
	int bi_nodenum;
	union cpusid cpusid;
	ushort *qmemcsr;			/* msvlp memory csr */
	register short *bptr;
	unsigned int *qmapbase;			/* q-bus map pointer */
	int i;
	cpusid.cpusid = mfpr(SID);
	cpu = cpusid.cpuany.cp_type;
	switch (cpu) {

#if defined(VAX8600)
	case VAX_8600:
		mbaddr = mbaddr8600;
		ubaddr = ubaddr8600;
		devaddr = udevaddr8600;
		nmba = sizeof (mbaddr8600) / sizeof (mbaddr8600[0]);
		nuba = sizeof (ubaddr8600) / sizeof (ubaddr8600[0]);
		break;
#endif

#if defined(VAX8200)
	case VAX_8200:
		ubaddr = ubaddr8200;
		devaddr = udevaddr8200;
		nuba=0;
		break;	
#endif
#if defined(VAX780)
	case VAX_780:
		mbaddr = mbaddr780;
		ubaddr = ubaddr780;
		devaddr = udevaddr780;
		nmba = sizeof (mbaddr780) / sizeof (mbaddr780[0]);
		nuba = sizeof (ubaddr780) / sizeof (ubaddr780[0]);
		break;
#endif

#if defined(VAX750)
	case VAX_750:
		mbaddr = mbaddr750;
		ubaddr = ubaddr750;
		devaddr = udevaddr750;
		nmba = sizeof (mbaddr750) / sizeof (mbaddr750[0]);
		nuba = 0;
		break;
#endif

#if defined(VAX730)
	case VAX_730:
		ubaddr = ubaddr730;
		devaddr = udevaddr730;
		break;
#endif

#ifdef MVAX
	case MVAX_I:
		devaddr = udevaddrmvax;
		break;

	case MVAX_II:
		devaddr = udevaddrmvax;
		ubaddr = ubaddrUVII;
		break;
#endif MVAX
	}
	/*
	 * Forward into the past...
	 */
#if defined(VAX8600) || defined(VAX780) || defined(VAX750)
	for (i = 0; i < nmba; i++)
		if (!badloc(mbaddr[i],sizeof(int)))
			mbaddr[i]->mba_cr = MBCR_INIT;
#endif
#if defined(VAX8600) || defined(VAX780) || defined(VAX750) || defined(VAX730)
	for (i = 0; i < nuba; i++)
		if (!badloc(ubaddr[i],sizeof(int)))
			ubaddr[i]->uba_cr = UBACR_ADINIT;
#endif
	switch( cpu)
	{
#ifdef	VAX8600
	case VAX_8600:
		break;
#endif	VAX8600
#ifdef  VAX8200
	case VAX_8200:
		
		for( bi_nodenum = 0; bi_nodenum < 16 ; bi_nodenum++) {
			
			bptr = (NEX8200(bi_nodenum)); 
		 	if (!badloc(((caddr_t) bptr),sizeof(int))) {

				((struct bi_nodespace *)bptr)->biic.biic_err_int = 0;
				switch ((short)((struct bi_nodespace *)bptr)->biic.biic_typ) {		
					case BI_BDA:

						if (can_bda == 0){
							can_bda = 1;		/* Could be a BDA */
							saveCSR = (long)bptr; /* Set up just in case */
						}


					case BI_BUA:
					case BI_BLA:

						ubaddr[nuba] = ((struct uba_regs *)bptr);
						devaddr[nuba] = ((caddr_t) UDEVADDR8200(bi_nodenum));
		
						badwrte(&(((struct bi_nodespace *)bptr)->biic.biic_ctrl));
						nuba++;
						break;

				}
			}
		
		}

		break;
	
#endif  VAX8200
#ifdef	VAX780
	case VAX_780:
		break;
#endif	VAX780
#ifdef	VAX750
	case VAX_750:
#endif	VAX750
#ifdef	VAX730
	case VAX_730:
#endif	VAX730
#if	defined(VAX750) || defined(VAX730)
		mtpr(IUR, 0);
		break;
#endif	VAX750 || VAX730
#ifdef MVAX
	case MVAX_I:
		mtpr(IUR, 0);
		/*
		 * Turn on parity detection for each memory board.
		 */
		for(qmemcsr=QMEMCSRBASE; qmemcsr <= QMEMCSREND ; qmemcsr++ )
			if( !badloc( qmemcsr, sizeof(ushort)) )
				*qmemcsr = 1;
		break;
	case MVAX_II:
		mtpr(IUR, 0);
		/*
		 * Turn on parity detection.
		 */
		*QMSERR = 1;
		/*
		 * Set up the q-bus map and enable.
		 */
		for(qmapbase = QMAPBASE , i=0 ; i<8192 ; qmapbase++, i++ )
			*qmapbase = i;
		*QIPCR = LMEAE ;
		break;
#endif MVAX
	}
	/* give devices a chance to recover... */
	if (nuba > 0)
		DELAY(2000000);
#ifdef VCONS
	/*
	 * configure the console
	 */
	for( i = 0 ; vcons_init[i] && (*vcons_init[i])() != 1 ; i++ )
		;
#endif VCONS
}

