#ifndef lint
static	char	*sccsid = "@(#)machdep.c	1.109	(ULTRIX)	3/18/87";
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

/************************************************************************
 * Modification history: /sys/vax/machdep.c
 *
 * 13-Feb-87 -- Chase
 * 	change computation of nmbclusters to stay consistent with the
 *	formula	in /sys/vax/genassym.c.
 *
 * 12-Feb-87 -- depp
 *	Changed the sizing of dmempt
 *
 * 12-Feb-87 -- pmk
 *	Changed 3 gendump cprintf's for clearer meaning. Bar
 *
 * 15-Jan-87 -- Chase
 *	change dmemmap size to 512
 *
 * 15-jan-87 -- koehler
 *	change the calculation for the number of gnodes.
 *
 * 08-Jan-87 -- prs
 *	Added check to boot() to ensure a gnode has a mount device
 *	associated with it before an update is attempted.
 *
 * 16-Dec-86 -- prs
 *	Added code to boot() that turns accounting off if running. This
 *	allows the usr file system to umounted cleanly during shutdown.
 *
 * 04-Dec-86 -- prs
 *	Added conditional to allow the gendump routine to work on a
 *	microVAX I
 *
 * 04-Dec-86 -- Robin Lewis
 *	Removed the quota calls associated with setting login limits.  Login 
 *	limit code is now independent from MAXUSER,cpu type, and src code.
 *	(see /upgrade data file)
 *
 * 11-Sept-86  --  Tim Burke
 *	Reduce maxclistusers from 128 to 75 in order to tie up fewer resources.
 *
 * 30-Aug-86 -- fred (Fred Canter)
 *	Added/fixed comments in microdelay routine.
 *	Fixed crash dumps for VAXstar.
 *
 * 14-Aug-86 -- tresvik
 *	removed CAN BE HALTED message for MVAX II and VAXstar
 *	Physmem fixed again to act as memory size limit
 *	plug in CPU and CPU_SUBTYPE to the RPB for installation
 *	
 * 07-Aug-86 -- jaw	fixed microdelay to reflex change in VAX SRM
 *	to the ipl of the interval timer interrupt.
 *
 * 06-Aug-86 -- jaw	fixed baddaddr to work on running system.
 *	Also fixes to ka8800 reboot code.
 *
 *  5-Aug-86 -- fred (Fred Canter)
 *	Added VAXstar console program mail box support.
 *
 * 23-Jul-86 -- prs
 *	Removed the generic config table genericconf.
 *	The genericconf table is now built in conf.c. This will
 *	ensure that the table will only contain configured drivers.
 *
 * 26-Jul-86 -- bglover
 *	Append first startup printfs to startup mesg in error logger
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 13-Jun-86 -- tresvik
 *	added support for network boot and installation by saving the
 *	address of vmb_info in the RPB for use be a user level program
 *	misc, fixes to Physmem, which got redefined to do something
 *	different.  It can now be preset to set an artificial top of
 *	memory as well as keep track of the actual amount for use by the
 *	sizer.  Increase min memory from 2 Meg to 3 Meg.
 *
 *  9-jun-86 -- bjg
 *	Set appendflg to log startup messages
 *
 *  2-Jun-86 -- depp
 *	Added passthough of "u.u_code" for SIGSEGV (see modification note
 *	"24 Feb 86 -- depp" below.
 *
 * 22-May-86 -- prs
 *	Added saving of u_area to partial dump code.
 *
 * 07-May-86 -- bjg
 *	Remove logstart() call; moved to clock.c.
 *
 * 16-Apr-86 -- darrell
 *	now calling badaddr with the macro BADADDR.
 *
 * 15-Apr-86 -- afd
 *	Re-wrote most of startup().  This changed the way we allocate
 *	system data structures.  The amount of space for "nbufs"
 *	is now calculated more intelligently.  The algorithm for
 *	this came from Berkeley 4.3.
 *
 *	The global variable "endofmem" now contains the last
 *	kernel virtual address that was used.
 *
 * 09 Apr 86 -- prs
 * 	Added common dump code taken from drivers to dumpsys.
 *	Also, added the generic dump routine, gendump, and
 *	partial crash dump support.
 *
 * 09-apr-86 -- jaw  use 8800 boot me command.
 *
 * 22-Mar-86 -- koehler
 *	changed the why the filesystem is handled when the system is 
 *	shutting down
 *
 * 19-Mar-86 -- pmk
 *	Changed microdelay so at ipl18 and above clock is used 
 *	and mvax delay now is appox. real microsec. delay.
 *
 * 12-mar-86 -- tresvik
 *	fixed loop for system page table too small; reduction of physmem
 *	to 2 meg.  Maxusers was being set too high and possibly higher
 *	than it's original setting.  Also, do not attempt to remap
 *	vmbinfo on retries.
 *
 * 11 Mar 86 -- robin
 *	Changed the login limit code to use a system call and counter.
 *
 * 11 Mar 86 -- lp
 *	Added n-buffered I/O support (in physstrat). Also
 *	added IOA reset in halt code for 86?0.
 *
 * 11 Mar 86 -- larry
 *	nmbclusters is now a function of maxusers
 *
 * 05 Mar 86 -- bjg
 *	Removed msgbuf from the kernel; replaced by error logger
 *
 * 24 Feb 86 -- depp
 *	Added new maps for kernel memory allocation mechanisms (dmempt*)
 *	and in the routine sendsig, added code passthrough for SIGBUS
 *	to complement the new system call "mprotect"
 *
 * 19-Feb-86 -- bjg  add call to log startup message (logstart();)
 *
 * 18-Feb-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 12-Feb-86	Darrell Dunnuck
 *	Removed the routines memerr, memenable, setcache, and tocons
 *	from here and put them in the appropiate modules (kaXXXX.c)
 *	by processor type.
 *	Added a new routine cachenbl.
 *
 * 12-Feb-86 -- jrs
 *	Added tbsync() calls to control mp translation buffer
 *
 * 04-Feb-86 -- tresvik
 *	added VMB boot path support
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 30 Jan 86 -- darrell
 *	All machine check headers, constants, and code has been
 *	removed from machdep.c and broken up by processor type
 *	and placed in separate modules.  The machine check 
 *	routines are now reached through the cpusw structure
 *	which is part of the machine dependent code
 *	restructuring.
 *
 *	The Generic machine check structure has been moved to 
 *	cpu.h
 *
 * 20 Jan 86 -- pmk
 *	Add getpammdata() for 8600.
 *	Add memory rountines for errlog and changed memerr().
 *	Add logmck() to mackinecheck() for errlog.
 *	Add rundown flag to boot() stop recursive panics.
 *
 * 02 Jan 86 -- darrell
 *	Removed 8600 memory array callout.  
 *
 *  2-jan-86 -- rjl
 *	Fixed single user kit login limits for uVAXen
 *
 * 23-sep-85 -- jaw
 *	fixed microdelay hang bug.
 *	VAX8200 must do "halt" on reboot system call so the message
 *	buffer remains in memory.
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code and lockf call.
 *
 * 15 Aug 85 -- darrell
 *	Removed a debug printf I left in, and fixed a spelling error.
 *
 * 26-jul-85 -- jaw
 *	fixed SRM violation....TBIA data MBZ.	
 *
 * 03-Jul-85 -- rjl
 *	Fixed calculation of machinecheck number for uVAX-II checks in
 *	the range of 80-83
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06-Jun-85 -- jaw
 *	cleanup for 8200.
 *
 * 18-May-85 -- rjl
 *	Fixed dump code so that the uVAX-II would reboot after a dump
 *	attempt.
 *
 *  5-May-5  - Larry Cohen
 *	decrease number of clists to reflect larger cblocks
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 29-Oct-84 tresvik
 *
 *	added recover from cache parity errors on the 11/750.
 *	added cache-on and cache-off functionality for machinecheck
 *	handling
 *	added timer to TB parity errors for 11/750.
 *	modified MAXUSERS to maxusers for MVAX for rjl
 *
 * 22-Feb-84 tresvik
 *
 *	fixed distributed bug in detecting 11/750 tb parity errors.
 *
 *	changed all printfs used to report memory failures to mprint's.
 *	added code to display on the console any hard memory failures
 *	detected after machine checks.
 *
 * In the beginning - tresvik
 *
 *	changed format of and corrected machine check logging.
 *	Corrections to the 730 list were made and addition of 750
 *	summary list.
 *
 *	added MS780E support.
 *
 *	changed format of memerr reporting for corrected ecc errors.
 *	This includes failing array call out.
 *
 *  9 Jan 84 --jmcg
 *	Need to alter validation of ptes for buffers to match new
 *	allocation scheme in binit.
 *
 *  2 Jan 84 --jmcg
 *	Added support for MicroVAX 1.
 *
 ************************************************************************/

