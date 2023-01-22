
#ifndef lint
static char *sccsid = "@(#)dhu.c	1.31	ULTRIX	2/12/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986, 1987 by		*
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

/*
 * dhu.c
 *
 * Modification history
 *
 * DHU11/DHV11/DHQ11 terminal driver
 *
 *  4-Apr-84 - Larry Cohen
 *
 *	Sleep in close to ensure DTR stays down for at least one
 *	second - only if modem line.  Open delays until close
 *	finishes in this case. -001
 *
 * 16-Jan-86 - Larry Cohen
 *
 *	Add full DEC standard 52 support.
 *
 * 10-Mar-86 - Tim Burke
 *
 *	Modified probe routine to wait for dhu self-test to complete.
 *
 * 18-Mar-86 - jaw
 *
 *	br/cvec changed to NOT use registers.
 *
 * 14-Apr-86 - jaw
 *
 *	Remove MAXNUBA references.....use NUBA only!
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added new DEVIOCGET ioctl request code. V2.0
 *
 * 13-Jun-86 - jaw
 *
 *	Fix to uba reset and drivers.
 *
 * 11-Jul-86 - ricky palmer
 *
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
 * 05-Aug-86 - Tim Burke
 *
 *	Modify dhurint to save present time in timestamp when
 *	carrier drops.
 *
 * 25-Aug-86 - Tim Burke
 *
 *	Fixes to Decstd52 modem control to close up line on false call, and
 *	insure that remaining processes are terminated.
 *
 * 26-Aug-86 - rsp (Ricky Palmer)
 *
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 *  4-Dec-86 - Tim Burke
 *	
 *	Bug fix to modem control.  In dhu_tty_drop routine, clear the stopped
 *	state to prevent modem lines from hanging on close.  Also removed a
 *	#define DHUDEBUG which shouldn't be here.
 *
 * 15-Dec-86 - Tim Burke
 *
 *	When a break occurs, (interpreted as a framing error) set the variable
 *	c to be the interrupt character.  There was a problem here due to the
 *	fact that sign extension is done which causes unwanted side affects. To
 *	solve this bug, the character is stripped to 8 bits.
 *
 *	Fix DEVIOGET to return propper modem status information.
 *
 *  9-Jan-87 - Tim Burke
 *
 *	Bug fix to TIOCMODEM to clear modem flags if signals are not up.
 *
 * 19-Jan-87 - Tim Burke
 *
 *	Bug fix to dhuclose to insure that line control register contents are
 *	cleared on the correct line.
 *
 * 28-Jan-87 - Tim Burke
 *
 *	Added the capability to ignore the "DSR" modem signal.  This is being
 *	done to allow modems that do not follow DEC Standard 52 to still 
 *	function as they would have prior to the addition of DECSTD52 code
 *	into the drivers.  If the driver is setup to ignore "DSR" then it will
 *	not be following DECSTD52.  To follow DECSTD52 set dhudsr to "1", to
 *	ignore "DSR" set dhudsr to be "0";
 *
 *  6-Feb-87 - Tim Burke
 *
 *	Removed printf of master reset failure in probe routine, as it may be
 *	incorrectly appearing. (Particularly in the DMF & DMZ drivers)
 */

#include "dhu.h"
#if NDHU > 0  || defined(BINARY)

#include "../data/dhu_data.c"

int dhudebug = 0;
int dhucdtime = 2;

char dhu_speeds[] = {
	0, 0, 01, 02, 03, 04, 0, 05,
	06, 07, 010, 012, 013, 015, 016, 0
	};

/*
 * Definition of the driver for the auto-configuration program.
 */
int	dhuprobe(), dhuattach(), dhurint(), dhuxint();
int	dhu_cd_drop(), dhu_dsr_check(), dhu_cd_down(), dhu_tty_drop();
struct	timeval dhuzerotime = {0,0};
u_short dhustd[] = { 0 };
struct	uba_driver dhudriver =
	{ dhuprobe, 0, dhuattach, 0, dhustd, "dhu", dhuinfo };


#ifndef PORTSELECTOR
#define ISPEED	B300
#define IFLAGS	(EVENP|ODDP|ECHO)
#else
#define ISPEED	B4800
#define IFLAGS	(EVENP|ODDP)
#endif

int	dhuact; 			/* mask of active dhu's */
int	dhustart(), ttrstrt();

#ifndef MODEM_CD
#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#endif

#define LINEMASK 0x0f	/* mask of higher bits of csr to get a line # */

/*
 * The clist space is mapped by the driver onto each UNIBUS.
 * The UBACVT macro converts a clist space address for unibus uban
 * into an i/o space address for the DMA routine.
 */
#define UBACVT(x, uban) 	(cbase[uban] + ((x)-(char *)cfree))

/*
 * Routine for configuration to force a dhu to interrupt.
 * Set to transmit at 9600 baud, and cause a transmitter interrupt.
 */
