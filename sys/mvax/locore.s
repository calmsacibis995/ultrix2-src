
/*
 *	*sccsid = "@(#)locore.s	1.66	(ULTRIX)	3/3/87";
 *	4.2BSD locore.s  6.3  83/08/12
 */

/************************************************************************
 *									*
 *			Copyright (c) 1983,86 by			*
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
 *
 * Modification history: /sys/vax/locore.s
 *
 * 03 Feb 87 -- depp and woodward
 *	Reinstated the "yech" code in Swtch()/Resume().  It was removed
 *	during the multiprocessing work.  It will only be invoked on the
 *	master CPU.
 *
 * 21 Jan 87 -- jaw
 *	performance fixes to syscall.
 *
 *  16-Dec-86 -- pmk
 *	Changed 750 console tu58 intr. branch to turintr instead of tudma.
 *	750 now uses MRSP protocol
 *
 *  27-Aug-86 -- fred (Fred Canter)
 *	Removed kludge used to identify pass 1 standard cell VAXstar CPUs.
 *
 *  5-Aug-86 -- fred (Fred Canter)
 *	Minor changes to VAXstar SLU pseudo DMA code (sspdma).
 *
 * 18-Jun-86 -- jaw
 *	bisst fix to set NOARB before hitting a node over the head.
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 22-May-86 -- prs
 *	Added time saving measure to the qioinit routine for
 *	the generic dump driver.
 *
 * 18-Apr-86 -- jaw	hooks for nmi faults and fixes to bierrors.
 *
 * 16-Apr-86 -- darrell
 *	changed badaddr to bbadaddr -- to allow the routine 
 *	badaddr for backward comaptibility.
 *
 * 15-Apr-86 -- jf
 *	Add support for system processes.
 *
 * 14-Apr-86 -- afd
 *	Re-did mutually exclusive MicroVAX areas.  Mainly this is
 *	putting MicroVAX into the case stmts with other cpus.
 *
 * 09-Apr-86 -- prs
 *	Added two qio calls to use the vmb boot driver. These routines
 *	are called from the generic dump driver.
 *
 * 09-Apr-86 -- jaw  add dispatchs for multiple bi error routines.
 *
 * 02-Apr-86 -- jrs
 *	Rewrite scheduler to eliminate special master queue.
 *	Add code in inter-processor interrupts for panic handling.
 *
 * 18-Mar-86 -- jrs
 *	Change intrslave to intrcpu to do one at a time.  Changed
 *	entry conditions for cpuindex and merged w/ cpuident.
 *	Added Jim's change to keep ipl at 1f until machine specific
 *	autoconfig called.
 *
 * 18-mar-86  -- jaw     br/cvec changed to NOT use registers.
 *
 * 10-Mar-86 -- pmk
 *	change panic() entry mask to 0x0fff, save r0-r11 for errlogging.
 *
 * 06-Mar-86 -- tresvik
 *	clear RPB rp_flag so dump will work after copying in VMB's RPB
 *
 * 03-Mar-86 -- jrs
 *	Added code to initialize secondary processor start address in rpb
 *
 * 25-Feb-86 -- jrs
 *	Once again removed cold start check for rxcd routine that was
 *	accidentally added back in previous edit
 *
 * 19-Feb-86 -- bjg
 *	Added sbi error logging, uba error logging
 *	Changed doadump to do tbia before accessing rbp to prevent 
 *	protection fault on 8600
 *
 * 18-Feb-86 -- jrs
 *	Change slavestart to _slavestart so we can pick it up from c.
 *	Also get slave into virtual mode a little sooner.
 *	Cold start check moves up to rxcd() routine.
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 12-Feb-86 -- pmk
 *	Added save pc,psl to bierror
 *
 * 12-Feb-86 -- jrs
 *	Changed scheduling loop to only do TBIA on demand
 *
 * 05-Feb-86 -- jrs
 *	Added save of pcbb address for adb backtrace
 *
 * 04-Feb-86 -- tresvik
 *	added VMB boot path support
 *
 * 03-Feb-86 -- jaw  Set ingnorebi error bit so to ignore errors VMB cause.
 *
 * 20 Jan 86 -- bjg
 *	Added stray interrupt error logging (logstray)
 *
 * 17-Jul-85 -- jrs
 *	Add run queue locking
 *
 * 26-Oct-85 -- jaw
 * 	Fix to bisst() to reset BDA properly once system is up.
 *
 * 25-Sep-85 -- jaw
 *	Fix to bisst to mask problem with BIIC SST bit.
 *	After a dump...halt a 8200 so memory is preserved.
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code.
 * 03-Sep-85 -- jaw
 *	Added BI error interrupt code.
 *
 * 09-AUG-85 -- darrell dunnuck
 *	Unibus zero vectors are now counted here in locore.  We set a
 *	timer in ubaerror on the first zero vector that comes in,
 *	and count them here in locore until the timer has expired.
 *	We never do a unibus reset no matter how many zero vectors we get.
 *
 * 26-Jul-85 -- jaw
 *	fixed up Violation to SRM...TBIA data MBZ.
 *
 * 18-Jul-85 -- tresvik
 *	Change the way the end of doadump causes a reboot.  Call tocons
 *	instead of duplicating C code here in assembly.  VAX 8600 requires
 *	special handling already defined in tocons.
 *
 * 11-Jul-85 -- tresvik
 *	Fix bug in handling write timeout.  Branching around the PANIC
 *	macro requires local label other than 1, since 1 is already used
 *	within the macro.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06-Jun-85 -- Jaw
 *	got rid of badwrite and replaced it with bisst(). 
 *
 * 05-May-85 -- Larry Cohen
 * 	network interrupt vectors are now configurable
 *
 * 16-Apr-85 -- lp
 *      Added cpu serial line support for VAX8200. Changed tu58 
 *      interface for non-MRSP machines. 
 *
 *  9 Apr 85 -- depp
 *	Added check for Allocated shared memory page in Fastreclaim
 *
 * 22-Mar-85 -- reilly
 *	Added support for floating point emulation for non microvaxes
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 3-MAR-85 -- darrell
 *    handle sbi interupts.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 17 Dec 84 -- jrs
 *	Change setjmp, resume to use new regs to avoid swap problems.
 *	Also add savectx for same reason.  Update code in copyin/copyout
 *	so it can be used with new inline program.
 *
 * 06 Nov 84 -- jrs
 *	Reroll code in swtch/resume to avoid double context switch
 *	and handle most likely case linearly.
 *
 * 31 Oct 84 -- jrs
 *	Remove auto calls to dh and dz timer routines in softclock.
 *	These are now done on a demand basis at a higher level
 *
 * 20 Aug 84 -- larry
 *	removed system page table and made it a seperate file - spt.s
 *	so that SYSPTSIZE could be configurable on a binary kernel.
 *
 *  5 May 84 -- rjl
 *	Added QVSS support. The bit map looks like regular memory so some
 *	of the normal autoconfiguration stuff has to be done here.
 *
 * 12 Apr 84 -- rjl
 *	Added float type check for MicroVAX
 *
 * 23 Mar 84 -- slr
 *	Added emulation support for MicroVAX
 *
 * 15 Feb 84 -- rjl
 *	Added support for MicroVAX intelligent boot. Now saving
 *	r10 and r11 as bootdevice and boothowto for later use by
 *	autoconfig.
 *
 *  2 Jan 84 --jmcg
 *	Added support for MicroVAX 1.
 *
 *************************************************************************/

#include "../machine/psl.h"
#include "../machine/pte.h"

#include "../h/errno.h"
#include "../h/cmap.h"
#include "../h/interlock.h"
#include "../h/cpudata.h"

#include "../vax/mtpr.h"
#include "../vax/pcb.h"
#include "../vax/trap.h"
#include "../vax/cpu.h"
#include "../vax/b_params.h"
#include "../vax/cons.h"
#include "../vax/clock.h"
#include "../vaxbi/bireg.h"
/*
#include "../vax/nexus.h"
#include "../vaxuba/ubareg.h"
*/
#include "../sas/vmb.h"

#include "dh.h"
#include "dz.h"
#include "ss.h"
#include "uu.h"
#include "ps.h"
#include "mba.h"

#define	ENTRY(name, regs) \
 	.globl _/**/name; .align 1; _/**/name: .word regs
#define R0 0x01
#define R1 0x02
#define R2 0x04
#define R3 0x08
#define R4 0x10
#define R5 0x20
#define R6 0x40

	.set	INTSTK,1	# handle this interrupt on the interrupt stack
	.set	HALT,3		# halt if this interrupt occurs
	.set	HIGH,0x1f	# mask for total disable
	.set	MCKVEC,4	# offset into scb of machine check vector
	.set	NBPG,512
	.set	PGSHIFT,9
	.set	SYSPROC,10	# system process priority level

/*
 * User structure is UPAGES at top of user space.
 */
	.globl	_u
	.set	_u,0x80000000 - UPAGES*NBPG

	.globl	_intstack
_intstack:
	.space	NISP*NBPG
eintstack:

/*
 * Do a dump.
 * Called by auto-restart.
 * May be called manually.
 */
	.align	2
	.globl	_doadump
_doadump:
	nop; nop				# .word 0x0101
#define _rpbmap _Sysmap 			# rpb, scb, UNI*vec, istack*4
	bicl2	$PG_PROT,_rpbmap
	bisl2	$PG_KW,_rpbmap
	mtpr	$0,$TBIA
	tstl	_rpb+RP_FLAG			# dump only once!
	bneq	1f
	incl	_rpb+RP_FLAG
	movl	sp,erpb
	movab	erpb,sp
	mfpr	$PCBB,-(sp)
	mfpr	$MAPEN,-(sp)
	mfpr	$IPL,-(sp)
	mtpr	$0,$MAPEN
	mtpr	$HIGH,$IPL
	pushr	$0x3fff
	calls	$0,_dumpsys
1:
	cmpl	$VAX_8200,_cpu
	beql	2f		
	pushl	$TXDB_BOOT			# reboot the system
	calls	$1,_tocons			# let tocons tell the console
						# (it handles VAX 8600)
2:
	halt					# come back and halt

/*
 * Where called	: qioinit is called in the generic dump driver.
 * Function	: Called to initialize the dump device.
 * Parameters	: None.
 * Side effects	: Computes global variable _qioentry, the entry point
 *		  into the qio routine of the new boot driver.
 */

	.globl	_qioinit
_qioinit:
	.word	0xffc				# save r2 - r11
	movl	_vmbinfo,r8			# r8 points to info list
	movl	INFO_RPBBAS(r8),r9		# required bu init routines
	movl	INFO_BTDRBAS(r8),r7		# get address of boot driver
	addl3	r7,BQO$L_QIO(r7),_qioentry	# compute address qio routine
	movl	$1,r0				# in case no init routine.
	tstl	BQO$L_UNIT_INIT(r7)		# does init routine exist ?
	beql	4f				# if not dont bother trying
	addl3	r7,BQO$L_UNIT_INIT(r7),r2	# calculate entry point
/*
 * KLUDGE (for [T]MSCP controller types) to force total controller
 * initialization, otherwise we wait an awful long time for
 * initialization to complete. Also used to set up driver rings to
 * talk to VMB, not UFS.
 */