/*	machdep.c	6.1	83/08/20	*/

#include "mba.h"

#include "../machine/reg.h"
#include "../machine/pte.h"
#include "../machine/psl.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/mount.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/reboot.h"
#include "../h/conf.h"
#include "../h/inode.h"
#include "../h/gnode.h"
#include "../h/file.h"
#include "../h/text.h"
#include "../h/clist.h"
#include "../h/callout.h"
#include "../h/cmap.h"
#include "../h/mbuf.h"
#include "../h/quota.h"
#include "../h/flock.h"
#include "../h/errlog.h"
#include "../h/cpudata.h"
#include "../h/cpuconf.h"
#include "../h/ioctl.h"
#include "../h/dump.h"
#include "../ufs/fs.h"

#include "../vax/frame.h"
#include "../vax/cons.h"
#include "../vax/cpu.h"
#include "../vax/mem.h"
#include "../vax/mtpr.h"
#include "../vax/scb.h"
#include "../vax/clock.h"
#include "../vax/rpb.h"
#include "../vax/nexus.h"
#include "../vax/ioa.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxmba/mbavar.h"
#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"
#include "../sas/vmb.h"


int	icode[] = {

	0x9f19af9f,				/* pushab [&"init",0]; pushab */
	0x02dd09af,				/* "/etc/init"; pushl $2 */
	0xbc5c5ed0,				/* movl sp,ap; chmk */
	0x2ffe110b,				/* $exec; brb .; "/ */
	0x2f637465,				/* etc/ */
	0x74696e69,				/* init" */
	0x00000000,				/* \0\0\0";  0 */
	0x00000014,				/* [&"init", */
	0x00000000,				/* 0] */
};

int	szicode = sizeof (icode);

struct uba_driver *uba_drive;
struct uba_hd	  *uba_head;
struct uba_device *uba_dev;

struct mba_driver *mba_drive;
struct mba_hd	  *mba_head;
struct mba_device *mba_dev;


/*
 * Define the MINIMUN amount of memory that Ultrix supports.
 * This info is used if the system page table is too small and
 *    we reduce physmem and try again.
 * These 2 defines give the value in both (512 byte) pages,
 *    and in megabytes.
 */
#define MINMEM_MB 3
#define MINMEM_PGS (2048*MINMEM_MB)

/* These global variables must be in data space */

int	full_dumpmag = 0x8fca0101;	/* full dump magic number */
int	partial_dumpmag = 0x8fca0104;	/* partial dump magic number */
int	dumpmag = 0;			/* magic number for savecore */
int	dumpsize = 0;			/* also for savecore */
int	elbuf_offset = 0;		/* page offset where elbuf exists */
int	uarea_phys = 0;			/* physical address of u area */
caddr_t endofmem = 0;			/* last kernel virtual mem addr */

long	num_dmem_pages;			/* number of pages for kernel memory
					   allocator (total allocatable) */

/*
 * Declare these as initialized data so we can patch them.
 */
int	nbuf = 0;
int	nswbuf = 0;
int	bufpages = 0;
int	sysptsize = 0;
int	Physmem = 0;

/*
 * Cache state (enable/disable)
 */
int	cache_state;

/*
 * Set up when using the VMB boot path for CI support and dump support
 */
int	*ci_ucode = 0;		/* If present, points to CI ucode */
int	ci_ucodesz = 0;		/* If present, size of CI ucode */
int	*vmbinfo = 0;		/* gets a physical address set in locore
				   which is passed in by the VMB boot path */

/*
 * Machine-dependent startup code
 */