/*ARGSUSED*/
dhuprobe(reg)
	caddr_t reg;
{
	register struct dhudevice *dhuaddr = (struct dhudevice *)reg;
	int totaldelay; 		/* Self-test timeout counter */

#ifdef lint
	if (ndhu11 == 0) ndhu11 = 1;
	dhurint(0); dhuxint(0);
#endif
	if(dhudebug)
		printf("dhuprobe\n");
	/*
	 * If a self test is not being done, start one up.  Wait for the
	 * self-test (to a max of 4 sec.) to complete before interrupting.
	 */

	if ((dhuaddr->csr.low & DHU_MRESET) == 0)
	    dhuaddr->csr.low |= DHU_MRESET;
	totaldelay = 0;
	while ( (dhuaddr->csr.low & DHU_MRESET) && ( totaldelay <= 70) ){
	    totaldelay++;
	    DELAY(50000);
	}
	/*
	 * This message may be incorrectly printed - particularly in the
	 * DMF & DMZ drivers.

	if (dhuaddr->csr.low & DHU_MRESET)
	    printf("Warning: DHU device failed to exit self-test\n");
	else if (dhuaddr->csr.high & DHU_DIAGFAIL)
	    printf("Warning: DHU self-test failure\n");
	 */

	dhuaddr->csr.low = 0;  /* transmit on channel 0 */
	dhuaddr->csr.high |= DHU_XIE; /* enable transmit interrupts */
	dhuaddr->tbuffad1 = 0;
	dhuaddr->tbuffcnt = 0;
	dhuaddr->lpr = dhu_speeds[B9600] << 12 | DHU_BITS7 | DHU_PENABLE;
	dhuaddr->tbuffad2.high |= DHU_XEN;
	dhuaddr->tbuffad2.low |= DHU_START;
	DELAY(100000);		/* wait 1/10'th of a sec for interrupt */
	{ char temp = dhuaddr->csr.high; /* clear transmit action */ }
	dhuaddr->csr.high = 0;	 /* disable transmit interrupts */
	if (cvec && cvec != 0x200) /* check to see if interrupt occurred */
		cvec -= 4;	   /* point to first interrupt vector (recv)*/
	return (sizeof (struct dhudevice));
}

/*
 * Routine called to attach a dhu.
 */
dhuattach(ui)
	struct uba_device *ui;
{
#ifdef DHUDEBUG
	if(dhudebug)
		printf("dhuattach %x, %d\n", ui->ui_flags, ui->ui_unit);
#endif DHUDEBUG
	dhusoftCAR[ui->ui_unit] = ui->ui_flags;
	dhudefaultCAR[ui->ui_unit] = ui->ui_flags;
}


/*
 * Open a DHU11 line, mapping the clist onto the uba if this
 * is the first dhu on this uba.  Turn on this dhu if this is
 * the first use of it.  Also wait for carrier.
 */