#define	IP	0
#define	SA	2
	cmpb	$BTD$K_UDA,RPB$B_DEVTYP(r9)	# is this a uda/qda/kda ?
	beql	1f				# yes, go hit it
	cmpb	$BTD$K_TK50,RPB$B_DEVTYP(r9)	# is this a TK50 controller ?
	beql	1f				# yes, go hit it
	brb	2f				# proceed with normal init
1:
	movl	RPB$L_CSRPHY(r9),r7		#get the IP register address
	clrw	IP(r7)				# poke it to make it step
1:
	movw	SA(r7),r0			# get the status register
	bitw	$0xfe00,SA(r7)			# is something active ?
	beql	1b				# if not, wait until there is
2:
	mfpr	$SCBB,r7			# required by BI (BDA driver)
	callg	INFO_VMBARGBAS(r8),(r2)		# do it
4:
	ret

/*
 * Where called	: qiois called in the generic dump driver.
 * Function	: Called to perform reading from core and writing to
 *		  the dump device.
 * Parameters	: 1) Address mode, physical or virtual.
 *		  2) Function, reading or writing virtual/physical space.
 *		  3) Starting block number, starting disk block number.
 *		  4) Transfer size, in bytes.
 *		  5) Address of buffer, memory location.
 * Side effects	: Performs io function and returns completion code.
 */

	.globl _qio
_qio:
	.word	0xe00				# save r9 - r11
	movl	_vmbinfo,r9			# point to the info list
	movl	INFO_RPBBAS(r9),r9		# get address of the RPB
	pushl	r9				# push address of RPB
	pushl	4(ap)				# push address mode
	pushl	8(ap)				# push I/O function code
	pushl	12(ap)				# push starting block number
	pushl	16(ap)				# push transfer size in bytes
	pushl	20(ap)				# push address of buffer
	calls	$6,*_qioentry			# call qio routine
	ret					# return completion code

/*
 * {fu,su},{byte,word}, all massaged by asm.sed to jsb's
 */
	.globl	_Fuword
_Fuword:
	prober	$3,$4,(r0)
	beql	fserr
	movl	(r0),r0
	rsb
fserr:
	mnegl	$1,r0
	rsb

	.globl	_Fubyte
_Fubyte:
	prober	$3,$1,(r0)
	beql	fserr
	movzbl	(r0),r0
	rsb

	.globl	_Suword
_Suword:
	probew	$3,$4,(r0)
	beql	fserr
	movl	r1,(r0)
	clrl	r0
	rsb

	.globl	_Subyte
_Subyte:
	probew	$3,$1,(r0)
	beql	fserr
	movb	r1,(r0)
	clrl	r0
	rsb


/*
 * Check address.
 * Given virtual address, byte count, and rw flag
 * returns 0 on no access.
 */
_useracc:	.globl	_useracc
	.word	0x0
	movl	4(ap),r0		# get va
	movl	8(ap),r1		# count
	tstl	12(ap)			# test for read access ?
	bneq	userar			# yes
	cmpl	$NBPG,r1			# can we do it in one probe ?
	bgeq	uaw2			# yes
uaw1:
	probew	$3,$NBPG,(r0)
	beql	uaerr			# no access
	addl2	$NBPG,r0
	acbl	$NBPG+1,$-NBPG,r1,uaw1
uaw2:
	probew	$3,r1,(r0)
	beql	uaerr
	movl	$1,r0
	ret

userar:
	cmpl	$NBPG,r1
	bgeq	uar2
uar1:
	prober	$3,$NBPG,(r0)
	beql	uaerr
	addl2	$NBPG,r0
	acbl	$NBPG+1,$-NBPG,r1,uar1
uar2:
	prober	$3,r1,(r0)
	beql	uaerr
	movl	$1,r0
	ret
uaerr:
	clrl	r0
	ret


_addupc:	.globl	_addupc
	.word	0x0
	movl	8(ap),r2		# &u.u_prof
	subl3	8(r2),4(ap),r0		# corrected pc
	blss	9f
	extzv	$1,$31,r0,r0		# logical right shift
	extzv	$1,$31,12(r2),r1	# ditto for scale
	emul	r1,r0,$0,r0
	ashq	$-14,r0,r0
	tstl	r1
	bneq	9f
	bicl2	$1,r0
	cmpl	r0,4(r2)		# length
	bgequ	9f
	addl2	(r2),r0 		# base
	probew	$3,$2,(r0)
	beql	8f
	addw2	12(ap),(r0)
9:
	ret
8:
	clrl	12(r2)
	ret

 
/*
 * Copy a null terminated string from the user address space into
 * the kernel address space.
 *
 * copyinstr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copyinstr, R6)
 	movl	12(ap),r6		# r6 = max length
 	jlss	8f
 	movl	4(ap),r1		# r1 = user address
 	bicl3	$~(NBPG*CLSIZE-1),r1,r2	# r2 = bytes on first page
 	subl3	r2,$NBPG*CLSIZE,r2
 	movl	8(ap),r3		# r3 = kernel address
1:
 	cmpl	r6,r2			# r2 = min(bytes on page, length left);
	jgeq	2f
	movl	r6,r2
2:
	prober	$3,r2,(r1)		# bytes accessible?
	jeql	8f
	subl2	r2,r6			# update bytes left count
	locc	$0,r2,(r1)		# null byte found?
	jneq	3f
	subl2	r2,r1			# back up pointer updated by `locc'
	movc3	r2,(r1),(r3)		# copy in next piece
	movl	$(NBPG*CLSIZE),r2	# check next page
	tstl	r6			# run out of space?
	jneq	1b
	movl	$ENOENT,r0		# set error code and return
	jbr	9f
3:
	tstl	16(ap)			# return length?
	beql	4f
	subl3	r6,12(ap),r6		# actual len = maxlen - unused pages
	subl2	r0,r6			#	- unused on this page
	addl3	$1,r6,*16(ap)		#	+ the null byte
4:
 	subl2	r0,r2			# r2 = number of bytes to move
	subl2	r2,r1			# back up pointer updated by `locc'
	incl	r2			# copy null byte as well
	movc3	r2,(r1),(r3)		# copy in last piece
	clrl	r0			# redundant
	ret
8:
	movl	$EFAULT,r0
9:
	tstl	16(ap)
	beql	1f
 	subl3	r6,12(ap),*16(ap)
1:
	ret

/*
 * Copy a null terminated string from the kernel
 * address space to the user address space.
 *
 * copyoutstr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copyoutstr, R6)
	movl	12(ap),r6		# r6 = max length
	jlss	8b
	movl	4(ap),r1		# r1 = kernel address
	movl	8(ap),r3		# r3 = user address
	bicl3	$~(NBPG*CLSIZE-1),r3,r2	# r2 = bytes on first page
	subl3	r2,$NBPG*CLSIZE,r2
1:
	cmpl	r6,r2			# r2 = min(bytes on page, length left);
	jgeq	2f
	movl	r6,r2
2:
	probew	$3,r2,(r3)		# bytes accessible?
	jeql	8b
	subl2	r2,r6			# update bytes left count
	locc	$0,r2,(r1)		# null byte found?
	jneq	3b
	subl2	r2,r1			# back up pointer updated by `locc'
	movc3	r2,(r1),(r3)		# copy in next piece
	movl	$(NBPG*CLSIZE),r2	# check next page
	tstl	r6			# run out of space?
	jneq	1b
	movl	$ENOENT,r0		# set error code and return
	jbr	9b

/*
 * Copy a null terminated string from one point to another in
 * the kernel address space.
 *
 * copystr(fromaddr, toaddr, maxlength, &lencopied)
 */
ENTRY(copystr, R6)
	movl	12(ap),r6		# r6 = max length
	jlss	8b
	movl	4(ap),r1		# r1 = src address
	movl	8(ap),r3		# r3 = dest address
1:
	movzwl	$65535,r2		# r2 = bytes in first chunk
	cmpl	r6,r2			# r2 = min(bytes in chunk, length left);
	jgeq	2f
	movl	r6,r2
2:
	subl2	r2,r6			# update bytes left count
	locc	$0,r2,(r1)		# null byte found?
	jneq	3b
	subl2	r2,r1			# back up pointer updated by `locc'
	movc3	r2,(r1),(r3)		# copy in next piece
	tstl	r6			# run out of space?
	jneq	1b
	movl	$ENOENT,r0		# set error code and return
	jbr	9b

/*
 * Copy specified amount of data from user space into the kernel
 * Copyin(from, to, len)
 *	r1 == from (user source address)
 *	r3 == to (kernel destination address)
 *	r5 == length
 */
	.globl	_Copyin 		# massaged to jsb, args R1 R3 R5
_Copyin:
	cmpl	r5,$(NBPG*CLSIZE)	# probing one page or less ?
	bgtru	1f			# no
	prober	$3,r5,(r1)		# bytes accessible ?
	beql	ersb			# no
	movc3	r5,(r1),(r3)
/*	clrl	r0			# redundant */
	rsb
1:
	blss	ersb			# negative length?
	pushl	r6			# r6 = length
	movl	r5,r6
	bicl3	$~(NBPG*CLSIZE-1),r1,r0 # r0 = bytes on first page
	subl3	r0,$(NBPG*CLSIZE),r0
	addl2	$(NBPG*CLSIZE),r0	# plus one additional full page
	jbr	2f

ciloop:
	movc3	r0,(r1),(r3)
	movl	$(2*NBPG*CLSIZE),r0	# next amount to move
2:
	cmpl	r0,r6
	bleq	3f
	movl	r6,r0
3:
	prober	$3,r0,(r1)		# bytes accessible ?
	beql	ersb1			# no
	subl2	r0,r6			# last move?
	bneq	ciloop			# no

	movc3	r0,(r1),(r3)
/*	clrl	r0			# redundant */
	movl	(sp)+,r6		# restore r6
	rsb

ersb1:
	movl	(sp)+,r6		# restore r6
ersb:
	movl	$EFAULT,r0
	rsb

/*
 * Copy specified amount of data from kernel to the user space
 * Copyout(from, to, len)
 *	r1 == from (kernel source address)
 *	r3 == to (user destination address)
 *	r5 == length
 */
	.globl	_Copyout		# massaged to jsb.  Args R1 R3 R5
_Copyout:
	cmpl	r5,$(NBPG*CLSIZE)	# moving one page or less ?
	bgtru	1f			# no
	probew	$3,r5,(r3)		# bytes writeable?
	beql	ersb			# no
	movc3	r5,(r1),(r3)
/*	clrl	r0			# redundant */
	rsb
1:
	blss	ersb			# negative length?
	pushl	r6			# r6 = length
	movl	r5,r6
	bicl3	$~(NBPG*CLSIZE-1),r3,r0 # r0 = bytes on first page
	subl3	r0,$(NBPG*CLSIZE),r0
	addl2	$(NBPG*CLSIZE),r0	# plus one additional full page
	jbr	2f

coloop:
	movc3	r0,(r1),(r3)
	movl	$(2*NBPG*CLSIZE),r0	# next amount to move
2:
	cmpl	r0,r6
	bleq	3f
	movl	r6,r0
