
#ifndef lint
static char *sccsid = "@(#)ss.c	1.13	ULTRIX	2/12/87";
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

#include "ss.h"
#if NSS > 0 || defined(BINARY)
/*
 *  VAXstar serial line unit driver
 *
 *  Modification History:
 *
 * 11-Feb-87 - rafiey (Ali Rafieymehr)
 *	Changed the driver to call read/write routines of the color driver
 *	for reads/writes.
 *
 * 28-Jan-87 - Tim Burke
 *
 *	Added the capability to ignore the "DSR" modem signal.  This is being
 *	done to allow modems that do not follow DEC Standard 52 to still 
 *	function as they would have prior to the addition of DECSTD52 code
 *	into the drivers.  If the driver is setup to ignore "DSR" then it will
 *	not be following DECSTD52.  To follow DECSTD52 set ssdsr to "1", to
 *	ignore "DSR" set ssdsr to be "0";
 *
 *  07-Jan-87 - rafiey (Ali Rafieymehr)
 *	Corrected the value of the color option in ubareg.h, therefore
 *	uncommented the two cases in sscons_init which were previously
 *	commented out.
 *
 *  16-Dec-86 - fred (Fred Canter)
 *	Changes for removal of DEV_MODEV_OFF devioctl flag.
 *
 *  11-Dec-86 - fred (Fred Canter)
 *	Bug fix: devio soft error count not incremented correctly
 *		 on any line other than zero.
 *	Bug fix: Sign extension of intr char during framing error
 *		 caused false silo overflow error indication.
 *
 *   2-Dec-86 - fred (Fred Canter)
 *	Fix rlogin hanging if console is diagnostic terminal
 *	on the printer port (BCC08 cable).
 *	Change minor device to 3 in ssselect().
 *
 *   4-Nov-86 - fred (Fred Canter)
 *	Disable support for silo alarm mode on VAXstar.
 *	It causes input silo overrun errors when using tip.
 *
 *   4-Nov-86 - tim  (Tim Burke)
 *	Clear TS_TSTOP on close to prevent line hanging if
 *	in stop state.
 *
 *  18-Sept-86 - tim  (Tim Burke)
 *	Lower ipl level upon receipt of a badcall on modem line.  This is
 *	done to insure that future interrupts get serviced.
 *
 *  30-Aug-86 -- fred (Fred Canter)
 *	Merged in Tim Burke's final dec standard 52 modem support changes.
 *	Fixed console putc to work in physical mode (for crash dump).
 *	Change for dummy sgcons_init and smcons_init routines.
 *
 *  26-Aug-86 -- rsp (Ricky Palmer)
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 *  24-Aug-86  -- fred (Fred Canter) and rafiey (Ali Rafieymehr)
 *	Support for smscreen (console message window).
 *	Allow uses to change softCAR only on line 2 (modem control line).
 *	Finish DEVIOCTL support.
 *	Remove ssreset() and other general cleanup.
 *	Hardwire critical line parameters into the driver stty in
 *	users' .profile can't make the line inoperative.
 *
 *  14-Aug-86  -- fred (Fred Canter)
 *	general cleanup, Tim's dec standard 52 stuff,
 *	no silo mode if graphics device present,
 *	complete redesign of ssputc().
 *
 *   5-Aug-86  -- fred (Fred Canter)
 *	Major changes, bug fixes, and passing characters to the
 *	bitmap graphics driver (sm.c).
 *
 *  24-Jul-86  -- tim  (Tim Burke)
 *	Added full DEC Standard 52 support.  Most changes occured in the 
 *	ssscan , ssopen, and ssclose routines to track modem signal status.
 *      Added the following modem control routines:
 *      ss_cd_drop, ss_dsr_check, ss_cd_down, ss_tty_drop, ss_start_tty.
 *
 *   2-Jul-86  -- fred (Fred Canter)
 *	Removed DZ32 code, changed from 8 to 4 lines per unit,
 *	moved ssconsinit(), ssputc(), & ssgetc() from cons.c to ss.c,
 *	other improvements.
 *
 *  18-Jun-86  -- fred (Fred Canter)
 *	Created this file (derived from dz.c).
 */

#include "../data/ss_data.c"

int ssdebug = 0;

/*
* Driver information for auto-configuration stuff.
*/
int	ssprobe(), ssattach(), ssrint();
int	ss_cd_drop(), ss_dsr_check(), ss_cd_down(), ss_tty_drop(); /* Modem */
u_short	ssstd[] = { 0 };
struct	uba_driver ssdriver =
{ ssprobe, 0, ssattach, 0, ssstd, "ss", ssinfo };


int	ssstart(), ssxint(), ssdma();
int	ttrstrt();
int	ssact;

/*
 * Graphics device driver entry points.
 * Used to call graphics device driver as needed.
 */
extern	(*vs_gdopen)();
extern	(*vs_gdclose)();
extern	(*vs_gdread)();
extern	(*vs_gdwrite)();
extern	(*vs_gdselect)();
extern	(*vs_gdkint)();
extern	(*vs_gdioctl)();
extern	(*vs_gdstop)();

#define	FASTTIMER	(hz/30)		/* rate to drain silos, when in use */
#define MODEM_UNIT	2		/* Modem control only on unit 2     */

int	sssilos;			/* mask of SLU's with silo in use */
int	sstimerintvl;			/* time interval for sstimer */
int	sshighrate = 100;		/* silo on if sschars > sshighrate */
int	sslowrate = 75;			/* silo off if ssrate < sslowrate */

/*
* The SLU doesn't interrupt on carrier transitions, so
* we have to use a timer to watch it.
*/
char	ss_timer;		/* timer started? */

char	ss_speeds[] =
{ 0,020,021,022,023,024,0,025,026,027,030,032,034,036,037,0 };

u_char sscan_previous;		/* Used to detect modem transitions */

#ifndef PORTSELECTOR
#define	ISPEED	B300
#define	IFLAGS	(EVENP|ODDP|ECHO)
#else
#define	ISPEED	B4800
#define	IFLAGS	(EVENP|ODDP)
#endif

extern	struct	nexus	nexus[];
struct	tty		sm_tty;

ssprobe(reg)
	caddr_t reg;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	int	 i;

	/*
	 * ONLY on a VAXstation 2000 or MicroVAX 2000
	 */
	if((cpu != MVAX_II) || (cpu_subtype != ST_VAXSTAR))
		return(0);
#ifdef lint
	ssrint(0); ssxint((struct tty *)0);
#endif
	ssaddr->nb_int_msk |= SINT_ST;
	ssaddr->sstcr = 0x8;		/*  enable line 3 */
	DELAY(100000);
	ssaddr->sstcr = 0;
	ssaddr->nb_int_msk &= ~SINT_ST;
	ssaddr->nb_int_reqclr = SINT_ST;
	if (cvec && cvec != 0x200)
		cvec -= 4;
	return (1);	/* 1 not sizeof anything, just says probe succeeded */
}