startup (firstaddr)
int	firstaddr;
{
	register int	unixsize;
	register unsigned   i;
	register struct pte *pte;
	int	mapaddr, j;
	register caddr_t v;
	int	maxbufs, base, residual;
	extern	char etext;
	int savefirstaddr;
	int maxclistusers;
	int vmbinfosz;


	extern int setptsize();
	extern int nproc;
	extern int ntext;
	extern int ngnode;
	extern int nfile;
	extern int nclist;
	extern int ncallout;
	extern int nchrout;
	extern int nport;
	extern int maxusers;
	extern int appendflg;
#ifdef QUOTA
	extern int nquota;
	extern int ndquot;
#endif QUOTA
	int tries = 0;	/* number of times we have tried to reconfigure */
#ifndef UPGRADE
#define UPGRADE 0
#endif
	char x[UPGRADE+1];

#ifdef INET
#define NETSLOP 20			/* for all the lousy servers*/
#else
#define NETSLOP 0
#endif

	/*  
	 *  At this point the IPL MUST be 1f (hex).   The following
         *  code is intended to clean up any interrupts between 
         *  ipl 16 and 1f.  Note that the printf rountine sets the ipl
	 *  to 16.	
	 */

	if (cpu == VAX_8800) 
		mtpr(CLRTOSTS,1);

	/*
	 * If we used adb on the kernel and set "Physmem" to some
	 *   value, then use that value for "physmem".
	 * This allows us to run in less physical memory than the
	 *   machine really has. (rr's idea).
	 */
	if (Physmem >= MINMEM_PGS) {
		physmem = Physmem;
	}
	/*
	 * Save the real amount of memory (or the above artificial
	 * amount) to allow the sizer to accurately configure physmem
	 * in the config file.  `Physmem' was originally added for the
	 * sizer utility - tomt
	 */
	Physmem = physmem;

	savefirstaddr = firstaddr;
	/*
	 * Initialize system page table size .
	 */
	setptsize();  /* set the size of sysptsize as defined in spt.s */
tryagain:
	/* The first time through the following
	 * variables should be the same value as determined
	 * in param.c .
	 * If the system page table is too small for the available
	 * physical memory, the physical memory (physmem) is artifically
	 * reduced to MINMEM_MB megabytes.
	 * This should allow booting generic kernels on systems with
	 * large physical memory.
	 * After booting successfully more accurate configuration parameters
	 * should be specified in the config file and the system rebooted.
	 */
	firstaddr = savefirstaddr;
	maxclistusers = (maxusers > 75) ? 75 : maxusers;
	nclist = 75 + 16 * maxclistusers;
	nchrout = 4096;
	if(tries != 0) {
		
		nproc = 20 + 8 * maxusers;
		ntext = 24 + maxusers + NETSLOP;
		ngnode = (nproc + 16 + maxusers) + 32 + ntext;
		nfile = 16 * (nproc + 16 + maxusers) / 10 + 32 + 2 * NETSLOP;
		ncallout = 16 + nproc;
		nport = nproc / 2;
#ifdef QUOTA
		nquota = (maxusers * 9)/7 + 3;
		ndquot = (maxusers*NMOUNT)/4 + nproc;
#endif
	}
	
	/* 
	 * The following algorithm should be the same in the system page
	 * table area: spt.s.  If change the algorithm here it should
	 * be changed in genassym.c .
	 */
	nmbclusters = nmbclusters * (maxusers/32 + 2);


	/*
	 * Initialize vmb information (at end of core).
	 * Size the information, adjust maxmem accordingly and map it into
	 * the kernel.  Flush the translation buffer.
	 */

	/*
	 * let maxmem be real memory as declared in locore
	 * until vmb_info is mapped in
	 */
	freemem=physmem;		/* should be the same at this point */
	if (vmbinfo && !tries) {	/* if VMB boot path and not a retry */
 		extern char Sysbase[];
		int *ip;

		vmbinfosz = maxmem - btop(vmbinfo);
		maxmem -= vmbinfosz;
		pte = vmbinfomap;
		for (i=0; i<vmbinfosz; i++)
			*(int *) pte++ = PG_V | PG_KW | (maxmem + i);
		mtpr (TBIA,0);
		if (vmb_info.ciucodebas && vmb_info.ciucodesiz) {
			ci_ucode = (int *)&vmb_info +
				(vmb_info.ciucodebas - vmbinfo);
			ci_ucodesz = vmb_info.ciucodesiz;
		}
		/*
		 * write enable the RPB
		 */
 		ip = (int *)Sysmap; *ip &= ~PG_PROT; *ip |= PG_KW;
 		mtpr(TBIS, Sysbase);	
		/*
		 * plug in the address of vmb_info
		 */
		rpb.vmbinfo = (long *)&vmb_info;
		/*
		 * plug in the cpu type and cpu subtype while the RPB 
		 * is still writeable
		 */
		rpb.cpu = cpu;
		rpb.cpu_subtype = cpu_subtype;
		/*
		 * write protect the RPB again
		 */
 		ip = (int *)Sysmap; *ip &= ~PG_PROT; *ip |= PG_KR;
 		mtpr(TBIS, Sysbase);	
	}
	/* 
	 * If physmem has been artificially reduced, set maxmem to be
	 * the same.  Otherwise, don't touch it.
	 */
	if (maxmem > physmem)		
		maxmem = physmem;

 /*
  * Good {morning,afternoon,evening,night}.
  */
#ifdef MVAX
	/*
	 * Setup the virtual console. vcons_init is an array of pointers
	 * to initialization functions in the qv/qdss drivers. They 
	 * return true if they could setup as the console.  This loop
	 * stops at the end of the list or when it finds a driver that
	 * succeeds in setting up the screen. Precidence is determined by
	 * order as is everything in life.
	 */
	if( cpu == MVAX_I || cpu == MVAX_II ) {
		extern (*vcons_init[])();

		for( i = 0 ; vcons_init[i] && (*vcons_init[i])() == 0 ; i++ )
			;
	}
#endif MVAX


	/*
	 * Allocate space for system data structures.
	 * The first available real memory address is in "firstaddr".
	 * As pages of memory are allocated, "firstaddr" is incremented.
	 * The first available kernel virtual address is in "v".
	 * As pages of kernel virtual memory are allocated, "v" is incremented.
	 * An index into the kernel page table corresponding to the
	 * virtual memory address maintained in "v" is kept in "mapaddr".
	 */
	mapaddr = firstaddr;
	v = (caddr_t) (0x80000000 | (firstaddr * NBPG));
#define valloc(name, type, num) \
		(name) = (type *)(v); (v) = (caddr_t)((name)+(num))
#define valloclim(name, type, num, lim) \
		(name) = (type *)(v); (v) = (caddr_t)((lim) = ((name)+(num)))
	valloc (istack, char, (NISP + 1) * NBPG * (maxcpu - 1));
	valloclim (gnode, struct gnode , ngnode, gnodeNGNODE);
	valloclim (file, struct file   , nfile, fileNFILE);
	valloclim (proc, struct proc   , nproc, procNPROC);
	valloclim (text, struct text   , ntext, textNTEXT);
	valloc (cfree, struct cblock   , nclist);
	valloc (callout, struct callout, ncallout);
	valloc (chrout, struct chrout, nchrout);
	valloc (swapmap, struct map, nswapmap = nproc * 2);
	valloc (argmap, struct map , ARGMAPSIZE);
	valloc (kernelmap, struct map  , nproc);
	valloc (mbmap, struct map  , nmbclusters / 4);
	valloc (nch, struct nch, nchsize);
 	valloc (flox, struct filock, flckinfo.recs );
 	valloc (flinotab, struct flino, flckinfo.fils );
	valloc (dmemmap, struct map, 512);
#ifdef QUOTA
	valloclim (quota, struct quota , nquota, quotaNQUOTA);
	valloclim (dquot, struct dquot , ndquot, dquotNDQUOT);
#endif QUOTA
	/*
	 * Determine how many buffers to allocate.
	 * Use 10% of memory. Insure a minimum of 16 buffers.
	 * We allocate 1/2 as many swap buffer headers (nswbuf)
	 * as file i/o buffers (nbuf).
	 */
	if (bufpages == 0)
		bufpages = physmem / 10 / CLSIZE;
	if (nbuf == 0) {
		nbuf = bufpages / 2;
		if (nbuf < 16)
			nbuf = 16;
	}
	if (nswbuf == 0) {
		nswbuf = (nbuf / 2) &~ 1;	/* force even */
		if (nswbuf > 256)
			nswbuf = 256;		/* sanity */
	}
	valloc(swbuf, struct buf, nswbuf);
	/*
	 * Now the amount of virtual memory remaining for buffers
	 * can be calculated, estimating needs for the cmap.
	 */
	ncmap = (maxmem*NBPG - ((int)v &~ 0x80000000)) /
		(CLBYTES + sizeof(struct cmap)) + 2;
	maxbufs = ((sysptsize * NBPG) -
	    ((int)(v + ncmap * sizeof(struct cmap)) - 0x80000000)) /
		(MAXBSIZE + sizeof(struct buf));
	/*
	 * If the system page table is too small for the available
	 * physical memory (not enough room for nbufs after the core map)
	 * then the size of physical memory (physmem) is artifically
	 * reduced to MINMEM_MB megabytes (the minimun amount supported).
	 * This should allow booting generic kernels on systems with
	 * large physical memory.
	 */
	if (maxbufs < 16) {
		printf("System page table too small, reducing physmem to %d meg\n",MINMEM_MB);
		if (tries++ > 1)
			panic ("sys pt too small");
		physmem=MINMEM_PGS; /* # of 512 byte pages = MINMEM_MB */
		maxusers=(maxusers > 8) ? 8 : maxusers;
		nbuf=bufpages=nswbuf=0;
		goto tryagain;
	}
	if (nbuf > maxbufs) {
/*
		printf("sysptsize limits number of buffers to %d\n", maxbufs);
*/
		nbuf = maxbufs;
	}
	if (bufpages > nbuf * (MAXBSIZE / CLBYTES))
		bufpages = nbuf * (MAXBSIZE / CLBYTES);
	valloc(buf, struct buf, nbuf);
	/*
	 * Allocate space for core map.
	 * Allow space for all of phsical memory minus the amount 
	 * dedicated to the system. The amount of physical memory
	 * dedicated to the system is the total virtual memory of
	 * the system thus far, plus core map, buffer pages,
	 * and buffer headers not yet allocated.
	 * Add 2: 1 because the 0th entry is unused, 1 for rounding.
	 */
	ncmap = (maxmem*NBPG - ((int)(v + bufpages*CLBYTES) &~ 0x80000000)) /
		(CLBYTES + sizeof(struct cmap)) + 2;
	valloclim(cmap, struct cmap, ncmap, ecmap);
	/*
	 * Clear space allocated thus far, and make r/w entries
	 * for the space in the kernel map.
	 */
	unixsize = btoc((int)v &~ 0x80000000);
	while (firstaddr < unixsize) {
		*(int *)(&Sysmap[firstaddr]) = PG_V | PG_KW | firstaddr;
		clearseg((unsigned)firstaddr);
		firstaddr++;
	}
	/*
	 * Now allocate buffers proper.  They are different than the above
	 * in that they usually occupy more virtual memory than physical.
	 * Set endofmem to the last used kernel virtual address.
	 */
	v = (caddr_t) ((int)(v + PGOFSET) &~ PGOFSET);
	valloc(buffers, char, MAXBSIZE * nbuf);
	endofmem = v;
	base = bufpages / nbuf;
	residual = bufpages % nbuf;
	mapaddr = firstaddr;
	for (i = 0; i < residual; i++) {
		for (j = 0; j < (base + 1) * CLSIZE; j++) {
			*(int *)(&Sysmap[mapaddr+j]) = PG_V | PG_KW | firstaddr;
			clearseg((unsigned)firstaddr);
			firstaddr++;
		}
		mapaddr += MAXBSIZE / NBPG;
	}
	for (i = residual; i < nbuf; i++) {
		for (j = 0; j < base * CLSIZE; j++) {
			*(int *)(&Sysmap[mapaddr+j]) = PG_V | PG_KW | firstaddr;
			clearseg((unsigned)firstaddr);
			firstaddr++;
		}
		mapaddr += MAXBSIZE / NBPG;
	}
/*
	printf("End of kernel real memory = %d, End of kernel virtual memory = %d\n",
		firstaddr*NBPG, ((int)endofmem & ~ 0x80000000));
	printf("Sys virt pgs=%d, sys real pgs=%d, #sys pte's=%d, #of sys ptes used = %d\n",
    	((int)(v + 1) & ~0x80000000) / NBPG, firstaddr*NBPG, sysptsize, mapaddr);
*/
	if (firstaddr >= physmem - 8 * UPAGES)
		panic ("no memory");
	mtpr (TBIA, 0);

 /*
  * Initialize callouts
  */
	callfree = callout;
	for (i = 1; i < ncallout; i++)
		callout[i - 1].c_next = &callout[i];
 /*
  * Initialize interrupt queue
  */
	chrfree = chrcur = &chrout[0];
	for (i = 1; i < nchrout; i++)
		chrout[i - 1].c_next = &chrout[i];
	chrout[nchrout - 1].c_next = &chrout[0]; /* circular llist */

 /*
  * Initialize memory allocator and swap
  * and user page table maps.
  *
  * THE USER PAGE TABLE MAP IS CALLED ``kernelmap''
  * WHICH IS A VERY UNDESCRIPTIVE AND INCONSISTENT NAME.
  */
	meminit (firstaddr, maxmem);
	maxmem = freemem;

	rminit (kernelmap, (long) USRPTSIZE, (long) 1,
		"usrpt", nproc);
	rminit (mbmap, (long) ((nmbclusters - 1) * CLSIZE), (long) CLSIZE,
		"mbclusters", nmbclusters / 4);
 /*
  * Initialize kernel memory allocator map.  For now, MVAX will be
  * initialized to MIN_DMEM_PAGES.  For the larger machines, initialize to
  * 10% of available pages upto MAX_DMEM_PAGES.  These are defined in
  * "../machine/vmparam.h"
  */
#ifdef MVAX
	if (cpu == MVAX_I || cpu == MVAX_II)
		num_dmem_pages =  MIN_DMEM_PAGES;
	else {
#endif MVAX
		num_dmem_pages = (long) (physmem / 10);
		if (num_dmem_pages < MIN_DMEM_PAGES)
			num_dmem_pages =  MIN_DMEM_PAGES;
		if (num_dmem_pages > MAX_DMEM_PAGES)
			num_dmem_pages = MAX_DMEM_PAGES;
#ifdef MVAX
	}
#endif MVAX
		rminit (dmemmap, num_dmem_pages, (long) 1, "dmempt", 512);

/*  
 * Log startup printfs
 *
*/

	appendflg = 1; 	/* tell error logger we're starting up */
	printf (version);
	printf ("real mem  = %d\n", ctob (physmem));
	printf ("avail mem = %d\n", ctob (maxmem));
#ifdef KM_DEBUG
	printf ("allocatable kernel mem = %d\n",ctob(num_dmem_pages));
#endif KM_DEBUG
	printf ("using %d buffers containing %d bytes of memory\n",
		nbuf, bufpages * CLBYTES);

/*
 * Configure the system.
 */
	configure ();

/*
 * Enable cache - (and floating point accelerator if necessary)
 */
	cachenbl();
	setcache (cache_state);

 /*
  * Clear restart inhibit flags.
  */
#ifdef VAX8800
	if (cpu==VAX_8800) {
		tocons (N_COMM|N_CLR_COLD);
		tocons (N_COMM|N_CLR_WARM);
	} else {
#endif
		tocons (TXDB_CWSI);
		tocons (TXDB_CCSI);
#ifdef VAX8800
	}	
#endif

}