3:
	probew	$3,r0,(r3)		# bytes writeable?
	beql	ersb1			# no
	subl2	r0,r6			# last move?
	bneq	coloop			# no

	movc3	r0,(r1),(r3)
/*	clrl	r0			# redundant */
	movl	(sp)+,r6		# restore r6
	rsb

/*
 * Interrupt vector routines
 */
	.globl	_waittime

#define SCBVEC(name)	.align 2; .globl _X/**/name; _X/**/name
#define PANIC(msg)	clrl _waittime; pushab 1f; \
			calls $1,_panic; 1: .asciz msg
#define PRINTF(n,msg)	pushab 1f; calls $n+1,_printf; M_MSG(msg)
#define M_MSG(msg)	  .data; 1: .asciz msg; .text
#define PUSHR		pushr $0x3f
#define POPR		popr $0x3f
#define SBI0	0
#define SBI1	1
#define SBI_ERROR 0
#define SBI_WTIME 1
#define SBI_ALERT 2
#define SBI_FLT 3
#define SBI_FAIL 4

SCBVEC(machcheck):
	PUSHR; pushab 6*4(sp); calls $1,_machinecheck; POPR;
	addl2 (sp)+,sp; rei
SCBVEC(kspnotval):
	PUSHR; PANIC("KSP not valid");
SCBVEC(powfail):
	halt
SCBVEC(chme): SCBVEC(chms): SCBVEC(chmu):
	PUSHR; PANIC("CHM? in kernel");
SCBVEC(cmrd):
	PUSHR; calls $0,_memerr; POPR; rei
SCBVEC(wtime):
	PUSHR
	pushal 6*4(sp)
	cmpl	$VAX_8600,_cpu
	bneq	2f		# 2f because 1: used in panic macro
	pushl $SBI_ERROR	# 0 - sbierror type
	pushl $SBI0		# sbi0
	calls $3, _logsbi	#log 8600 sbi error 
	POPR
	PANIC("sbia0error")
2:
	pushl $SBI_WTIME		# 1 - wtime error type
	pushl $SBI0		# sbi0
	calls $3, _logsbi	#log non-8600 sbi error
	PRINTF(1,"write timeout %x\n"); POPR;
	PANIC("wtimo")

SCBVEC(sbi0alert):
	cmpl	$VAX_8200,_cpu
	bneq	2f
	PUSHR
	calls	$0,_ka820rxcd
	POPR
	rei
	
2:	
	PUSHR; pushal 6*4(sp); pushl $SBI_ALERT; pushl $SBI0; calls $3, _logsbi; POPR; 
	PANIC("sbi0alert")

SCBVEC(sbi0flt):
	cmpl	$VAX_8800,_cpu
	bneq	2f
	PUSHR
	calls	$0,_ka8800nmifault
	POPR
	rei

2:
	PUSHR; pushal 6*4(sp); pushl $SBI_FLT; pushl $SBI0; calls $3, _logsbi; POPR;
	PANIC("sbi0flt")
SCBVEC(sbi0fail):
	PUSHR; pushal 6*4(sp); pushl $SBI_FAIL; pushl $SBI0; calls $3, _logsbi; POPR;
	PANIC("sbi0fail")

SCBVEC(sbi1fail):
	PUSHR; pushal 6*4(sp); pushl $SBI_FAIL; pushl $SBI1; calls $3, _logsbi; POPR; 
	PANIC("sbi1fail")
SCBVEC(sbi1alert):
	PUSHR; pushal 6*4(sp); pushl $SBI_ALERT; pushl $SBI1; calls $3, _logsbi; POPR; 
	PANIC("sbi1alert")
SCBVEC(sbi1flt):
	PUSHR; pushal 6*4(sp); pushl $SBI_FLT; pushl $SBI1; calls $2, _logsbi; POPR; 
	PANIC("sbi1flt")
SCBVEC(sbi1error):
	PUSHR; pushal 6*4(sp); pushl $SBI_ERROR; pushl $SBI1; calls $3, _logsbi; POPR; 
	PANIC("sbia1error")

#if NMBA > 0
SCBVEC(mba3int):
	PUSHR; pushl $3; brb 1f
SCBVEC(mba2int):
	PUSHR; pushl $2; brb 1f
SCBVEC(mba1int):
	PUSHR; pushl $1; brb 1f
SCBVEC(mba0int):
	PUSHR; pushl $0
1:	calls $1,_mbintr
	POPR
	incl	_cnt+V_INTR
	rei
#endif

#if defined(VAX780) || defined (VAX8600)

/*
 * Registers for the uba handling code
 */
#define rUBANUM r0
#define rUBAHD	r1
#define rUVEC	r3
#define rUBA	r4
#define rUBAPC  r5
/* r2 are scratch */