ssattach(ui)
	register struct uba_device *ui;
{
	register struct pdma *pdp = &sspdma[ui->ui_unit*4];
	register struct tty *tp = &ss_tty[ui->ui_unit*4];
	register int cntr;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	extern ssscan();

	for (cntr = 0; cntr < 4; cntr++) {
		/* dzdevice looks wrong, but see vaxuba/pdma.h for reason */
		pdp->p_addr = (struct dzdevice *)&ssaddr->sscsr;
		pdp->p_arg = (int)tp;
		pdp->p_fcn = ssxint;
		pdp++, tp++;
	}
	sssoftCAR[ui->ui_unit] = ui->ui_flags;
	ssdefaultCAR[ui->ui_unit] = ui->ui_flags;
	ssmodem = 0; 
	if (ss_timer == 0) {
		ss_timer++;
		timeout(ssscan, (caddr_t)0, hz);
		sstimerintvl = FASTTIMER;
	}
}

/*ARGSUSED*/
ssopen(dev, flag)
	dev_t dev;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register struct tty *tp;
	register int unit;
	int inuse;  /*hold state of inuse bit while blocked waiting for carr*/

	/*
	 * If a diagnostic console is attached to SLU line 3,
	 * don't allow open of the printer port (also line 3).
	 * This could cause lpr to write to the console.
	 */
	if((vs_cfgtst&VS_L3CON) && (major(dev) == SSMAJOR)) {
		if((minor(dev)&03) == 3)
			return (ENXIO);
	}
	unit = minor(dev);
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&03) == 0))
		unit |= 3;	/* diag console on SLU line 3 */
	if (unit >= ss_cnt || sspdma[unit].p_addr == 0)
		return (ENXIO);
	/*
	 * Call the graphics device open routine
	 * if there is one and the open if for the fancy tube.
	 */
	if ((vs_gdopen && (unit <= 1)) || (vs_gdopen && (unit == 2) &&
	    (major(dev) == CONSOLEMAJOR)))
		return((*vs_gdopen)(dev, flag));
	tp = &ss_tty[unit];
	while (tp->t_state&TS_CLOSING) { /* let DTR stay down for awhile */
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	tp->t_addr = (caddr_t)&sspdma[unit];
	tp->t_oproc = ssstart;
	if ((tp->t_state & TS_ISOPEN) == 0) {
		ttychars(tp);
		/*
		 * Prevent spurious startups by making the 500ms timer
 		 * initially high.
		 */
		if (unit == MODEM_UNIT)
			ssmodem = MODEM_DSR_START;
#ifndef PORTSELECTOR
		if (tp->t_ispeed == 0) {
#endif
			tp->t_dev = dev;	/* needed by timeouts */
			if((major(dev)==CONSOLEMAJOR) && ((minor(dev)&3) == 0)) {
			    tp->t_ospeed = tp->t_ispeed = B9600;
			    tp->t_flags = ANYP|ECHO|CRMOD;
			} else {
			    tp->t_ospeed = tp->t_ispeed = ISPEED;
			    tp->t_flags = IFLAGS;
			}
#ifndef PORTSELECTOR
		}
#endif
		/* tp->t_state |= TS_HUPCLS; */
		ssparam(unit);		/* enables interrupts */
	} else if (tp->t_state&TS_XCLUDE && u.u_uid != 0){
		return (EBUSY);
	}
	(void) spl5();

	/*
	 * No modem control provided for lines with softCAR set.
	 * Modem control provided only for line 2.
	 */
#	ifdef DEBUG
	if (ssdebug)
		cprintf("ssopen: UNIT = %x\n",unit);
#	endif DEBUG
	if ((unit != MODEM_UNIT) || (sssoftCAR[unit>>2] & (1<<(unit&03))) ) {
		/*
		 * This is a local connection - ignore carrier 
		 * receive enable interrupts enabled above via ssparam() 
		 */
		tp->t_state |= TS_CARR_ON;		/* ssscan sets */
		if (unit == MODEM_UNIT)
			ssaddr->ssdtr |= (SS_RDTR|SS_RRTS);
		(void) spl0();
		return ((*linesw[tp->t_line].l_open)(dev, tp));
	}
	/* 
	 *  this is a modem line 
	 */
	/* receive enable interrupts enabled above via ssparam() */
	ssaddr->ssdtr |= (SS_RDTR|SS_RRTS);

	/*
	 * After DSR first comes up we must wait for the other signals
	 * before commencing transmission.
         */
	if ((flag&O_NDELAY)==0) {
		/*
		 * Delay before examining other signals if DSR is being followed
		 * otherwise proceed directly to ss_dsr_check to look for 
		 * carrier detect and clear to send.
		 */
		if (ssdsr) {
			 if ((ssaddr->ssmsr)&SS_RDSR) {
				ssmodem |= (MODEM_DSR_START|MODEM_DSR);
				tp->t_dev = dev; /* need it for timeouts */
				/* 
		 		* Give Carrier and CTS 30 sec. to come up.  
		 		* Prevent any transmission in the first 500ms.
		 		*/
				timeout(ss_dsr_check, tp, hz*30);  
				timeout(ss_dsr_check, tp, hz/2);
			}
		}
		/* 
	 	 * Ignoring DSR so immediately check for CD & CTS.
		 */
		else {
				ssmodem |= (MODEM_DSR_START|MODEM_DSR);
				ss_dsr_check(tp);
		}
	}
#	ifdef DEBUG
	if (ssdebug)
		cprintf("ssopen:  line=%d, state=%x, tp=%x\n", unit,
			tp->t_state, tp);
#	endif DEBUG
	if (flag & O_NDELAY) 
		tp->t_state |= TS_ONDELAY;
	else
		while ((tp->t_state & TS_CARR_ON) == 0) {
			tp->t_state |= TS_WOPEN;
			inuse = tp->t_state&TS_INUSE;
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
			/*
			 * See if we were awoken by a false call to the modem
			 * line by a non-modem.
 			 */
			if (ssmodem&MODEM_BADCALL){
				(void) spl0();
				return(EWOULDBLOCK);
			}
			/* if we opened "block if in use"  and
			 *  the terminal was not inuse at that time
			 *  but is became "in use" while we were
			 *  waiting for carrier then return
			 */
			if ((flag & O_BLKINUSE) && (inuse==0) &&
				(tp->t_state&TS_INUSE)) {
					(void) spl0();
					return(EALREADY);
			}
		}
	(void) spl0();
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*ARGSUSED*/
ssclose(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	int ss;
	extern int wakeup();

	unit = minor(dev);
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&03) == 0))
		unit |= 3;	/* diag console on SLU line 3 */
	/*
	 * Call the craphics device close routine
	 * if ther is one and the close is for it.
	 */
	if ((vs_gdclose && (unit <= 1)) || (vs_gdclose && (unit == 2) &&
	    (major(dev) == CONSOLEMAJOR))){
		(*vs_gdclose)(dev, flag);
		return;
	}
	ss = unit >> 2;
#	ifdef DEBUG
	if (ssdebug)
		cprintf("ssclose: unit=%x, ss=%x\n",unit,ss);