/*
 * Enable Cache
 * 
 * The actual routines are entered through cpusw, and are located
 * in the appropiate cpu dependent routine kaXXX.c
 */

cachenbl()
{
	if ((*cpusw[cpu].v_cachenbl)() < 0 )
		panic("No cachenbl routine configured\n");
}

#ifdef PGINPROF
/*
 * Return the difference (in microseconds)
 * between the	current time and a previous
 * time as represented	by the arguments.
 * If there is a pending clock interrupt
 * which has not been serviced due to high
 * ipl, return error code.
 */
vmtime (otime, olbolt, oicr)
register int	otime, olbolt, oicr;
{

	if (mfpr (ICCS) & ICCS_INT)
		return (-1);
	else
		return (((time.tv_sec - otime) * 60 + lbolt - olbolt) * 16667 + mfpr (ICR) - oicr);
}
#endif

/*
 * Send an interrupt to process.
 *
 * Stack is set up to allow sigcode stored
 * in u. to call routine, followed by chmk
 * to sigcleanup routine below.  After sigcleanup
 * resets the signal mask and the stack, it
 * returns to user who then unwinds with the
 * rei at the bottom of sigcode.
 */
sendsig (p, sig, sigmask)
int	(*p) (), sig, sigmask;
{
	register struct sigcontext *scp;	/* know to be r11 */
	register int   *regs;
	register struct sigframe
		{
		int	sf_signum;
		int	sf_code;
		struct sigcontext  *sf_scp;
		int	(*sf_handler) ();
		struct sigcontext  *sf_scpcopy;
		} *fp;				/* known to be r9 */
	int	oonstack;

	regs = u.u_ar0;
	oonstack = u.u_onstack;
	scp = (struct sigcontext   *) regs[SP] - 1;
#define mask(s) (1<<((s)-1))
	if (!u.u_onstack && (u.u_sigonstack & mask (sig))) {
		fp = (struct sigframe  *) u.u_sigsp - 1;
		u.u_onstack = 1;
	}
	else
		fp = (struct sigframe  *) scp - 1;
 /*
  * Must build signal handler context on stack to be returned to
  * so that rei instruction in sigcode will pop ps and pc
  * off correct stack.	The remainder of the signal state
  * used in calling the handler must be placed on the stack
  * on which the handler is to operate so that the calls
  * in sigcode will save the registers and such correctly.
  */
	if (!oonstack && (int) fp <= USRSTACK - ctob (u.u_ssize))
		grow ((unsigned) fp);
	;
#ifndef lint
	asm ("probew $3,$20,(r9)");
	asm ("jeql bad");
#else
	if (useracc ((caddr_t) fp, sizeof (struct sigframe), 1))
		goto bad;
#endif
	if (!u.u_onstack && (int) scp <= USRSTACK - ctob (u.u_ssize))
		grow ((unsigned) scp);
	;					/* Avoid asm() label botch */
#ifndef lint
	asm ("probew $3,$20,(r11)");
	asm ("beql bad");
#else
	if (useracc ((caddr_t) scp, sizeof (struct sigcontext) , 1))
		goto bad;
#endif
	fp -> sf_signum = sig;
	if (sig == SIGSEGV || sig == SIGBUS || sig == SIGILL || sig == SIGFPE){
		fp -> sf_code = u.u_code;
		u.u_code = 0;
	}
	else
		fp -> sf_code = 0;
	fp -> sf_scp = scp;
	fp -> sf_handler = p;
 /*
  * Duplicate the pointer to the sigcontext structure.
  * This one doesn't get popped by the ret, and is used
  * by sigcleanup to reset the signal state on inward return.
  */
	fp -> sf_scpcopy = scp;
	/* sigcontext goes on previous stack */
	scp -> sc_onstack = oonstack;
	scp -> sc_mask = sigmask;
	/* setup rei */
	scp -> sc_sp = (int) & scp -> sc_pc;
	scp -> sc_pc = regs[PC];
	scp -> sc_ps = regs[PS];
	regs[SP] = (int) fp;
	regs[PS] &= ~(PSL_CM | PSL_FPD);
	regs[PC] = (int) u.u_pcb.pcb_sigc;
	return;

bad:
	asm ("bad:");
 /*
  * Process has trashed its stack; give it an illegal
  * instruction to halt it in its tracks.
  */
	u.u_signal[SIGILL] = SIG_DFL;
	sig = mask (SIGILL);
	u.u_procp -> p_sigignore &= ~sig;
	u.u_procp -> p_sigcatch &= ~sig;
	u.u_procp -> p_sigmask &= ~sig;
	psignal (u.u_procp, SIGILL);
}

