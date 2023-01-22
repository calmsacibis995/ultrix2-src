
#ifndef lint
static char *sccsid = "@(#)dmf.c	1.35	ULTRIX	2/12/87";
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
 * dmf.c  6.1	7/29/83
 *
 * Modification history
 *
 * DMF32 terminal driver
 *
 *  5-May-85 - Larry Cohen
 *
 *	Derived from 4.2BSD labeled: dmf.c	6.1	83/07/29.
 *	Add dma support, switch between dma and silo - derived from UCB.
 *
 * 16-Jan-86 - Larry Cohen
 *
 *	Add full DEC standard 52 support.
 *
 * 18-Mar-86 - jaw
 *
 *	br/cvec changed to NOT use registers.
 *
 * 14-Apr-86 - jaw
 *
 *	remove MAXNUBA references.....use NUBA only!
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added new DEVIOCGET ioctl request code. V2.0
 *
 * 11-Jul-86 - ricky palmer
 *
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
 * 05-Aug-86 - Tim Burke
 *
 *	In dmfrint, record present time in timestamp in the event of
 *	a carrier drop.
 *
 * 25-Aug-86 - Tim Burke
 *
 *	Insure that line drops and restarts on false calls.  Also change
 *	state to ~TS_CARR_ON to terminate all processes on line.
 *
 * 26-Aug-86 - rsp (Ricky Palmer)
 *
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 *  4-Dec-86 - Tim Burke
 *
 *	Bug fix to modem control.  In dmf_tty_drop routine, clear the stopped
 *	state to prevent modem lines from hanging on close.  
 *
 *  4-Dec-86 - Tim Burke
 *
 *	Bug fix to dmfreset code.  Saves software programmable interrupt
 *	vector as established in the probe routine so that it may be restored
 *	in the reset routine.
 *
 * 15-Dec-86 - Tim Burke
 *
 *	When a break occurs, (interpreted as a framing error) set the variable
 *	c to be the interrupt character.  There was a problem here due to the
 *	fact that sign extension is done which causes unwanted side affects. To
 *	solve this bug, the character is stripped to 8 bits.
 *
 *	Modified probe routine to wait for self test to complete.
 *
 *	Fix DEVIOGET to return propper modem status information.
 *
 *  9-Jan-87 - Tim Burke
 *
 *	Bug fix to TIOCMODEM to clear modem flags if signals are not up.
 *
 * 28-Jan-87 - Tim Burke
 *
 *	Added the capability to ignore the "DSR" modem signal.  This is being
 *	done to allow modems that do not follow DEC Standard 52 to still 
 *	function as they would have prior to the addition of DECSTD52 code
 *	into the drivers.  If the driver is setup to ignore "DSR" then it will
 *	not be following DECSTD52.  To follow DECSTD52 set dmfdsr to "1", to
 *	ignore "DSR" set dmfdsr to be "0";
 *
 *  6-Feb-87 - Tim Burke
 *
 *	Removed printf of master reset failure in probe routine, as it may be
 *	incorrectly appearing. (Particularly in the DMF & DMZ drivers)
 */

#include "dmf.h"
#if NDMF > 0  || defined(BINARY)

#include "../data/dmf_data.c"

/*
 * The line printer port is indicated by a minor device code of 128+x.
 * The flags field of the config file is interpreted as:
 *
 * bits 	meaning
 * ---- 	-------
 * 0-7		soft carrier bits for ttys part of dmf32
 * 8-15 	number of cols/line on the line printer
 *			if 0, 132 will be used.
 * 16-23	number of lines/page on the line printer
 *			if 0, 66 will be used.
 *
 */
/*
 * Definition of the driver for the auto-configuration program.
 */
int	dmfprobe(), dmfattach(), dmfrint(), dmfxint() ;
int	dmf_cd_drop(), dmf_dsr_check(), dmf_cd_down(), dmf_tty_drop();
struct	timeval dmfzerotime = {0,0};
int	dmfcdtime = 2;
int	dmflint();
struct	uba_device *dmfinfo[NDMF];
u_short dmfstd[] = { 0 };
struct	uba_driver dmfdriver =
	{ dmfprobe, 0, dmfattach, 0, dmfstd, "dmf", dmfinfo };

int	dmf_timeout = 10;		/* silo timeout, in ms */
int	dmf_mindma = 4; 		/* don't dma below this point */

/*
 * Local variables for the driver
 */
char	dmf_speeds[] =
	{ 0, 0, 1, 2, 3, 4, 0, 5, 6, 7, 010, 012, 014, 016, 017, 0 };



#define ASLP 1		/* waiting for interrupt from dmf */
#define OPEN 2		/* line printer is open */
#define ERROR 4 	/* error while printing, driver
			 refuses to do anything till closed */

int	dmfact; 			/* mask of active dmf's */
int	dmfstart(), ttrstrt();

#ifndef MODEM_CD
#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#endif

/*
 * The clist space is mapped by the driver onto each UNIBUS.
 * The UBACVT macro converts a clist space address for unibus uban
 * into an i/o space address for the DMA routine.
 */
#define UBACVT(x, uban) 	(cbase[uban] + ((x)-(char *)cfree))

int dmfdebug;

/*
 * Routine for configuration to set dmf interrupt.
 */
