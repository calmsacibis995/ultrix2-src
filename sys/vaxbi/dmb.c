
#ifndef lint
static char *sccsid = "@(#)dmb.c	1.24	ULTRIX	1/29/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986, 1987 by			*
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
 * dmb.c
 *
 * Modification history
 *
 * DMB32 terminal driver
 *
 *
 * 28-Jan-87 - Tim Burke
 *
 *	Added the capability to ignore the "DSR" modem signal.  This is being
 *	done to allow modems that do not follow DEC Standard 52 to still 
 *	function as they would have prior to the addition of DECSTD52 code
 *	into the drivers.  If the driver is setup to ignore "DSR" then it will
 *	not be following DECSTD52.  To follow DECSTD52 set dmbdsr to "1", to
 *	ignore "DSR" set dmbdsr to be "0";
 *
 *	Removed unnecessary return values from dmblint, dmb_dsr_check, 
 *	dmb_cd_drop, dmb_tty_drop. A few other lint cleanups with Jim
 *	Woodward.
 *
 *  9-Jan-87 - Tim Burke
 *
 *	Bug fix to TIOCMODEM to clear modem flags if signals are not up.
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
 *  4-Dec-86 - Tim Burke
 *
 *	Bug fix to modem control.  In dmb_tty_drop routine, clear the stopped
 *	state to prevent modem lines from hanging on close.  
 *
 * 26-Sep-86 - afd (Al Delorey)
 *
 *	Enable external vector in the User Interface Interrupt Control
 *	Register (a BIIC reg).  Original DMB32 always had this bit set, but
 *	that violated BI standard. So driver does it in dmbinit() & dmbreset().
 *	
 * 26-Aug-86 - rsp (Ricky Palmer)
 *
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 * 25-Aug-86 - Tim Burke
 *
 *	Insure that a modem will drop and restart line on a flase call.  Also
 *	set state to ~TS_CARR_ON on line drop to terminate associated processes.
 *
 * 05-Aug-86 - Tim Burke
 *
 *	In dmbrint, record the present timestamp in the event of a
 *	carrier drop.
 *
 * 11-Jul-86 - ricky palmer
 *
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
 * 29-May-86 - afd & tim burke
 *	Enable DTR & RTS in open routine for both hardwired and modem lines.
 *	This was being done only for modem lines.
 *
 * 26-Apr-86 - ricky palmer
 *	Added new DEVIOCGET ioctl request code. V2.0
 *
 *  4-Apr-86 - afd
 *	Call "bidev_vec()" form dmbinit() to set up interrupt vector handlers.
 *
 */

#include "dmb.h"
#if NDMB > 0 || defined (BINARY)

#include "../data/dmb_data.c"

/*
 * The line printer on dmbx is indicated by a minor device code of 128+x
 * The flags field of the config file is interpreted as follows:
 *
 * bits 	meaning
 * ---- 	-------
 * 0-7		soft carrier bits for the async tty lines on DMB32
 * 8-15 	number of cols/line on the line printer (if 0, 132 will be used)
 * 16-23	number of lines/page on the line printer (if 0, 66 will be used)
 *
 */


#ifdef DEBUG
int dmbdebug = 1;
#define printd if (dmbdebug) printf
#define printd10 if (dmbdebug >= 10) printf
#endif

extern struct bidata bidata[];

/*
 * Definition of the driver for the auto-configuration program.
 */
int	dmbinit(), dmbattach(), dmbaint(), dmbsint(), dmblint();
struct	uba_device *dmbinfo[NDMB];
u_short dmbstd[] = { 0 };
struct	uba_driver dmbdriver =
	{ dmbinit, 0, dmbattach, 0, dmbstd, "dmb", dmbinfo };

/* function type definitions */

int	dmbstart(), ttrstrt();
int	wakeup();

/* For for DEC std 52, modem control */

int	dmb_cd_drop(), dmb_dsr_check(), dmb_cd_down(), dmb_tty_drop();
struct	timeval dmbzerotime = {0,0};
int	dmbcdtime = 2;
#ifndef MODEM_CD
#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#endif

/*
 * Local variables for the driver
 */
int	dmb_timeout = 10;		/* receive fifo timeout, in ms */

char dmb_speeds[] =
	{ 0, 0, 01, 02, 03, 04, 0, 05, 06, 07, 010, 012, 013, 015, 016, 017 };
	/*  50	75  110 134 150   300 600 1200 1800 2400 4800 9600 19.2 38.4 */

#define ASLP 1		/* waiting for interrupt from dmb for printer port */
#define OPEN 2		/* line printer is open */
#define ERROR 4 	/* error while printing, driver
			   refuses to do anything till closed */

/*
 * Routine for configuration to see if the DMB32 is functional
 */
/*ARGSUSED*/
dmbinit(nxv,nxp,cpup,binumber,binode,ui)
	caddr_t nxv;			/* virt addr of device */
	caddr_t nxp;			/* phys addr of device */
	struct cpusw *cpup;
	int binumber;			/* this bi number on the system */
	int binode;			/* the node number on this bi */
	struct uba_device *ui;
{
	register struct dmb_device *addr;

	addr = (struct dmb_device *)nxv;
#	ifdef DEBUG
	printd("dmbinit: addr = 0x%x, devtype = 0x%x, revcode = 0x%x, nodeid = %d\n",
		addr,
		addr->dmb_biic.biic_typ & 0xFFFF,
		(addr->dmb_biic.biic_typ >> 16) & 0xFFFF,
		addr->dmb_biic.biic_ctrl & 0xF);
#	endif