/*ARGSUSED*/
dhuopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dhu;
	register struct dhudevice *addr;
	register struct uba_device *ui;
	int s;
	int inuse;  /*hold state of inuse bit while blocked waiting for carr*/

	unit = minor(dev);
	dhu = unit >> 4;
	if (unit >= nNDHU*16 || (ui = dhuinfo[dhu])== 0 || ui->ui_alive == 0)
		return (ENXIO);
	tp = &dhu11[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
		return (EBUSY);
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuopen:  line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif
	while (tp->t_state&TS_CLOSING) /* let DTR stay down for awhile */
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	addr = (struct dhudevice *)ui->ui_addr;

	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dhustart;
	tp->t_state |= TS_WOPEN;
	/*
	 * While setting up state for this uba and this dhu,
	 * block uba resets which can clear the state.
	 */
	s = spl5();
	while (tty_ubinfo[ui->ui_ubanum] == -1)
		/* need this lock because uballoc can sleep */
		sleep(&tty_ubinfo[ui->ui_ubanum], TTIPRI);
	if (tty_ubinfo[ui->ui_ubanum] == 0) {
		tty_ubinfo[ui->ui_ubanum] = -1;
		tty_ubinfo[ui->ui_ubanum] =
		    uballoc(ui->ui_ubanum, (caddr_t)cfree,
			nclist*sizeof(struct cblock), 0);
		wakeup(&tty_ubinfo[ui->ui_ubanum]);
	}
	if (ui->ui_hd->uba_type & UBAUVI)
		cbase[ui->ui_ubanum] =
		    tty_ubinfo[ui->ui_ubanum]&0x3fffff;
	else
		cbase[ui->ui_ubanum] =
		    tty_ubinfo[ui->ui_ubanum]&0x3ffff;
	if ((dhuact&(1<<dhu)) == 0)
		dhuact |= (1<<dhu);

	splx(s);
	/*
	 * If this is first open, initialize tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		dhumodem[unit] = MODEM_DSR_START; /* prevents spurious
						     startups */
#ifndef PORTSELECTOR
		if (tp->t_ispeed == 0) {
#endif
			tp->t_dev = dev;  /* timeouts need this */
			tp->t_ispeed = ISPEED;
			tp->t_ospeed = ISPEED;
			tp->t_flags = IFLAGS;
#ifndef PORTSELECTOR
		}
#endif
		dhuparam(unit);
	}
	/*
	 * Wait for carrier, then process line discipline specific open.
	 */


	if (addr->fun.fs.stat & DHU11) {
		/*
		 * only the dhu11 has a timer. manual says we have to
		 * point to line 0 before we set the timer
		 */
		addr->csr.low = DHU_RIE|(0 & LINEMASK); /* set to line 0 */
		addr->run.rxtimer = 10;
	}
	s=spl5();
	if (dhusoftCAR[dhu] & (1<<(unit&LINEMASK))) {
		/* this is a local connection - ignore carrier */
		tp->t_state |= TS_CARR_ON;
		dhumodem[unit] |= MODEM_CTS|MODEM_CD|MODEM_DSR;
		addr->csr.low = DHU_RIE|(unit & LINEMASK); /* set to line #*/
							  /* enable interrupts*/
		addr->lnctrl &= ~(DHU_MODEM);
		addr->lnctrl |= (DHU_DTR|DHU_RTS|DHU_REN);
		splx(s);
		return ((*linesw[tp->t_line].l_open)(dev, tp));
	}
	addr->csr.low = DHU_RIE|(unit & LINEMASK);
	addr->lnctrl |= (DHU_DTR|DHU_RTS|DHU_MODEM|DHU_REN);

	
	/*
 	 * If the DSR signal is followed, give carrier 30 secs to come up,
	 * and do not transmit/receive data for the first 500ms.  Otherwise
	 * immediately to to dhu_dsr_check tolook for CD and CTS.
 	 */
	if ((flag&O_NDELAY)==0) {
		if (dhudsr) {
			if ((addr->fun.fs.stat)&DHU_DSR) {
				dhumodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
				tp->t_dev = dev; /* need it for timeouts */
				timeout(dhu_dsr_check, tp, hz*30);
				timeout(dhu_dsr_check, tp, hz/2);
			}
		}
		else {
			dhumodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			dhu_dsr_check(tp);
		}
	}
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuopen:  line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif
	if (flag & O_NDELAY)
		tp->t_state |= TS_ONDELAY;
	else
		while ((tp->t_state&TS_CARR_ON)==0) {
			inuse = tp->t_state&TS_INUSE;
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
 			/*
 			 * See if wakeup is due to a false call.
 			 */
 			if (dhumodem[unit]&MODEM_BADCALL){
				splx(s);
 				return(EWOULDBLOCK);
			}
			/* if we opened "block if in use"  and
			 *  the terminal was not inuse at that time
			 *  but is became "in use" while we were
			 *  waiting for carrier then return
			 */
			if ((flag & O_BLKINUSE) && (inuse==0) &&
				(tp->t_state&TS_INUSE)) {
					splx(s);
					return(EALREADY);
			}
		}
	splx(s);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Close a DHU11 line.
 */
/*ARGSUSED*/
dhuclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit, s;
	register struct dhudevice *addr;
	register dhu;
	extern int wakeup();
	int turnoff = 0;

	unit = minor(dev);
	dhu = unit >> 4;
	tp = &dhu11[unit];
	addr = (struct dhudevice *)tp->t_addr;
	tp->t_state |= TS_CLOSING;
	if (tp->t_line)
		(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_state&TS_HUPCLS) || (tp->t_state&TS_ISOPEN)==0) {
		s = spl5();
		turnoff++;
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~(DHU_DTR|DHU_RTS);  /* turn off DTR */
		tp->t_state &= ~TS_CARR_ON; /* prevents recv intr. timeouts */
		if ((dhusoftCAR[dhu] & (1<<(unit&LINEMASK)))==0)  {
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuclose: DTR drop line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif

			/*
			 * Wait an additional 5 seconds for DSR to drop if
			 * the DSR signal is being watched.
			 */
			if (dhudsr && (addr->fun.fs.stat&DHU_DSR)) {
				timeout(wakeup, (caddr_t) &tp->t_dev, 5*hz);
				sleep((caddr_t)&tp->t_dev, PZERO-10);
			}
			timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
			sleep((caddr_t)&tp->t_dev, PZERO-10);
		}
		splx(s);
	}
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuclose: line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif
	dhusoftCAR[dhu] &= ~(1<<(unit&LINEMASK));
	dhusoftCAR[dhu] |= (1<<(unit&LINEMASK)) & dhudefaultCAR[dhu];
	ttyclose(tp);  /* remember this will clear out t_state */

	if (turnoff) {
		/* we have to do this after the ttyclose so that output
		 * can still drain
		 */
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl = NULL; /* turn off interrupts also */
		splx(s);
	}
	dhumodem[unit] = 0;
	wakeup((caddr_t)&tp->t_rawq); /* wake up anyone in dhuopen */
}