/*ARGSUSED*/
dmfprobe(reg, ctlr)
	caddr_t reg;
	int ctlr;
{
	register struct dmfdevice *dmfaddr = (struct dmfdevice *)reg;
	register int i;
	register int totaldelay;
/*
	register unsigned int a;
	static char *dmfdevs[]=
		{"parallel","line printer","synch","asynch"};
*/

#ifdef lint
	dmfxint(0); dmfrint(0);
	dmfsrint(); dmfsxint(); dmfdaint(); dmfdbint(); dmflint();
#endif
	if(dmfdebug)
		printf("dmfprobe\n");
	/*
	 * If a self test is not being done, start one up.  Wait for the
	 * self-test (to a max of 4 sec.) to complete before interrupting.
	 */

	if ((dmfaddr->dmfcsr & DMF_CLR) == 0)
	    dmfaddr->dmfcsr |= DMF_CLR;
	totaldelay = 0;
	while ( (dmfaddr->dmfcsr & DMF_CLR) && ( totaldelay <= 70) ){
	    totaldelay++;
	    DELAY(50000);
	}
	/*
	 * This message may be incorrectly printed - particularly in the
	 * DMF & DMZ drivers.
	if (dmfaddr->dmfcsr & DMF_CLR)
	    printf("Warning: DMF device failed to exit self-test\n");
	 */

	br = 0x15;
	cvec = (uba_hd[numuba].uh_lastiv -= 4*8);
	/*
	 * Save software programmable interrupt vector for use in dmfreset.
	 */
	dmf_vector[ctlr] = (cvec >> 2);
	dmfaddr->dmfccsr0 = dmf_vector[ctlr];
/*
	a = (dmfaddr->dmfccsr0>>12) & 0xf;
	for(i=0;a != 0;++i,a >>= 1)
	{
		if(a&1)
			printf("\t(%s)\n",dmfdevs[i]);
	}
*/
	dmfaddr->dmfl[0] = DMFL_RESET;
	/* NEED TO SAVE IT SOMEWHERE FOR OTHER DEVICES */
	return (sizeof (struct dmfdevice));
}

/*
 * Routine called to attach a dmf.
 */
dmfattach(ui)
	struct uba_device *ui;
{
	register int cols = (ui->ui_flags>>8) & 0xff;
	register int lines = (ui->ui_flags>>16) & 0xff;

	dmfsoftCAR[ui->ui_unit] = ui->ui_flags & 0xff;
	dmfdefaultCAR[ui->ui_unit] = ui->ui_flags & 0xff;
	dmfl_softc[ui->ui_unit].dmfl_cols = cols==0?DMFL_DEFCOLS:cols;
	dmfl_softc[ui->ui_unit].dmfl_lines = lines==0?DMFL_DEFLINES:lines;
}


/*
 * Open a DMF32 line, mapping the clist onto the uba if this
 * is the first dmf on this uba.  Turn on this dmf if this is
 * the first use of it.
 */