	/*
	 * If neither the async lines nor the printer port is functioning,
	 * then say so.
	 */
	if ((addr->dmb_mainthigh & (DMB_ALP | DMB_PP)) == 0)
		{
		printf("dmbinit: device broken\n");
		return(0);
		}
	/*
	 * Enable external vector
	 */
	addr->dmb_biic.biic_int_ctrl |= BIIC_EXVEC;

	/*
	 * Set up interrupt vectors for DMB32.	It has 4.
	 * BIIC (14), sync (15), async (16), & printer (17).
	 */
	bidev_vec(binumber,binode,LEVEL15,ui);
	return(1);
}

dmbattach(ui)
	struct uba_device *ui;
{
	register int cols;
	register int lines;
	register struct dmb_device *addr;

	addr = (struct dmb_device *)ui->ui_addr;
#	ifdef DEBUG
	printd("dmbattach: addr = 0x%x, flags = 0x%x, unit = %d\n",
		addr, ui->ui_flags, ui->ui_unit);
#	endif
	cols = (ui->ui_flags>>8) & 0xff;
	lines = (ui->ui_flags>>16) & 0xff;
	/*
	 * Set soft carrier (local lines) & line printer characteristics
	 */
	dmbsoftCAR[ui->ui_unit] = ui->ui_flags & 0xff;
	dmbdefaultCAR[ui->ui_unit] = ui->ui_flags & 0xff;
	dmbl_softc[ui->ui_unit].dmbl_cols = (cols == 0?DMBL_DEFCOLS:cols);
	dmbl_softc[ui->ui_unit].dmbl_lines = (lines == 0?DMBL_DEFLINES:lines);
	/*
	 * Fill in system page table address and size.
	 * Put in dummy for global page table address and size.
	 */
	addr->dmb_spte = (long)mfpr(SBR);
	addr->dmb_spts = (long)mfpr(SLR);
	addr->dmb_gpte = (long)dmb_tty;
	addr->dmb_gpts = 0;
	addr->dmb_maintlow |= DMB_PTEVALID;
}

/*
 * Open a DMB32 line.
 * Turn on this dmb if this is the first use of it.
 */
/*ARGSUSED*/
dmbopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit, dmb;
	register struct dmb_device *addr;
	register struct uba_device *ui;
	int s;
	int inuse;  /* hold state of inuse bit while blocked waiting for carr */

	unit = minor(dev);
	if (unit & 0200)
		return(dmblopen(dev,flag));
	dmb = unit >> 3;
#	ifdef DEBUG
	printd("dmbopen: dev = 0x%x, flag = 0x%x, unit = 0x%x\n", dev, flag, unit);
#	endif
	if (unit >= ndmb || (ui = dmbinfo[dmb])== 0 || ui->ui_alive == 0)
		return (ENXIO);
	tp = &dmb_tty[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
		return (EBUSY);
	while (tp->t_state&TS_CLOSING) /* let DTR stay down for awhile */
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	addr = (struct dmb_device *)ui->ui_addr;
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dmbstart;
	tp->t_state |= TS_WOPEN;
	/*
	 * If this is first open, initialze tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0)
		{
		dmbmodem[unit] = MODEM_DSR_START; /* prevents spurious startups */
		ttychars(tp);
		if (tp->t_ispeed == 0)
			{
			tp->t_dev = dev;  /* timeouts need this */
			tp->t_ispeed = B300;
			tp->t_ospeed = B300;
			tp->t_flags = ODDP|EVENP|ECHO;
			}
		addr->dmb_acsr2 = (dmb_timeout << 16);
		}
	/*
	 * Interrupts are enabled in dmbparam.
	 */
	dmbparam(unit);
	/*
	 * Wait for carrier, then process line discipline specific open.
	 */
	s = spl5();
	addr->dmb_acsr = (unit & 07) | DMB_IE;	/* line select */
	addr->dmb_lpr |= (DMB_DTR | DMB_RTS);
	if (dmbsoftCAR[dmb] & (1<<(unit&07)))
		{
#		ifdef  DEBUG
		printd("dmbopen: local, tp=0x%x\n", tp);
#		endif
		tp->t_state |= TS_CARR_ON;
		dmbmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
	else if ((flag & O_NDELAY) == 0)
		{
		/*
		 * DSR should not normally come up until DTR is asserted
		 * However if TS_HUPCL is not set, it is
		 * possible to get here with all modem signals
		 * already asserted.  Or we could be dealing with
		 * a very slow modem and it has not deasserted DSR yet.
		 * Interrupts are enabled earlier in dmbparam.
		 * 
		 * If the DSR signal is being followed, wait at most
		 * 30 seconds for CD, and don't transmit in the first 
		 * 500ms.  Otherwise immediately look for CD|CTS.
		 */
		if (dmbdsr) {
			if (addr->dmb_lstatlow & DMB_DSR)
				{
#				ifdef DEBUG
				printd("dmbopen: modem, unit=%d\n", unit);
#				endif
				dmbmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
				tp->t_dev = dev; /* need it for timeouts */
				timeout(dmb_dsr_check, tp, hz*30);
				timeout(dmb_dsr_check, tp, hz/2);
				}
			}
		else 	{
			dmbmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			dmb_dsr_check(tp);
			}
		}
	if (flag & O_NDELAY)
		tp->t_state |= TS_ONDELAY;
	else
		while ((tp->t_state & TS_CARR_ON) == 0)
			{
			tp->t_state |= TS_WOPEN;
			inuse = tp->t_state&TS_INUSE;
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
			/*
			 * See if wakeup was caused by a false start.
			 */
			if (dmbmodem[unit]&MODEM_BADCALL){
				splx(s);
				return(EWOULDBLOCK);
			}
			/*
			 *  If we opened "block if in use"  and
			 *  the terminal was not in use at that time
			 *  but it became "in use" while we were
			 *  waiting for carrier then return.
			 */
			if ((flag & O_BLKINUSE) && (inuse==0) &&
			    (tp->t_state&TS_INUSE))
				{
				splx(s);
				return(EALREADY);
				}
			}
	splx(s);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}