#	endif DEBUG
	tp = &ss_tty[unit];
	/*
	 * Do line discipline specific close functions then return here
	 * in the old line disc for final closing.
	 */
	if (tp->t_line)
		(*linesw[tp->t_line].l_close)(tp);
	/*
	 * ssbrk is write-only and sends a BREAK (SPACE condition) until
         * the break control bit is cleared. Here we are clearing any
 	 * breaks for this line on close.
	 */
	ssaddr->ssbrk = (ss_brk[ss] &= ~(1 << (unit&03)));
	if ((tp->t_state&(TS_HUPCLS|TS_WOPEN)) || (tp->t_state&TS_ISOPEN)==0) {
		/*
		 * Drop appropriate signals to terminate the connection.
		 */
		ssaddr->ssdtr &= ~(SS_RDTR|SS_RRTS);
		tp->t_state &= ~TS_CARR_ON;   /* prevents recv intr. timeouts */
		if ((unit == MODEM_UNIT) && ((sssoftCAR[unit>>2] & (1<<(unit&03)))==0)){
			/*drop DTR for at least a sec. if modem line*/
#			ifdef DEBUG
			if (ssdebug)
				cprintf("ssclose: DTR drop, state =%x\n",
					tp->t_state);
#			endif DEBUG
			tp->t_state |= TS_CLOSING;
			/*
			 * Wait at most 5 sec for DSR to go off.  Also hold
			 * DTR down for a period.
			 */
			if (ssdsr && (ssaddr->ssmsr & SS_RDSR)) {
				timeout(wakeup, (caddr_t) &tp->t_dev, 5*hz);
				sleep((caddr_t)&tp->t_dev, PZERO-10);
			}
			/*
			 * Hold DTR down for 200+ ms.
			 */
			timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
			sleep((caddr_t)&tp->t_dev, PZERO-10);

			tp->t_state &= ~(TS_CLOSING);
			wakeup((caddr_t)&tp->t_rawq);
		}
		/*
		 * No disabling of interrupts is done.  Characters read in on
		 * a non-open line will be discarded.
		 */
	}
	/* reset line to default mode */
	sssoftCAR[ss] &= ~(1<<(unit&03));
	sssoftCAR[ss] |= (1<<(unit&03)) & ssdefaultCAR[ss];
	if (unit == MODEM_UNIT)
		ssmodem = 0;
	ttyclose(tp);
}

/*
 * ssread() shared with graphics device drivers (sm & sg).
 */

ssread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	register int unit;

	unit = minor(dev);
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&03) == 0))
		unit |= 3;	/* diag console on SLU line 3 */
	if((unit == 1) && vs_gdread)
	    return((*vs_gdread)(dev, uio));  /* color option */
	if (vs_gdopen && (unit == 2) && (major(dev) == CONSOLEMAJOR))
	    tp = &sm_tty;
	else
	    tp = &ss_tty[unit];
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

/*
 * sswrite() shared with graphics device drivers (sm & sg).
 */

sswrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	register int unit;

	unit = minor(dev);
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&03) == 0))
		unit |= 3;	/* diag console on SLU line 3 */
	if((unit == 1) && vs_gdwrite)	
	    return((*vs_gdwrite)(dev, uio)); /* color option */
	/*
	 * Don't allow writes to the mouse,
	 * just fake the I/O and return.
	 */
	if (vs_gdopen && (unit == 1)) {
		uio->uio_offset = uio->uio_resid;
		uio->uio_resid = 0;
		return(0);
	}
	if (vs_gdopen && (unit == 2) && (major(dev) == CONSOLEMAJOR))
	    tp = &sm_tty;
	else
	    tp = &ss_tty[unit];
	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

ssselect(dev, rw)
dev_t dev;
{
	register int unit = minor(dev);
	
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&3) == 0))
		dev |= 3;
	if((unit == 1) && vs_gdselect)
		return((*vs_gdselect)(dev, rw));
	else
		return(ttselect(dev, rw));
}

/*
 * Used to pass mouse (or tablet) reports to the graphics
 * device driver interrupt service routine.
 * Entire report passed instead of byte at a time.
 */
struct	mouse_report	current_rep;
u_short	sm_pointer_id;
#define	MOUSE_ID	0x2
 
