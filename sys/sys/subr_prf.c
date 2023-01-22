#ifndef lint
static char *sccsid = "@(#)subr_prf.c	1.21	ULTRIX	1/15/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1983,86 by			*
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
 * Maintenance History
 *
 * 14-Jan-87 -- pmk
 *	Added check for valid ksp in panic()
 *
 * 04-Dec-86 -- pmk
 *	Changed mprintf errlog amount to 256 char., added intr & 
 *	kernel stack address to errlog stack info. and changed
 *	how intr. stack amount is calculated
 *
 * 22-Jul-86 -- bjg
 *	Change appendflg to log ALL startup messages together
 *
 * 09-Jun-86 -- bjg
 *	Added check for appendflg to log startup mesgs
 *
 * 30-May-86 -- pmk 
 *	Added prtstk routine to format/print stack dumps with call frame
 *	lables.
 *
 * 02-Apr-86 -- jrs
 *	Clean up panic so slaves can panic and notify master to bring
 *	whole system down cleanly
 *
 * 19-Mar-86 -- pmk
 *	Cleaned up panic delay and changed output format.
 *
 * 18-Mar-86 -- jrs
 *	Cleaned up cpu determination.  Allow slave to printf now
 * 
 * 05-Mar-86 pmk
 *	Added append option to printf for autoconf.
 *	Changed delay depend for panic printout.
 *
 * 12-Feb-86 pmk
 *	Changed errlogging of asc printf & mprintf.
 *	Added cpu depend delay to panic printout.
 *
 * 05-Feb-86 pmk
 *	Fixed the way panic string is null terminated.
 *	Added use of NISP from param.h for stack size.
 *
 * 20-jan-86 bglover
 *	Added size field support for %s in printf, mprintf, cprintf
 *
 * 20-jan-86 pmk
 *	Added binary error logging support for panic, printf, mprintf.
 *	Added cprintf routine.
 *
 * 10/1/85 - Larry Cohen
 *	include ioctl.h so that tty.h knows about window size structure
 *
 * 22-Feb-84 tresvik
 *	Add mprintf for use with diagnostic printouts from drivers.
 *	Output does not go to the console, instead it goes to msgbuf.
 *
 * 	Added a carriage return to the end of the harderr output string
 *	format.
 */

/*	subr_prf.c	6.1	83/07/29	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/seg.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/reboot.h"
#include "../h/vm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/types.h"
#include "../h/errlog.h"
#include "../h/kmalloc.h"
#include "../h/cpudata.h"

#ifdef vax
#include "../vax/mtpr.h"
#include "../vax/cpu.h"
#endif

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */
char	*panicstr;

/* For configuration printf's to be errlogged together */
int	appendflg = 0;
char	*cfgbufp;

/* Used for logging ascii messages */
static int msgsize;

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 *
 * One additional format: %b is supported to decode error registers.
 * Usage is:
 *	printf("reg=%b\n", regval, "<base><arg>*");
 * Where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 32), give the name of the register.  Thus
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 * would produce output:
 *	reg=2<BITTWO,BITONE>
 *
 * printf will also accept a field number, zero filling to length.
 * 	printf(" %8x\n",regval); max field size is 11.
 *
 * printf will also log the ascii to the errlog.
 */