/*
 * Close a DMB32 line.
 */
/*ARGSUSED*/
dmbclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit;
	register int dmb;
	register struct dmb_device *addr;
	int s;

	unit = minor(dev);
	if (unit & 0200){
		dmblclose(dev,flag);
		return;
	}
	dmb = unit >> 3;

	tp = &dmb_tty[unit];
	addr = (struct dmb_device *)tp->t_addr;
#	ifdef DEBUG
	printd("dmbclose: dev = 0x%x, flag = 0x%x, unit = 0x%x, addr = 0x%x\n",
		dev, flag, unit, addr);
#	endif
	if (tp->t_line)
		(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_state&TS_HUPCLS) || (tp->t_state&TS_ISOPEN)==0)
		{
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(DMB_DTR | DMB_RTS);
		tp->t_state &= ~TS_CARR_ON; /* prevents recv intr. timeouts */
		if ((dmbsoftCAR[dmb] & (1<<(unit&07)))==0)
			{
			/*
			 * Drop DTR for at least a half sec. if modem line
			 */
			tp->t_state |= TS_CLOSING;
			/*
			 * Wait for DSR to drop
			 */
			addr = (struct dmb_device *)tp->t_addr;
			addr->dmb_acsr = DMB_IE | (unit&07);
			/*
 			 * If the DSR signal is being followed, give the
			 * modem at most 5 seconds to deassert it.
 			 */
			if (dmbdsr && (addr->dmb_lstatlow & DMB_DSR))
				{
				timeout(wakeup, (caddr_t) &tp->t_dev, 5*hz);
				sleep((caddr_t)&tp->t_dev, PZERO-10);
				}
			timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
			sleep((caddr_t)&tp->t_dev, PZERO-10);
			tp->t_state &= ~(TS_CLOSING);
			wakeup((caddr_t)&tp->t_rawq);
			}
		addr->dmb_lpr &= ~DMB_RXENA;
		splx(s);
		}
	dmbsoftCAR[dmb] &= ~(1<<(unit&07));
	dmbsoftCAR[dmb] |= (1<<(unit&07)) & dmbdefaultCAR[dmb];
	ttyclose(tp);
	dmbmodem[unit] = 0;
}


dmbread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

	if (minor(dev) & 0200)		/* read from lp device */
		return(ENXIO);
	tp = &dmb_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

dmbwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

	if (minor(dev) & 0200)
		return(dmblwrite(dev,uio));
	tp = &dmb_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * DMB32 interrupts at one location with async interrupts.
 * Examine Transmitter Action bit in "tbuf" register to determine
 *   if there is a transmit interrupt.
 * Examine Data Valid bit in "rbuf" register to determine
 *   if there is a receive interrupt.
 */

dmbaint(dmb)
	int dmb;
{
	register struct dmb_device *addr;
	register struct uba_device *ui;

	ui = dmbinfo[dmb];
	addr = (struct dmb_device *)ui->ui_addr;
	if (addr->dmb_tbuf & DMB_TXACT)
	    dmbxint(dmb);
	if (addr->dmb_rbuf & DMB_DATAVALID)
	    dmbrint(dmb);
}

/*
 * DMB32 receiver interrupt.
 */