/*ARGSUSED*/
ssrint(ss)
	int ss;
{
	register struct tty *tp;
	register int c;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register struct tty *tp0;
	register int unit;
	int overrun = 0;
	struct ss_softc *sc;
	struct mouse_report *new_rep;
	u_short data;

	if ((ssact & (1<<ss)) == 0)
		return;
	unit = ss * 4;
	tp0 = &ss_tty[unit];
	new_rep = &current_rep;			/* mouse report pointer */
	while (ssaddr->sscsr&SS_RDONE) {	/* character present */
		ssaddr->nb_int_reqclr = SINT_SR;
		c = ssaddr->ssrbuf;
		sschars[ss]++;
		unit = (c>>8)&03;
		tp = tp0 + unit;
		if (tp >= &ss_tty[ss_cnt])
			continue;
		sc = &ss_softc[ss];
		/*
		 * If console is a graphics device,
		 * pass keyboard input characters to
		 * its device driver's receive interrupt routine.
		 * Save up complete mouse report and pass it.
		 */
		if ((unit <= 1) && vs_gdkint) {
		    if(unit == 0) {		/* keyboard char */
			(*vs_gdkint)(c);
			continue;
		    } else {			/* mouse or tablet report */
			if (sm_pointer_id == MOUSE_ID) { /* mouse report */
			    data = c & 0xff;	/* get report byte */
			    ++new_rep->bytcnt;	/* inc report byte count */

			    if (data & START_FRAME) { /* 1st byte of report? */
				new_rep->state = data;
				if (new_rep->bytcnt > 1)
			            new_rep->bytcnt = 1;  /* start new frame */
			    }

			    else if (new_rep->bytcnt == 2)	/* 2nd byte */
			            new_rep->dx = data;

			    else if (new_rep->bytcnt == 3) {	/* 3rd byte */
				    new_rep->dy = data;
				    new_rep->bytcnt = 0;
				    (*vs_gdkint)(0400);	/* 400 says line 1 */
			    }
			    continue;
			} else { /* tablet report */
			    data = c;	/* get report byte */
			    ++new_rep->bytcnt;	/* inc report byte count */

			    if (data & START_FRAME) { /* 1st byte of report? */
				new_rep->state = data;
				if (new_rep->bytcnt > 1)
			            new_rep->bytcnt = 1;  /* start new frame */
			    }

			    else if (new_rep->bytcnt == 2)	/* 2nd byte */
			            new_rep->dx = data & 0x3f;

			    else if (new_rep->bytcnt == 3)  	/* 3rd byte */
				    new_rep->dx |= (data & 0x3f) << 6;

			    else if (new_rep->bytcnt == 4)  	/* 4th byte */
			            new_rep->dy = data & 0x3f;

			    else if (new_rep->bytcnt == 5){ 	/* 5th byte */
				    new_rep->dy |= (data & 0x3f) << 6;
				    new_rep->bytcnt = 0;
				    (*vs_gdkint)(0400);	/* 400 says line 1 */
			    }
			    continue;
			}
		    }
		}
		if ((tp->t_state & TS_ISOPEN) == 0) {
			wakeup((caddr_t)&tp->t_rawq);
#ifdef PORTSELECTOR
			if ((tp->t_state&TS_WOPEN) == 0)
#endif
			continue;
		}
		if (c&SS_FE) {
			if (tp->t_flags & RAW) {
				c = 0;
			} else {
				c = tp->t_intrc;	/* sign extended! */
				c &= 0377;
			}
		}
		if (c&SS_DO) {
			if(overrun == 0) {
				printf("ss%d: input silo overflow\n", ss);
				overrun = 1;
			}
			sc->sc_softcnt[unit]++;
		}
		if (c&SS_PE) {
			if (((tp->t_flags & (EVENP|ODDP)) == EVENP)
		  	  || ((tp->t_flags & (EVENP|ODDP)) == ODDP)) {
				sc->sc_softcnt[unit]++;
				continue;
			}
		}
#if NHC > 0
		if (tp->t_line == HCLDISC) {
			HCINPUT(c, tp);
		} else
#endif
		(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*ARGSUSED*/
ssioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register int unit;
	register struct tty *tp;
	register int ss;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int s;
	struct uba_device *ui;
	struct ss_softc *sc;
	struct devget *devget;
	int error;

	unit = minor(dev);
	if((vs_cfgtst&VS_L3CON) &&
	   (major(dev) == CONSOLEMAJOR) &&
	   ((unit&03) == 0))
		unit |= 3;	/* diag console on SLU line 3 */
	/*
	 * If there is a graphics device and the ioctl call
	 * is for it, pass the call to the graphics driver.
	 */
	if (vs_gdioctl && (unit <= 1)) {
		error = (*vs_gdioctl)(dev, cmd, data, flag);
		return(error);
	}
	tp = &ss_tty[unit];
	ss = unit >> 2;
	ui = ssinfo[ss];
	sc = &ss_softc[ui->ui_unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			ssparam(unit);
		return (error);
	}
	switch (cmd) {

	case TIOCSBRK:
		ssaddr->ssbrk = (ss_brk[ss] |= 1 << (unit&03));
		break;

	case TIOCCBRK:
		ssaddr->ssbrk = (ss_brk[ss] &= ~(1 << (unit&03)));
		break;

	case TIOCSDTR:
		(void) ssmctl(dev, SS_DTR|SS_RTS, DMBIS);
		break;

	case TIOCCDTR:
		(void) ssmctl(dev, SS_DTR|SS_RTS, DMBIC);
		break;

	case TIOCMSET:
		(void) ssmctl(dev, dmtoss(*(int *)data), DMSET);
		break;

	case TIOCMBIS:
		(void) ssmctl(dev, dmtoss(*(int *)data), DMBIS);
		break;

	case TIOCMBIC:
		(void) ssmctl(dev, dmtoss(*(int *)data), DMBIC);
		break;

	case TIOCMGET:
		*(int *)data = sstodm(ssmctl(dev, 0, DMGET));
		break;

	case TIOCNMODEM:  /* ignore modem status */
		/* 
		 * By setting the software representation of modem signals
		 * to "on" we fake the system into thinking that this is an
		 * established modem connection.
		 */
		s = spl5();
		sssoftCAR[ss] |= (1<<(unit&03));  
		if (*(int *)data) /* make mode permanent */
			ssdefaultCAR[ss] |= (1<<(unit&03));  
		tp->t_state |= TS_CARR_ON;
		splx(s);
		break;

	case TIOCMODEM:  /* look at modem status - sleep if no carrier */
		if(unit != MODEM_UNIT)
			break;		/* ONLY line 2 has modem control */
		s = spl5();
		sssoftCAR[ss] &= ~(1<<(unit&03));  
		if (*(int *)data) /* make mode permanent */
			ssdefaultCAR[ss] &= ~(1<<(unit&03));  
		/*
		 * See if all signals necessary for modem connection are present
		 * 
	    	 * If ssdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((ssdsr && ((ssaddr->ssmsr&SS_XMIT) == SS_XMIT)) ||
		   ((ssdsr==0) && ((ssaddr->ssmsr&SS_NODSR) == SS_NODSR))) {
			tp->t_state &= ~(TS_ONDELAY);
			tp->t_state |= TS_CARR_ON;
			ssmodem = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else {
			tp->t_state &= ~(TS_CARR_ON);
			ssmodem &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
		}
		splx(s);
		break;

	case TIOCWONLINE:
		if(unit != MODEM_UNIT)
			break;		/* ONLY line 2 has modem control */
		s = spl5();
		/*
		 * See if all signals necessary for modem connection are present
		 * 
	    	 * If ssdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if ((ssdsr && ((ssaddr->ssmsr&SS_XMIT) == SS_XMIT)) ||
		   ((ssdsr==0) && ((ssaddr->ssmsr&SS_NODSR) == SS_NODSR))) {
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~(TS_ONDELAY);
			ssmodem = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else
			while ((tp->t_state & TS_CARR_ON) == 0) 
				sleep((caddr_t)&tp->t_rawq, TTIPRI);
		splx(s);
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));
		if(unit == MODEM_UNIT) {
		    if(sssoftCAR[ss] & (1<<(unit&3))) {
		       sc->sc_category_flags[unit] |= DEV_MODEM;
		       sc->sc_category_flags[unit] &= ~DEV_MODEM_ON;
		    } else
		       sc->sc_category_flags[unit] |= (DEV_MODEM|DEV_MODEM_ON);
		}
		devget->category = DEV_TERMINAL;	/* terminal cat.*/
		devget->bus = DEV_NB;			/* NO bus	*/
		bcopy(DEV_VS_SLU,devget->interface,
		      strlen(DEV_VS_SLU));		/* interface	*/
		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));		/* terminal	*/
		devget->adpt_num = 0;			/* NO adapter	*/
		devget->nexus_num = 0;			/* fake nexus 0	*/
		devget->bus_num = 0;			/* NO bus	*/
		devget->ctlr_num = ss;			/* cntlr number */
		devget->slave_num = unit%4;		/* line number 	*/
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,
		      strlen(ui->ui_driver->ud_dname)); /* Ultrix "ss"	*/
		devget->unit_num = unit;		/* ss line?	*/
		devget->soft_count =
		      sc->sc_softcnt[unit];		/* soft err cnt	*/
		devget->hard_count =
		      sc->sc_hardcnt[unit];		/* hard err cnt	*/
		devget->stat = sc->sc_flags[unit];	/* status	*/
		devget->category_stat =
		      sc->sc_category_flags[unit];	/* cat. stat.	*/
		break;

	default:
		return (ENOTTY);
	}
	return (0);
}

dmtoss(bits)
	register int bits;
{
	register int b;

	b = (bits >>1) & 0370;
	if (bits & SML_ST) b |= SS_ST;
	if (bits & SML_RTS) b |= SS_RTS;
	if (bits & SML_DTR) b |= SS_DTR;
	if (bits & SML_LE) b |= SS_LE;
	return(b);
}