dhuread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];

	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

dhuwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * DHU11 receiver interrupt.
 */
dhurint(dhu)
	int dhu; /* module number */
{
	register struct tty *tp;
	register c, unit;
	register struct dhudevice *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	int overrun = 0;
	register u_char *modem0, *modem;
	int modem_cont;

	ui = dhuinfo[dhu];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dhudevice *)ui->ui_addr;
	tp0 = &dhu11[dhu<<4];  /* first tty structure that corresponds
				* to this dhu11 module
				*/
	modem0 = &dhumodem[dhu<<4];
	/*
	 * Loop fetching characters from receive fifo for this
	 * dhu until there are no more in the receive fifo.
	 */
	while ((c = addr->run.rbuf) < 0) {
		/* if c < 0 then data valid is set */
		unit = (c>>8)&LINEMASK;
		tp = tp0 + unit; /* tty struct for this line */
		modem = modem0 + unit;
#ifdef DHUDEBUG
		if (dhudebug > 2)
			mprintf("dhurint0: c=%x, tp=%x\n", c, tp);
#endif
		/* check for modem transitions */
		if ((c & DHU_STAT)==DHU_STAT) {
			if (c & DHU_DIAG) /* ignore diagnostic info */
				continue;
#ifdef DHUDEBUG
			if (dhudebug)
				mprintf("dhurint: c=%x, tp=%x\n", c, tp);
#endif
				
			if (dhusoftCAR[dhu] & (1<<unit))
				continue;

			/* set to line #*/
			addr->csr.low = DHU_RIE|(unit & LINEMASK);
			modem_cont = 0;

			/* examine modem status */

			/*
			 * Drop DTR immediately if DSR has gone away.
			 * If really an active close then do not
			 *    send signals.
			 */

			if ((addr->fun.fs.stat&DHU_DSR)==0) {
				if (tp->t_state&TS_CLOSING) {
					untimeout(wakeup, (caddr_t) &tp->t_dev);
					wakeup((caddr_t) &tp->t_dev);
					continue;
				}
				if (tp->t_state&TS_CARR_ON) {
#ifdef DHUDEBUG
					if (dhudebug)
						mprintf("dhurint: DSR dropped,line=%d\n",unit);
#endif DHUDEBUG
					/*
 					 * Only drop if DSR is being followed.
 					 */
					if (dhudsr) {
						dhu_tty_drop(tp);
						/*
						 * Moved the continue here, so
						 * that if both DSR & CD change
						 * in same interval, we'll see
						 * both.
						 */
						continue;
					}
				}
			}

			/*
			 * Check for transient CD drops.
			 * Only drop DTR if CD is down for more than 2 secs.
			 */
			if (tp->t_state&TS_CARR_ON)
			    if ((addr->fun.fs.stat&DHU_CD)==0){
				if ( *modem & MODEM_CD) {
				    /* only start timer once */
				    if (dhudebug)
					mprintf("dhurint, cd_drop, tp=%x\n", tp);
				    *modem &= ~MODEM_CD;
				    dhutimestamp[minor(tp->t_dev)] = time;
				    timeout(dhu_cd_drop, tp, hz*dhucdtime);
				    modem_cont = 1;
				}
			    } else
				/*
				 * CD has come up again.
				 * Stop timeout from occurring if set.
				 * If interval is more than 2 secs then
				 *  drop DTR.
				 */
				if ((*modem&MODEM_CD)==0) {
					untimeout(dhu_cd_drop, tp);
					if (dhu_cd_down(tp)) {
						/* drop connection */
						dhu_tty_drop(tp);
					}
					*modem |= MODEM_CD;
				        modem_cont = 1;
				}

			/* CTS flow control check */

			if (tp->t_state&TS_CARR_ON)
				if ((addr->fun.fs.stat&DHU_CTS)==0) {
					tp->t_state |= TS_TTSTOP;
					*modem &= ~MODEM_CTS;
#ifdef DHUDEBUG
					if (dhudebug)
					   mprintf("dhurint: CTS stop, line=%d\n", unit);
#endif DHUDEBUG
					dhustop(tp, 0);
					continue;
				} else if ((*modem&MODEM_CTS)==0) {
					    tp->t_state &= ~TS_TTSTOP;
					    *modem |= MODEM_CTS;
#ifdef DHUDEBUG
					    if (dhudebug)
					       mprintf("dhurint: CTS start, line=%d\n", unit);
#endif DHUDEBUG
					    dhustart(tp);
					    continue;
					}

			/*
			 * Avoid calling dhu_start_tty for a CD transition if
			 * the connection has already been established.
			 */
			if (modem_cont)
				continue;
			/*
			 * If 500 ms timer has not expired then dont
			 * check anything yet.
			 * Check to see if DSR|CTS|CD are asserted.
			 * If so we have a live connection.
			 * If DSR is set for the first time we allow
			 * 30 seconds for a live connection.
			 *
		    	 * 
		    	 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		    	 * for CD|CTS only.
		    	 */
			if (dhudsr) {
				if ((addr->fun.fs.stat&DHU_XMIT)==DHU_XMIT
			    	&& (*modem&MODEM_DSR_START)==0)
					dhu_start_tty(tp);
				else
			    	if ((addr->fun.fs.stat&DHU_DSR) &&
					(*modem&MODEM_DSR)==0) {
					*modem |= (MODEM_DSR_START|MODEM_DSR);
					/*
				 	* we should not look for CTS|CD for
				 	* about 500 ms.
				 	*/
					timeout(dhu_dsr_check, tp, hz*30);
					timeout(dhu_dsr_check, tp, hz/2);
			    	}
			}
			/* 
			 * Ignore DSR.
			 */
			else {
				if ((addr->fun.fs.stat&(DHU_CD|DHU_CTS))==(DHU_CD|DHU_CTS))
					dhu_start_tty(tp);
			}



			continue;
		}


#ifndef PORTSELECTOR
		if ((tp->t_state&TS_ISOPEN)==0) {
#else
		if ((tp->t_state&(TS_ISOPEN|TS_WOPEN))==0) {
#endif
			wakeup((caddr_t)tp);
			continue;
		}
		if (c & DHU_PERR)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP )
				continue;
		if ((c & DHU_OVERR) && overrun == 0) {
			printf("dhu%d: recv. fifo overflow\n", dhu);
			overrun = 1;
		}
		if (c & DHU_FERR)
			/*
			 * At framing error (break) generate
			 * a null (in raw mode, for getty), or a
			 * interrupt (in cooked/cbreak mode).
			 */
			if (tp->t_flags&RAW)
				c = 0;
			else{
				c = tp->t_intrc;
				/*
				 * Skip extraneous sign extension bits.
				 */
				c &= 0377;
			}
#if NHC > 0
		if (tp->t_line == HCLDISC) {
			HCINPUT(c, tp);
		} else
#endif
			(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * Ioctl for DHU11.
 */
/*ARGSUSED*/
dhuioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register int dhu, unit;
	register struct dhudevice *addr;
	register struct tty *tp;
	register int s;
	struct uba_device *ui;
	struct dhu_softc *sc;
	struct devget *devget;
	int error;

	unit = minor(dev);
	tp = &dhu11[unit];
	dhu = unit >> 4;	   /* module number */
	ui = dhuinfo[dhu];
	sc = &dhu_softc[dhu];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			dhuparam(unit);
		return (error);
	}
	addr = (struct dhudevice *)tp->t_addr;
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuioctl: unit=%d, cmd=%d\n", unit, cmd&0xff);
#endif
	switch (cmd) {

	case TIOCSBRK:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl |= DHU_BREAK;
		splx(s);
		break;

	case TIOCCBRK:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~DHU_BREAK;
		splx(s);
		break;

	case TIOCSDTR:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl |= (DHU_DTR|DHU_RTS);
		splx(s);
		break;

	case TIOCCDTR:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~(DHU_DTR|DHU_RTS);
		splx(s);
		break;

	/* handle maintenance mode */
	case TIOCSMLB:
		if (u.u_uid)
			return(EPERM);
		s=spl5();
		addr->csr.low = DHU_RIE|(unit&LINEMASK ); /*enable interrupts*/
		addr->lnctrl |= (DHU_MAINT);
		splx(s);
		break;

	case TIOCCMLB:
		if (u.u_uid)
			return(EPERM);
		s=spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~(DHU_MAINT);
		splx(s);
		break;

	case TIOCNMODEM:  /* ignore modem status */
		s = spl5();
		dhusoftCAR[dhu] |= (1<<(unit&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dhudefaultCAR[dhu] |= (1<<(unit&LINEMASK));
		tp->t_state |= TS_CARR_ON;
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~(DHU_MODEM);
		dhumodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		splx(s);
		break;

	case TIOCMODEM:  /* dont ignore modem status  */
		s = spl5();
		dhusoftCAR[dhu] &= ~(1<<(unit&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dhudefaultCAR[dhu] &= ~(1<<(unit&LINEMASK));
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl |= DHU_MODEM;
		/* 
		 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		 * for CD|CTS only.
		 */
		if ((dhudsr && ((addr->fun.fs.stat&DHU_XMIT)==DHU_XMIT)) ||
		   ((dhudsr == 0) && ((addr->fun.fs.stat&(DHU_CD|DHU_CTS)) == (DHU_CD|DHU_CTS)))) {
				tp->t_state |= TS_CARR_ON;
				tp->t_state &= ~TS_ONDELAY;
				dhumodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
			}
			else {
				dhumodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
				tp->t_state &= ~(TS_CARR_ON);
			}
		splx(s);
		break;

	case TIOCWONLINE:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		/* 
		 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		 * for CD|CTS only.
		 */
		if ((dhudsr && ((addr->fun.fs.stat&DHU_XMIT)==DHU_XMIT)) ||
		   ((dhudsr == 0) && ((addr->fun.fs.stat&(DHU_CD|DHU_CTS)) == (DHU_CD|DHU_CTS)))) {
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dhumodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else
			while ((tp->t_state & TS_CARR_ON) == 0)
				sleep((caddr_t)&tp->t_rawq, TTIPRI);
		splx(s);
		break;

	case TIOCMGET:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		*(int *)data = dhutodm(addr->lnctrl,addr->fun.fs.stat);
		splx(s);
		break;

	case TIOCMSET:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl = dmtodhu(*(int *)data);
		splx(s);
		break;

	case TIOCMBIS:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl |= dmtodhu(*(int *)data);
		splx(s);
		break;

	case TIOCMBIC:
		s = spl5();
		addr->csr.low = DHU_RIE|(unit & LINEMASK);
		addr->lnctrl &= ~(dmtodhu(*(int *)data));
		splx(s);
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));

		if(dhusoftCAR[dhu] & (1<<(unit&LINEMASK))) {
			sc->sc_category_flags[unit] |= DEV_MODEM;
			sc->sc_category_flags[unit] &= ~DEV_MODEM_ON;
		}
		else
			sc->sc_category_flags[unit] |= (DEV_MODEM|DEV_MODEM_ON);

		devget->category = DEV_TERMINAL;

		if(ui->ui_hd->uba_type & (UBAUVI|UBAUVII)) {
			devget->bus = DEV_QB;
		} else {
			devget->bus = DEV_UB;
		}

		switch (devget->bus) {

		case DEV_UB:
			bcopy(DEV_DHU11,devget->interface,
			      strlen(DEV_DHU11));
			break;

		case DEV_QB:
			bcopy(DEV_DHV11,devget->interface,
			      strlen(DEV_DHV11));
			break;
		}

		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));		/* terminal	*/
		devget->adpt_num = ui->ui_adpt; 	/* which adapter*/
		devget->nexus_num = ui->ui_nexus;	/* which nexus	*/
		devget->bus_num = ui->ui_ubanum;	/* which UBA/QB */
		devget->ctlr_num = dhu; 		/* which interf.*/
		devget->slave_num = unit%16;		/* which line	*/
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,
		      strlen(ui->ui_driver->ud_dname)); /* Ultrix "dhu" */
		devget->unit_num = unit;		/* dh[vu] line #*/
		devget->soft_count =
		      sc->sc_softcnt[unit];		/* soft er. cnt.*/
		devget->hard_count =
		      sc->sc_hardcnt[unit];		/* hard er cnt. */
		devget->stat = sc->sc_flags[unit];	/* status	*/
		devget->category_stat =
		      sc->sc_category_flags[unit];	/* cat. stat.	*/
		break;

	default:
		return (ENOTTY);
	}
	return (0);
}