dmbrint(dmb)
	int dmb;
{
	register struct tty *tp;
	register long c;
	register struct dmb_device *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	register int line;
	int overrun = 0;
	u_char *modem0, *modem;
	int modem_cont;

	ui = dmbinfo[dmb];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dmb_device *)ui->ui_addr;
	tp0 = &dmb_tty[dmb<<3];
	modem0 = &dmbmodem[dmb<<3];
	/*
	 * Loop fetching characters from the receive fifo for this
	 * dmb until there are no more in the fifo.
	 */
	while ((c = addr->dmb_rbuf) < 0)
		{	/* if c < 0 then data valid bit is set */
		addr->dmb_rbuf = 0;		/* pop the fifo */
		line = (c>>16)&07;
		tp = tp0 + line;
		modem = modem0 + line;
#		ifdef  DEBUG
		printd10("dmbrint: tp = 0x%x, c = 0x%x\n", tp, c);
#		endif
		/*
		 * Check for modem transitions
		 */
		if (c & DMB_NONCHAR)
			{
			if (c & DMB_DIAG)
				continue;	/* ignore diagnostic info */
#				ifdef DEBUG
				printd("dmbrint: modem change, line = %d, tp = 0x%x, c = 0x%x, lstat = 0x%x\n", line, tp, c, addr->dmb_lstatlow);
#				endif DEBUG
			if (dmbsoftCAR[dmb] & 1<<line)
				continue;
			addr->dmb_acsr = (line | DMB_IE);
			modem_cont = 0;
			/*
			 * Drop DTR immediately if DSR gone away.
			 * If really an active close then do not
			 *    send signals.
			 */
			if ((addr->dmb_lstatlow & DMB_DSR) == 0)
				{
				if (tp->t_state&TS_CLOSING)
					{
					untimeout(wakeup, (caddr_t) &tp->t_dev);
					wakeup((caddr_t) &tp->t_dev);
#						ifdef	DEBUG
						printd("dmbrint: dsr closing down, line=%d\n",line);
#						endif	DEBUG
					continue;
					}
				 if (tp->t_state & TS_CARR_ON)
					{
#						ifdef	DEBUG
						printd("dmbrint: DSR drop, line = %d\n",line);
#						endif	DEBUG
					/*
 					 * Drop line if DSR is being followed.
 					 */
					if (dmbdsr)
						{
						dmb_tty_drop(tp);
						/*
						 * Move continue here in order
						 * to examine other transitions.
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
				if ((addr->dmb_lstatlow & DMB_DCD)==0)
					{
					if (*modem & MODEM_CD)
						{
						/* only start timer once */
#						ifdef DEBUG
						printd("dmbrint, cd_drop,line = %d\n",line);
#						endif DEBUG
						*modem &= ~MODEM_CD;
						dmbtimestamp[minor(tp->t_dev)] = time;
						timeout(dmb_cd_drop, tp, hz*dmbcdtime);
						modem_cont = 1;
						}
					}
				else
				    /*
				     * CD has come up again.
				     * Stop timeout from occurring if set.
				     * If interval is more than 2 secs then
				     *	drop DTR.
				     */
				    if ((*modem & MODEM_CD)==0)
					    {
					    untimeout(dmb_cd_drop, tp);
					    if (dmb_cd_down(tp)){
						    /* drop connection */
						dmb_tty_drop(tp);
					    }
					    *modem |= MODEM_CD;
					    modem_cont = 1;
					    }
			/* CTS flow control check */

			if (tp->t_state&TS_CARR_ON)
				if ((addr->dmb_lstatlow & DMB_CTS)==0)
					{
					tp->t_state |= TS_TTSTOP;
					*modem &= ~MODEM_CTS;
#					ifdef DEBUG
					printd("dmbrint: CTS stop, line=%d\n",line);
#					endif DEBUG
					dmbstop(tp, 0);
					continue;
					}
				else if ((*modem&MODEM_CTS)==0)
					{
					tp->t_state &= ~TS_TTSTOP;
					*modem |= MODEM_CTS;
#					ifdef DEBUG
					printd("dmbrint: CTS start, line=%d\n",line);
#					endif DEBUG
					dmbstart(tp);
					continue;
					}
			/*
			 * Avoid calling dmb_start_tty due to a CD transition 
			 */
			if (modem_cont)
				continue;


			/*
			 * If 500 ms timer has not expired then don't
			 * check anything yet.
			 * Check to see if DSR|CTS|DCD are asserted.
			 * If so we have a live connection.
			 * If DSR is set for the first time we allow
			 * 30 seconds for a live connection.
			 *
			 * If the DSR signal is being followed, wait at most
			 * 30 seconds for CD, and don't transmit in the first 
			 * 500ms.  Otherwise immediately look for CD|CTS.
			 */
			if (dmbdsr) {
				if ((addr->dmb_lstatlow & DMB_XMIT) == DMB_XMIT
			    	&& (*modem & MODEM_DSR_START)==0)
					dmb_start_tty(tp);
				else
			    	if ((addr->dmb_lstatlow & DMB_DSR) &&
					(*modem & MODEM_DSR)==0)
					{
					*modem |= (MODEM_DSR_START|MODEM_DSR);
					/*
				 	* We should not look for CTS|CD for
				 	* about 500 ms.
				 	*/
					timeout(dmb_dsr_check, tp, hz*30);
					timeout(dmb_dsr_check, tp, hz/2);
					}
			}
			/*
			 * Ignore DSR
			 */
			else
				if ((addr->dmb_lstatlow & DMB_NODSR) == DMB_NODSR)
					dmb_start_tty(tp);

			continue;
			} /* end of modem transition tests */
		if ((tp->t_state&TS_ISOPEN)==0)
			{
			wakeup((caddr_t)&tp->t_rawq);
			continue;
			}
		if (c & DMB_PARITYERR)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP)
				continue;
		if ((c & DMB_OVERRUNERR) && overrun == 0)
			{
			printf("dmb%d: fifo overflow\n", dmb);
			overrun = 1;
			}
		if (c & DMB_FRAMEERR)
			/*
			 * At framing error (break) generate
			 * a null (in raw mode, for getty), or an
			 * interrupt (in cooked/cbreak mode).
			 */
			if (tp->t_flags&RAW)
				c = 0;
			else {
				c = tp->t_intrc;
				/*
				 * Strip extraneous sign extension bits.
				 */
				c &= 0377;
			}

#if NHC > 0
		if (tp->t_line == HCLDISC)
			{
			HCINPUT(c, tp);
			}
		else
#endif
			(*linesw[tp->t_line].l_rint)(c, tp);
		}  /* end while c < 0 */
}

/*
 * Ioctl for DMB32.
 */
/*ARGSUSED*/
dmbioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register int unit = minor(dev);
	register struct tty *tp;
	register int dmb;
	register struct dmb_device *addr;
	register int s;
	struct uba_device *ui;
	struct dmb_softc *sc;
	struct devget *devget;
	int error;

	if (unit & 0200)
		return (ENOTTY);
	tp = &dmb_tty[unit];
	dmb = unit >> 3;
	ui = dmbinfo[dmb];
	sc = &dmb_softc[ui->ui_unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0)
		{
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			dmbparam(unit);
		return (error);
		}
	addr = (struct dmb_device *)tp->t_addr;

#	ifdef DEBUG
	if (dmbdebug)
	   printd("dmbioctl: unit = %d, cmd = %d, data = 0x%x, flag = 0x%x\n",
		unit, cmd & 0xff, data, flag);