/*VARARGS1*/
printf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	struct el_rec *elrp;
	struct el_msg *elmsgp;
	caddr_t km_alloc();

	if ((appendflg) && (cfgbufp == 0)) {
	    cfgbufp = km_alloc(EL_SIZE2048,KM_CLRSG);
	    if (cfgbufp != NULL) {
	        elmsgp = (struct el_msg *)cfgbufp;
	        elmsgp->msg_len = 1;
	    }
	}
	if (cfgbufp != NULL) {
	    elmsgp = (struct el_msg *)cfgbufp;
	    elmsgp->msg_len--;
	    msgsize = EL_SIZE2048 - 1;
	    prf(elmsgp, fmt, &x1, 3);
	    elmsgp->msg_asc[elmsgp->msg_len++] = '\0';
	    if ((appendflg == 0) || (elmsgp->msg_len > EL_SIZEAPPND)) {
	        elrp = ealloc(elmsgp->msg_len + 2,EL_PRILOW);
	        if (elrp != NULL) {
	            LSUBID(elrp,ELMSGT_SU,EL_UNDEF,EL_UNDEF,
			        EL_UNDEF,EL_UNDEF,EL_UNDEF);
		    elrp->el_body.elmsg.msg_len = elmsgp->msg_len;
	    	    bcopy(elmsgp->msg_asc, elrp->el_body.elmsg.msg_asc,
			  elmsgp->msg_len);
	            EVALID(elrp);
		}
	        elmsgp->msg_len = 1;
		if (appendflg == 0) {
		    km_free(cfgbufp,EL_SIZE2048);
		    cfgbufp = NULL;
		}
	    }
	}
	else {
	    elrp = ealloc(EL_SIZE128 + 2,EL_PRILOW);
	    if (elrp != NULL) {
	        LSUBID(elrp,(!appendflg)?ELMSGT_INFO:ELMSGT_SU,
			    EL_UNDEF,EL_UNDEF,
			    EL_UNDEF,EL_UNDEF,EL_UNDEF);
	        elmsgp = &elrp->el_body.elmsg;
	        elmsgp->msg_len = 0;
		msgsize = EL_SIZE128 - 1;
	        prf(elmsgp, fmt, &x1, 0);
	        elmsgp->msg_asc[elmsgp->msg_len++] = '\0';
	        EVALID(elrp);
	    }
	    else
	        prf((struct el_msg *)0, fmt, &x1, 0);
	}
}

/*
 * Uprintf prints to the current user's terminal,
 * guarantees not to sleep (so can be called by interrupt routines)
 * and does no watermark checking - (so no verbose messages).
 * Also uprintf doesn't log msg to errlog pass null pointer.
 */
/*VARARGS1*/
uprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{

	prf((struct el_msg *)0, fmt, &x1, 2);
}

/* 
 * Cprintf prints ONLY to the console. This is for reporting 
 * information when the errlog mech. has a problem. You don't
 * want to call your self, passes null pointer.
 */
/*VARARGS1*/
cprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{

	prf((struct el_msg *)0, fmt, &x1, 0);
}

/* 
 * Mprintf is used whenever Kernel printout should appear in the messages
 * file via the Kernel message buffers but need not be printed on the console
 * terminal, i.e. hardware failures which are considered soft and corrected.
 *
 * Mprintf will log all messages to errlog by ealloc and evalid.
 */