/*
 * Routine to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 * Pop these values in preparation for rei which
 * follows return from this routine.
 */
sigcleanup () {
	register struct sigcontext *scp;

	scp = (struct sigcontext   *) fuword ((caddr_t) u.u_ar0[SP]);
	if ((int) scp == -1)
		return;
	;
#ifndef lint
	/* only probe 12 here because that's all we need */
	asm ("prober $3,$12,(r11)");
	asm ("bnequ 1f; ret; 1:");
#else
	if (useracc ((caddr_t) scp, sizeof (*scp), 0))
		return;
#endif
	u.u_onstack = scp -> sc_onstack & 01;
	u.u_procp -> p_sigmask =
	scp -> sc_mask & ~(mask (SIGKILL) | mask (SIGCONT) | mask (SIGSTOP));
	u.u_ar0[SP] = scp -> sc_sp;
}
#undef mask

#ifdef notdef
dorti () {
	struct frame	frame;
	register int	sp;
	register int	reg, mask;
	extern int  ipcreg[];

	(void) copyin ((caddr_t) u.u_ar0[FP], (caddr_t) & frame, sizeof (frame));
	sp = u.u_ar0[FP] + sizeof (frame);
	u.u_ar0[PC] = frame.fr_savpc;
	u.u_ar0[FP] = frame.fr_savfp;
	u.u_ar0[AP] = frame.fr_savap;
	mask = frame.fr_mask;
	for (reg = 0; reg <= 11; reg++) {
		if (mask & 1) {
			u.u_ar0[ipcreg[reg]] = fuword ((caddr_t) sp);
			sp += 4;
		}
		mask >>= 1;
	}
	sp += frame.fr_spa;
	u.u_ar0[PS] = (u.u_ar0[PS] & 0xffff0000) | frame.fr_psw;
	if (frame.fr_s)
		sp += 4 + 4 * (fuword ((caddr_t) sp) & 0xff);
	/* phew, now the rei */
	u.u_ar0[PC] = fuword ((caddr_t) sp);
	sp += 4;
	u.u_ar0[PS] = fuword ((caddr_t) sp);
	sp += 4;
	u.u_ar0[PS] |= PSL_USERSET;
	u.u_ar0[PS] &= ~PSL_USERCLR;
	u.u_ar0[SP] = (int) sp;
}
#endif

/*
 * this routine sets the cache to the state passed.  enabled/disabled
 * 
 * The actual routines are entered through cpusw, and are located
 * in the appropiate cpu dependent routine kaXXX.c
 */

setcache(state)
int state;
{
	if ((*cpusw[cpu].v_setcache)(state) < 0 )
		panic("No setcache routine configured\n");
}


/*
 * Memenable enables the memory controller corrected data reporting.
 * This runs at regular intervals, turning on the interrupt.
 * The interrupt is turned off, per memory controller, when error
 * reporting occurs.  Thus we report at most once per memintvl.
 * 
 * The actual routines are entered through cpusw, and are located
 * in the appropiate cpu dependent routine kaXXX.c
 */
int	memintvl = MEMINTVL;

memenable ()
{
	if ((*cpusw[cpu].v_memenable)() < 0 )
		panic("No memenable routine configured\n");
	else
	if (memintvl > 0)
		timeout (memenable, (caddr_t) 0, memintvl * hz);
}

/*
 * Memerr is the interrupt routine for corrected read data
 * interrupts.	It calls the apporpriate routine which looks
 * to see which memory controllers have unreported errors,
 * reports them, and disables further reporting for a time
 * on those controller.
 * 
 * The actual routines are entered through cpusw, and are located
 * in the appropiate cpu dependent routine kaXXX.c
 */
memerr ()
{
	if ((*cpusw[cpu].v_memerr)() < 0 )
		panic("No memory error handler configured\n");
}

/*
 * Invalidate single all pte's in a cluster
 */
tbiscl (v)
unsigned	v;
{
	register	caddr_t addr;		/* must be first reg var */
	register int	i;

	asm (".set TBIS,58");
	addr = ptob (v);
	for (i = 0; i < CLSIZE; i++) {
#ifdef lint
		mtpr (TBIS, addr);
#else
		asm ("mtpr r11,$TBIS");
#endif
		addr += NBPG;
	}
	tbsync();
}

int	waittime = -1;
int	shutting_down = 0;