#	endif

	switch (cmd) {

	case TIOCSBRK:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr |= DMB_BREAK;
		splx(s);
		break;

	case TIOCCBRK:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~DMB_BREAK;
		splx(s);
		break;

	case TIOCSDTR:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr |= (DMB_DTR | DMB_RTS);
		splx(s);
		break;

	case TIOCCDTR:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(DMB_DTR | DMB_RTS);
		splx(s);
		break;

	case TIOCMSET:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(DMB_DTR | DMB_RTS);
		addr->dmb_lpr |= dmtodmb(*(int *)data);
		splx(s);
		break;

	case TIOCMBIS:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr |= dmtodmb(*(int *)data);
		splx(s);
		break;

	case TIOCMBIC:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(dmtodmb(*(int *)data));
		splx(s);
		break;

	case TIOCMGET:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		*(int *)data = dmbtodm(addr->dmb_lpr, addr->dmb_lstatlow);
		splx(s);
		break;

	case TIOCNMODEM:  /* ignore modem status */
		s = spl5();
		dmbsoftCAR[dmb] |= (1<<(unit&07));
		if (*(int *)data) /* make mode permanent */
			dmbdefaultCAR[dmb] |= (1<<(unit&07));
		dmbmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		tp->t_state |= TS_CARR_ON;
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(DMB_REPORT);
		splx(s);
		break;

	case TIOCMODEM:  /* look at modem status */
		s = spl5();
		dmbsoftCAR[dmb] &= ~(1<<(unit&07));
		if (*(int *)data) /* make mode permanent */
			dmbdefaultCAR[dmb] &= ~(1<<(unit&07));
		addr->dmb_acsr = (unit & 07) | DMB_IE;	/* line select */
		addr->dmb_lpr |= DMB_REPORT;
	   	/* 
	    	 * If dmbdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((dmbdsr && ((addr->dmb_lstatlow & DMB_XMIT) == DMB_XMIT)) ||
		    ((dmbdsr == 0) && ((addr->dmb_lstatlow & DMB_NODSR) == DMB_NODSR)))
			{
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dmbmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
			}
		else {
			dmbmodem[unit] & = ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
			tp->t_state &= ~(TS_CARR_ON);
		}
		splx(s);
		break;
	case TIOCWONLINE:
		s = spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;	/* line select */
	   	/* 
	    	 * If dmbdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((dmbdsr && ((addr->dmb_lstatlow & DMB_XMIT) == DMB_XMIT)) ||
		    ((dmbdsr == 0) && ((addr->dmb_lstatlow & DMB_NODSR) == DMB_NODSR)))
			{
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dmbmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
			}
		else
			while ((tp->t_state & TS_CARR_ON) == 0)
				sleep((caddr_t)&tp->t_rawq, TTIPRI);
		splx(s);
		break;

	/* handle maintenance mode */
	case TIOCSMLB:
		if (u.u_uid)
			return(EPERM);
		s=spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr |= (DMB_MAINT);
		splx(s);
		break;

	case TIOCCMLB:
		if (u.u_uid)
			return(EPERM);
		s=spl5();
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		addr->dmb_lpr &= ~(DMB_MAINT);
		splx(s);
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));

		if(dmbsoftCAR[dmb] & (1<<(unit&07))) {
			sc->sc_category_flags[unit] |= DEV_MODEM;
			sc->sc_category_flags[unit] &= ~DEV_MODEM_ON;
		}
		else
			sc->sc_category_flags[unit] |= (DEV_MODEM|DEV_MODEM_ON);

		devget->category = DEV_TERMINAL;
		devget->bus = DEV_BI;
		bcopy(DEV_DMB32,devget->interface,
		      strlen(DEV_DMB32));
		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));		/* terminal	*/
		devget->adpt_num = ui->ui_adpt; 	/* which adapter*/
		devget->nexus_num = ui->ui_nexus;	/* which node	*/
		devget->bus_num = ui->ui_ubanum;	/* which BI	*/
		devget->ctlr_num = dmb; 		/* which interf.*/
		devget->slave_num = unit%8;		/* which line	*/
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,
		      strlen(ui->ui_driver->ud_dname)); /* Ultrix "dmb" */
		devget->unit_num = unit;		/* which dmb?	*/
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

dmbtodm(lpr,lstat)
	register long lpr;
	register u_short lstat;
{
	register int b = 0;
	if (lpr&DMB_RTS)  b |= TIOCM_RTS;
	if (lpr&DMB_DTR)  b |= TIOCM_DTR;
	if (lstat&DMB_DCD)  b |= TIOCM_CD;
	if (lstat&DMB_CTS)  b |= TIOCM_CTS;
	if (lstat&DMB_RI)  b |= TIOCM_RI;
	if (lstat&DMB_DSR)  b |= TIOCM_DSR;
	return(b);
}


dmtodmb(bits)
	register int bits;
{
	register long lpr = 0;
	if (bits&TIOCM_RTS) lpr |= DMB_RTS;
	if (bits&TIOCM_DTR) lpr |= DMB_DTR;
	return(lpr);
}


/*
 * Set parameters from open or stty into the DMB hardware
 * registers.
 */
dmbparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dmb_device *addr;
	register long lpar;
	int s;

	tp = &dmb_tty[unit];
	addr = (struct dmb_device *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	addr->dmb_acsr = (unit & 07) | DMB_IE;	/* line select */
	if ((tp->t_ispeed)==0)
		{
		tp->t_state |= TS_HUPCLS;
		addr->dmb_lpr &= ~(DMB_DTR | DMB_RTS); /* turn off DTR & RTS but leave enabled */
		splx(s);
		return;
		}
	/* else - set line parameters */
	lpar = (dmb_speeds[tp->t_ospeed]<<28) | (dmb_speeds[tp->t_ispeed]<<24);
	if ((tp->t_ispeed) == B134)
		lpar |= DMB_BITS6|DMB_PARITYENAB;
	else if (tp->t_flags & (RAW|LITOUT))
		lpar |= DMB_BITS8;
	else
		lpar |= DMB_BITS7|DMB_PARITYENAB;
	if (tp->t_flags&EVENP)
		lpar |= DMB_EVENPARITY;
	if ((tp->t_ospeed) == B110)
		lpar |= DMB_TWOSTOPB;
	/*
	 * Set "Report Modem" bit to get interrupts on modem status changes.
	 * Enable receiver, and set Transmission interrupt delay so that
	 *   xmit interrupt does not actually happen until all characters
	 *   in current DMA have been transmitted.
	 * Make all lines modem lines, in case a line is
	 *   changed from local to remote (modem).
	 */
	lpar |= (DMB_REPORT | DMB_RXENA | DMB_TXINTDELAY | DMB_DTR | DMB_RTS);
	addr->dmb_lstathigh |= DMB_TXENA;
	addr->dmb_lpr = lpar;
	splx(s);
}