sstodm(bits)
	register int bits;
{
	register int b;

	b = (bits << 1) & 0360;
	if (bits & SS_DSR) b |= SML_DSR;
	if (bits & SS_DTR) b |= SML_DTR;
	if (bits & SS_ST) b |= SML_ST;
	if (bits & SS_RTS) b |= SML_RTS;
	return(b);
}

ssparam(unit)
	register int unit;
{
	register struct tty *tp;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int lpr;

	tp = &ss_tty[unit];
	if (sssilos & (1 << (unit >> 2)))
		ssaddr->sscsr = SS_MSE | SS_SAE;
	else
		ssaddr->sscsr = SS_MSE;
/*
 *	Reversing the order of the following two lines fixes the
 *	problem where the console device locks up if you type a
 *	character during autoconf and you must halt/continue to
 *	unlock the console. Interrupts were being enabled on the SLU
 *	before the ssact flag was set, so the driver would just return
 *	and not process the waiting character (caused by you typing).
 *	This locked up the cosnole SLU (interrupt posted but never really
 *	servcied). Halting the system caused the console firmware to unlock
 *	the SLU because it needs to use it.
 *	Should ssparam() be called as spl5??????
 */
	ssact |= (1<<(unit>>2));
	ssaddr->nb_int_msk |= (SINT_ST|SINT_SR);
	if (tp->t_ispeed == 0) {
		(void) ssmctl(unit, SS_OFF, DMSET);	/* hang up line */
		return;
	}
	lpr = (ss_speeds[tp->t_ispeed]<<8) | (unit & 03);
	if (tp->t_flags & (RAW|LITOUT))
		lpr |= BITS8;
	else
		lpr |= (BITS7|PENABLE);
	if ((tp->t_flags & EVENP) == 0)
		lpr |= OPAR;
	if (tp->t_ispeed == B110)
		lpr |= TWOSB;
	/*
	 * If diagnostic console on line 3,
	 * line parameters must be: 9600 BPS, 8 BIT, NO PARITY, 1 STOP.
	 * Same for color/monochrome video, except 4800 BPS.
	 * Mouse/tablet: 4800 BPS, 8 BIT, ODD PARITY, 1 STOP.
	 * If none of the above, assume attached console on line 0,
	 * same paramaters as diagnostic console on line 3.
	 */
	if ((unit == 3) && (vs_cfgtst&VS_L3CON))
		ssaddr->sslpr = (SS_RE | SS_B9600 | BITS8 | 3);
	else if (unit == 0) {
		if (vs_gdopen)
			ssaddr->sslpr = (SS_RE | SS_B4800 | BITS8);
		else
			ssaddr->sslpr = (SS_RE | SS_B9600 | BITS8);
	} else if (vs_gdopen && (unit == 1))
		ssaddr->sslpr = (SS_RE | SS_B4800 | OPAR | PENABLE | BITS8 | 1);
	else
		ssaddr->sslpr = lpr;
}

ssxint(tp)
	register struct tty *tp;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register struct pdma *dp;
	register ss, unit;

	dp = (struct pdma *)tp->t_addr;
	ss = minor(tp->t_dev) >> 2;
	unit = minor(tp->t_dev) & 3;
	if ((vs_cfgtst&VS_L3CON) && (unit == 0) &&
	    (major(tp->t_dev) == CONSOLEMAJOR)) {
		unit = 3;
		ss = 0;
	}

	/*
	 * Don't know if the following is true for the
	 * VAXstar SLU, but it stays in just to be safe.
	 * Fred Canter -- 6/28/86
	 */
	/* it appears that the ss will ocassionally give spurious
	   transmit ready interupts even when not enabled.  If the
	   line was never opened, the following is necessary */

	if (dp == NULL) {
		tp->t_addr = (caddr_t) &sspdma[unit];
		dp = (struct pdma *) tp->t_addr;
	}
	tp->t_state &= ~TS_BUSY;
	if (tp->t_state & TS_FLUSH)
		tp->t_state &= ~TS_FLUSH;
	else {
		ndflush(&tp->t_outq, dp->p_mem-tp->t_outq.c_cf);
		dp->p_end = dp->p_mem = tp->t_outq.c_cf;
	}
	if (tp->t_line)
		(*linesw[tp->t_line].l_start)(tp);
	else
		ssstart(tp);
	if (tp->t_outq.c_cc == 0 || (tp->t_state&TS_BUSY)==0)
		ssaddr->sstcr &= ~(1<<unit);
}