dhutodm(lnctrl,lstat)
	register u_short lnctrl;
	register char lstat;
{
	register int b = 0;
	if (lnctrl&DHU_RTS)  b |= TIOCM_RTS;
	if (lnctrl&DHU_DTR)  b |= TIOCM_DTR;
	if (lstat&DHU_CD)  b |= TIOCM_CD;
	if (lstat&DHU_CTS)  b |= TIOCM_CTS;
	if (lstat&DHU_RING)  b |= TIOCM_RI;
	if (lstat&DHU_DSR)  b |= TIOCM_DSR;
	return(b);
}


dmtodhu(bits)
	register int bits;
{
	register u_short lnctrl = 0;
	if (bits&TIOCM_RTS) lnctrl |= DHU_RTS;
	if (bits&TIOCM_DTR) lnctrl |= DHU_DTR;
	return(lnctrl);
}

/*
 * Set parameters from open or stty into the DHU hardware
 * registers.
 */
dhuparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register int lpar;
	int s;

	tp = &dhu11[unit];
	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	addr->csr.low = DHU_RIE|(unit&LINEMASK);
	if ((tp->t_ispeed)==0) {
		tp->t_state |= TS_HUPCLS;
		addr->lnctrl = DHU_REN; /*turn off DTR & RTS but leave enabled*/
		splx(s);
		return;
	}
	lpar = ((dhu_speeds[tp->t_ospeed])<<12) | ((dhu_speeds[tp->t_ispeed])<<8);
	if ((tp->t_ispeed) == B134)
		lpar |= DHU_BITS6|DHU_PENABLE; /* no half duplex on dhu11 ? */
	else if (tp->t_flags & (RAW|LITOUT))
		lpar |= DHU_BITS8;
	else
		lpar |= DHU_BITS7|DHU_PENABLE;
	if (tp->t_flags&EVENP)
		lpar |=  DHU_EVENPAR;
	if ((tp->t_ospeed) == B110)
		lpar |= DHU_TWOSB;
	addr->lpr = lpar;
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuparam: tp=%x, lpr=%x\n", tp, lpar);
#endif
	splx(s);
}