/*
 * DMB32 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */

dmbxint(dmb)
	int dmb;
{
	int unit = dmb * 8;
	struct tty *tp0 = &dmb_tty[unit];
	register struct tty *tp;
	register struct dmb_device *addr;
	register struct uba_device *ui;
	register long tbuf;
	u_short cntr;

	ui = dmbinfo[dmb];
	addr = (struct dmb_device *)ui->ui_addr;
	while ((tbuf = addr->dmb_tbuf) < 0)
		{			/* xmitter action is set if "< 0" */
		addr->dmb_tbuf = 0;	/* must write to clear xmitter act bit */
		/*
		 * Check Error byte and if any error bits set,
		 * decode the error.
		 */
		switch (tbuf & DMB_ERRMASK) {
		case DMB_TXDMAERROR:
			printf("dmb%d: DMA Error. tbuf = 0x%x\n", dmb, tbuf);
			break;
		case DMB_MSGERR:
			printf("dmb%d: Message Error. tbuf = 0x%x\n", dmb, tbuf);
			break;
		case DMB_LASTCHERR:
			printf("dmb%d: Last character Incomplete. tbuf = 0x%x\n", dmb, tbuf);
			break;
		case DMB_BUFERR:
			printf("dmb%d: Buffer Error. tbuf = 0x%x\n", dmb, tbuf);
			break;
		case DMB_MODEMERR:
			printf("dmb%d: Modem Error. tbuf = 0x%x\n", dmb, tbuf);
			break;
		case DMB_INTERNALERR:
			printf("dmb%d: Internal Error. tbuf = 0x%x\n", dmb, tbuf);
			break;
		}
		/* tbuf becomes the line number */
		tbuf = tbuf & 07;
		tp = tp0 + tbuf;
#		ifdef DEBUG
		printd10("dmbxint: unit=0x%x, line=%d, tp=0x%x, c_cc=%d\n",
				unit, tbuf, tp, tp->t_outq.c_cc);
#		endif
		tp->t_state &= ~TS_BUSY;
		if (tp->t_state&TS_FLUSH)
			tp->t_state &= ~TS_FLUSH;	/* was a non ^S stop */
		else	{
			/*
			 * Determine number of characters transmitted so far
			 * and flush these from the tty output queue.
			 * (unit is the dmb unit number so add in the line #.)
			 */
			addr->dmb_acsr = DMB_IE | tbuf;  /* select line */
			cntr = dmb_numchars[unit+tbuf] -
				((addr->dmb_tbuffct >> DMB_TXCHARCT) & 0xffff);
			ndflush(&tp->t_outq, (int)cntr);
			}
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dmbstart(tp);
		}
}


/*
 * Start (restart) transmission on the given DMB32 line.
 */

dmbstart(tp)
	register struct tty *tp;
{
	register struct dmb_device *addr;
	register int unit, nch;
	int s;

	unit = minor(tp->t_dev);
	addr = (struct dmb_device *)tp->t_addr;

	/*
	 * Must hold interrupts in following code to prevent
	 * state of the tp from changing.
	 */
	s = spl5();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 * Also do not transmit if not CTS.
	 */
	if ((tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) ||
	   ((tp->t_state & TS_CARR_ON) && (dmbmodem[unit]&MODEM_CTS)==0))
		{
		splx(s);
		return;
		}
	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT(tp))
		{
		if (tp->t_state&TS_ASLEEP)
			{
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
			}
		if (tp->t_wsel)
			{		/* for select system call */
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
			}
		}
	/*
	 * Now restart transmission unless the output queue is empty.
	 */
	if (tp->t_outq.c_cc == 0)
		{
		splx(s);
		return;
		}
	if (tp->t_flags & (RAW|LITOUT))
		nch = ndqb(&tp->t_outq, 0);	/* # of consecutive chars */
	else	{
		nch = ndqb(&tp->t_outq, 0200);
		/*
		 * If first thing on queue is a delay process it.
		 */
		if (nch == 0)
			{
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			splx(s);
			return;
			}
		}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch)
		{
		addr->dmb_acsr = DMB_IE | (unit & 07);	/* line select */
		/*
		 * Wait for dma start to clear
		 */
		while (addr->dmb_lnctrl & DMB_TXDMASTART)
			{
#			ifdef DEBUG
			printd("dmbstart: TXDMASTART set.\n");
#			endif
			}
		if (addr->dmb_lnctrl & DMB_TXOUTABORT)
			addr->dmb_lnctrl &= ~(DMB_TXOUTABORT);
		addr->dmb_lstathigh |= DMB_TXENA;
		addr->dmb_tbuffadd = (long)tp->t_outq.c_cf;
		addr->dmb_tbuffct = (nch << DMB_TXCHARCT);
		/*
		 * Save the number of characters that we are DMA'ing,
		 * for use in the transmit interrupt routine.
		 * (unit is the whole minor device, unit # & line #.)
		 */
		dmb_numchars[unit] = nch;
		addr->dmb_lnctrl |= DMB_TXDMASTART;
		tp->t_state |= TS_BUSY;
		}
	splx(s);
}