boot (paniced, arghowto)
int	paniced, arghowto;
{
	register int	howto;			/* r11 == how to boot */
	register int	devtype;		/* r10 == major of root dev */
	struct cpusw *cpup;
	register struct mount *mp;
	register struct gnode *gp;
	register struct pte *ptep;
	extern struct gnode *acctp;
	extern struct gnode *savacctp;
	int s;
#ifdef lint
	howto = 0;
	devtype = 0;
	printf ("howto %d, devtype %d\n", arghowto, devtype);
#endif
	cpup = &cpusw[cpu];
	howto = arghowto;
	rundown++;
	shutting_down++;
	/*
	 * If accounting is on, turn it off. This allows the usr
	 * filesystem to be umounted cleanly.
	 */
	if (savacctp) {
		acctp = savacctp;
		savacctp = NULL;
	}
	if (gp = acctp) {
		(void) GRELE(gp);
		acctp = NULL;
	}

	if ((howto & RB_NOSYNC) == 0 && waittime < 0 && bfreelist[0].b_forw) {
		waittime = 0;
		(void) spl4 ();
		/*
		 * If a gnode has been accessed, changed, or modified,
		 * and the gnode has a mount device asscoiated with it,
		 * update the gnode.
		 */
		for(gp = &gnode[ngnode]; --gp >= gnode;) {
			if ((gp->g_flag & GUPD || gp->g_flag & GACC ||
			    gp->g_flag & GCHG || gp->g_flag & GMOD) &&
			    (gp->g_mp && (gp->g_mp->m_dev != NODEV))) {	
				(void) GUPDATE(gp, &time, &time, 1, u.u_cred);
			}
		}
		if(paniced != RB_PANIC)
			for(mp = &mount[NMOUNT - 1]; mp >= mount; mp--) 
				if(mp->m_bufp) {
#ifdef GFSDEBUG
					extern short GFS[];
					if(GFS[6])
						printf("boot: doing mp 0x%x dev 0x%x flags 0x%x\n",
						mp, mp->m_dev,mp->m_flags);
						
#endif
					update (mp);
					if(mp != mount) /* unmount cleanly */
						GUMOUNT(mp, 1);
					else	/* MISTER ROOT cannot unmount */
						break;
				}
		printf ("syncing disks... ");
		/* 
		 * spl5 because we don't want to have a user program
		 * scheduled
		 */
		s = spl5();
		bhalt();
		splx(s);
		printf ("done\n");
	}
#if defined(VAX8600)
 	if(cpu == VAX_8600) {
 		register int i;
 		int *ip;
 		struct sbia_regs *sbiad;
 		extern int ioanum;
 		extern char Sysbase[];
 		sbiad = (struct sbia_regs *)ioa;
 		ip = (int *)Sysmap+1; *ip &= ~PG_PROT; *ip |= PG_KW;
 		mtpr(TBIS, Sysbase);	
 		for(i=0; i<ioanum; i++) {
 			if(BADADDR((caddr_t)sbiad, 4))
 				continue;
 			sbiad->sbi_unjam = 0;
 			sbiad = (struct sbia_regs *)((char *)sbiad + 
 				cpup->pc_ioasize);
 		}
 		ip = (int *)Sysmap+1; *ip &= ~PG_PROT; *ip |= PG_KR;
 		mtpr(TBIS, Sysbase);	
 	}
#endif VAX8600
	splx (0x1f);				/* extreme priority */
	devtype = major (rootdev);
	if (howto & RB_HALT) {
		mtpr (IPL, 0x1f);
#ifdef MVAX
		if( cpu == MVAX_II ) {
			if(cpu_subtype == ST_MVAXII)
			    ((struct qb_regs *)nexus)->qb_cpmbx = RB_HALTMD;
			if(cpu_subtype == ST_VAXSTAR)
			    ((struct nb_regs *)nexus)->nb_cpmbx = RB_VS_HALTMD;
			for (;;)
			asm ("halt");
		}
#endif MVAX
		printf ("\nTHE PROCESSOR CAN NOW BE HALTED.\n");
		for (;;);
	} else {
		if (paniced == RB_PANIC) {
			ptep = uaddr(u.u_procp);
			uarea_phys = (long)ptob(ptep->pg_pfnum);
			doadump ();		/* TXDB_BOOT's itsself */
			/* NOTREACHED */
		}

#ifdef VAX8800
		/* VAX8200 memory is killed by TXDB_BOOT! */
		if (cpu == VAX_8800) {
			tocons(N_COMM | N_BOOT_ME);
			asm("halt");
		}
#endif
#ifdef VAX8200
		if (cpu == VAX_8200) {		
			mtpr(MAPEN,0);
			rpb.rp_flag = 1;
			asm("halt");
		}
#endif
		tocons (TXDB_BOOT);
	}
#if defined(VAX750) || defined(VAX730) || defined(MVAX) || defined(VAX8200)
	if ((cpu != VAX_780) && (cpu != VAX_8600)) {
		asm ("movl r11,r5");
	}					/* boot flags go in r5 */
	if (cpu == MVAX_II) {
		if(cpu_subtype == ST_MVAXII)
			((struct qb_regs *)nexus)->qb_cpmbx = RB_REBOOT;
		if(cpu_subtype == ST_VAXSTAR)
			((struct nb_regs *)nexus)->nb_cpmbx = RB_VS_REBOOT;
	}
#endif
	for (;;)
	asm ("halt");
	/* NOTREACHED */
}

tocons (c)
{
	if ((*cpusw[cpu].v_tocons)(c) < 0 )
		panic("No tocons routine configured\n");
}

#define phys(number)	((number)&0x7fffffff)

int status;
char *start;			/* Index for partial dump */
int blk, bc;

int unit_ultrix;		/* unit plug num of dump dev */
int unit_physical;		/* physical unit num */
struct vmb_info *vmbinfop;	/* temp hold for physical vmbinfo */
struct rpb *rpbp;		/* system rpb */

long longp;

struct genericconf *gb;

gendump(dev, dumpinfo)
	dev_t dev;			/* dump device */
	struct dumpinfo dumpinfo;	/* dump info */
{

/*
 * If a partial dump is being performed, then num will be equal to the
 * size of the dump device less the size of the u_area plus the
 * size of the error logger buffer.
 * Room for these must be made available, so when
 * the system reboots, the error logging mechanism will be able to
 * report why the system went down.
 */

#define DBSIZE 16

	if (!vmbinfo) {
		cprintf("no vmbinfo structure exists for gendump\n");
		cprintf("calling drivers dump routine ");
		return (0);
	}

	/* Set gb pointing to correct entry in table */

	status = 0;
	for (gb = genericconf; gb->gc_driver; gb++)
		if (major(dev) == major(gb->gc_root)) {
			status = 1;
			break;
		}
	if (!status) {
		cprintf("dump device type not found in genconf table\n");
		cprintf("calling drivers dump routine ");
		return(0);
	}

	/* make rpbp equal to rpb in high memory, not low */

	vmbinfop = (struct vmb_info *)(phys((int)vmbinfo));
	rpbp     = (struct rpb      *)(vmbinfop->rpbbas);

	/* set unit equal to unit plug number of dump device */

	unit_ultrix = minor(dev) >> 3;
	unit_physical = -1;

	switch(rpbp->devtyp) {

	/*
	 * traverse unibus and massbus structures to determine physical
	 * unit number dump device is on.
	 */

	case BTD$K_MB:
#if (NMBA > 0)
		for (mba_dev =(struct mba_device *)mbdinit;mba_dev->mi_driver;mba_dev++) {
			if (mba_dev->mi_alive) {
				mba_drive = mba_dev->mi_driver;
				if (*mba_drive->md_dname == *gb->gc_name)
				if (mba_dev->mi_unit == unit_ultrix) {
					mba_head = mba_dev->mi_hd;
					longp = (long)mba_head->mh_physmba;
					if (longp == rpbp->adpphy) {
					  unit_physical=(long)mba_dev->mi_drive;
					  break;
					}
				}
			}
		}
#endif
		break;
	case BTD$K_BDA:
		for (uba_dev =(struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				longp = (long)uba_dev->ui_physaddr;
				longp &= 0xffffff00;
				if (longp == rpbp->csrphy) {
				   uba_drive = uba_dev->ui_driver;
				   if (*uba_drive->ud_dname ==*gb->gc_name)
				      if (uba_dev->ui_unit == unit_ultrix) {
				         unit_physical =(long)uba_dev->ui_slave;
					 break;
				      }
				}
			}
		}
		break;
	case BTD$K_DQ:
		for (uba_dev =(struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				uba_drive = uba_dev->ui_driver;
				if (*uba_drive->ud_dname ==*gb->gc_name)
				if (uba_dev->ui_unit == unit_ultrix) {
					uba_head = uba_dev->ui_hd;
					longp = (long)uba_head->uh_physuba;
					if (longp == rpbp->adpphy) {
						unit_physical = (long)uba_dev->ui_slave;
						break;
					}
				}
			}
		}
		break;
	default:
		for (uba_dev =(struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				if ((long)uba_dev->ui_physaddr ==rpbp->csrphy) {
					uba_drive = uba_dev->ui_driver;
					if (*uba_drive->ud_dname ==*gb->gc_name)
					if (uba_dev->ui_unit == unit_ultrix) {
					     uba_head = uba_dev->ui_hd;
					     longp = (long)uba_head->uh_physuba;
					     if ((longp == rpbp->adpphy) ||
						 (cpu_subtype == ST_VAXSTAR) ||
						 (cpu == MVAX_I)) {
					 	  unit_physical = (long)uba_dev->ui_slave;
						  break;
					     }
					}
				}
			}
		}
	} /* switch */