SCBVEC(ua6int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $6,rUBANUM; moval _uba_hd+(6*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua5int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $5,rUBANUM; moval _uba_hd+(5*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua4int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $4,rUBANUM; moval _uba_hd+(4*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua3int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $3,rUBANUM; moval _uba_hd+(3*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua2int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $2,rUBANUM; moval _uba_hd+(2*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua1int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $1,rUBANUM; moval _uba_hd+(1*UH_SIZE),rUBAHD; brb 1f
SCBVEC(ua0int):
	PUSHR; moval 6*4(sp), rUBAPC; movl $0,rUBANUM; moval _uba_hd+(0*UH_SIZE),rUBAHD;

1:
	incl	_cnt+V_INTR
	mfpr	$IPL,r2 			/* r2 = mfpr(IPL); */
	movl	UH_UBA(rUBAHD),rUBA		/* uba = uhp->uh_uba; */
	movl	UBA_BRRVR-0x14*4(rUBA)[r2],rUVEC
					/* uvec = uba->uba_brrvr[r2-0x14] */
ubanorm:
	bleq	ubaerror
	addl2	UH_VEC(rUBAHD),rUVEC		/* uvec += uh->uh_vec */
	bicl3	$3,(rUVEC),r1
	jmp	2(r1)				/* 2 skips ``pushr $0x3f'' */
ubaerror:
	tstl	rUVEC				/* set condition codes */
	beql	zvec				/* branch if zero vector */
ubaerr1:
	PUSHR; calls $0,_ubaerror; POPR 	/* ubaerror r/w's r0-r5 */
	tstl rUVEC; jneq ubanorm		/* rUVEC contains result */
	brb	rtrn	
zvec:
	tstl	UH_ZVFLG(rUBAHD)		/* zero vector timer flg set? */
	beql	ubaerr1				/* yes -- inc counter here */
						/* no  -- call ubaerror */
	incl	UH_ZVCNT(rUBAHD)		/* bump zero vector counter */
rtrn:
	POPR
	rei

#endif VAX780 || VAX8600
/* Console xmit/rcv interrupt routines */

SCBVEC(cnrint):
	PUSHR; pushl $0; calls $1,_cnrint; POPR; incl _cnt+V_INTR; rei
SCBVEC(cnxint):
	PUSHR; pushl $0; calls $1,_cnxint; POPR; incl _cnt+V_INTR; rei

/* 3 Serial lines on VAX8200 == cpu type #5 */
/* pushl $1,$2,$3 is the index into the struct tty cons array */

SCBVEC(cnrint1):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $1; calls $1,_cnrint
1:
	POPR; incl _cnt+V_INTR; rei
SCBVEC(cnxint1):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $1; calls $1,_cnxint
1:
	POPR; incl _cnt+V_INTR; rei
SCBVEC(cnrint2):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $2; calls $1,_cnrint
1:
	POPR; incl _cnt+V_INTR; rei
SCBVEC(cnxint2):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $2; calls $1,_cnxint
1:
	POPR; incl _cnt+V_INTR; rei
SCBVEC(cnrint3):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $3; calls $1,_cnrint
1:
	POPR; incl _cnt+V_INTR; rei
SCBVEC(cnxint3):
	PUSHR; cmpl $5,_cpu; bneq 1f; pushl $3; calls $1,_cnxint
1:
	POPR; incl _cnt+V_INTR; rei

SCBVEC(hardclock):
	PUSHR
	mtpr $ICCS_RUN|ICCS_IE|ICCS_INT|ICCS_ERR,$ICCS
	pushl 4+6*4(sp); pushl 4+6*4(sp);
	calls $2,_hardclock			# hardclock(pc,psl)
#if NPS > 0
	tstb	_nNPS
	beql	1f
	pushl	4+6*4(sp); pushl 4+6*4(sp);
	calls	$2,_psextsync
#endif
1:	POPR;
	incl	_cnt+V_INTR		## temp so not to break vmstat -= HZ
	rei

SCBVEC(intqueue):			# Entry point for SIRR 7's
	PUSHR				#  @ IPL 7
	calls	$0,_intqueue
	POPR
	rei
SCBVEC(softclock):
	PUSHR
	pushl	4+6*4(sp); pushl 4+6*4(sp)
	calls	$2,_softclock			# softclock(pc,psl)
	POPR
	rei
#include "../net/netisr.h"
	.globl	_netisr
SCBVEC(netintr):
	pushr	$0x83f
	moval	_netisr_tab, r11     	# while (netisr_tab[i] >= 0)
startwhile:
	tstl	(r11)
	jlss	endwhile
	bbcc	(r11)+,_netisr,1f;  calls $0,*(r11)+; jbr startwhile; 1:	
	tstl	(r11)+
	jbr 	startwhile
endwhile:
	popr	$0x83f
	rei
SCBVEC(sysprocint):
	pushr	$0x3fff			# save all registers
	calls	$0,_sysprocintr
	popr	$0x3fff
	rei
/*
 * System process switch routine.
 */
_Swtchsysproc:	.globl	_Swtchsysproc
	.word	0xfff
	movl	fp,-(sp)		# save frame pointer
	mfpr	$IPL,-(sp)		#   and processor priority level
	movl	sp,*8(ap)		# store current state
	movl	4(ap),sp		# recover previous state
	mtpr	(sp)+,$IPL		#   and processor priority level
	movl	(sp)+,fp		# restore frame pointer
	ret				#   and resume previous system process

/*
 * Build an initial stack frame for a system process.
 */
_Initsysproc:	.globl	_Initsysproc
	.word	0x0
	movl	4(ap),r0		# get system process stack pointer
	movl	$_Runsysproc+2,-(r0)	# return PC (runs system process)
	clrl	-(r0)			# saved FP
	clrl	-(r0)			# saved AP
	clrl	-(r0)			# no saved registers/callg entry
	clrl	-(r0)			# no condition handler
	movl	r0,-(r0)		# save frame pointer
	movl	$SYSPROC,-(r0)		# default system process priority
	movl	r0,*8(ap)		# store system process state
	ret
SCBVEC(consdin):
	PUSHR;
	casel	_cpu,$1,$VAX_MAX
0:
	.word	1f-0b		# 1 is 780
	.word	3f-0b		# 2 is 750
	.word	3f-0b		# 3 is 730
	.word	4f-0b		# 4 is 8600
	.word	5f-0b		# 5 is 8200
	.word	1f-0b		# 6
	.word	1f-0b		# 7 is MVAXI
	.word	1f-0b		# 8 is MVAXII
1:	halt
#if defined(VAX750) || defined(VAX730)
2:	jsb	 tudma		 # 750
/*	mfpr	$CSRD,r5         # If using new tudma code
	pushl	$0
	pushl	r5
	pushal	_turintr
	calls	$3,_chrqueue
	brb	1f    */
3:	calls $0,_turintr	# 750/730
#else
2:
3:
#endif
	brb 1f
4:
#if defined(VAX8600)
	calls $0,_crlintr	# 8600
#endif
	brb 1f
5:
#if defined(VAX8200)
	calls $0,_rx5_intr	#8200
#endif
1:
	POPR
	incl _cnt+V_INTR
	rei
SCBVEC(consdout):
	PUSHR;
	casel	_cpu,$1,$VAX_MAX
0:
	.word	1f-0b		# 1 is 780
	.word	2f-0b		# 2 is 750
	.word	2f-0b		# 3 is 730
	.word	1f-0b		# 4 is 8600
	.word	1f-0b		# 5
	.word	1f-0b		# 6
	.word	1f-0b		# 7 is MVAXI
	.word	1f-0b		# 8 is MVAXII
1:	halt
2:
#if defined(VAX750) || defined(VAX730)
	calls $0,_tuxintr	# 750/730
	POPR
	incl _cnt+V_INTR
	rei
#endif

#if NDZ > 0
/*
 * DZ pseudo dma routine:
 *	r0 - controller number
 */
	.align	1
	.globl	dzdma
dzdma:
	mull2	$8*20,r0
	movab	_dzpdma(r0),r3		# pdma structure base
					# for this controller
dzploop:
	movl	r3,r0
	movl	(r0)+,r1		# device register address
	movzbl	1(r1),r2		# get line number
	bitb	$0x80,r2		# TRDY on?
	beql	dzprei			# no
	bicb2	$0xf8,r2		# clear garbage bits
	mull2	$20,r2
	addl2	r2,r0			# point at line's pdma structure
	movl	(r0)+,r2		# p_mem
	cmpl	r2,(r0)+		# p_mem < p_end ?
	bgequ	dzpcall 		# no, go call dzxint
	movb	(r2)+,6(r1)		# dztbuf = *p_mem++
	movl	r2,-8(r0)
	brb	dzploop 		# check for another line
dzprei:
	POPR
	incl	_cnt+V_PDMA
	rei

dzpcall:
	pushl	r3
	pushl	(r0)+			# push tty address
	calls	$1,*(r0)		# call interrupt rtn
	movl	(sp)+,r3
	brb	dzploop 		# check for another line
#endif

#if NSS > 0
/*
 * VAXstar SLU pseudo dma routine:
 *	r0 - controller number
 * CAUTION:
 *	The VAXstar system unit spec says that byte reads
 *	of the SLU CSR are not allowed. This is not an absolute
 *	restriction. It would only cause problems if there were
 *	bits in the CSR which were modified by reading.
 */
	.align	1
	.globl	ssdma
ssdma:
	mull2	$8*20,r0
	movab	_sspdma(r0),r3		# pdma structure base
					# for this controller
ssploop:
	movl	r3,r0
	movl	(r0)+,r1		# device register address
	movzbl	1(r1),r2		# get line number
	bitb	$0x80,r2		# TRDY on?
	beql	ssprei			# no
	bicb2	$0xfc,r2		# clear garbage bits
	mull2	$20,r2
	addl2	r2,r0			# point at line's pdma structure
	movl	(r0)+,r2		# p_mem
	cmpl	r2,(r0)+		# p_mem < p_end ?
	bgequ	sspcall 		# no, go call ssxint
	movb	(r2)+,12(r1)		# sstbuf = *p_mem++
	movl	r2,-8(r0)
	brb	ssploop 		# check for another line
ssprei:
	POPR
	incl	_cnt+V_PDMA
	rei

sspcall:
	pushl	r3
	pushl	(r0)+			# push tty address
	calls	$1,*(r0)		# call interrupt rtn
	movl	(sp)+,r3
	brb	ssploop 		# check for another line
#endif

#if NUU > 0 && defined(UUDMA)
/*
 * Pseudo DMA routine for tu58 (on DL11)
 *	r0 - controller number
 */
	.align	1
	.globl	uudma
uudma:
	movl	_uudinfo[r0],r2
	movl	16(r2),r2		# r2 = uuaddr
	mull3	$48,r0,r3
	movab	_uu_softc(r3),r5	# r5 = uuc

	cvtwl	2(r2),r1		# c = uuaddr->rdb
	bbc	$15,r1,1f		# if (c & UUDB_ERROR)
	movl	$13,16(r5)		#	uuc->tu_state = TUC_RCVERR;
	rsb				#	let uurintr handle it
1:
	tstl	4(r5)			# if (uuc->tu_rcnt) {
	beql	1f
	movb	r1,*0(r5)		#	*uuc->tu_rbptr++ = r1
	incl	(r5)
	decl	4(r5)			#	if (--uuc->tu_rcnt)
	beql	2f			#		done
	tstl	(sp)+
	POPR				#	registers saved in ubglue.s
	rei				# }
2:
	cmpl	16(r5),$8		# if (uuc->tu_state != TUS_GETH)
	beql	2f			#	let uurintr handle it
1:
	rsb
2:
	mull2	$14,r0			# sizeof(uudata[ctlr]) = 14
	movab	_uudata(r0),r4		# data = &uudata[ctlr];
	cmpb	$1,(r4) 		# if (data->pk_flag != TUF_DATA)
	bneq	1b
#ifdef notdef
	/* this is for command packets */
	beql	1f			#	r0 = uuc->tu_rbptr
	movl	(r5),r0
	brb	2f
1:					# else
#endif
	movl	24(r5),r0		#	r0 = uuc->tu_addr
2:
	movzbl	1(r4),r3		# counter to r3 (data->pk_count)
	movzwl	(r4),r1 		# first word of checksum (=header)
	mfpr	$IPL,-(sp)		# s = spl5();
	mtpr	$0x15,$IPL		# to keep disk interrupts out
	clrw	(r2)			# disable receiver interrupts
3:	bbc	$7,(r2),3b		# while ((uuaddr->rcs & UUCS_READY)==0);
	cvtwb	2(r2),(r0)+		# *buffer = uuaddr->rdb & 0xff
	sobgtr	r3,1f			# continue with next byte ...
	addw2	2(r2),r1		# unless this was the last (odd count)
	brb	2f

1:	bbc	$7,(r2),1b		# while ((uuaddr->rcs & UUCS_READY)==0);
	cvtwb	2(r2),(r0)+		# *buffer = uuaddr->rdb & 0xff
	addw2	-2(r0),r1		# add to checksum..
2:
	adwc	$0,r1			# get the carry
	sobgtr	r3,3b			# loop while r3 > 0
/*
 * We're ready to get the checksum
 */
1:	bbc	$7,(r2),1b		# while ((uuaddr->rcs & UUCS_READY)==0);
	cvtwb	2(r2),12(r4)		# get first (lower) byte
1:	bbc	$7,(r2),1b
	cvtwb	2(r2),13(r4)		# ..and second
	cmpw	12(r4),r1		# is checksum ok?
	beql	1f
	movl	$14,16(r5)		# uuc->tu_state = TUS_CHKERR
	brb	2f			# exit
1:
	movl	$11,16(r5)		# uuc->tu_state = TUS_GET (ok)
2:
	movw	$0x40,(r2)		# enable receiver interrupts
	mtpr	(sp)+,$IPL		# splx(s);
	rsb				# continue processing in uurintr
#endif

#if defined(VAX750) || defined(BINARY)
/*
 * Pseudo DMA routine for VAX-11/750 console tu58
 *	    (without MRSP)
 */
	.align	1
	.globl	tudma
tudma:
	movab	_tu,r5			# r5 = tu
	tstl	4(r5)			# if (tu.tu_rcnt) {
	beql	3f
	mfpr	$CSRD,r1		# get data from tu58
	movb	r1,*0(r5)		#	*tu.tu_rbptr++ = r1
	incl	(r5)
	decl	4(r5)			#	if (--tu.tu_rcnt)
	beql	1f			#		done
	tstl	(sp)+
	POPR				#	registers saved in ubglue.s
	rei				#	data handled, done
1:					# }
	cmpl	16(r5),$8		# if (tu.tu_state != TUS_GETH)
	beql	2f			#	let turintr handle it
3:
	rsb
2:
	movab	_tudata,r4		# r4 = tudata
	cmpb	$1,(r4) 		# if (tudata.pk_flag != TUF_DATA)
	bneq	3b			#	let turintr handle it
1:					# else
	movl	24(r5),r1		# get buffer pointer to r1
	movzbl	1(r4),r3		# counter to r3
	movzwl	(r4),r0 		# first word of checksum (=header)
	mtpr	$0,$CSRS		# disable receiver interrupts
3:
	bsbw	5f			# wait for next byte
	mfpr	$CSRD,r5
	movb	r5,(r1)+		# *buffer = rdb
	sobgtr	r3,1f			# continue with next byte ...
	mfpr	$CSRD,r2		# unless this was the last (odd count)
	brb	2f

1:	bsbw	5f			# wait for next byte
	mfpr	$CSRD,r5
	movb	r5,(r1)+		# *buffer = rdb
	movzwl	-2(r1),r2		# get the last word back from memory
2:
	addw2	r2,r0			# add to checksum..
	adwc	$0,r0			# get the carry
	sobgtr	r3,3b			# loop while r3 > 0
/*
 * We're ready to get the checksum.
 */
	bsbw	5f
	movab	_tudata,r4
	mfpr	$CSRD,r5
	movb	r5,12(r4)		# get first (lower) byte
	bsbw	5f
	mfpr	$CSRD,r5
	movb	r5,13(r4)		# ..and second
	movab	_tu,r5
	cmpw	12(r4),r0		# is checksum ok?
	beql	1f
	movl	$14,16(r5)		# tu.tu_state = TUS_CHKERR
	brb	2f			# exit
1:
	movl	$11,16(r5)		# tu.tu_state = TUS_GET
2:
	mtpr	$0x40,$CSRS		# enable receiver interrupts
	rsb				# continue processing in turintr
/*
 * Loop until a new byte is ready from
 * the tu58, make sure we don't loop forever
 */
5:
	movl	$5000,r5		# loop max 5000 times
1:
	mfpr	$CSRS,r2
	bbs	$7,r2,1f
	sobgtr	r5,1b
	movab	_tu,r5
	movl	$13,16(r5)		# return TUS_RCVERR
	tstl	(sp)+			# and let turintr handle it
1:
	rsb
#endif

/*
 * Stray SCB interrupt catcher
 */
.text
SCBVEC(passrel):
	rei

	.data
	.align	2
#define PJ	PUSHR;jsb _Xstray
	.globl	_scb_stray
_scb_stray:
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
#if defined(VAX8600)
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
#endif VAX8600
#undef PJ
	.text
SCBVEC(stray):
	/*
	 * Calculate and report the vector location of the stray
	 * scb interrupt
	 */
	subl3	$_scb_stray+8,(sp)+,r0
	ashl	$-1,r0,-(sp)
	mfpr	$IPL,-(sp)
	pushl	$1		#ELSI_SCB
	calls $3, _logstray;	#log stray interrupt
	POPR
	rei

/*
 * Stray UNIBUS interrupt catch routines
 */
	.data
	.align	2
#define PJ	PUSHR;jsb _Xustray
	.globl	_catcher
_catcher:
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ
	PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ;PJ

	.globl	_cold
_cold:	.long	1

	.globl	_br
_br:	.long 	0

	.globl	_cvec
_cvec:	.long	0
	.data

	.text
SCBVEC(ustray):
	blbc	_cold,1f
	mfpr	$IPL,_br
	subl3	$_catcher+8,(sp)+,_cvec
	ashl	$-1,_cvec,_cvec
	POPR
	rei
1:
	subl3	$_catcher+8,(sp)+,r0
	ashl	$-1,r0,-(sp)
	mfpr	$IPL,-(sp)
	pushl	$2		#ELSI_UNI
	calls $3, _logstray;	#log stray interrupt
	POPR
	rei

/*
 * Trap and fault vector routines
 */
#define TRAP(a) pushl $T_/**/a; jbr alltraps

/*
 * Ast delivery (profiling and/or reschedule)
 */
SCBVEC(astflt):
	pushl $0; TRAP(ASTFLT)
SCBVEC(privinflt):
/*
 *	Check to make sure that we either have the floationg emulation code
 *	linked in or the entire emulation code
 */
	tstl	_vaxopcdec		# floating point emulation load ?
	bneq	2f			# br if yes
	tstl	_vaxemulbegin		# Is any emulation loaded in?
	beql	1f			# br if no
	bneq	3f			# br if yes

2:	jmp	*_vaxopcdec		# See if it is one that we can emulate
	.globl	_Xprivinflt1
_Xprivinflt1:				# Special symboll used by vax$opcdec if
					# instruction can not be emulated.
/*
 *	We check for the vax$special because this is the way the emulation
 *	code changes it's current mode to kernel
 */
3:
	cmpl	_vaxspecialhalt,(sp)	# Is this the special address
	bneq	1f			# br if no
	addl2	$(4*2),sp		# Pop all of the priv stuff off and
	jmp	*_vaxspecialcont	# jmp to emulation code to continue
					# it's ways
1:
	pushl $0; TRAP(PRIVINFLT)
SCBVEC(xfcflt):
	pushl $0; TRAP(XFCFLT)
SCBVEC(resopflt):
	pushl $0; TRAP(RESOPFLT)
SCBVEC(resadflt):
	pushl $0; TRAP(RESADFLT)
SCBVEC(bptflt):
	pushl $0; TRAP(BPTFLT)
SCBVEC(compatflt):
	TRAP(COMPATFLT);
SCBVEC(tracep):
	pushl $0; TRAP(TRCTRAP)
SCBVEC(arithtrap):
	TRAP(ARITHTRAP)
SCBVEC(protflt):

	tstl	_vaxemulbegin		# Emulation loaded in?
	beql	2f			# br if no
/*
 *	Check to see if the pc is in the emulation space, if so go
 *	to the emulation handler for the exception
 */
	cmpl	8(sp),_vaxemulbegin	# If less the vax$emul_being
	blssu	1f			# then br
	cmpl	8(sp),_vaxemulend	# If greater than vax$emul_end
	bgequ	1f			# then br
	jmp	*_exeacviolat		# else jmp to the emulator's
					# exception handler
1:
	.globl	_Xprotflt1
_Xprotflt1:				# Special symbol used to interface
					# the emulation code to ultrix
					# signalling
2:
	blbs	(sp)+,segflt
	TRAP(PROTFLT)
segflt:
	TRAP(SEGFLT)
SCBVEC(transflt):
	bitl	$2,(sp)+
	bnequ	tableflt
	jsb	Fastreclaim		# try and avoid pagein
	TRAP(PAGEFLT)
tableflt:
	TRAP(TABLEFLT)

alltraps:
	mfpr	$USP,-(sp); calls $0,_trap; mtpr (sp)+,$USP
	incl	_cnt+V_TRAP
	addl2	$8,sp			# pop type, code
	mtpr	$HIGH,$IPL		## dont go to a higher IPL (GROT)
	rei

SCBVEC(syscall):
	pushl	$T_SYSCALL
	mfpr	$USP,-(sp); calls $0,_syscall; mtpr (sp)+,$USP
	incl	_cnt+V_SYSCALL
	addl2	$8,sp			# pop type, code
	mtpr	$HIGH,$IPL		## dont go to a higher IPL (GROT)
	rei

SCBVEC(ipintr):
	tstl	_panicstr		# see if panicing
	beql	0f			# no, just reschedule
	jsb	cpuindex		# see who we are
	bneq	0f			# not master, don't worry (yet)
	pushab	1f			# panic message
	calls	$1,_panic		# we are master, go do final panic work
0:
	mtpr	$ASTLVL_USER,$ASTLVL	# need to reschedule, set ast
	rei

	.data
1:	.asciz	"secondary processor requested"
	.text

/*
 * System page table was here
 */

/*
 * Initialization
 *
 * ipl 0x1f; mapen 0; scbb, pcbb, sbr, slr, isp, ksp not set
 */

	.globl	_slavestart
_slavestart:
	# mtpr	$0,$TXCS		# shut down transmit interrupts
	mtpr	$0,$ICCS
	mtpr	$_scb-0x80000000,$SCBB
	mtpr	$_Sysmap-0x80000000,$SBR
	mtpr	$_Syssize,$SLR
/* double map the kernel into the virtual user addresses of phys mem */
	mtpr	$_Sysmap,$P0BR
	mtpr	$_Syssize,$P0LR
	mtpr	$0,$TBIA		# clean tb
	mtpr	$1,$MAPEN		# enable mapping
	jmp	*$0f			# run in virt space
0:
	bbs	$LOCK_ACTV,_lockword,0b	# wait for master, hold other slaves
	bbssi	$LOCK_ACTV,_lockword,0b
	cmpl	_maxcpu,_activecpu	# have we seen max conf'ed cpus?
	bneq	1f			# no, we're okay
	halt				# firewall, we are n+1st cpu, oops!
/* set ISP */
1:
	movl	_istack,r2		# base of stack
	addl3	r2,$(NISP+1)*NBPG,sp	# top of this stack
	bbcc	$31,r2,2f		# clear sys space bit
2:
	ashl	$-PGSHIFT,r2,r2		# make page offset
	bicl2	$PG_PROT,_Sysmap[r2]	# make read only as readzone
	bisl2	$PG_KR,_Sysmap[r2]
	mtpr	$0,$TBIA		# clean tb
	addl2	$(NISP+1)*NBPG,_istack	# advance for stack for next cpu
	jsb	cpuident		# who are we?
	movl	_activecpu,r2		# take next cpu slot
	mull2	$C_SIZE,r2
	addl2	$_cpudata,r2
	movl	r0,C_IDENT(r2)		# save our identity for matching
	movl	$1,C_NOPROC(r2)		# no user proc at this time
	bisl2	$CPU_RUN,C_STATE(r2)	# we are now running
	incl	_activecpu		# update active count
	bbcci	$LOCK_ACTV,_lockword,4f	# release for other slaves
4:
	pushl	r2			# save cpu data table index
	calls	$0,_startrtclock	# start interval timer
	movl	(sp)+,r3		# need table index in idle loop
	pushl	$0x04010000		# psl = mode(k,k) ipl=1 is=1
	pushl	$idle0
	rei				# rei into idle loop (best we can do)

	.data
	.globl	_cpu
	.globl	_cpu_subtype
	.globl	_vs_cfgtst
_cpu:	.long	0
_cpu_subtype:	.long	0
_vs_cfgtst:	.long	0
	.text
	.globl	start
start:
	.word	0
/*
 * If we are booted there should be an arg passed in.  If present, then
 * save the arg otherwise, assume the old boot path
 */
	tstl	(ap)			# arg present?
	beql	1f			# if not, skip ahead
	movl	4(ap),_vmbinfo		# otherwise, save the address passed
1:
/* set system control block base and system page table params */
	mtpr	$0,$TXCS		# shut down transmit interrupts
	mtpr	$0,$ICCS
	mtpr	$_scb-0x80000000,$SCBB
	mtpr	$_Sysmap-0x80000000,$SBR
	mtpr	$_Syssize,$SLR
/* double map the kernel into the virtual user addresses of phys mem */
	mtpr	$_Sysmap,$P0BR
	mtpr	$_Syssize,$P0LR
/* set ISP and get cpu type (and sub-type) */
	movl	$_intstack+NISP*NBPG,sp
	mfpr	$SID,r0
	movab	_cpu,r1
	extzv	$24,$8,r0,(r1)
	cmpl	$MVAX_II,_cpu
	bneq	1f
	movl	*$SID_EXT,r0
	movab	_cpu_subtype,r1
	extzv	$24,$8,r0,(r1)
	cmpl	$ST_VAXSTAR,_cpu_subtype
	bneq	1f
	clrb	*$VS_IORESET		# init VAXstar I/O controllers
	movzbl	*$VS_CFGTST,_vs_cfgtst	# save VAXstar config/test register
1:
/* init RPB */
	movl	_vmbinfo,r7		# point to the info list
	beql	1f			# is there one?
	movc3	INFO_RPBSIZ(r7),*INFO_RPBBAS(r7),_rpb
					# plug in VMB's RPB
	movl	INFO_MEMSIZ(r7),r7	# get the size of memory
1:
	movab	_rpb,r0
	movl	r0,(r0)+			# rp_selfref
	movab	_doadump,r1
	movl	r1,(r0)+			# rp_dumprout
	movl	$0x1f,r2
	clrl	r3
1:	addl2	(r1)+,r3; sobgtr r2,1b
	movl	r3,(r0)+			# rp_chksum
	clrl	(r0)				# make sure rp_flag is 0

/* slave starts executing at rpb+0x100.  Why?  Because VMS does it that way.
   This code fills in a jump instruction and somewhere to go */
	bicl3	$0x80000000,$_slavestart,_rpb+RP_BUGCHK	# slave start routine
	movl	$0xf9bf17,_rpb+RP_WAIT		# indirect jump of .-4

	tstl	r7				# was memsiz passed via VMB?
	bneq	9f				# if so, skip old mechanism
/*
 * THIS SECTION IS SKIPPED WHEN USING VMB TO BOOT ULTRIX - It is left
 * for backwards compatibility with older boot paths.
 * It counts up memory.
 */
/*
 * On microvaxen we've saved the memory size at address 0xf0000;
 * calculated by V1.x boot code.  This size is computed via the pfn bitmap.
 */
#define MEMSIZEADDR	0xf0000
	casel	_cpu,$1,$VAX_MAX
0:
	.word	1f-0b		# 1 is 780
	.word	1f-0b		# 2 is 750
	.word	1f-0b		# 3 is 730
	.word	1f-0b		# 4 is 8600
	.word	1f-0b		# 5 is 8200
	.word	1f-0b		# 6 is 8800
	.word	7f-0b		# 7 is MicroVAX 1
	.word	7f-0b		# 8 is MicroVAX Chip
5:
	movl	$0x00400000,r7		# get the size of memory
	jbr	9f
/* MVAX memsize was determined in the boot code */
7:
	movl	*$MEMSIZEADDR,r7		# get the size of memory
	brb	9f
/* count up memory by testing addresses at every 64K boundary until if fails */
1:	pushl	$4; pushl r7; calls $2,_bbadaddr; tstl r0; bneq 9f
	acbl	$MAX_MEM*1024-1,$64*1024,r7,1b
9:
/* clear memory from kernel bss and pages for proc 0 u. and page table */
	movab	_edata,r6
	movab	_end,r5
	bbcc	$31,r5,0f; 0:
	addl2	$(UPAGES*NBPG)+NBPG+NBPG,r5
1:	clrq	(r6); acbl r5,$8,r6,1b
/* trap() and syscall() save r0-r11 in the entry mask (per ../h/reg.h) */
/* panic() save r0-r11 in the entry mask also, for panic errlog. */
	bisw2	$0x0fff,_trap
	bisw2	$0x0fff,_syscall
	bisw2	$0x0fff,_panic
/* initialize system page table: scb and int stack writeable */
	clrl	r2
	movab	eintstack,r1; bbcc $31,r1,0f; 0: ashl $-PGSHIFT,r1,r1
1:	bisl3	$PG_V|PG_KW,r2,_Sysmap[r2]; aoblss r1,r2,1b
/* make rpb read-only as red zone for interrupt stack */
	bicl2	$PG_PROT,_rpbmap
	bisl2	$PG_KR,_rpbmap
/* make kernel text space read-only */
	movab	_etext+NBPG-1,r1; bbcc $31,r1,0f; 0: ashl $-PGSHIFT,r1,r1
1:	bisl3	$PG_V|PG_KR,r2,_Sysmap[r2]; aoblss r1,r2,1b
/* make kernel data, bss, read-write */
	movab	_end+NBPG-1,r1; bbcc $31,r1,0f; 0:; ashl $-PGSHIFT,r1,r1
1:	bisl3	$PG_V|PG_KW,r2,_Sysmap[r2]; aoblss r1,r2,1b

/*
 * Allow the user access to the kernel space containing
 * the emulation code.
 */
	tstl	_vaxemulbegin		# Emulation loaded in
	beql	2f			# br if no?
	addl3	_vaxemulend,$NBPG-1,r1; bbcc $31,r1,0f; 0: ashl $-PGSHIFT,r1,r1
	movl	_vaxemulbegin,r2; bbcc $31,r2,0f; 0: ashl $-PGSHIFT,r2,r2
1:	bisl3	$PG_V|PG_URKR,r2,_Sysmap[r2]; aoblss r1,r2,1b
2:

/*
 * If cpu type is uVAX I or uVAX II,
 * Test the processor to see if it has dfloat support. If not it could
 * either be a gfloat uVAX-I or a uVAX-II without the fpu.
 */
	cmpl	$MVAX_I,_cpu		# is cpu a uVAX I?
	bneq	1f			#   no, so see if its a uVAX II
	brb	2f			#   yes, so start the float test
1:
	cmpl	$MVAX_II,_cpu		# is cpu a uVAX II?
	bneq	3f			#   no, so skip this section
2:
	mfpr	$SCBB,r1
	movl	0x10(r1),r2		# Save the vector
	movab	0f,0x10(r1)		# Plug in our vector
	tstd	r0
/*
 * If we reach here then no trap occured.
 */
	incl	_fl_ok			# If no exception then set okay
	brb	1f
	.align	2			# must align to word !!!!
0:
	movab	1f,(sp) 		# Return to vector restoration
	rei
1:
	movl	r2,0x10(r1)		# Put the vector back
/*
 * Setup the scb vectors to point to the emulation code.
 */
	mfpr	$SCBB,r1
	movl	$vax$emulate,0xc8(r1)
	movl	$vax$emulate_fpd,0xcc(r1)
3:

/* now go to mapped mode */
	mtpr	$0,$TBIA; mtpr $1,$MAPEN; jmp *$0f; 0:
/* init mem sizes */
	ashl	$-PGSHIFT,r7,_maxmem
	movl	_maxmem,_physmem
	movl	_maxmem,_freemem
/* setup context for proc[0] == Scheduler */
	movab	_end+NBPG-1,r6
	bicl2	$NBPG-1,r6		# make page boundary
/* setup page table for proc[0] */
	bbcc	$31,r6,0f; 0:
	ashl	$-PGSHIFT,r6,r3 		# r3 = btoc(r6)
	bisl3	$PG_V|PG_KW,r3,_Usrptmap	# init first upt entry
	incl	r3
	movab	_usrpt,r0
	mtpr	r0,$TBIS
/* init p0br, p0lr */
	mtpr	r0,$P0BR
	mtpr	$0,$P0LR
/* init p1br, p1lr */
	movab	NBPG(r0),r0
	movl	$0x200000-UPAGES,r1
	mtpr	r1,$P1LR
	mnegl	r1,r1
	moval	-4*UPAGES(r0)[r1],r2
	mtpr	r2,$P1BR
/* setup mapping for UPAGES of _u */
	movl	$UPAGES,r2; movab _u+NBPG*UPAGES,r1; addl2 $UPAGES,r3; jbr 2f
1:	decl	r3
	moval	-NBPG(r1),r1;
	bisl3	$PG_V|PG_URKW,r3,-(r0)
	mtpr	r1,$TBIS
2:	sobgeq	r2,1b
/* initialize (slightly) the pcb */
	movab	UPAGES*NBPG(r1),PCB_KSP(r1)
	mnegl	$1,PCB_ESP(r1)
	mnegl	$1,PCB_SSP(r1)
	movl	r1,PCB_USP(r1)
	mfpr	$P0BR,PCB_P0BR(r1)
	mfpr	$P0LR,PCB_P0LR(r1)
	movb	$4,PCB_P0LR+3(r1)		# disable ast
	mfpr	$P1BR,PCB_P1BR(r1)
	mfpr	$P1LR,PCB_P1LR(r1)
	jsb	cpuident			# who are we?
	movl	_activecpu,r2			# take next cpu slot
	movl	r2,PCB_CPUNDX(r1)		# must be running on next cpu
	mull2	$C_SIZE,r2
	addl2	$_cpudata,r2
	movl	r0,C_IDENT(r2)			# save our identity for matching
	bisl2	$CPU_RUN,C_STATE(r2)		# we are now running
	incl	_activecpu			# update active count
	movl	$CLSIZE,PCB_SZPT(r1)		# init u.u_pcb.pcb_szpt
	movl	r11,PCB_R11(r1)
	movl	r10,PCB_R10(r1)
	movab	1f,PCB_PC(r1)			# initial pc
	movl	$0x1f0000,PCB_PSL(r1)		# mode(k,k), ipl=1f
	ashl	$PGSHIFT,r3,r3
	mtpr	r3,$PCBB			# first pcbb
/* set regs, p0br, p0lr, p1br, p1lr, astlvl, ksp and change to kernel mode */
	ldpctx
	rei
/* put signal trampoline code in u. area */
1:	movab	_u,r0
	movc3	$16,sigcode,PCB_SIGC(r0)
/* save reboot flags in global _boothowto */
	movl	r11,_boothowto
	movl	r10,_bootdevice
#ifdef defined(BINARY)
/*
 * compute login limit and save with boot flag
 * in high order word.
 * LOGLIMxxx are defined in cpu.h.
 * TODO:  add MicroVAX, and Superstar as well as other VAXen.
 */
	pushl	r11		# now, use r11 as place to do work
	casel	_cpu,$1,$VAX_MAX
0:
	.word	1f-0b		# 1 is 780
	.word	2f-0b		# 2 is 750
	.word	3f-0b		# 3 is 730
	.word	4f-0b		# 4 is 8600 (treated as 780)
	.word	9f-0b		# 5 is 8200
	.word	7f-0b		# 6 is 8800
	.word	5f-0b		# 7 is a MicroVAX I
	.word	5f-0b		# 8 is a MicroVAX Chip
	# fall here if cputype doesn't jive.
	brb 3f					# CPU not known. Should we do anything?
1:	# 780
	ashl	$16, $LOGLIM780, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
2:	# 750
	ashl	$16, $LOGLIM750, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
3:	# 730 and unknown's
	ashl	$16, $LOGLIM730, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
4:	# 8600
	ashl	$16, $LOGLIM8600, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
9:	# 8200
	ashl	$16, $LOGLIM8200, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
5:
	ashl	$16, $LOGLIMMVAX, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
7: #8800
	ashl	$16, $LOGLIM8800, r11		# move login limit to high word
	bisl2	(sp)+, r11			# OR bootflags back in
	brb 6f
6:
#endif BINARY
/* calculate firstaddr, and call main() */
	movab	_end+NBPG-1,r0; bbcc $31,r0,0f; 0:; ashl $-PGSHIFT,r0,-(sp)
	addl2	$UPAGES+1,(sp); calls $1,_main
/* proc[1] == /etc/init now running here; run icode */
	pushl	$PSL_CURMOD|PSL_PRVMOD; pushl $0; rei

/* signal trampoline code: it is known that this code takes exactly 16 bytes */
/* in ../vax/pcb.h and in the movc3 above */
sigcode:
	calls	$4,5(pc)			# params pushed by sendsig
	chmk	$139				# cleanup mask and onsigstack
	rei
	.word	0x7f				# registers 0-6 (6==sp/compat)
	callg	(ap),*16(ap)
	ret

/*
 * Primitives
 */

#ifdef VAX8800

SCBVEC(bi0err):
	PUSHR
	pushal 6*4(sp)
	pushl   $0		#bi 0
	calls $2,_bierrors
	POPR
	rei

SCBVEC(bi1err):
	PUSHR
	pushal 6*4(sp)
	pushl   $1		#bi 1
	calls $2,_bierrors
	POPR
	rei

SCBVEC(bi2err):
	PUSHR
	pushal 6*4(sp)
	pushl   $2		#bi 2
	calls $2,_bierrors
	POPR
	rei

SCBVEC(bi3err):
	PUSHR
	pushal 6*4(sp)
	pushl   $3		#bi 3
	calls $2,_bierrors
	POPR
	rei
#endif

	.data
	.globl	_ignorebi
_ignorebi: .long 1
	.text
	.globl	_bisst
_bisst:
	.word	0
	movl 	_ignorebi,r1
	movl	$1,_ignorebi
	movl	4(ap),r3
	bisl2	$BICTRL_NOARB,(r3)
	movl	$100,r0
	bisl2	$BICTRL_STS+BICTRL_SST,(r3)
1:
	sobgtr	r0,1b

2:
	movl	$100,r0

1:
	
	pushr	$0x1f
	pushl	$100000
	calls 	$1,_microdelay
	popr	$0x1f

	bitl	$BICTRL_BROKE,(r3)
	beql	7f

	decl	r0
	bneq	1b
	
	incl 	r0		
	movl	r1,_ignorebi
	ret

7:
	clrl	r0
	movl	r1,_ignorebi
	ret

/*
 * bbadaddr(addr, len)
 *	see if access addr with a len type instruction causes a machine check
 *	len is length of access (1=byte, 2=short, 4=long)
 */
	.globl	_bbadaddr
_bbadaddr:
	.word	0
	movl	$1,r0
	mfpr	$IPL,r1
	mtpr	$HIGH,$IPL
	movl	_scb+MCKVEC,r2
	movl	4(ap),r3
	movl	8(ap),r4
	movab	9f+INTSTK,_scb+MCKVEC
	bbc	$0,r4,1f; tstb	(r3)
1:	bbc	$1,r4,1f; tstw	(r3)
1:	bbc	$2,r4,1f; tstl	(r3)

	movl	$100,r4
1:
	sobgtr r4,1b

	clrl	r0			# made it w/o machine checks
2:	movl	r2,_scb+MCKVEC
	mtpr	r1,$IPL
	ret
	.align	2
9:
	casel	_cpu,$1,$VAX_MAX
0:
	.word	8f-0b		# 1 is 780
	.word	5f-0b		# 2 is 750
	.word	5f-0b		# 3 is 730
	.word	4f-0b		# 4 is 8600
	.word	5f-0b		# 5 is 8200
	.word	6f-0b		# 6 is 8800
	.word	5f-0b		# 7 is MicroVAX 1
	.word	5f-0b		# 8 is MicroVAX Chip
6:
	calls	$0,_nmifaultclear
5:
#if defined(VAX750) || defined(VAX730) || defined (MVAX) || defined(VAX8200)
	mtpr	$0xf,$MCESR
#endif
	brb	1f
8:
#if VAX780
	mtpr	$0xc0000,$SBIFS /* set sbi interrupts and clear fault latch */
	brb	1f
#endif VAX780
4:
#if defined(VAX8600)
	mtpr	$0,$EHSR
	brb	1f
#endif VAX8600
1:
	addl2	(sp)+,sp		# discard mchchk trash
	movab	2b,(sp)
	rei
/*
 * non-local goto's
 */
#ifdef notdef		/* this is now expanded completely inline */
	.globl	_Setjmp
_Setjmp:
	movl	fp,(r0)+	# current stack frame
	movl	(sp),(r0)	# resuming pc
	clrl	r0
	rsb
#endif

#define PCLOC 16	/* location of pc in calls frame */
#define APLOC 8 	/* location of ap,fp in calls frame */

	.globl	_Longjmp
_Longjmp:
	movl	(r0)+,newfp	# must save parameters in memory as all
	movl	(r0),newpc	# registers may be clobbered.
1:
	cmpl	fp,newfp	# are we there yet?
	bgequ	2f		# yes
	moval	1b,PCLOC(fp)	# redirect return pc to us!
	ret			# pop next frame
2:
	beql	3f		# did we miss our frame?
	pushab	4f		# yep ?!?
	calls	$1,_panic
3:
	movl	newpc,r0	# all done, just return to the `setjmp'
	jmp	(r0)		# ``rsb''

	.data
newpc:	.space	4
newfp:	.space	4
4:	.asciz	"longjmp"
	.text
/*
 * setjmp that saves all registers as the call frame may not
 * be available to recover them in the usual mannor by longjmp.
 * Called before swapping out the u. area, restored by resume()
 * below.
 */

	.globl	_savectx
	.align	1
_savectx:
	.word	0
	movl	4(ap),r0
	movq	r6,(r0)+
	movq	r8,(r0)+
	movq	r10,(r0)+
	movq	APLOC(fp),(r0)+ # save ap, fp
	addl3	$8,ap,(r0)+	# save sp
	movl	PCLOC(fp),(r0)	# save pc
	clrl	r0
	ret

	.globl	_whichqs
	.globl	_qs
	.globl	_cnt
	.globl	_runrun
/*
 * The following primitives use the fancy VAX instructions
 * much like VMS does.	_whichqs tells which of the 32 queues _qs
 * have processes in them.  Setrq puts processes into queues, Remrq
 * removes them from queues.  The running process is on no queue,
 * other processes are on a queue related to p->p_pri, divided by 4
 * actually to shrink the 0-127 range of priorities into the 32 available
 * queues.
 */

/*
 * Setrq(p), using fancy VAX instructions.
 *
 * Call should be made at spl6(), and p->p_stat should be SRUN
 */
	.globl	_Setrq		# <<<massaged to jsb by "asm.sed">>>
_Setrq:
	tstl	P_RLINK(r0)		## firewall: p->p_rlink must be 0
	bneq	set1			##
	movzbl	P_PRI(r0),r1		# put on queue which is p->p_pri / 4
	ashl	$-2,r1,r1
	movaq	_qs[r1],r2
	insque	(r0),*4(r2)		# at end of queue
	bbss	r1,_whichqs,set0	# mark queue non-empty
set0:
	rsb
set1:
	pushab	set2			##
	calls	$1,_panic		##

set2:	.asciz	"setrq"

/*
 * Remrq(p), using fancy VAX instructions
 *
 * Call should be made at spl6().
 */
	.globl	_Remrq		# <<<massaged to jsb by "asm.sed">>>
_Remrq:
	movzbl	P_PRI(r0),r1
	ashl	$-2,r1,r1
	bbcc	r1,_whichqs,rem1
	remque	(r0),r2
	beql	rem0
	bbss	r1,_whichqs,rem0
rem0:
	clrl	P_RLINK(r0)		## for firewall checking
	rsb

rem1:
	pushab	rem2			# it wasn't recorded to be on its q
	calls	$1,_panic

rem2:	.asciz	"remrq"

sw0:	.asciz	"swtch"

/*
 * When no processes are on the runq, Swtch branches to idle
 * to wait for something to come ready.
 */
idle2:
	mtpr	$0x1,$IPL			# reenable in case in lock wait
	mtpr	$0x18,$IPL			# no clock while locked
	bbs	$LOCK_RQ,_lockword,idle2	# relock run queue
	bbssi	$LOCK_RQ,_lockword,idle2
	movab	0f,_lastlock			# record who got lock
0:
	brb	sw1
	.globl	Idle
Idle: idle:
	bbcci	$LOCK_RQ,_lockword,idle0	# free run queue
idle0:
	mtpr	$1,$IPL 		# must allow interrupts here
	tstl	_whichqs		# see if anything in run queues
	bneq	idle2			# something to do
	brb	idle0			# nothing to do, wait for change

/*
 * Swtch(), using fancy VAX instructions
 */
/*
 *	Commentary on the "yech" code -- depp
 *
 *	This code consists of storing away (in the process' PCB) the
 *	contents of CMAP2 before saving process context and restoring it
 *	to CMAP2 following the loading of process context.  
 *
 *	CMAP2 is only used in copyseg() (during segment	duplication during 
 *	a traditional fork()).  This value is stored away to insure that 
 *	if a page fault occurs in copyseg() that the contents of CMAP2 
 *	will not be corrupted if a subsequent fork() (and copyseg) occurs 
 *	while the process is blocked.  The process WILL block if while doing
 *	the "movc3" instruction in copyseg() the parent's page (the page being
 *	copied) is not valid or reclaimable.  
 */
	.globl	_Swtch
_Swtch: 			# <<<massaged to jsb by "asm.sed">>>
	mull3	_u+PCB_CPUNDX,$C_SIZE,r3
	addl2	$_cpudata,r3
	movl	$1,C_NOPROC(r3)
	incl	_cnt+V_SWTCH
	mtpr	$0x18,$IPL		# must be non-zero when on intr stack
	tstl 	_u+PCB_CPUNDX
	bneq	sw2
	movl	_CMAP2,_u+PCB_CMAP2	# yech
sw2:
	svpctx
sw1:
	cmpl	$_cpudata,r3		# see if we are master cpu
	bneq	slv			# no, do limited queue checking
	ffs	$0,$32,_whichqs,r0	# look for non-empty queue
	bneq	sw4			# Found queue, go process
	brb	idle			# if none, idle loop
slv:
	tstl	_slavehold		# since we are slave, we might be held
	bneq	idle
	ffs	$12,$20,_whichqs,r0	# slave only check > P_USER
	beql	idle
slvsw:
	bbc	r0,_whichqs,slvnext	# nothing in this queue, try next
	movaq	_qs[r0],r1		# get queue header
	movl	(r1),r2			# get addr of first queue element
slvloop:
	cmpl	r1,r2			# if at end of queue, try next
	beql	slvnext
	bitl	$SMASTER,P_FLAG(r2)	# see if proc must be master only
	beql	slvout			# no, our search is over
	movl	(r2),r2			# try next queue element
	brb	slvloop
slvnext:
	incl	r0			# try next queue
	cmpl	$32,r0			# see if all queues tried
	bneq	slvsw			# no, we keep going
	brw	idle			# yes, back to the garage
slvout:
	remque	(r2),r2			# Dequeue the lucky winner
	bvc	sw5			# if successfull, back to common code
	brb	badsw			# dequeue failed, this is bad news
sw4:
	movaq	_qs[r0],r1
	remque	*(r1),r2		# r2 = p = highest pri process
	bvc	sw5			# make sure something was there
badsw:
	bbcci	$LOCK_RQ,_lockword,bdsw1 # free run queue if error
bdsw1:
	pushab	sw0
	calls	$1,_panic
	/*NOTREACHED*/

sw5:	bneq	sw6
	bbcc	r0,_whichqs,sw6		# still more procs in this queue
sw6:
	clrl	_runrun
	clrq	C_RUNRUN(r3)		# note also clears noproc
	incl	C_SWITCH(r3)		# monitor switches
	movl	r2,C_PROC(r3)
	tstl	P_WCHAN(r2)		## firewalls
	bneq	badsw			##
	cmpb	P_STAT(r2),$SRUN	##
	bneq	badsw			##
	clrl	P_RLINK(r2)		##
	bitl	$CPU_TBI,C_STATE(r3)	# see if TBI is needed
	beql	sw7			# nope
	mtpr	$0,$TBIA		# clear TB
	bicl2	$CPU_TBI,C_STATE(r3)	# clear TBI request
sw7:
	movl	*P_ADDR(r2),r0
	movl	r0,C_PADDR(r3)		# save for adb backtrace
	bbcci	$LOCK_RQ,_lockword,sw8	# clear lock
sw8:
	ashl	$PGSHIFT,r0,r0		# r0 = pcbb(p)
	brb	res0
/* fall into... */

/*
 * Resume(pf)
 */
	.globl	_Resume 	# <<<massaged to jsb by "asm.sed">>>
_Resume:
	tstl 	_u+PCB_CPUNDX
	bneq	res2
	movl	_CMAP2,_u+PCB_CMAP2	# yech
res2:
	mtpr	$0x18,$IPL		# vax rei requires new ipl <= current
	svpctx
res0:
	mtpr	r0,$PCBB
	ldpctx
	jsb	cpuindex		# record where we are running
	movl	r0,_u+PCB_CPUNDX
	bneq	res3
	movl	_u+PCB_CMAP2,_CMAP2	# yech
	mtpr	$_CADDR2,$TBIS
res3:
	tstl	_u+PCB_SSWAP
	bneq	res1
	rei

res1:
	movl	_u+PCB_SSWAP,r0 		# longjmp to saved context
	clrl	_u+PCB_SSWAP
	movq	(r0)+,r6
	movq	(r0)+,r8
	movq	(r0)+,r10
	movq	(r0)+,r12
	movl	(r0)+,r1
	cmpl	r1,sp				# must be a pop
	bgequ	1f
	pushab	2f
	calls	$1,_panic
	/* NOTREACHED */
1:
	movl	r1,sp
	movl	(r0),(sp)			# address to return to
	movl	$PSL_PRVMOD,4(sp)		# ``cheating'' (jfr)
	rei

2:	.asciz	"ldctx"

/*
 * Determine identity of cpu we are running on and possibly its table index
 *	Note: Due to dependencies in _start, we can not touch r1 + r3 in
 *	the cpuident case !!!
 */

	.globl	cpuident
cpuident:
	movl	$1,r4				# indicate we only want ident
	brb	cpui0

	.globl	cpuindex
cpuindex:
	clrl	r4				# indicate we want index
cpui0:
	casel	_cpu,$1,$VAX_MAX
0:
	.word	3f-0b		# 1 is 780
	.word	3f-0b		# 2 is 750
	.word	3f-0b		# 3 is 730
	.word	3f-0b		# 4 is 8600
	.word	1f-0b		# 5 is 8200
	.word	2f-0b		# 6 is 8800
	.word	3f-0b		# 7 is MVAXI
	.word	3f-0b		# 8 is MVAXII

1:
#ifdef	VAX8200
	mfpr	$BINID,r0		# VAX8200
#endif	VAX8200
	brb	cpui1

2:
#ifdef	VAX8800
	mfpr	$SID,r0			# VAX8800 - no need to mask exact bit
					# just need something unique
#endif	VAX8800
	brb	cpui1

3:
	clrl	r0			# no choice for uniprocessors

cpui1:
	tstl	r4			# done if all we want is ident
	bneq	3f

/*
 * Determine table index of designated cpu
 */

	movl	_activecpu,r1		# number of entries to search
	mull3	$C_SIZE,r1,r2
	addl2	$_cpudata+C_IDENT,r2	# table offset of entry
1:
	decl	r1			# check next highest entry
	subl2	$C_SIZE,r2
	cmpl	(r2),r0			# have we found what we seek
	beql	2f			# yes, return value
	tstl	r1			# see if we are out of table
	bneq	1b			# no, keep looking
	pushab	4f			# firewall
	calls	$1,_panic
2:
	movl	r1,r0			# return value is index
3:
	rsb
4:
	.asciz	"cpuindex"		# panic message

/*
 * interrupt one of the processors
 */
ENTRY(intrcpu, 0)
	casel	_cpu,$1,$VAX_MAX
0:
	.word	9f-0b		# 1 is 780
	.word	9f-0b		# 2 is 750
	.word	9f-0b		# 3 is 730
	.word	9f-0b		# 4 is 8600
	.word	1f-0b		# 5 is 8200
	.word	4f-0b		# 6 is 8800
	.word	9f-0b		# 7 is MVAXI
	.word	9f-0b		# 8 is MVAXII

1:
#ifdef	VAX8200
	mull3	4(ap),$C_SIZE,r2	# index for designated cpu
	addl2	$_cpudata+C_IDENT,r2	# table offset of cpu ids
	movl	$1,r3			# single bit
	ashl	(r2),r3,r3		# make into proc mask
	mtpr	r3,$IPIR		# zot the other processor
#endif	VAX8200
	ret

4:
#ifdef	VAX8800
	mtpr	$0,$INOP		# VAX8800
#endif	VAX8800
	ret

9:	movl	$0,r0			# no choice for uniprocessors
	ret


#ifdef THIS_IS_COMMENTED_OUT
/* I have put these two routines in inline. They are heavily used
 * in creating zero filled pages (5 places in kernel) and in fork(vmdup())
 * where the pages are copied
 */
/*
 * Copy 1 relocation unit (NBPG bytes)
 * from user virtual address to physical address
 */
_copyseg:	.globl	_copyseg
	.word	0x0
	bisl3	$PG_V|PG_KW,8(ap),_CMAP2
	mtpr	$_CADDR2,$TBIS	# invalidate entry for copy
	movc3	$NBPG,*4(ap),_CADDR2
	ret

/*
 * zero out physical memory
 * specified in relocation units (NBPG bytes)
 */
_clearseg:	.globl	_clearseg
	.word	0x0
	bisl3	$PG_V|PG_KW,4(ap),_CMAP1
	mtpr	$_CADDR1,$TBIS
	movc5	$0,(sp),$0,$NBPG,_CADDR1
	ret
#endif THIS_IS_COMMENTED_OUT
/*
 * kernacc - check for kernel access privileges
 *
 * We can't use the probe instruction directly because
 * it ors together current and previous mode.
 */
	.globl	_kernacc
_kernacc:
	.word	0x0
	movl	4(ap),r0	# virtual address
	bbcc	$31,r0,kacc1
	bbs	$30,r0,kacerr
	mfpr	$SBR,r2 	# address and length of page table (system)
	bbss	$31,r2,0f; 0:
	mfpr	$SLR,r3
	brb	kacc2
kacc1:
	bbsc	$30,r0,kacc3
	mfpr	$P0BR,r2	# user P0
	mfpr	$P0LR,r3
	brb	kacc2
kacc3:
	mfpr	$P1BR,r2	# user P1 (stack)
	mfpr	$P1LR,r3
kacc2:
	addl3	8(ap),r0,r1	# ending virtual address
	addl2	$NBPG-1,r1
	ashl	$-PGSHIFT,r0,r0
	ashl	$-PGSHIFT,r1,r1
	bbs	$31,4(ap),kacc6
	bbc	$30,4(ap),kacc6
	cmpl	r0,r3		# user stack
	blss	kacerr		# address too low
	brb	kacc4
kacc6:
	cmpl	r1,r3		# compare last page to P0LR or SLR
	bgtr	kacerr		# address too high
kacc4:
	movl	(r2)[r0],r3
	bbc	$31,4(ap),kacc4a
	bbc	$31,r3,kacerr	# valid bit is off
kacc4a:
	cmpzv	$27,$4,r3,$1	# check protection code
	bleq	kacerr		# no access allowed
	tstb	12(ap)
	bneq	kacc5		# only check read access
	cmpzv	$27,$2,r3,$3	# check low 2 bits of prot code
	beql	kacerr		# no write access
kacc5:
	aoblss	r1,r0,kacc4	# next page
	movl	$1,r0		# no errors
	ret
kacerr:
	clrl	r0		# error
	ret
/*
 * Extracted and unrolled most common case of pagein (hopefully):
 *	resident and not on free list (reclaim of page is purely
 *	for the purpose of simulating a reference bit)
 *
 * Built in constants:
 *	CLSIZE of 2, USRSTACK of 0x7ffff000, any bit fields
 *	in pte's or the core map
 */
	.text
	.globl	Fastreclaim
Fastreclaim:
	PUSHR
	extzv	$9,$23,28(sp),r3	# virtual address
	bicl2	$1,r3			# v = clbase(btop(virtaddr));
	movl	_u+U_PROCP,r5		# p = u.u_procp
					# from vtopte(p, v) ...
	cmpl	r3,P_TSIZE(r5)
	jgequ	2f			# if (isatsv(p, v)) {
	ashl	$2,r3,r4
	addl2	P_P0BR(r5),r4		#	tptopte(p, vtotp(p, v));
	movl	$1,r2			#	type = CTEXT;
	jbr	3f
2:
	subl3	P_SSIZE(r5),$(0x400000-UPAGES),r0
	cmpl	r3,r0
	jgequ	2f			# } else if (isadsv(p, v)) {
	ashl	$2,r3,r4
	addl2	P_P0BR(r5),r4		#	dptopte(p, vtodp(p, v));
	clrl	r2			#	type = !CTEXT;
	jbr	3f
2:
	cvtwl	P_SZPT(r5),r4		# } else (isassv(p, v)) {
	ashl	$7,r4,r4
	subl2	$0x400000,r4
	addl2	r3,r4
	ashl	$2,r4,r4
	addl2	P_P0BR(r5),r4		#	sptopte(p, vtosp(p, v));
	clrl	r2			#	type = !CTEXT;
3:					# }
	bitb	$0x82,3(r4)
	beql	2f			# if (pte->pg_v || pte->pg_fod)
	POPR; rsb			#	let pagein handle it
2:
	bitb	$0x20,2(r4)
	beql	2f			# if (pte->pg_alloc)
	POPR; rsb			#	let pagein handle it
2:
	bicl3	$0xffe00000,(r4),r0
	jneq	2f			# if (pte->pg_pfnum == 0)
	POPR; rsb			#	let pagein handle it
2:
	subl2	_firstfree,r0
	ashl	$-1,r0,r0
	incl	r0			# pgtocm(pte->pg_pfnum)
	mull2	$CMAPSZ,r0
	addl2	_cmap,r0		# &cmap[pgtocm(pte->pg_pfnum)]
	tstl	r2
	jeql	2f			# if (type == CTEXT &&
	jbc	$CMAP_INTRANS,(r0),2f	#     c_intrans)
	POPR; rsb			#	let pagein handle it
2:
	jbc	$CMAP_FREE,(r0),2f	# if (c_free)
	POPR; rsb			#	let pagein handle it
2:
	bisb2	$0x80,3(r4)		# pte->pg_v = 1;
	jbc	$26,4(r4),2f		# if (anycl(pte, pg_m)
	bisb2	$0x04,3(r4)		#	pte->pg_m = 1;
2:
	bicw3	$0x7f,2(r4),r0
	bicw3	$0xff80,6(r4),r1
	bisw3	r0,r1,6(r4)		# distcl(pte);
	ashl	$PGSHIFT,r3,r0
	mtpr	r0,$TBIS
	addl2	$NBPG,r0
	mtpr	r0,$TBIS		# tbiscl(v);
	tstl	r2
	jeql	2f			# if (type == CTEXT)
	movl	P_TEXTP(r5),r0
	movl	X_CADDR(r0),r5		# for (p = p->p_textp->x_caddr; p; ) {
	jeql	2f
	ashl	$2,r3,r3
3:
	addl3	P_P0BR(r5),r3,r0	#	tpte = tptopte(p, tp);
	bisb2	$1,P_FLAG+3(r5) 	#	p->p_flag |= SPTECHG;
	movl	(r4),(r0)+		#	for (i = 0; i < CLSIZE; i++)
	movl	4(r4),(r0)		#		tpte[i] = pte[i];
	movl	P_XLINK(r5),r5		#	p = p->p_xlink;
	jneq	3b			# }
2:					# collect a few statistics...
	incl	_u+U_RU+RU_MINFLT	# u.u_ru.ru_minflt++;
	moval	_cnt,r0
	incl	V_FAULTS(r0)		# cnt.v_faults++;
	incl	V_PGREC(r0)		# cnt.v_pgrec++;
	incl	V_FASTPGREC(r0) 	# cnt.v_fastpgrec++;
	incl	V_TRAP(r0)		# cnt.v_trap++;
	POPR
	addl2	$8,sp			# pop pc, code
	mtpr	$HIGH,$IPL		## dont go to a higher IPL (GROT)
	rei

	.data
_qioentry:
	.long 0

	.data
	.globl	_fl_ok
_fl_ok: .long	0
	.text