/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dmbstop(tp, flag)
	register struct tty *tp;
{
	register struct dmb_device *addr;
	register int unit, s;

	addr = (struct dmb_device *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY)
		{
		/*
		 * Device is transmitting; stop output.
		 * We can continue later by examining the character count.
		 */
		unit = minor(tp->t_dev);
		addr->dmb_acsr = (unit & 07) | DMB_IE;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;	/* NOT a ctl-S */
		addr->dmb_lnctrl |= DMB_TXOUTABORT;  /* abort DMA transmission */
		}
	splx(s);
}


/*
 * Reset state of driver if BI reset was necessary.
 * Reset the csr and lpr registers on open lines, and
 * restart transmitters.
 */
dmbreset(binum)
	int binum;				/* bi adapter that reset */
{
	register int dmb, unit;
	register struct tty *tp;
	register struct uba_device *ui;
	register struct dmb_device *addr;
	register int i;

	for (dmb = 0; dmb < nNDMB; dmb++)
		{
		ui = dmbinfo[dmb];
		if (ui == 0 || ui->ui_alive == 0 || ui->ui_adpt != binum)
			continue;
		printf(" dmb%d", dmb);
		addr = (struct dmb_device *)ui->ui_addr;
		/*
		 * Enable external vector
		 */
		addr->dmb_biic.biic_int_ctrl |= BIIC_EXVEC;

		addr->dmb_acsr2 = (dmb_timeout << 16);	/* 10 ms */
		unit = dmb * 8;
		for (i = 0; i < 8; i++)
			{
			tp = &dmb_tty[unit];
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN))
				{
				dmbparam(unit); 	/* resets lpr reg */
				tp->t_state &= ~TS_BUSY;
				dmbstart(tp);
				}
			unit++; 		/* increment the line number */
			}
		}
}


/*
 * dmblopen -- open the line printer port on a dmb32
 */
/*ARGSUSED*/
dmblopen(dev,flag)
	dev_t dev;
	int flag;
{
	register int dmb;
	register struct dmbl_softc *sc;
	register struct uba_device *ui;
	register struct dmb_device *addr;

	dmb = minor(dev) & 07 ;
	ui = dmbinfo[dmb];
	sc = &dmbl_softc[dmb];
	if ((sc->dmbl_state & OPEN) || (ui == 0) || (ui->ui_alive == 0))
		{
		return(ENXIO);
		}
	addr = (struct dmb_device *)ui->ui_addr;
	if ((addr->dmb_pcsrhigh & DMB_PROFFLINE))
		{
#		ifdef DEBUG
		printd("dmb%d: line printer offline/jammed\n", dmb);
		return(EIO);
#		endif
		}
	if ((addr->dmb_pcsrhigh & DMB_PRCONNECT) == 0)
		{
		printf("dmb%d: line printer disconnected\n", dmb);
		return(EIO);
		}
	/*
	 * disable interrupts
	 */
	addr->dmb_pcsrlow = 0;
	sc->dmbl_state |= OPEN;
	return(0);
}

/*ARGSUSED*/
dmblclose(dev,flag)
	dev_t dev;
	int flag;
{
	register int dmb;
	register struct dmbl_softc *sc;

	dmb = minor(dev) & 07;
	sc = &dmbl_softc[dmb];
	dmblout(dev,"\f",1);
	sc->dmbl_state = 0;
	/*
	 * disable interrupts
	 */
	((struct dmb_device *)(dmbinfo[dmb]->ui_addr))->dmb_pcsrlow = 0;
	return(0);
}

dmblwrite(dev,uio)
	dev_t dev;
	struct uio *uio;
{
	register unsigned int n;
	register int error;
	register struct dmbl_softc *sc;

	sc = &dmbl_softc[minor(dev)&07];
	if (sc->dmbl_state & ERROR)
		return(EIO);
	while(n = min(DMBL_BUFSIZE,(unsigned)uio->uio_resid))
		{
		if (error = uiomove(&sc->dmbl_buf[0],(int)n,UIO_WRITE,uio))
			{
			printf("uio move error\n");
			return(error);
			}
		if (error = dmblout(dev,&sc->dmbl_buf[0],n))
			return(error);
		}
	return(0);
}


/*
 * dmblout -- start io operation to dmb line printer
 *		cp is addr of buf of n chars to be sent.
 *
 *	-- dmb will NOT be put in formatted output mode, this will
 *		be selectable from an ioctl if the
 *		need ever arises.
 */
dmblout(dev,cp,n)
	dev_t dev;
	char *cp;
	int n;
{
	register struct dmbl_softc *sc;
	register int dmb;
	register struct uba_device *ui;
	register struct dmb_device *addr;
	register unsigned s;

	dmb = minor(dev) & 07;
	sc = &dmbl_softc[dmb];
	if (sc->dmbl_state & ERROR)
		return(EIO);
	ui = dmbinfo[dmb];
	addr = (struct dmb_device *)ui->ui_addr;
	addr->dmb_pbufad = (long) cp;
	addr->dmb_pbufct = (n << DMB_PRCHARCT);
	addr->dmb_psiz = (sc->dmbl_lines << DMB_PRPAGE) | (sc->dmbl_cols);
	sc->dmbl_state |= ASLP;
	s=spl5();
	addr->dmb_pcsrlow = DMB_PRIE;
	addr->dmb_pctrl |= DMB_PRSTART;
	while(sc->dmbl_state & ASLP)
		{
		sleep(&sc->dmbl_buf[0],(PZERO+8));
		while(sc->dmbl_state&ERROR)
			{
			timeout(dmblint,dmb,10*hz);
			sleep(&sc->dmbl_state,(PZERO+8));
			}
		/*if (sc->dmbl_state&ERROR) return (EIO);*/
		}
	splx(s);
	return(0);
}