/*
 * DHU11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhuxint(dhu)
	int dhu;  /* module number */
{
	register struct tty *tp;
	register struct dhudevice *addr;
	register struct uba_device *ui;
	register int unit;
	u_short cntr;
	char csrxmt;

	ui = dhuinfo[dhu];
	addr = (struct dhudevice *)ui->ui_addr;
	while ((csrxmt = addr->csr.high) < 0) {
		if (csrxmt & DHU_DMAERR)
			printf("dhu%d:%d DMA ERROR\n", dhu, csrxmt&0xf);
		if (csrxmt & DHU_DIAGFAIL)
			printf("dhu%d: DIAG. FAILURE\n", dhu);
		unit = dhu * 16;
		unit |= csrxmt&LINEMASK;
		tp = &dhu11[unit];
#ifdef DHUDEBUG
		if (dhudebug)
			mprintf("dhuxint: unit=%x, tp=%x, c_cc=%d\n",
				unit, tp, tp->t_outq.c_cc);
#endif
		tp->t_state &= ~TS_BUSY;
		if (tp->t_state&TS_FLUSH)
			tp->t_state &= ~TS_FLUSH;
		else {
			addr->csr.low = (unit&LINEMASK)|DHU_RIE;
			/*
			 * Determine number of chars transmitted
			 * so far and flush these from the tty
			 * output queue.
			 * Do arithmetic in a short to make up
			 * for lost 16&17 bits (in tbuffad2).
			 */
			cntr = addr->tbuffad1 -
				UBACVT(tp->t_outq.c_cf, ui->ui_ubanum);
			ndflush(&tp->t_outq, cntr);
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dhustart(tp);
	}
}

/*
 * Start (restart) transmission on the given DHU11 line.
 */
dhustart(tp)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int car, dhu, unit, nch, line;
	int s;

	unit = minor(tp->t_dev);
	dhu = unit >> 4;
	line = unit & LINEMASK; /* unit now equals the line number */
	addr = (struct dhudevice *)tp->t_addr;

	/*
	 * Must hold interrupts in following code to prevent
	 * state of the tp from changing.
	 */
	s = spl5();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 * Also do not transmit if not CTS
	 */
	if ((tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) ||
		 ((tp->t_state&TS_CARR_ON) && (dhumodem[unit]&MODEM_CTS)==0))
		goto out;
	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers.
	 */
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhustart0: tp=%x, LO=%d, cc=%d \n", tp,
			TTLOWAT(tp), tp->t_outq.c_cc);