/*VARARGS1*/
mprintf(fmt, x1)
	char *fmt;
	unsigned x1;
{
	struct el_rec *elrp;
	struct el_msg *elmsgp;
	
	elrp = ealloc(EL_SIZE256 + 2,EL_PRILOW);
	if (elrp != NULL) {
	    LSUBID(elrp,ELMSGT_INFO,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
	    elmsgp = &elrp->el_body.elmsg;
	    elmsgp->msg_len = 0;
	    msgsize = EL_SIZE256 - 1;
	    prf(elmsgp, fmt, &x1, 1);
	    elmsgp->msg_asc[elmsgp->msg_len++] = '\0';
	    EVALID(elrp);
	}
}

/*
 * Prf now accepts a logmsg pointer which in turns passes to printn()
 * and putchar(). Also prf now looks for a field length number to
 * pass to printn().
 */
prf(msgp, fmt, adx, touser)
	struct el_msg *msgp;
	register char *fmt;
	register u_int *adx;
{
	register int b, c, i;
	char *s;
	int any;
	int num;

loop:
	while ((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(msgp, c, touser);
	}
	num = 0;
again:
	c = *fmt++;
	/* THIS CODE IS VAX DEPENDENT IN HANDLING %l? AND %c */
	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		if (num == 0)
			num = c - '0';
		else
			num = 10 * num + c - '0';
		goto again;
	case 'l':
		goto again;
	case 'x': case 'X':
		b = 16;
		goto number;
	case 'd': case 'D':
	case 'u':		/* what a joke */
		b = 10;
		goto number;
	case 'o': case 'O':
		b = 8;
number:
		printn(msgp, num, (u_long)*adx, b, touser);
		break;
	case 'c':
		b = *adx;
		for (i = 24; i >= 0; i -= 8)
			if (c = (b >> i) & 0x7f)
				putchar(msgp, c, touser);
		break;
	case 'b':
		b = *adx++;
		s = (char *)*adx;
		printn(msgp, num, (u_long)b, *s++, touser);
		any = 0;
		if (b) {
			putchar(msgp, '<', touser);
			while (i = *s++) {
				if (b & (1 << (i-1))) {
					if (any)
						putchar(msgp, ',', touser);
					any = 1;
					for (; (c = *s) > 32; s++)
						putchar(msgp, c, touser);
				} else
					for (; *s > 32; s++)
						;
			}
			if (any)
				putchar(msgp, '>', touser);
		}
		break;

	case 's':
		s = (char *)*adx;
		if (num > 0) {
			while ((c = *s++) && (num-- > 0))
				putchar(msgp, c, touser);
		}
		else {
			while (c = *s++) 
				putchar(msgp, c, touser);
		}
		break;

	case '%':
		putchar(msgp, '%', touser);
		break;
	}
	adx++;
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 *
 * Printn now accepts msgp which it passes to putchar().
 * Also the num parm is used to determine field length, zero filling.
 */
printn(msgp, num, n, b, touser)
	struct el_msg *msgp;
	int num;
	u_long n;
{
	char prbuf[11];
	register char *cp;
	int i = 0;

	if (num > 11)
		num = 11;

	if (b == 10 && (int)n < 0) {
		putchar(msgp, '-', touser);
		n = (unsigned)(-(int)n);
	}
	cp = prbuf;
	do {
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
		i++;
		if (n == 0 && i < num )
			while ( i < num) {
				*cp++ = '0';
				i++;
			}
	} while (n);
	do
		putchar(msgp, *--cp, touser);
	while (cp > prbuf);
}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then reboots.
 * If we are called twice, then we avoid trying to
 * sync the disks as this often leads to recursive panics.
 */
panic(s)
	char *s;
{
	register int *reg11;		/* must be first reg. declaration */
	int bootopt = RB_AUTOBOOT;
	int i;
	int num1 = 0;
	int num2 = 0;
	char *strp;
	int *regp;
	struct el_rec *elrp;
	struct el_pnc *elpp;
	int cpindex;
	int fp, ap1, ap2;

	asm("movl r13,r11");
	slavehold = 1;
	cpindex = cpuindex();

	cprintf("\npanic: %s\n", s);
	if ((cpudata[cpindex].c_state & CPU_PANIC) != 0) {
		if (cpindex != 0) {
			/* if we are slave, just stop on multiple panic */
			while (1) {
				;
			}
		}
		bootopt |= RB_NOSYNC;
		boot(RB_PANIC, bootopt);
		/* NO RETURN */
	}
	else {
		panicstr = s;
		cpudata[cpindex].c_state |= CPU_PANIC;
	}

	elrp = ealloc(EL_PNCSIZE, EL_PRISEVERE);
	if (elrp != NULL) {
	    LSUBID(elrp,ELSW_PNC,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
	    elpp = &elrp->el_body.elpnc;
	    regp = &elpp->pncregs.pnc_ksp;

	    strp = s;
	    for (i = 1; *strp++ != '\0' && i < EL_SIZE64; i++) ;
	    bcopy(s, elpp->pnc_asc, i);
	    elpp->pnc_asc[i-1] = '\0';

	    elpp->pnc_sp = (int)reg11++;
	    elpp->pnc_ap = *++reg11;
	    elpp->pnc_fp = *++reg11;
	    elpp->pnc_pc = *++reg11;

	    for (i = 0; i < 32; i++)
	        if ((EL_REGMASK & (1 << i)) != 0)
		    *regp++ = mfpr(i);

/* Verify ksp is valid, using -
   kernacc(addr, bcnt, rw) caddr_t addr; unsigned bcnt; { return (0); } */

	    if (kernacc((caddr_t)elpp->pncregs.pnc_ksp, 4, B_READ)) {
	        num1 = 0x80000000 - elpp->pncregs.pnc_ksp;
	        if (num1 > EL_STKDUMP || num1 < 0)
	            num1 = EL_STKDUMP;
	        if (kernacc((caddr_t)(elpp->pncregs.pnc_ksp+(num1-4)),4,B_READ)) {
	            bcopy((char *)elpp->pncregs.pnc_ksp,
		          (char *)elpp->kernstk.stack, num1);
		}
		else num1 = 0;
	    }
	    elpp->kernstk.addr = elpp->pncregs.pnc_ksp;
	    elpp->kernstk.size = num1;
	    if (num1 < EL_STKDUMP) 
	        bzero((char *)elpp->kernstk.stack + num1,EL_STKDUMP - num1);
	    
	    num2 = ((elpp->pncregs.pnc_isp + 511) & (~0x1ff)) - 
		     elpp->pncregs.pnc_isp;
	    if (num2 > EL_STKDUMP || num2 < 0)
	        num2 = EL_STKDUMP;
	    bcopy((char *)elpp->pncregs.pnc_isp,(char *)elpp->intstk.stack,num2);
	    elpp->intstk.addr = elpp->pncregs.pnc_isp;
	    elpp->intstk.size = num2;
	    if (num2 < EL_STKDUMP) 
	        bzero((char *)elpp->intstk.stack + num2,EL_STKDUMP - num2);
	    
	    EVALID(elrp);

	    cprintf("sp\t= %8x\tap\t= %8x\tfp\t= %8x\n",
		    elpp->pnc_sp,elpp->pnc_ap,elpp->pnc_fp);
	    DELAY(250000);
	    cprintf("pc\t= %8x\tksp\t= %8x\tusp\t= %8x\n",
		    elpp->pnc_pc,elpp->pncregs.pnc_ksp,elpp->pncregs.pnc_usp);
	    DELAY(250000);
	    cprintf("isp\t= %8x\tp0pr\t= %8x\tp0lr\t= %8x\n",
		elpp->pncregs.pnc_isp,elpp->pncregs.pnc_p0br,elpp->pncregs.pnc_p0lr);
	    DELAY(250000);
	    cprintf("p1br\t= %8x\tp1lr\t= %8x\tsbr\t= %8x\n",
		elpp->pncregs.pnc_p1br,elpp->pncregs.pnc_p1lr,elpp->pncregs.pnc_sbr);
	    DELAY(250000);
	    cprintf("slr\t= %8x\tpcbb\t= %8x\tscbb\t= %8x\n",
		elpp->pncregs.pnc_slr,elpp->pncregs.pnc_pcbb,elpp->pncregs.pnc_scbb);
	    DELAY(250000);
	    cprintf("ipl\t= %8x\tastlvl\t= %8x\tsisr\t= %8x\n",
		elpp->pncregs.pnc_ipl,elpp->pncregs.pnc_astlvl,elpp->pncregs.pnc_sisr);
	    DELAY(250000);
	    cprintf("iccs\t= %8x\n\n",elpp->pncregs.pnc_iccs);

	    fp = elpp->pnc_sp;
	    ap1 = ap2 = 0;
	    cprintf("interrupt stack:\n");
	    prtstk(&fp,&ap1,&ap2,&elpp->pncregs.pnc_isp,&elpp->intstk);
	    cprintf("\nkernel stack:\n");
	    prtstk(&fp,&ap1,&ap2,&elpp->pncregs.pnc_ksp,&elpp->kernstk);
	    cprintf("\n");
	}

	if (cpindex != 0) {
		/* if we are slave, let master do the reboot */
		intrcpu(0);
		while (1) {
			;
		}
	}
	boot(RB_PANIC, bootopt);
}

/*
 * Format/print stack dumps with call frame lables
 * "*" beginning of call frame, "#" beginning of arg frame
 */
prtstk(fp,ap1,ap2,sp,stkp)
int *fp;
int *ap1;
int *ap2;
int *sp;
struct el_stkdmp *stkp;
{
	register int i, j, num;
	int next = 0;
	int regmsk = 0;

	cprintf("%08x: ",*sp);
	num = stkp->size >> 2;
	for (i = 0; i < num; i++) {
	    cprintf("%08x",stkp->stack[i]);
	    if (next) {
		if (regmsk) {
		    for (j = 0; j < 12; j++) {
		        if (regmsk & (1 << j)) {
			    cprintf(" r%d",j);
			    regmsk &= ~(1 << j);
			    break;
			}
		    }
		}
		else
		    next = 0;
	    }
	    if (*fp == *sp + (i * 4))
		cprintf(" *");
	    if ((*fp)+4 == *sp + (i * 4))
		regmsk = (stkp->stack[i] & 0x0fff0000) >> 16;
	    if ((*fp)+8 == *sp + (i * 4)) {
		cprintf(" ap");
		*ap1 = *ap2;
		*ap2 = stkp->stack[i];
	    }
	    if ((*fp)+12 == *sp + (i * 4))
		cprintf(" fp");
	    if ((*fp)+16 == *sp + (i * 4)) {
		cprintf(" pc");
		*fp = stkp->stack[i-1];
		next = 1;
	    }
	    if (*ap1 == *sp + (i * 4)) {
		cprintf(" #");
	    }
	    cprintf("\t");
	    if (((i+1) % 4) == 0) {
		if ((i+1) == num)
		    cprintf("\n");
		else
		    cprintf("\n%08x: ",*sp + ((i+1) * 4));
	        DELAY(250000);
	    }
	}
}

/*
 * Warn that a system table is full.
 */
tablefull(tab)
	char *tab;
{

	printf("%s: table is full\n", tab);
}

/*
 * Hard error is the preface to plaintive error messages
 * about failing disk transfers.
 */
harderr(bp, cp)
	struct buf *bp;
	char *cp;
{

	printf("%s%d%c: hard error sn%d\n", cp,
	    dkunit(bp), 'a'+(minor(bp->b_dev)&07), bp->b_blkno);
}

/*
 * Print a character on console or users terminal.
 * If destination is console then the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
/*ARGSUSED*/
putchar(msgp, c, touser)
	struct el_msg *msgp;
	register int c;
{

	if (touser == 2) {
		register struct tty *tp = u.u_ttyp;

		if (tp && (tp->t_state&TS_CARR_ON)) {
			register s = spl6();
			if (c == '\n')
				(void) ttyoutput('\r', tp);
			(void) ttyoutput(c, tp);
			ttstart(tp);
			splx(s);
		}
		return;
	}
	if (c != '\0' && c != '\r' && c != 0177
#ifdef vax
	    && mfpr(MAPEN)
#endif
	    ) {
		/* put item in el_msg struct for errlogging */
		if (msgp != 0) {
		    if (msgp->msg_len < msgsize)
		            msgp->msg_asc[msgp->msg_len++] = c;
		}
	}
	if (c == 0)
		return;
	/* 
	 * If this routine was called by mprintf then do not allow the text to
	 * go to the console terminal.
	 */
	if (touser != 1)
		cnputc(c);
}