ssstart(tp)
	register struct tty *tp;
{
	register struct pdma *dp;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int cc;
	int s, ss, unit;

	dp = (struct pdma *)tp->t_addr;
	s = spl5();
	/*
	 * Do not do anything if currently delaying, or active.  Also only
	 * transmit when CTS is up.
	 */
	unit = minor(tp->t_dev) & 3;
	if ((tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) ||
		(unit == MODEM_UNIT) && (!(sssoftCAR[unit>>2] & (1<<(unit&03))))
		&& ((tp->t_state&TS_CARR_ON) && (ssmodem&MODEM_CTS)==0))
		goto out;
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
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
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (tp->t_flags & (RAW|LITOUT))
		cc = ndqb(&tp->t_outq, 0);
	else {
		cc = ndqb(&tp->t_outq, 0200);
		if (cc == 0) {
			cc = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	tp->t_state |= TS_BUSY;
	dp->p_end = dp->p_mem = tp->t_outq.c_cf;
	dp->p_end += cc;
	if ((vs_cfgtst&VS_L3CON) && (unit == 0) && 
	    (major(tp->t_dev) == CONSOLEMAJOR))
		unit = 3;
	ssaddr->sstcr |= (1<<unit);
out:
	splx(s);
}

/*
 * Stop output on a line.
 */

/*ARGSUSED*/
ssstop(tp, flag)
	register struct tty *tp;
{
	register struct pdma *dp;
	register int s;
	int	unit;

	/*
	 * If there is a graphics device and the stop call
	 * is for it, pass the call to the graphics device driver.
	 */
	unit = minor(tp->t_dev);
	if (vs_gdstop && (unit <= 1)) {
		(*vs_gdstop)(tp, flag);
		return;
	}
	dp = (struct pdma *)tp->t_addr;
	s = spl5();
	if (tp->t_state & TS_BUSY) {
		dp->p_end = dp->p_mem;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
	}
	splx(s);
}

/*
 *	***** FOR YOUR INFORMATION *****
 *
 *	     Fred Canter -- 7/1/86
 *
 * 	The VAXstar console SLU supports modem control
 *	only on line 2. Modem control ioctls for other lines
 *	will not return an error, but also will not do anything!
 *	Hopefully, this is the lesser of two evils when it comes to
 *	breaking users' programs.
 *
 *	Line 2 has more modem control signals than ULTRIX uses
 *	right now (see VAXstar system unit spec. chapter 11).
 *
 *	CAUTION: the SML_* definitions in ssreg.h must match the
 *	TIOCM_* definitions in ioctl.h.
 *
 *	The line number in the dev argument to this routine will be
 *	wrong (0 s/b 3) if VS_L3CON is set in the configuration and test
 *	register, i.e., a diagnostic console terminal is attached to
 *	the printer port. This fact is ignored because this routine only
 *	acts on line 2 anyway.
 *
 */

ssmctl(dev, bits, how)
	dev_t dev;
	int bits, how;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int unit, mbits;
	int b, s;

	unit = minor(dev);
	if(unit != MODEM_UNIT)
		return(0);	/* only line 2 has modem control */
	b = 0x4;
	s = spl5();
	mbits = (ssaddr->ssdtr & SS_RDTR) ? SS_DTR : 0;
	mbits |= (ssaddr->ssdtr & SS_RRTS) ? SS_RTS : 0;
	mbits |= (ssaddr->ssmsr & SS_RCD) ? SS_CD : 0;
	mbits |= (ssaddr->ssmsr & SS_RDSR) ? SS_DSR : 0;
	mbits |= (ssaddr->ssmsr & SS_RCTS) ? SS_CTS : 0;
	mbits |= (ssaddr->sstbuf & b) ? SS_RI : 0;
	switch (how) {
	/* No actual bit settings in device are really done ! */
	case DMSET:
		mbits = bits;
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
	if (mbits & SS_DTR)
		ssaddr->ssdtr |= (b|SS_RRTS);
	else
		ssaddr->ssdtr &= ~(b|SS_RRTS);
	(void) splx(s);
	return(mbits);
}

/*
 * Allows silo alarm mode, if set.
 * Enabling silo alarm mode will most likely
 * cause silo overrun errors. The system can't
 * seem to keep up?????????
 */

ss_allow_silos = 0;

int sstransitions, ssfasttimers;		/*DEBUG*/
ssscan()
{
	register i;
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register bit;
	register struct tty *tp;
	int oldsssilos = sssilos;
	int sstimer();
	register u_char sscan_modem;

	for (i = 0; i < ss_cnt; i++) {
		if (sspdma[i].p_addr == 0)
			continue;
		tp = &ss_tty[i];
		sscan_modem = 0;
		/* Modem Control only on line 2 */
		if ((i == MODEM_UNIT) && ((sssoftCAR[i>>2]&04)==0) ) {
			/*
			 * Drop DTR immediately if DSR has gone away.
	                 * If really an active close then do not
	                 *    send signals.
			 */
			if (!(ssaddr->ssmsr&SS_RDSR)) {
	                        if (tp->t_state&TS_CLOSING) {
	                              untimeout(wakeup, (caddr_t) &tp->t_dev);
	                              wakeup((caddr_t) &tp->t_dev);
	                        }
				if (tp->t_state&TS_CARR_ON) {
					/*
					 * Only drop if DECSTD52 is followed.
					 */
					if (ssdsr)
						ss_tty_drop(tp);
				}
			}
			else {		/* DSR has come up */
				/*
				 * If DSR comes up for the first time we allow
				 * 30 seconds for a live connection.
				 */
				if (ssdsr && ((ssmodem&MODEM_DSR)==0)) {
					ssmodem |= (MODEM_DSR_START|MODEM_DSR);
					/* 
				 	* we should not look for CTS|CD for
				 	* about 500 ms.  
				 	*/
					timeout(ss_dsr_check, tp, hz*30);
					timeout(ss_dsr_check, tp, hz/2);
			    	}
			}
			/*
			 * Look for modem transitions in an already established
			 * connection.
			 */
			if (tp->t_state & TS_CARR_ON) {
				if (ssaddr->ssmsr&SS_RCD) {
					/*
					 * CD has come up again.
					 * Stop timeout from occurring if set.
					 * If interval is more than 2 secs then
					 *  drop DTR.
					 */
					if ((ssmodem&MODEM_CD)==0) { 
						untimeout(ss_cd_drop, tp);
						if (ss_cd_down(tp)){
							/* drop connection */
							ss_tty_drop(tp);
						}
						ssmodem |= MODEM_CD;
					}
				}
				else {
					/* 
					 * Carrier must be down for greater than 2 secs
					 * before closing down the line.
					 */
					if ( ssmodem & MODEM_CD) {
					    /* only start timer once */
					    ssmodem &= ~MODEM_CD;
					    /* 
					     * Record present time so that if carrier
					     * comes up after 2 secs , the line will drop.
					     */
					    sstimestamp = time;
					    timeout(ss_cd_drop, tp, hz*2);
					}
				}
				/* CTS flow control check */
		
				if (!(ssaddr->ssmsr&SS_RCTS)) {
					/* 
					 * Only allow transmission when CTS is set.
					 */
					tp->t_state |= TS_TTSTOP;
					ssmodem &= ~MODEM_CTS;
#					ifdef DEBUG
					if (ssdebug)
					   cprintf("ssscan: CTS stop, tp=%x,line=%d\n", tp,i);
#					endif DEBUG
					ssstop(tp, 0);
				} else if ((ssmodem&MODEM_CTS)==0) { 
					    /* 
					     * Restart transmission upon return of CTS.
					     */
					    tp->t_state &= ~TS_TTSTOP;
					    ssmodem |= MODEM_CTS;
#					    ifdef DEBUG
					    if (ssdebug)
					       cprintf("ssscan: CTS start, tp=%x,line=%d\n", tp,i);
#					    endif DEBUG
					    ssstart(tp);
				}
	
	
			} 
			
			/* 
			 * See if a modem transition has occured.  If we are waiting
			 * for this signal cause action to be taken via ss_tty_start.
			 */
			 if ((sscan_modem=ssaddr->ssmsr&SS_XMIT) != sscan_previous){
				/*
			 	* If 500 ms timer has not expired then dont
			 	* check anything yet.
			 	* Check to see if DSR|CTS|CD are asserted.
			 	* If so we have a live connection.
			 	*/
#				ifdef DEBUG
				if (ssdebug)
					cprintf("ssscan: MODEM Transition,sscan_modem=%x, sscan_prev=%x\n",sscan_modem,sscan_previous);
#				endif DEBUG
	   			/* 
	    	 		 * If ssdsr is set look for DSR|CTS|CD,else look
	    	 		 * for CD|CTS only.
	    	 		 */
				if (ssdsr) {
				    if (((ssaddr->ssmsr&SS_XMIT)==SS_XMIT) 
				        && ((ssmodem&MODEM_DSR_START)==0)
				        && ((tp->t_state&TS_CARR_ON)==0)) {
#					    ifdef DEBUG
					    if (ssdebug)
						cprintf("ssscan:SS_XMIT call ss_start,line=%d\n",i);
#				            endif DEBUG
					    ss_start_tty(tp);
				    }
				}
				/* 
				 * Ignore DSR.	
				 */
				else
					if ((ssaddr->ssmsr&SS_NODSR)==SS_NODSR)
						ss_start_tty(tp);
						
			}
			sscan_previous = sscan_modem; /* save for next iteration */
		}
	}
	for (i = 0; i < nNSS; i++) {
		ave(ssrate[i], sschars[i], 8);
		/*
		 * Allow silo mode only if no graphics device.
		 * Silo mode blows away with mouse tracking.
		 */
		if (ss_allow_silos && (vs_gdopen == 0)) {
		    if (sschars[i] > sshighrate && ((sssilos&(1 << i)) == 0)) {
			ssaddr->sscsr = SS_MSE | SS_SAE;
			ssaddr->nb_int_msk |= (SINT_ST|SINT_SR);
			sssilos |= (1 << i);
			sstransitions++;		/*DEBUG*/
		    } else if ((sssilos&(1 << i)) && (ssrate[i] < sslowrate)) {
			ssaddr->sscsr = SS_MSE;
			ssaddr->nb_int_msk |= (SINT_ST|SINT_SR);
			sssilos &= ~(1 << i);
		    }
		}
		sschars[i] = 0;
	}
	if (sssilos && !oldsssilos)
		timeout(sstimer, (caddr_t)0, sstimerintvl);
	timeout(ssscan, (caddr_t)0, hz);
}

sstimer()
{
	register int ss;
	register int s;

	if (sssilos == 0)
		return;
	s = spl5();
	ssfasttimers++;		/*DEBUG*/
	for (ss = 0; ss < nNSS; ss++)
		if (sssilos & (1 << ss))
			ssrint(ss);
	splx(s);
	timeout(sstimer, (caddr_t) 0, sstimerintvl);
}

/* save record of sbis present for sbi error logging for 780 and 8600 */
extern long sbi_there;	/* bits 0-15 for nexi,sbi0; 16-31 for nexi on sbi1*/

/*
 * VAXstar virtual console initialization routine.
 * Configures the VAXstar serial line controller as the console,
 * maps in all of the VAXstar's address space, and
 * decides if LK201 keystrokes must be passed to the bit-map
 * or color driver's console routines.
 */
int	ssputc();
int	ssgetc();
extern	(*v_consputc)();
extern	(*v_consgetc)();
extern	int	sgcons_init();
extern	int	smcons_init();

sscons_init()
{
	register struct nb_regs *ssaddr;
	register char	*nxv, *nxp;	/* nexus virtual/phys addr pointers */
	int	i;
	int	constype;
        struct cpusw *cpup;		/* pointer to cpusw structure  */

	/*
	 * ONLY on a VAXstation 2000 or MicroVAX 2000
	 */
	if((cpu != MVAX_II) || (cpu_subtype != ST_VAXSTAR))
		return(0);
        /*
         * find the cpusw entry that matches this machine.
         */
	cpup = &cpusw[cpu];
        if( cpup == NULL )
                return(0);
        
	/*
	 * Map the nexus.
	 */
	nxv = (char *)nexus;
	nxp = (char *)cpup->v_nexaddr(0,0);
	nxaccess (nxp, Nexmap[0], cpup->pc_nexsize);
	/*
	 * See if there is anything there.
	 */
	if ((*cpup->v_badaddr)((caddr_t) nxv, 4))
		return(-1);
	sbi_there |= 1<<0;
	/*
	 * May as well map the rest of I/O space
	 * while we are at it.
	 */
	nxp = (char *)cpup->v_umaddr(0,0);
	nxaccess (nxp, QMEMmap[0], QMEMSIZEVS);
	nxp = (char *)NMEMVAXSTAR;
	nxaccess (nxp, NMEMmap[0], NMEMSIZEVS);
	/*
	 * TODO:
	 *	Following is place holder for color driver
	 */
	nxp = (char *)SGMEMVAXSTAR;
	nxaccess (nxp, SGMEMmap[0], SGMEMSIZEVS);
	nxp = (char *)SHMEMVAXSTAR;
	nxaccess (nxp, SHMEMmap[0], SHMEMSIZEVS);
	/*
	 * Determine console device and inititialize it.
	 * If L3CON is set use the diagnostic console,
	 * if not use the console_id from NVR or the CFGTST register MULTU bit,
	 * if console_id says unknown we guess based on the
	 * VIDOPT and CURTEST bits in the configuration register.
	 * Order of precedence is:
	 *	Diagnostic console on SLU port 3 (BCC08 cable).
	 *	Terminal on SLU port 0 (if MULTU bit set).
	 *	Keyboard on SLU port 0 (color video, MULTU bit clear).
	 *	Keyboard on SLU port 0 (base monochrome video, MULTU bit clear).
	 * The BCC08 cable enables halt on break.
	 *
	 * The VAXstar SLU is always present and must always be
	 * initialized, even if the console is the bitmap or color
	 * display. So, we inititialize it here.
	 */
	ssaddr = (struct nb_regs *)nexus;
	ssaddr->sscsr = SS_CLR;
	for(i=0; i<100000; i++)
		if((ssaddr->sscsr&SS_CLR) == 0)
			break;
	ssaddr->nb_int_msk &= ~(SINT_ST|SINT_SR);
	ssaddr->nb_int_reqclr = (SINT_ST|SINT_SR);
	ssaddr->sscsr = SS_MSE;
	i = (SS_RE | SS_B9600 | BITS8);
	if(vs_cfgtst&VS_L3CON)
		i |= 0x3;	/* diag. console on line 3 */
	ssaddr->sslpr = i;
	cdevsw[0] = cdevsw[SSMAJOR];
	constype = (ssaddr->nb_console_id >> 2) & 0xff;
	if ((vs_cfgtst&VS_L3CON) || (vs_cfgtst&VS_MULTU)) {
		v_consputc = ssputc;
		v_consgetc = ssgetc;
	} else {
		switch(constype) {

		case VS_CID_COLOR:
			sgcons_init();
			break;
		case VS_CID_BITMAP:
			smcons_init();
			break;
		case VS_CID_UNKNOWN:		/* FALLTHROUGH */
		default:
			if(vs_cfgtst&VS_VIDOPT)
				sgcons_init();
			else if(vs_cfgtst&VS_CURTEST)
				smcons_init();
			else {
				v_consputc = ssputc;
				v_consgetc = ssgetc;
			}
			break;
		}
	}
	return(1);
}

/*
 * VAXstar SLU (dzq like) console putc routine.
 * Calls ss_putc() to output each character.
 */

ssputc(c)
	register int c;
{
	ss_putc(c);
	if (c == '\n')
		ss_putc('\r');
}

/*
 * This routine outputs one character to the console
 * on line 0 or 3 depending on the state of L3CON
 * in the VAXstar configuration and test register.
 * Characters must be printed without disturbing
 * output in progress on other lines!
 * This routines works with the SLU in interrupt or
 * non-interrupt mode of operation.
 * Characters are output as follows:
 *	spl5, remember if console line active.
 *	set console line tcr bit.
 *	wait for TRDY on console line (save status if wrong line).
 *	start output of character.
 *	wait for output complete.
 *	if any lines were active, set their tcr bits,
 *	otherwise clear the xmit ready interrupt.
 *
 */

/*
 * Physical address of VAXstar "fake" nexus.
 * Used to access SLU and interrupt cntlr registers
 * when the machine is on physical mode (during crash dump).
 */
#define	VS_PHYSNEXUS	0x20080000

ss_putc(c)
	register int c;
{
	register struct nb_regs *ssaddr;
	int	s, tcr, ln, tln, timo;
	int	physmode;

	if( (mfpr(MAPEN) & 1) == 0 ) {
		physmode = 1;
		ssaddr = (struct nb_regs *)VS_PHYSNEXUS;
	} else {
		physmode = 0;
		ssaddr = (struct nb_regs *)nexus;
	}
	if(physmode == 0)
		s = spl5();
	tln = (vs_cfgtst&VS_L3CON) ? 0x3 : 0x0;
	tcr = (ssaddr->sstcr & (1<<tln));
	ssaddr->sstcr |= (1<<tln);
	while (1) {
		timo = 1000000;
		while ((ssaddr->sscsr&SS_TRDY) == 0)
			if(--timo == 0)
				break;
		if(timo == 0)
			break;
		ln = (ssaddr->sscsr>>8) & 3;
		if (ln != tln) {
			tcr |= (1 << ln);
			ssaddr->sstcr &= ~(1 << ln);
			continue;
		}
		ssaddr->sstbuf = c&0xff;
		while (1) {
			while ((ssaddr->sscsr&SS_TRDY) == 0) ;
			ln = (ssaddr->sscsr>>8) & 3;
			if (ln != tln) {
				tcr |= (1 << ln);
				ssaddr->sstcr &= ~(1 << ln);
				continue;
			}
			break;
		}
		break;
	}
	ssaddr->sstcr &= ~(1<<tln);
	if (tcr == 0)
		ssaddr->nb_int_reqclr = SINT_ST;
	else
		ssaddr->sstcr |= tcr;
	if(physmode == 0)
		splx(s);
}

/*
 * This routine operates on the following assumptions:
 * 1. putc must have happened first, so SLU already inited.
 * 2. getc will happed before slu reveive interrupt enabled so
 *    don't need to worry about int_req or int_msk registers.
 */
ssgetc()
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int c, line;

	/*
	 * Line number we expect input from.
	 */
	if(vs_cfgtst&VS_L3CON)
		line = 3;
	else
		line = 0;
	while (1) {
		while ((ssaddr->sscsr&SS_RDONE) == 0) ;
		c = ssaddr->ssrbuf;
		if(((c >> 8) & 3) != line)	/* wrong line mumber */
			continue;
		if(c&(SS_DO|SS_FE|SS_PE))	/* error */
			continue;
		break;
	}
	return(c & 0xff);
}

/*
 * Modem Control Routines
 */
/*
 *
 * Function:
 *
 *	ss_cd_drop
 *
 * Functional description:
 *
 * 	Determine if carrier has dropped.  If so call ss_tty_drop to terminate
 * 	the connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ss_cd_drop(tp)
register struct tty *tp;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int unit = minor(tp->t_dev);

	if ((tp->t_state&TS_CARR_ON) &&
		((ssaddr->ssmsr&SS_RCD) == 0)) {
#		ifdef DEBUG
        	if (ssdebug)
	       	    cprintf("ss_cd:  no CD, tp=%x\n", tp);
#		endif DEBUG
		ss_tty_drop(tp);
		return;
	}
	ssmodem |= MODEM_CD;
#	ifdef DEBUG
        if (ssdebug)
	    cprintf("ss_cd:  CD is up, tp=%x\n", tp);
#	endif DEBUG
}
/*
 *
 * Function:
 *
 *	ss_dsr_check
 *
 * Functional description:
 *
 * 	DSR must be asserted for a connection to be established.  Here we either
 * 	start or terminate a connection on the basis of DSR.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ss_dsr_check(tp)
register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	if (ssmodem&MODEM_DSR_START) {
#		ifdef DEBUG
        	if (ssdebug)
	       	    cprintf("ss_dsr_check0:  tp=%x\n", tp);
#		endif DEBUG
		ssmodem &= ~MODEM_DSR_START;
	   	/* 
	    	 * If ssdsr is set look for DSR|CTS|CD, otherwise look 
	    	 * for CD|CTS only.
	    	 */
		if (ssdsr) {
			if ((ssaddr->ssmsr&SS_XMIT) == SS_XMIT)
				ss_start_tty(tp);
		}
		else {
			if ((ssaddr->ssmsr&SS_NODSR) == SS_NODSR)
				ss_start_tty(tp);
		}
		return;
	}
	if ((tp->t_state&TS_CARR_ON)==0)  
		ss_tty_drop(tp);
}