#endif
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
		if (tp->t_state&TS_ASLEEP) {  /* wake up when output done */
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_wsel) {   /* for select system call */
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
	}
	/*
	 * Now restart transmission unless the output queue is
	 * empty.
	 */
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (tp->t_flags & (RAW|LITOUT))
		nch = ndqb(&tp->t_outq, 0); /* number of consecutive chars */
	else {
		nch = ndqb(&tp->t_outq, 0200);
		/*
		 * If first thing on queue is a delay process it.
		 */
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch) {
		car = UBACVT(tp->t_outq.c_cf, dhuinfo[dhu]->ui_ubanum);
#ifdef DHUDEBUG
		if (dhudebug)
			mprintf("dhustart1: line=%x, nch=%d\n", line, nch);
#endif
		addr->csr.low = DHU_RIE|line; /* select line */
		/* wait for DMA START to clear */
		while(addr->tbuffad2.low & DHU_START)
			if (dhudebug)
				printf("dhu11: start set\n");
		if (addr->lnctrl & DHU_XABORT) /* clear abort if already set */
			addr->lnctrl &= ~(DHU_XABORT);
		addr->csr.high = DHU_XIE;
		addr->tbuffad1 = car;
		addr->tbuffcnt = nch;
		addr->tbuffad2.high = DHU_XEN;
		/* get extended address bits and start DMA output */
		if (dhuinfo[dhu]->ui_hd->uba_type&UBAUVI)
			addr->tbuffad2.low = ((car>>16)&0x3f)|DHU_START;
		else
			addr->tbuffad2.low = ((car>>16)&0x3)|DHU_START;
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}

/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dhustop(tp, flag)
	register struct tty *tp;
{
	register struct dhudevice *addr;
	register int unit, s;

	addr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output.
		 * We can continue later
		 * by examining the address where the dhu stopped.
		 */
		unit = minor(tp->t_dev);
		addr->csr.low = (unit&LINEMASK)|DHU_RIE;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
		addr->lnctrl |= DHU_XABORT;  /* abort DMA transmission */
	}
	splx(s);
}

/*
 * Reset state of driver if UBA reset was necessary.
 * Reset the csr and lpr registers on open lines, and
 * restart transmitters.
 */