	if (unit_physical == -1) {
		cprintf("gendump can't determine dump devices slave #\n");
		cprintf("calling drivers dump routine\n");
		return(0);
	}

	/*
	 * The restart parameter block rpbp points to is used by the
	 * qio calls, qioinit and qio. Set the physical unit number
	 * in the rpb to the dump devices physical unit plug.
	 */

	rpbp->unit = unit_physical;

	/* Call the drivers init routine, if one exists. */

	status = qioinit();
	if (status & 1)
		start = 0;
	else {
		cprintf("dump device not ready\n");
		return(0);
	}

	dumpinfo.blkoffs += dumplo;

	/*
	 * If a full dump is being performed, then this loop
	 * will dump all of core. If a partial dump is being
	 * performed, then as much of core as possible will
	 * be dumped, leaving room for the u_area and error logger
	 * buffer. Please note that dumpsys predetermined what
	 * type of dump will be performed.
	 */

	while ((dumpinfo.size_to_dump > 0) || dumpinfo.partial_dump) {
		blk = dumpinfo.size_to_dump > DBSIZE ? DBSIZE : dumpinfo.size_to_dump;
		bc  = blk * NBPG;
		status = qio(PHYSMODE,
		 	     IO$_WRITELBLK,
			     dumpinfo.blkoffs,
			     bc,
			     start);
		if (status & 1) {
			start += bc;
			dumpinfo.size_to_dump -= blk;
			dumpinfo.blkoffs += blk;
		}
		else {
			cprintf("dump i/o error: bn = %d, ", dumpinfo.blkoffs);
			if (status == SS$_CTLRERR)
				cprintf("fatal controller error\n");
			else
				cprintf("dump driver status = 0x%x\n", status);
			return(0);
		}
		if ((dumpinfo.size_to_dump <= 0) && (dumpinfo.partial_dump)) {
			
			/*
			 * If a partial dump is being performed....
			 */

			/* Set size_to_dump to the number of pages to dump */
			dumpinfo.size_to_dump = 
			   dumpinfo.pdump[NUM_TO_DUMP-dumpinfo.partial_dump].num_blks;
			/* Set start to starting address */
			start = 0;
			start += 
			   dumpinfo.pdump[NUM_TO_DUMP-dumpinfo.partial_dump].start_addr;
			dumpinfo.partial_dump--;
		}
	}
	return(1);
}

struct pt *parttbl;
struct dumpinfo dumpinfo;
int flg;
int status;			/* temp hold */
int blk, bc, maxsz;
int ind;			/* temp loop index */
int save_size = 0;		/* temp value hold */

extern int nodev();		/* check for no dump routine */

/*
 * Doadump comes here after turning off memory management and
 * getting on the dump stack, either when called above, or by
 * the auto-restart code.
 */
dumpsys ()
{

	rpb.rp_flag = 1;
#ifdef notdef
	if ((minor (dumpdev) & 07) != 1)
		return;
#endif
	dumpsize = physmem;

	/*
	 * Perform ioctl call to get partition information.
	 */

	status = bdevsw[major(dumpdev)].d_ioctl
	     (dumpdev, DIOCGETPT, (struct pt *)parttbl, flg);
	if (status) {
		cprintf("dump device ioctl failed\n");
		return;
	}
	 
	if (parttbl->pt_part[minor(dumpdev)&07].pi_nblocks <= 0) {
		cprintf("dump area improper\n");
		return;
	}

	/*
	 * Set size_to_dump to number of pages in core.
	 */

	dumpinfo.size_to_dump = maxfree;

	/*
	 * Set maxsz to the size of the dump device in blocks.
	 */

	maxsz   = parttbl->pt_part[minor(dumpdev)&07].pi_nblocks;

	/*
	 * Set blkoffs to the beginning of the dump device in blocks.
	 */

	dumpinfo.blkoffs = parttbl->pt_part[minor(dumpdev)&07].pi_blkoff;

	if (dumplo < 0) {
		cprintf("dump offset improper\n");
		return;
	}

	/*
	 * Initialize partial dump switch in dumpinfo
	 * struct.
	 */

	dumpinfo.partial_dump = 0;

	/*
	 * Check to see if a partial dump has to be performed.
	 */

	if (dumpinfo.size_to_dump + dumplo >= maxsz) {   /* Partial dump */
		/* set partial dump switch */
		dumpinfo.partial_dump = NUM_TO_DUMP;
		/*
		 * Calculate size of u area in pages
		 */
		dumpinfo.pdump[0].num_blks = UPAGES;
		/*
		 * Starting address of u area calculated in boot()
		 */
		dumpinfo.pdump[0].start_addr = uarea_phys;
		/*
		 * Calculate the size of the error logger buffer.
		 */
		dumpinfo.pdump[1].num_blks = EL_DUMPSIZE / NBPG;
		if ((EL_DUMPSIZE % NBPG) != 0)
			dumpinfo.pdump[1].num_blks++;
		/*
		 * Calculate beginning address of elbuf
		 */
		dumpinfo.pdump[1].start_addr = phys((int)&elbuf);

		/*
		 * Set dumpsize equal to the amount to be dumped,
		 * including the error logger buffer.
		 */
		dumpsize = maxsz - (dumplo + 1);
		/*
		 * Since we can't dump all of core onto the dump device,
		 * we must calculate the amount we can save, leaving room
		 * for the u area and the error logger buffer elbuf.
		 */
		for (ind = 0; ind < NUM_TO_DUMP; ind++)
			save_size += dumpinfo.pdump[ind].num_blks;
		dumpinfo.size_to_dump = dumpsize - save_size;
	}

	cprintf ("\ndumping to dev %x, offset %d\n", dumpdev, dumplo);

	if (dumpinfo.partial_dump) {
		cprintf("Partial dump of %d pages ", dumpsize);
		dumpmag = partial_dumpmag;   /* partial dump sig for savecore */
	}
	else {
		cprintf("Dump of %d pages ", dumpinfo.size_to_dump);
		dumpmag = full_dumpmag;	     /* full dump signal for savecore */
	}

	/*
	 * If there is a possibility that the dump and boot device
	 * are on the same controller, call the generic dump
	 * driver.
	 */

	if (major(dumpdev) == major(rootdev)) {
		/*
		 * If the generic dump driver is being used, then the offset
		 * within the page where elbuf will be relocated to if a
		 * partial dump is being performed will be zero. This is
		 * because the generic dump driver dumps memory
		 * addresses, not pages of memory like other dump routines.
		 */
		if (dumpinfo.partial_dump)
			elbuf_offset = 0;
		status = gendump(dumpdev, dumpinfo);

		/*
		 * A return code of -1 means a fatal error
		 * occurred in the generic dump driver.
		 */

		if (status == -1)
			cprintf("Entire dump aborted\n");
		else
		if (status == 1)
			cprintf("successful\n");

		/*
		 * If the generic dump driver returned a 0,
		 * then it couldn't determine if the dump
		 * and boot devices were on the same controller.
		 */
	}
	if (status == 0) {

		/*
		 * Other dump drivers are limited to dumping pages
		 * of memory. In order for savecore to know the
		 * exact location of elbuf in case of a partial dump,
		 * the global variable elbuf_offset will be set.
		 * It's value will be read by savecore to determine
		 * the offset within the page of memory to extract
		 * elbuf.
		 */
		if (dumpinfo.partial_dump) {
			elbuf_offset = dumpinfo.pdump[1].start_addr % NBPG;
			if (elbuf_offset) {
				/*
				 * If elbuf was not sitting on a page
				 * boundry, then we must make sure
				 * we leave enough room for the
				 * extra page we must dump.
				 */
				dumpinfo.size_to_dump--;
				dumpinfo.pdump[1].num_blks++;
			}
		}
		if (bdevsw[major(dumpdev)].d_dump == nodev)
			cprintf("\nDump routine does not exist\n");
		else	
		switch ((*bdevsw[major (dumpdev)].d_dump)
			(dumpdev, dumpinfo)) {

			case ENXIO:
				cprintf ("\ndump device bad\n");
				break;

			case EFAULT:
				cprintf ("\ndump device not ready\n");
				break;

			case EINVAL:
				cprintf ("\ndump area improper\n");
				break;

			case EIO:
				cprintf ("\ndump i/o error\n");
				break;

			default:
				cprintf (" succeeded\n");
				break;
		} /* switch */
	}  /* status */
#ifdef MVAX
#define CPMBX	 0x200B801c	/* MVAXII physical addr of console prm com */
#define VS_CPMBX 0x200B0038	/* VAXstar physical addr of console prm com */
	/*
	 * Clear restart in progress flag in cpmbx register and tell it
	 * to reboot.
	 */
	if( cpu == MVAX_II ) {
		if (cpu_subtype == ST_MVAXII)
			*(u_short *)CPMBX = RB_REBOOT;
		if (cpu_subtype == ST_VAXSTAR)
			*(u_long *)VS_CPMBX = RB_VS_REBOOT;
	}
#endif MVAX
}