/*
 *
 * Function:
 *
 *	ss_cd_down
 *
 * Functional description:
 *
 *	Determine whether or not carrier has been down for > 2 sec.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	1 - if carrier was down for > 2 sec.
 *	0 - if carrier down <= 2 sec.
 *
 */
ss_cd_down(tp)
struct tty *tp;
{
	int msecs;
	int unit = minor(tp->t_dev);

	msecs = 1000000 * (time.tv_sec - sstimestamp.tv_sec) + 
		(time.tv_usec - sstimestamp.tv_usec);
	if (msecs > 2000000){
#		ifdef DEBUG
		if (ssdebug)
			cprintf("ss_cd_down: msecs > 20000000\n");
#		endif DEBUG
		return(1);
	}
	else{
#		ifdef DEBUG
		if (ssdebug)
			cprintf("ss_cd_down: msecs < 20000000\n");
#		endif DEBUG
		return(0);
	}
}
/*
 *
 * Function:
 *
 *	ss_tty_drop
 *
 * Functional description:
 *
 *	Terminate a connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ss_tty_drop(tp)
struct tty *tp;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int unit;
  	if (tp->t_flags&NOHANG) 
		return;
	unit = minor(tp->t_dev);
#	ifdef DEBUG
	if (ssdebug)
		cprintf("ss_tty_drop: unit=%d\n",unit);
#	endif DEBUG
	/* 
	 * Notify any processes waiting to open this line.  Useful in the
	 * case of a false start.
	 */
	ssmodem = MODEM_BADCALL;
	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP);
	wakeup((caddr_t)&tp->t_rawq);
  	gsignal(tp->t_pgrp, SIGHUP);
	gsignal(tp->t_pgrp, SIGCONT);
	ssaddr->ssdtr &= ~(SS_RDTR|SS_RRTS);
}
/*
 *
 * Function:
 *
 *	ss_start_tty
 *
 * Functional description:
 *
 *	Establish a connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ss_start_tty(tp)
	register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	tp->t_state &= ~(TS_ONDELAY);
	tp->t_state |= TS_CARR_ON;
#	ifdef DEBUG
        if (ssdebug)
	       cprintf("ss_start_tty:  tp=%x\n", tp);
#	endif DEBUG
	if (ssmodem&MODEM_DSR)
		untimeout(ss_dsr_check, tp);
	ssmodem |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	sstimestamp.tv_sec = sstimestamp.tv_usec = 0;
	wakeup((caddr_t)&tp->t_rawq);
}
#endif