dhureset(uban)
	int uban;
{
	register int dhu, unit, s;
	register struct tty *tp;
	register struct uba_device *ui;
	register struct dhudevice *addr;
	int i;

	if (tty_ubinfo[uban] == 0)
		return;  /* there are no dhu11's in use */
	if (uba_hd[uban].uba_type&UBAUVI)
		cbase[uban] = tty_ubinfo[uban]&0x3fffff;
	else
		cbase[uban] = tty_ubinfo[uban]&0x3ffff;
	dhu = 0;
	if (dhudebug)
		mprintf("dhureset\n");
	for (dhu = 0; dhu < nNDHU; dhu++) {
		ui = dhuinfo[dhu];
		if (ui == 0 || ui->ui_alive == 0 || ui->ui_ubanum != uban)
			continue;
		addr = (struct dhudevice *)ui->ui_addr;
		printf(" dhu%d", dhu);
		unit = dhu * 16;
		for (i = 0; i < 16; i++) {
			tp = &dhu11[unit];
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
				dhuparam(unit);
				s = spl5();
				addr->csr.low = (i&LINEMASK)|DHU_RIE;
				addr->lnctrl |= DHU_DTR|DHU_RTS|DHU_REN;
				splx(s);
				tp->t_state &= ~TS_BUSY;
				dhustart(tp);
			}
			unit++;
		}
	}
	dhutimer();
}

/*
 * At software clock interrupt time or after a UNIBUS reset
 * empty all the dh silos.
 */
dhutimer()
{
	register int dhu;
	register int s = spl5();

	for (dhu = 0; dhu < nNDHU; dhu++)
		dhurint(dhu);
	splx(s);
}

dhu_cd_drop(tp)
register struct tty *tp;
{
	register struct dhudevice *addr = (struct dhudevice *)tp->t_addr;
	register int unit = minor(tp->t_dev);

	addr->csr.low = DHU_RIE|(unit & LINEMASK);
	if ((tp->t_state&TS_CARR_ON) &&
		((addr->fun.fs.stat&DHU_CD) == 0)) {
		if (dhudebug)
		    mprintf("dhu_cd:  no CD, tp=%x\n", tp);
		dhu_tty_drop(tp);
		return;
	}
	dhumodem[minor(tp->t_dev)] |= MODEM_CD;
	if (dhudebug)
	    mprintf("dhu_cd:  CD is up, tp=%x\n", tp);
}
dhu_dsr_check(tp)
register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	register struct dhudevice *addr = (struct dhudevice *)tp->t_addr;
	if (dhumodem[unit]&MODEM_DSR_START) {
		if (dhudebug)
		    mprintf("dhu_dsr_check0:  tp=%x\n", tp);
		dhumodem[unit] &= ~MODEM_DSR_START;
		addr->csr.low = DHU_RIE|(unit&LINEMASK);
		/* 
		 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		 * for CD|CTS only.
		 */
		if (dhudsr) {
			if ((addr->fun.fs.stat&DHU_XMIT)==DHU_XMIT)
				dhu_start_tty(tp);
		}
		else
			if ((addr->fun.fs.stat&(DHU_CD|DHU_CTS))==(DHU_CD|DHU_CTS))
				dhu_start_tty(tp);
		return;
	}
	if ((tp->t_state&TS_CARR_ON)==0)  {
		dhu_tty_drop(tp);
		if (dhudebug)
		    mprintf("dhu_dsr_check:  no carrier, tp=%x\n", tp);
	}
	else
		if (dhudebug)
		    mprintf("dhu_dsr_check:  carrier is up, tp=%x\n", tp);
}

/*
 *  cd_down return 1 if carrier has been down for at least 2 secs.
 */
dhu_cd_down(tp)
struct tty *tp;
{
	int msecs;
	int unit = minor(tp->t_dev);

	msecs = 1000000 * (time.tv_sec - dhutimestamp[unit].tv_sec) +
		(time.tv_usec - dhutimestamp[unit].tv_usec);
	if (msecs > 2000000)
		return(1);
	else
		return(0);
}

dhu_tty_drop(tp)
struct tty *tp;
{
	register struct dhudevice *addr = (struct dhudevice *)tp->t_addr;
	register int unit;
	if (tp->t_flags&NOHANG)
		return;
	unit = minor(tp->t_dev);
 	dhumodem[unit] = MODEM_BADCALL;
 	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP);
 	wakeup((caddr_t)&tp->t_rawq);
	gsignal(tp->t_pgrp, SIGHUP);
	gsignal(tp->t_pgrp, SIGCONT);
	addr->csr.low = DHU_RIE|(unit&LINEMASK);
	addr->lnctrl &= ~(DHU_DTR|DHU_RTS);  /* turn off DTR */
}
dhu_start_tty(tp)
	register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	tp->t_state &= ~TS_ONDELAY;
	tp->t_state |= TS_CARR_ON;
	if (dhudebug)
	       mprintf("dhu_start_tty:  tp=%x\n", tp);
	if (dhumodem[unit]&MODEM_DSR)
		untimeout(dhu_dsr_check, tp);
	dhumodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	dhutimestamp[unit] = dhuzerotime;
	wakeup((caddr_t)&tp->t_rawq);
}
#endif