/*
 * dmblint -- handle an interrupt from the line printer part of the dmb32
 */

dmblint(dmb)
	int dmb;
{
	register struct uba_device *ui;
	register struct dmbl_softc *sc;
	register struct dmb_device *addr;

	ui = dmbinfo[dmb];
	sc = &dmbl_softc[dmb];
	addr = (struct dmb_device *)ui->ui_addr;
	addr->dmb_pcsrlow = 0;		/* disable interrupts */

	if (sc->dmbl_state & ERROR)
		{
#		ifdef DEBUG
		printd("dmb%d: intr while in error state\n", dmb);
#		endif
		if ((addr->dmb_pcsrhigh & DMB_PROFFLINE) == 0)
			sc->dmbl_state &= ~ERROR;
		wakeup(&sc->dmbl_state);
		return;
		}
	if (addr->dmb_pctrl & DMB_PRERROR)
		printf("dmb%d: Printer DMA Error\n", dmb);
	if (addr->dmb_pcsrhigh & DMB_PROFFLINE)
		{
#		ifdef DEBUG
		printd("dmb%d: printer offline\n", dmb);
#		endif
		sc->dmbl_state |= ERROR;
		}
#	ifdef DEBUG
	if (addr->dmb_pctrl & DMB_PRSTART)
		printd("DMB%d printer intr w/ DMA START still set\n", dmb);
	else
		{
		printd("bytes= %d\n", addr->dmb_pcar & DMB_PRCHAR);
		printd("lines= %d\n",addr->dmb_pcar & DMB_PRLINE);
		}
#	endif
	sc->dmbl_state &= ~ASLP;
	wakeup(&sc->dmbl_buf[0]);
}

/* stub for synchronous device interrupt routine which is not supported */

dmbsint() { printf("dmbsint\n"); }


dmb_cd_drop(tp)
	register struct tty *tp;
{
	register struct dmb_device *addr;
	register unit;

	unit = minor(tp->t_dev);
	addr = (struct dmb_device *)tp->t_addr;
	addr->dmb_acsr = DMB_IE | (unit&07);
	if ((tp->t_state&TS_CARR_ON) &&
	   ((addr->dmb_lstatlow & DMB_DCD) == 0))
		{
#		ifdef DEBUG
		printd("dmb_cd:  no CD, tp=0x%x\n", tp);
#		endif
		dmb_tty_drop(tp);
		return;
		}
	dmbmodem[unit] |= MODEM_CD;
#	ifdef DEBUG
	printd("dmb_cd:  CD is up, tp=0x%x\n", tp);
#	endif
}


dmb_dsr_check(tp)
	register struct tty *tp;
{
	int unit;
	register struct dmb_device *addr;

	unit = minor(tp->t_dev);
	addr = (struct dmb_device *)tp->t_addr;
	if (dmbmodem[unit] & MODEM_DSR_START)
		{
		dmbmodem[unit] &= ~MODEM_DSR_START;
		addr->dmb_acsr = DMB_IE | (unit&07);
	   	/* 
	    	 * If dmzdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if (dmbdsr) {
			if ((addr->dmb_lstatlow & DMB_XMIT) == DMB_XMIT)
				dmb_start_tty(tp);
		}
		else {
			if ((addr->dmb_lstatlow & DMB_NODSR) == DMB_NODSR) 
				dmb_start_tty(tp);
		}
		return;
		}
	if ((tp->t_state&TS_CARR_ON)==0)
		{
		dmb_tty_drop(tp);
#		ifdef DEBUG
		printd("dmb_dsr: no carrier, tp=0x%x\n", tp);
#		endif
		}
#	ifdef DEBUG
	else
		printd("dmb_dsr:  carrier is up, tp=0x%x\n", tp);
#	endif
}

/*
 *  cd_down return 1 if carrier has been down for at least 2 secs.
 */
dmb_cd_down(tp)
	struct tty *tp;
{
	int msecs;
	register int unit;

	unit = minor(tp->t_dev);
	msecs = 1000000 * (time.tv_sec - dmbtimestamp[unit].tv_sec) +
		(time.tv_usec - dmbtimestamp[unit].tv_usec);
	if (msecs > 2000000)
		return(1);
	else
		return(0);
}

dmb_tty_drop(tp)
	struct tty *tp;
{
	register struct dmb_device *addr;
	register int unit;

	if (tp->t_flags & NOHANG)
		return;
	unit = minor(tp->t_dev);
	dmbmodem[unit] = MODEM_BADCALL;
	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP);
	wakeup((caddr_t)&tp->t_rawq);
	gsignal(tp->t_pgrp, SIGHUP);
	gsignal(tp->t_pgrp, SIGCONT);
	addr = (struct dmb_device *)tp->t_addr;
	addr->dmb_acsr = DMB_IE | ((minor(tp->t_dev)) & 07);
	addr->dmb_lpr &= ~(DMB_DTR | DMB_RTS); /* turn off DTR & RTS */
}

dmb_start_tty(tp)
	register struct tty *tp;
{
	register int unit = minor(tp->t_dev);

	unit = minor(tp->t_dev);
	tp->t_state &= ~TS_ONDELAY;
	tp->t_state |= TS_CARR_ON;
	if (dmbmodem[unit]&MODEM_DSR)
		untimeout(dmb_dsr_check, tp);
	dmbmodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	dmbtimestamp[unit] = dmbzerotime;
	wakeup((caddr_t)&tp->t_rawq);
}
#endif