/*
 * Machine check handlers.
 * 
 * The actual routines are entered through cpusw, and are located
 * in the appropiate cpu dependent routine kaXXX.c
 */

machinecheck (cmcf)
caddr_t cmcf;
{
	if ((*cpusw[cpu].v_machcheck)(cmcf) < 0 )
		panic("No machine check handler configured\n");
}

/*
 * delay for n microseconds, limited to somewhat over 2000 microseconds
 */

microdelay(n)
int n;
{
	/*
	 * int n above can't be a register int because it blows
	 * away r11 which is kept throughout autoconf time.  MVAX will
	 * delay even longer now that it is a memory location
	 */
	struct timeval et, nowt;
	int saveiccs,s;

	/*
	 * For VAXstation 2200 (AKA, VAXstar) and MICROVAX 1800 (AKA, TEAMmate),
	 * measurements with 1 second granularity (using TOY seconds register)
	 * show a delay of n = 10000000 (10 sec) yields an actual delay
	 * between 11 and 12 seconds (+ 10 to 20 %). -- Fred Canter 8/30/86
	 */
	if ((cpu == MVAX_I) || (cpu == MVAX_II)) {
		if (cpu == MVAX_I)
			n /= 6; 		/* we're slow */
		else
			n /= 2;
		while (--n >= 0);		/* sorry, no ICR */
	} else {
		/* if clock not enabled or ipl above 0x17 */
		/* change so if ipl > 0x15.  this was change to VAX SRM */
		if ( !(mfpr(ICCS) & ICCS_RUN) || (mfpr(IPL) >= 0x16)) {
			s=spl6();
			saveiccs = mfpr(ICCS);	/* save value */
			mtpr(NICR, -n); 	/* load neg n */
			mtpr(ICCS, ICCS_RUN+ICCS_TRANS+ICCS_INT+ICCS_ERR);
			while ( !(mfpr(ICCS) & ICCS_INT));	/* wait */

			/* restore interval counter to previous state */
			mtpr(NICR,-1000000/hz);
			mtpr(ICCS, saveiccs+ICCS_TRANS+ICCS_ERR); /*restore*/

			splx(s);

		} else {
			/* clock is running so call mircotime */
			microtime(&et);
			et.tv_sec += n/1000000;
			et.tv_usec += n%1000000;
			if( et.tv_usec > 1000000) {
				et.tv_usec -= 1000000;
				et.tv_sec++;
			}
			do
				microtime(&nowt);
			while ( nowt.tv_sec < et.tv_sec || 
				(nowt.tv_usec < et.tv_usec && nowt.tv_sec <= et.tv_sec));
		}
	}
}

microtime (tvp)
struct timeval *tvp;
{
	int	s = spl6 ();

	tvp -> tv_sec = time.tv_sec;
	tvp -> tv_usec = time.tv_usec + (1000000/hz) + mfpr (ICR);
	while (tvp -> tv_usec > 1000000) {
		tvp -> tv_sec++;
		tvp -> tv_usec -= 1000000;
	}
	splx (s);
}

physstrat (bp, strat, prio)
register struct buf *bp;
int	(*strat) (), prio;
{
	int	s;

	(*strat) (bp);
	/* pageout daemon doesn't wait for pushed pages or N-buffered */
	if (bp->b_flags & (B_DIRTY|B_RAWASYNC))
		return;
	s = spl6 ();
	while ((bp->b_flags & B_DONE) == 0)
	sleep ((caddr_t) bp, prio);
	splx (s);
}

extern int cold;

badaddr(addr,len) 
caddr_t addr;
int len;
{
	int status,s;
	int *ip;

	if (cold) status=(((*cpusw[cpu].v_badaddr)(addr, len)));
	
	else {
 		ip = (int *)Sysmap+ (btop(((int)&scb.scb_stray)&0x7fffffff));
		*ip &= ~PG_PROT; 
		*ip |= PG_KW;
 		mtpr(TBIS, Sysbase);	

		s=spl7();
		switch(cpu) {

		case VAX_8600:
		case VAX_780:
			ubaclrint(); 
			status=(((*cpusw[cpu].v_badaddr)(addr, len)));
			status|=ubasetint(); 
			break;

		case VAX_8800:
		case VAX_8200:
			biclrint();
			status=(((*cpusw[cpu].v_badaddr)(addr, len)));
			bisetint();
			break;

		default:
			status=(((*cpusw[cpu].v_badaddr)(addr, len)));
			break;
		}
 		*ip &= ~PG_PROT; *ip |= PG_KR;
 		mtpr(TBIS, &scb.scb_stray);	
		splx(s);
	}

	return(status);

}

extern int nNUBA;

ubaclrint() {
	struct uba_regs *ubap;
	int i;

	for (i=0; i<nNUBA; i++) {
		
		if (ubap = uba_hd[i].uh_uba) 
			ubap->uba_sr=ubap->uba_sr;
	
	}
}


ubasetint() {
	struct uba_regs *ubap;
	int i,ubaerror;
	
	ubaerror=0;
	for (i=0; i<nNUBA; i++) {
		if (ubap = uba_hd[i].uh_uba) {
		
			if(ubap->uba_sr) ubaerror=1;
			ubap->uba_sr=ubap->uba_sr;
			ubap->uba_cr=	UBACR_IFS | UBACR_BRIE |
					UBACR_USEFIE | UBACR_SUEFIE |
					(ubap->uba_cr & 0x7c000000);
		}
	}
	return(ubaerror);
}