/*ARGSUSED*/
dmfopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dmf;
	register struct dmfdevice *addr;
	register struct uba_device *ui;
	int s;
	int inuse;  /*hold state of inuse bit while blocked waiting for carr*/

	unit = minor(dev);
	if(unit & 0200)
		return(dmflopen(dev,flag));
	dmf = unit >> 3;
	if (unit >= ndmf || (ui = dmfinfo[dmf])== 0 || ui->ui_alive == 0)
		return (ENXIO);
	tp = &dmf_tty[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
		return (EBUSY);
	while (tp->t_state&TS_CLOSING) /* let DTR stay down for awhile */
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	addr = (struct dmfdevice *)ui->ui_addr;
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dmfstart;
	tp->t_state |= TS_WOPEN;
	/*
	 * While setting up state for this uba and this dmf,
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
	cbase[ui->ui_ubanum] = tty_ubinfo[ui->ui_ubanum]&0x3ffff;
	addr->dmfcsr |= DMF_IE;
	splx(s);
	/*
	 * If this is first open, initialze tty state to default.
	 */
	if ((tp->t_state&TS_ISOPEN) == 0) {
		dmfmodem[unit] = MODEM_DSR_START; /* prevents spurious
						     startups */
		ttychars(tp);
		if (tp->t_ispeed == 0) {
			tp->t_dev = dev;  /* timeouts need this */
			tp->t_ispeed = B300;
			tp->t_ospeed = B300;
			tp->t_flags = ODDP|EVENP|ECHO;
		}
		addr->dmfrsp = dmf_timeout;
	}
	dmfparam(unit);
	/*
	 * Wait for carrier, then process line discipline specific open.
	 */

	s = spl5();
	dmfmctl(dev, DMF_ON, DMSET);	/* set DTR */
	if (dmfsoftCAR[dmf] & (1<<(unit&07)))  {
#ifdef DMFDEBUG
		if (dmfdebug)
			mprintf("dmfopen: local, unit=%d\n", unit);
#endif
		tp->t_state |= TS_CARR_ON;
		dmfmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
	}
	  else
		if ((flag&O_NDELAY)==0) {
		    /*
		     * DSR should not come up until DTR is asserted
		     * normally.  However if TS_HUPCL is not set it is
		     * possible to get here with all modem signals
		     * already asserted.  Or we could be dealing with
		     * an enormously slow modem and it has not deasserted
		     * DSR yet.
		     */

		    addr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
		    /*
		     * If the DSR signal is being followed, wait at most
		     * 30 seconds for CD, and don't transmit in the first 
		     * 500ms.  Otherwise immediately look for CD|CTS.
		     */
		    if (dmfdsr) {
		    	if (addr->dmfrms&DMF_DSR) {
#ifdef DMFDEBUG
			    if (dmfdebug)
				mprintf("dmfopen: modem, unit=%d\n", unit);
#endif
			    dmfmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			    tp->t_dev = dev; /* need it for timeouts */
			    timeout(dmf_dsr_check, tp, hz*30);
			    timeout(dmf_dsr_check, tp, hz/2);
		        }
		    }
		    else {
			    dmfmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			    dmf_dsr_check(tp);
		    }
		}

	if (flag & O_NDELAY)
		tp->t_state |= TS_ONDELAY;
	else
	  while ((tp->t_state & TS_CARR_ON) == 0) {
		tp->t_state |= TS_WOPEN;
		inuse = tp->t_state&TS_INUSE;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
		/*
		 * See if wakeup was due to a false call.
		 */
		if (dmfmodem[unit]&MODEM_BADCALL){
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
 * Close a DMF32 line.
 */
/*ARGSUSED*/
dmfclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register unit;
	register dmf, s;
	register struct dmfdevice *addr;

	unit = minor(dev);
	if(unit & 0200)
		return(dmflclose(dev,flag));
	dmf = unit >> 3;

	tp = &dmf_tty[unit];
	if (tp->t_line)
		(*linesw[tp->t_line].l_close)(tp);
	(void) dmfmctl(unit, DMF_BRK, DMBIC);
	if ((tp->t_state&TS_HUPCLS) || (tp->t_state&TS_ISOPEN)==0) {
		(void) dmfmctl(unit, DMF_OFF, DMSET);
		tp->t_state &= ~TS_CARR_ON; /* prevents recv intr. timeouts */

		if ((dmfsoftCAR[dmf] & (1<<(unit&07)))==0)  {
			/*drop DTR for at least a half sec. if modem line*/
			tp->t_state |= TS_CLOSING;

			/* wait for DSR to drop */
			addr = (struct dmfdevice *)tp->t_addr;
			addr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
			/*
			 * If the DSR signal is being followed, give the modem
			 * at most 5 seconds to drop.
			 */
			if (dmfdsr && (addr->dmfrms&DMF_DSR)) {
				timeout(wakeup, (caddr_t) &tp->t_dev, 5*hz);
				sleep((caddr_t)&tp->t_dev, PZERO-10);
			}
			timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
			sleep((caddr_t)&tp->t_dev, PZERO-10);
			tp->t_state &= ~(TS_CLOSING);
			wakeup((caddr_t)&tp->t_rawq);
		}
	}
	dmfsoftCAR[dmf] &= ~(1<<(unit&07));
	dmfsoftCAR[dmf] |= (1<<(unit&07)) & dmfdefaultCAR[dmf];
	ttyclose(tp);
	dmfmodem[unit] = 0;
}

dmfread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

	if(minor(dev)&0200) return(ENXIO);
	tp = &dmf_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

dmfwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;

	if(minor(dev)&0200)
		return(dmflwrite(dev,uio));
	tp = &dmf_tty[minor(dev)];
	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * DMF32 receiver interrupt.
 */
dmfrint(dmf)
	int dmf;
{
	register struct tty *tp;
	register c;
	register struct dmfdevice *addr;
	register struct tty *tp0;
	register struct uba_device *ui;
	register int line;
	int overrun = 0, s;
	register u_char *modem0, *modem;
	int modem_cont;

	ui = dmfinfo[dmf];
	if (ui == 0 || ui->ui_alive == 0)
		return;
	addr = (struct dmfdevice *)ui->ui_addr;
	tp0 = &dmf_tty[dmf<<3];
	modem0 = &dmfmodem[dmf<<3];
	/*
	 * Loop fetching characters from the silo for this
	 * dmf until there are no more in the silo.
	 */
	while ((c = addr->dmfrbuf) < 0) {
		line = (c>>8)&07;
		tp = tp0 + line;
		modem = modem0 + line;
#ifdef	DMFDEBUG
		if (dmfdebug>1)
			mprintf("dmfrint: tp=%x, c=%x\n", tp, c);
#endif
		/* check for modem transitions */
		if (c & DMF_DSC) {
#ifdef	DMFDEBUG
			if (dmfdebug)
				mprintf("dmfrint: DSC, tp=%x, rms=%x\n",
					tp, addr->dmfrms);
#endif
			if (dmfsoftCAR[dmf] & 1<<line)
				continue;
			addr->dmfcsr = DMF_IE | DMFIR_TBUF | line;

			modem_cont = 0;

			/*
			 * Drop DTR immediately if DSR gone away.
			 * If really an active close then do not
			 *    send signals.
			 */


			if ((addr->dmfrms&DMF_DSR)==0)	{
				 if (tp->t_state&TS_CLOSING) {
					untimeout(wakeup, (caddr_t) &tp->t_dev);
					wakeup((caddr_t) &tp->t_dev);
#ifdef DMFDEBUG
					if (dmfdebug)
					   mprintf("dmfrint: dsr closing down, tp=%x\n", tp);
#endif
					continue;
				 }

				 if (tp->t_state&TS_CARR_ON) {
					/*
 					 * Only drop line if DSR is being followed.
 					 */
					if (dmfdsr) {
						dmf_tty_drop(tp);
						continue;
					}
				 }
			}
			/*
			 * Check for transient CD drops.
			 * Only drop DTR if CD is down for more than 2 secs.
			 */

			if (tp->t_state&TS_CARR_ON)
			    if ((addr->dmfrms&DMF_CAR)==0) {
				if (*modem & MODEM_CD) {
				    /* only start timer once */
				    if (dmfdebug)
					mprintf("dmfrint, cd_drop, tp=%x\n", tp);
				    *modem &= ~MODEM_CD;
				    dmftimestamp[minor(tp->t_dev)] = time;
				    timeout(dmf_cd_drop, tp, hz*dmfcdtime);
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
					untimeout(dmf_cd_drop, tp);
					if (dmf_cd_down(tp))
						/* drop connection */
						dmf_tty_drop(tp);
					*modem |= MODEM_CD;
				        modem_cont = 1;
			       }

			/* CTS flow control check */

			if (tp->t_state&TS_CARR_ON)
				if ((addr->dmfrms&DMF_CTS)==0) {
					tp->t_state |= TS_TTSTOP;
					*modem &= ~MODEM_CTS;
					if (dmfdebug)
					   mprintf("dmfrint: CTS stop, tp=%x\n", tp);
					dmfstop(tp, 0);
					continue;
				} else if ((*modem&MODEM_CTS)==0) {
					    tp->t_state &= ~TS_TTSTOP;
					    *modem |= MODEM_CTS;
					    if (dmfdebug)
					       mprintf("dmfrint: CTS start, tp=%x\n", tp);
					    dmfstart(tp);
					    continue;
					}

			/*
			 * Avoid calling dmfstart due to a carrier transition.
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
			 * If the DSR signal is being followed, wait at most
			 * 30 seconds for CD, and don't transmit in the first 
			 * 500ms.  Otherwise immediately look for CD|CTS.
			 */
			if (dmfdsr) {
				if ((addr->dmfrms&DMF_XMIT)==DMF_XMIT
				    && (*modem&MODEM_DSR_START)==0)
					dmf_start_tty(tp);
				else
				    if ((addr->dmfrms&DMF_DSR) &&
					(*modem&MODEM_DSR)==0) {
					*modem |= (MODEM_DSR_START|MODEM_DSR);
					/*
					 * we should not look for CTS|CD for
					 * about 500 ms.
					 */
					timeout(dmf_dsr_check, tp, hz*30);
					timeout(dmf_dsr_check, tp, hz/2);
				    }
			}
			/*
			 * Ignore DSR 
			 */
			else
				if ((addr->dmfrms & DMF_NODSR) == DMF_NODSR)
					dmf_start_tty(tp);


			continue;
			} /* end of modem transition tests */

		if ((tp->t_state&TS_ISOPEN)==0) {
			wakeup((caddr_t)&tp->t_rawq);
			continue;
		}
		if (c & DMF_PE)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP )
				continue;
		if ((c & DMF_DO) && overrun == 0) {
			printf("dmf%d: silo overflow\n", dmf);
			overrun = 1;
		}
		if (c & DMF_FE)
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
				 * Strip extraneous sign extension bits.
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
 * Ioctl for DMF32.
 */
/*ARGSUSED*/
dmfioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register int unit = minor(dev);
	register struct dmfdevice *dmfaddr;
	register struct tty *tp;
	register int dmf;
	register int s;
	struct uba_device *ui;
	struct dmf_softc *sc;
	struct devget *devget;
	int error;

	if(unit & 0200)
		return (ENOTTY);
	tp = &dmf_tty[unit];
	dmf = unit >> 3;
	ui = dmfinfo[dmf];
	sc = &dmf_softc[ui->ui_unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			dmfparam(unit);
		return (error);
	}
	switch (cmd) {

	case TIOCSBRK:
		(void) dmfmctl(dev, DMF_BRK, DMBIS);
		break;

	case TIOCCBRK:
		(void) dmfmctl(dev, DMF_BRK, DMBIC);
		break;

	case TIOCSDTR:
		(void) dmfmctl(dev, DMF_DTR|DMF_RTS, DMBIS);
		break;

	case TIOCCDTR:
		(void) dmfmctl(dev, DMF_DTR|DMF_RTS, DMBIC);
		break;

	case TIOCMSET:
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMSET);
		break;

	case TIOCMBIS:
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMBIS);
		break;

	case TIOCMBIC:
		(void) dmfmctl(dev, dmtodmf(*(int *)data), DMBIC);
		break;

	case TIOCMGET:
		*(int *)data = dmftodm(dmfmctl(dev, 0, DMGET));
		break;
	case TIOCNMODEM:  /* ignore modem status */
		s = spl5();
		dmfsoftCAR[dmf] |= (1<<(unit&07));
		if (*(int *)data) /* make mode permanent */
			dmfdefaultCAR[dmf] |= (1<<(unit&07));
		dmfmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		tp->t_state |= TS_CARR_ON;
		splx(s);
		break;
	case TIOCMODEM:  /* look at modem status - sleep if no carrier */
		s = spl5();
		dmfsoftCAR[dmf] &= ~(1<<(unit&07));
		if (*(int *)data) /* make mode permanent */
			dmfdefaultCAR[dmf] &= ~(1<<(unit&07));
		dmfaddr = (struct dmfdevice *)(dmf_tty[unit].t_addr);
		dmfaddr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
	   	/* 
	    	 * If dmfdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((dmfdsr && ((dmfaddr->dmfrms & DMF_XMIT)==DMF_XMIT)) ||
		    ((dmfdsr == 0) && ((dmfaddr->dmfrms & DMF_NODSR)==DMF_NODSR))){
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dmfmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else {
			tp->t_state &= ~(TS_CARR_ON);
			dmfmodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
		}
		splx(s);
		break;
	case TIOCWONLINE:
		s = spl5();
#ifdef DMFDEBUG
		if (dmfdebug)
			mprintf("dmfioctl: WONLINE, tp=%x, state=%x\n",
				tp, tp->t_state);
#endif
		dmfaddr = (struct dmfdevice *)(dmf_tty[unit].t_addr);
		dmfaddr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
	   	/* 
	    	 * If dmfdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((dmfdsr && ((dmfaddr->dmfrms & DMF_XMIT)==DMF_XMIT)) ||
		    ((dmfdsr == 0) && ((dmfaddr->dmfrms & DMF_NODSR)==DMF_NODSR))){
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dmfmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else
			while ((tp->t_state & TS_CARR_ON) == 0)
				sleep((caddr_t)&tp->t_rawq, TTIPRI);
		splx(s);
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));

		if(dmfsoftCAR[dmf] & (1<<(unit&07))) {
			sc->sc_category_flags[unit] |= DEV_MODEM;
			sc->sc_category_flags[unit] &= ~DEV_MODEM_ON;
		}
		else
			sc->sc_category_flags[unit] |= (DEV_MODEM|DEV_MODEM_ON);

		devget->category = DEV_TERMINAL;
		devget->bus = DEV_UB;
		bcopy(DEV_DMF32,devget->interface,
		      strlen(DEV_DMF32));
		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));		/* terminal	*/
		devget->adpt_num = ui->ui_adpt; 	/* which adapter*/
		devget->nexus_num = ui->ui_nexus;	/* which nexus	*/
		devget->bus_num = ui->ui_ubanum;	/* which UBA	*/
		devget->ctlr_num = dmf; 		/* which interf.*/
		devget->slave_num = unit%8;		/* which line	*/
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,
		      strlen(ui->ui_driver->ud_dname)); /* Ultrix "dmf" */
		devget->unit_num = unit;		/* which dmf?	*/
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

dmtodmf(bits)
	register int bits;
{
	register int b;

	b = bits & 012;
	if (bits & DML_ST) b |= DMF_RATE;
	if (bits & DML_RTS) b |= DMF_RTS;
	if (bits & DML_USR) b |= DMF_USRW;
	return(b);
}

dmftodm(bits)
	register int bits;
{
	register int b;

	b = (bits & 012) | ((bits >> 7) & 0760) | DML_LE;
	if (bits & DMF_USRR) b |= DML_USR;
	if (bits & DMF_RTS) b |= DML_RTS;
	return(b);
}


/*
 * Set parameters from open or stty into the DMF hardware
 * registers.
 */
dmfparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct dmfdevice *addr;
	register int lpar, lcr;
	int s;

	tp = &dmf_tty[unit];
	addr = (struct dmfdevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spl5();
	addr->dmfcsr = (unit&07) | DMFIR_LCR | DMF_IE;
	if ((tp->t_ispeed)==0) {
		tp->t_state |= TS_HUPCLS;
		(void) dmfmctl(unit, DMF_OFF, DMSET);
		splx(s);
		return;
	}
	lpar = (dmf_speeds[tp->t_ospeed]<<12) | (dmf_speeds[tp->t_ispeed]<<8);
	lcr = DMFLCR_ENA;
	if ((tp->t_ispeed) == B134)
		lpar |= BITS6|PENABLE;
	else if (tp->t_flags & (RAW|LITOUT))
		lpar |= BITS8;
	else {
		lpar |= BITS7|PENABLE;
		/* CHECK FOR XON/XOFF AND SET lcr |= DMF_AUTOX; */
	}
	if (tp->t_flags&EVENP)
		lpar |= EPAR;
	if ((tp->t_ospeed) == B110)
		lpar |= TWOSB;
	lpar |= (unit&07);
	addr->dmflpr = lpar;
	SETLCR(addr, lcr);
#ifdef DMFDEBUG
	if (dmfdebug)
		mprintf("dmfparam: tp=%x, lpr=%o, lcr=%o\n",
			tp, addr->dmflpr, addr->dmfun.dmfirw);
#endif
	splx(s);
}

/*
 * DMF32 transmitter interrupt.
 * Restart the idle line.
 */


dmfxint(dmf)
	int dmf;
{
	int u = dmf * 8;
	struct tty *tp0 = &dmf_tty[u];
	register struct tty *tp;
	register struct dmfdevice *addr;
	register struct uba_device *ui;
	register int t;
	short cntr;

	ui = dmfinfo[dmf];
	addr = (struct dmfdevice *)ui->ui_addr;
	while ((t = addr->dmfcsr) & DMF_TI) {
		if (t & DMF_NXM)
			/* SHOULD RESTART OR SOMETHING... */
			printf("dmf%d: NXM line %d\n", dmf, t >> 8 & 7);
		t = t >> 8 & 7;
		tp = tp0 + t;
		tp->t_state &= ~TS_BUSY;
		if (tp->t_state&TS_FLUSH)
			tp->t_state &= ~TS_FLUSH;
		else if (dmf_dma[u + t]) {
			/*
			 * Do arithmetic in a short to make up
			 * for lost 16&17 bits.
			 */
			addr->dmfcsr = DMFIR_TBA | DMF_IE | t;
			cntr = addr->dmftba -
			    UBACVT(tp->t_outq.c_cf, ui->ui_ubanum);
			ndflush(&tp->t_outq, (int)cntr);
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			dmfstart(tp);
	}
}




/*
 * Start (restart) transmission on the given DMF32 line.
 */

dmfstart(tp)
	register struct tty *tp;
{
	register struct dmfdevice *addr;
	register int unit, nch, line;
	int s;
	register int dmf;

	unit = minor(tp->t_dev);
	dmf = unit >> 3;
	line = unit & 07;
	addr = (struct dmfdevice *)tp->t_addr;

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
		 ((tp->t_state & TS_CARR_ON) && (dmfmodem[unit]&MODEM_CTS)==0))
		goto out;
	/*
	 * If there are still characters in the silo,
	 * just reenable the transmitter.
	 */
	addr->dmfcsr = DMF_IE | DMFIR_TBUF | line;
	if (addr->dmftsc) {
		addr->dmfcsr = DMF_IE | DMFIR_LCR | line;
		SETLCR(addr, addr->dmflcr|DMF_TE);
		tp->t_state |= TS_BUSY;
		goto out;
	}
	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_wsel) {
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
		nch = ndqb(&tp->t_outq, 0);
	else {
		if ((nch = ndqb(&tp->t_outq, 0200)) == 0) {
			/*
			* If first thing on queue is a delay process it.
			*/
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch >= dmf_mindma) {
		register car;

		dmf_dma[minor(tp->t_dev)] = 1;
		addr->dmfcsr = DMF_IE | DMFIR_LCR | line;
		SETLCR(addr, addr->dmflcr|DMF_TE);
		car = UBACVT(tp->t_outq.c_cf, dmfinfo[dmf]->ui_ubanum);
		addr->dmfcsr = DMF_IE | DMFIR_TBA | line;
		addr->dmftba = car;
		addr->dmftcc = ((car >> 2) & 0xc000) | nch;
		tp->t_state |= TS_BUSY;
	} else if (nch) {
		register char *cp = tp->t_outq.c_cf;
		register int i;

		dmf_dma[minor(tp->t_dev)] = 0;
		nch = MIN(nch, DMF_SILOCNT);
		addr->dmfcsr = DMF_IE | DMFIR_LCR | line;
		SETLCR(addr, addr->dmflcr|DMF_TE);
		addr->dmfcsr = DMF_IE | DMFIR_TBUF | line;
		for (i = 0; i < nch; i++)
			addr->dmftbuf = *cp++;
		ndflush(&tp->t_outq, nch);
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}




/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
/*ARGSUSED*/
dmfstop(tp, flag)
	register struct tty *tp;
{
	register struct dmfdevice *addr;
	register int unit, s;

	addr = (struct dmfdevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output
		 * by selecting the line and disabling
		 * the transmitter.  If this is a flush
		 * request then flush the output silo,
		 * otherwise we will pick up where we
		 * left off by enabling the transmitter.
		 */
		unit = minor(tp->t_dev);
		addr->dmfcsr = DMFIR_LCR | (unit&07) | DMF_IE;
		SETLCR(addr, addr->dmflcr &~ DMF_TE);
		if ((tp->t_state&TS_TTSTOP)==0) {
			tp->t_state |= TS_FLUSH;
			SETLCR(addr, addr->dmflcr|DMF_FLUSH);
		} else
			tp->t_state &= ~TS_BUSY;
	}
	splx(s);
}

/*
 * DMF32 modem control
 */
dmfmctl(dev, bits, how)
	dev_t dev;
	int bits, how;
{
	register struct dmfdevice *dmfaddr;
	register int unit, mbits, lcr;
	int s;

	unit = minor(dev);
	dmfaddr = (struct dmfdevice *)(dmf_tty[unit].t_addr);
	unit &= 07;
	s = spl5();
	dmfaddr->dmfcsr = DMF_IE | DMFIR_TBUF | unit;
	mbits = dmfaddr->dmfrms << 8;
	dmfaddr->dmfcsr = DMF_IE | DMFIR_LCR | unit;
	mbits |= dmfaddr->dmftms;
	lcr = dmfaddr->dmflcr;
	switch (how) {
	case DMSET:
		mbits = (mbits &0xff00) | bits;
		break;

	case DMBIS:
		mbits |= bits;
		break;

	case DMBIC:
		mbits &= ~bits;
		break;

	case DMGET:
		(void) splx(s);
		return(mbits);
	}
	if (mbits & DMF_BRK)
		lcr |= DMF_RBRK;
	else
		lcr &= ~DMF_RBRK;
	lcr = ((mbits & 037) << 8) | (lcr & 0xff);
	dmfaddr->dmfun.dmfirw = lcr;
	(void) splx(s);
	return(mbits);
}

/*
 * Reset state of driver if UBA reset was necessary.
 * Reset the csr, lpr, and lcr registers on open lines, and
 * restart transmitters.
 */
dmfreset(uban)
	int uban;
{
	register int dmf, unit;
	register struct tty *tp;
	register struct uba_device *ui;
	register struct dmfdevice *addr;
	int i;

	if (tty_ubinfo[uban] == 0)
		return;
	cbase[uban] = tty_ubinfo[uban]&0x3ffff;
	for (dmf = 0; dmf < nNDMF; dmf++) {
		ui = dmfinfo[dmf];
		if (ui == 0 || ui->ui_alive == 0 || ui->ui_ubanum != uban)
			continue;
		printf(" dmf%d", dmf);
		addr = (struct dmfdevice *)ui->ui_addr;
		addr->dmfccsr0 = dmf_vector[dmf];
		addr->dmfcsr = DMF_IE;
		addr->dmfrsp = dmf_timeout;
		unit = dmf * 8;
		for (i = 0; i < 8; i++) {
			tp = &dmf_tty[unit];
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
				dmfparam(unit);
				(void) dmfmctl(unit, DMF_ON, DMSET);
				tp->t_state &= ~TS_BUSY;
				dmfstart(tp);
			}
			unit++;
		}
	}
}

/* dmflopen -- open the line printer port on a dmf32
 *
 */
dmflopen(dev,flag)
dev_t dev;
int flag;
{
	register int dmf;
	register struct dmfl_softc *sc;
	register struct uba_device *ui;
	register struct dmfdevice *addr;


	dmf = minor(dev) & 07 ;
	if(((sc= &dmfl_softc[dmf])->dmfl_state & OPEN) ||
		((ui=dmfinfo[dmf]) == 0) || ui->ui_alive == 0)
			return(ENXIO);
	addr = (struct dmfdevice *)ui->ui_addr;
	if((addr->dmfl[0] & DMFL_OFFLINE))
	{
		/*printf("dmf%d: line printer offline/jammed\n", dmf);*/
		return(EIO);
	}
	if((addr->dmfl[0]&DMFL_CONV))
	{
		printf("dmf%d: line printer disconnected\n", dmf);
		return(EIO);
	}

	addr->dmfl[0] = 0;
	sc->dmfl_state |= OPEN;
	return 0;
}

dmflclose(dev,flag)
dev_t dev;
int flag;
{
	register int dmf= minor(dev) & 07;
	register struct dmfl_softc *sc = &dmfl_softc[dmf];

	dmflout(dev,"\f",1);
	sc->dmfl_state = 0;
	if(sc->dmfl_info != 0)
		ubarelse((struct dmfdevice *)(dmfinfo[dmf])->ui_ubanum,
			&(sc->dmfl_info));

	((struct dmfdevice *)(dmfinfo[dmf]->ui_addr))->dmfl[0]=0;
	return 0;
}

dmflwrite(dev,uio)
dev_t dev;
struct uio *uio;
{
	register unsigned int n;
	register int error;
	register struct dmfl_softc *sc;

	sc = &dmfl_softc[minor(dev)&07];
	if(sc->dmfl_state&ERROR) return(EIO);
	while(n=min(DMFL_BUFSIZ,(unsigned)uio->uio_resid))
	{
		if(error=uiomove(&sc->dmfl_buf[0],(int)n,
			UIO_WRITE,uio))
		{
			printf("uio move error\n");
			return(error);
		}
		if(error=dmflout(dev,&sc->dmfl_buf[0],n))
		{
			return(error);
		}
	}
	return 0;
}


/* dmflout -- start io operation to dmf line printer
 *		cp is addr of buf of n chars to be sent.
 *
 *	-- dmf will be put in formatted output mode, this will
 *		be selectable from an ioctl if the
 *		need ever arises.
 */
dmflout(dev,cp,n)
dev_t dev;
char *cp;
int n;
{
	register struct dmfl_softc *sc;
	register int dmf;
	register struct uba_device *ui;
	register struct dmfdevice *d;
	register unsigned info;
	register unsigned i;

	dmf = minor(dev) & 07 ;
	sc= &dmfl_softc[dmf];
	if(sc->dmfl_state&ERROR) return(EIO);
	ui= dmfinfo[dmf];
	/* allocate unibus resources, will be released when io
	 * operation is done
	 */
	sc->dmfl_info=
	info=
		uballoc(ui->ui_ubanum,cp,n,0);
	d= (struct dmfdevice *)ui->ui_addr;
	d->dmfl[0] = (2<<8) | DMFL_FORMAT; /* indir reg 2 */
	/* indir reg auto increments on r/w */
	/* SO DON'T CHANGE THE ORDER OF THIS CODE */
	d->dmfl[1] = 0; /* prefix chars & num */
	d->dmfl[1] = 0; /* suffix chars & num */
	d->dmfl[1] = info;	/* dma lo 16 bits addr */

	/* NOT DOCUMENTED !! */
	d->dmfl[1] = -n;		/* number of chars */
	/* ----------^-------- */

	d->dmfl[1] = ((info>>16)&3) /* dma hi 2 bits addr */
		| (1<<8) /* auto cr insert */
		| (1<<9) /* use real ff */
		| (1<<15); /* no u/l conversion */
	d->dmfl[1] = sc->dmfl_lines	/* lines per page */
		| (sc->dmfl_cols<<8);	/* carriage width */
	sc->dmfl_state |= ASLP;
	i=spl5();
	d->dmfl[0] |= DMFL_PEN|DMFL_IE;
	while(sc->dmfl_state & ASLP)
	{
		sleep(&sc->dmfl_buf[0],(PZERO+8));
		while(sc->dmfl_state&ERROR)
		{
			timeout(dmflint,dmf,10*hz);
			sleep(&sc->dmfl_state,(PZERO+8));
		}
		/*if(sc->dmfl_state&ERROR) return (EIO);*/
	}
	splx(i);
	return(0);
}
/* dmflint -- handle an interrupt from the line printer part of the dmf32
 *
 */

dmflint(dmf)
int dmf;
{

	register struct uba_device *ui;
	register struct dmfl_softc *sc;
	register struct dmfdevice *d;

	ui= dmfinfo[dmf];
	sc= &dmfl_softc[dmf];
	d= (struct dmfdevice *)ui->ui_addr;

	d->dmfl[0] &= ~DMFL_IE;

	if(sc->dmfl_state&ERROR)
	{
		/*printf("dmf%d: intr while in error state \n", dmf);*/
		if((d->dmfl[0]&DMFL_OFFLINE) == 0)
			sc->dmfl_state &= ~ERROR;
		wakeup(&sc->dmfl_state);
		return;
	}
	if(d->dmfl[0]&DMFL_DMAERR)
	{
		printf("dmf%d:NXM\n", dmf);
	}
	if(d->dmfl[0]&DMFL_OFFLINE)
	{
		/*printf("dmf%d:printer error\n", dmf);*/
		sc->dmfl_state |= ERROR;
	}
	if(d->dmfl[0]&DMFL_PDONE)
	{
#ifdef notdef
		printf("bytes= %d\n",d->dmfl[1]);
		printf("lines= %d\n",d->dmfl[1]);
#endif
	}
	sc->dmfl_state &= ~ASLP;
	wakeup(&sc->dmfl_buf[0]);
	if(sc->dmfl_info != 0)
		ubarelse(ui->ui_ubanum,&sc->dmfl_info);
	sc->dmfl_info = 0;

}

/* stubs for interrupt routines for devices not yet supported */

dmfsrint() { printf("dmfsrint\n"); }

dmfsxint() { printf("dmfsxint\n"); }

dmfdaint() { printf("dmfdaint\n"); }

dmfdbint() { printf("dmfdbint\n"); }

dmf_cd_drop(tp)
register struct tty *tp;
{
	register struct dmfdevice *addr = (struct dmfdevice *)tp->t_addr;
	register unit = minor(tp->t_dev);

	addr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
	if ((tp->t_state&TS_CARR_ON) &&
		((addr->dmfrms&DMF_CAR) == 0)) {
#ifdef DMFDEBUG
		if (dmfdebug)
		    mprintf("dmf_cd:  no CD, tp=%x\n", tp);
#endif
		dmf_tty_drop(tp);
		return;
	}
	dmfmodem[unit] |= MODEM_CD;
#ifdef DMFDEBUG
	if (dmfdebug)
	    mprintf("dmf_cd:  CD is up, tp=%x\n", tp);
#endif
}


dmf_dsr_check(tp)
register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	register struct dmfdevice *addr = (struct dmfdevice *)tp->t_addr;
	if (dmfmodem[unit]&MODEM_DSR_START) {
		dmfmodem[unit] &= ~MODEM_DSR_START;
		addr->dmfcsr = DMF_IE | DMFIR_TBUF | (unit&07);
	   	/* 
	    	 * If dmfdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if (dmfdsr) {
			if ((addr->dmfrms&DMF_XMIT)==DMF_XMIT)
				dmf_start_tty(tp);
		}
		else {
			if ((addr->dmfrms&DMF_NODSR)==DMF_NODSR)
				dmf_start_tty(tp);
		}
		return;
	}
	if ((tp->t_state&TS_CARR_ON)==0)  {
		dmf_tty_drop(tp);
		if (dmfdebug)
		    mprintf("dmf_dsr:  no carrier, tp=%x\n", tp);
	}
#ifdef DMFDEBUG
	else
		if (dmfdebug)
		    mprintf("dmf_dsr:  carrier is up, tp=%x\n", tp);
#endif
}

/*
 *  cd_down return 1 if carrier has been down for at least 2 secs.
 */
dmf_cd_down(tp)
struct tty *tp;
{
	int msecs;
	register int unit = minor(tp->t_dev);

	msecs = 1000000 * (time.tv_sec - dmftimestamp[unit].tv_sec) +
		(time.tv_usec - dmftimestamp[unit].tv_usec);
	if (msecs > 2000000)
		return(1);
	else
		return(0);
}

dmf_tty_drop(tp)
struct tty *tp;
{
	register struct dmfdevice *addr = (struct dmfdevice *)tp->t_addr;
	register int line;
	register int unit;
	if (tp->t_flags&NOHANG)
		return;
	unit = minor(tp->t_dev);
	line = unit & 07;
	dmfmodem[unit] = MODEM_BADCALL;
	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP);
	wakeup((caddr_t)&tp->t_rawq);
	gsignal(tp->t_pgrp, SIGHUP);
	gsignal(tp->t_pgrp, SIGCONT);
	addr->dmfcsr=DMF_IE | DMFIR_LCR | line;
	addr->dmftms = 0;
}

dmf_start_tty(tp)
	register struct tty *tp;
{
	register int unit = minor(tp->t_dev);
	tp->t_state &= ~TS_ONDELAY;
	tp->t_state |= TS_CARR_ON;
	if (dmfmodem[unit]&MODEM_DSR)
		untimeout(dmf_dsr_check, tp);
	dmfmodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	dmftimestamp[unit] = dmfzerotime;
	wakeup((caddr_t)&tp->t_rawq);
}
#endif
