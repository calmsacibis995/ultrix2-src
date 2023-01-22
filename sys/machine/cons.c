
#ifndef lint
static	char	*sccsid = "@(#)cons.c	1.24  ULTRIX  10/23/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
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
 * Modification History
 *
 * 06-Aug-86 -- jaw	fixes to 8800 console support.
 *
 *  2-Jul-86  -- fred (Fred Canter)
 *	Moved sscons_init(), ssputc(), & ssgetc() to ss.c.
 *	No longer any VAXstar specific code in cons.c.
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 29-Apr-86 -- jaw	fixes to 8800 console.
 *
 * 15-Apr-86 -- afd
 *	v_console() was changed to v_consputc().
 *
 * 09-Apr-86 -- jaw  only VAX8800 now toggles TXCS_IE.
 *
 * 03-Apr-86 -- jaw  left out some ifdefs.
 *
 * 02-Apr-86 -- jaw  add support for nautilus console and memory adapter
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 07 Nov 85 -- lp
 *	Disable use of EMM. Only snap file commands allowed on
 *	logical_cons.
 * 10 Oct 85 -- lp
 *	Added 8600 remote line support. Also moved 780 floppy output
 *	to be routed through chrqueue (so we dont get confused
 *	about who a character is intended for). Prevent non-8600
 *	8200 machines from using extra lines code (they can't).
 *	Make sure interrupts are off in 8600 cnputc.
 *
 * 19 Aug 85 -- lp
 *	Changed cnstart routine to only call cnoutput if not aready
 *	outputting characters.
 *
 * 13 Aug 85 -- rjl
 *	Changed virtual console put call to be through a function pointer
 *	instead of directly to allow dynamic configuring of the system
 *	console.
 *
 * 16 Jul 85 -- lp
 *	Cleanup for 1.2 ft. Added ioctl setspeed for Scorpio.
 *
 * 15 Mar 85 -- lp
 *	Added support for Scorpio slu's (serial line units).
 *	Added notion of interrupt queue so we can get characters
 *	in at elevated IPL the reschedule tty action at a lower IPL.
 *
 * 20 Nov 84 -- rjl
 *	Changed qvss support to virtual console support with the idea that
 *	a virtual console could be added at a later time.
 *
 * 11 Aug 84 -- rjl
 *	Added MVAX support for system console
 */

/*
 * VAX console driver (and floppy interface)
 */
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/systm.h"
#include "../h/uio.h"

#include "../vax/cpu.h"
#include "../vax/cons.h"
#include "../vax/mtpr.h"

#if defined(VAX8200) || defined(VAX8600)
#define NCONS	4
#else
#define NCONS	1
#endif

#define DEFAULTSPEED B4800

struct	tty cons[NCONS];
int tty8800rec[3] = { 0, 0, 0};
int txcs8800ie;
int rx8800ie;
int	cnstart();
int	ttrstrt();
char	partab[];
int cold;


#ifdef	VAX8200
static int rxcs[] = { RXCS, RXCS1, RXCS2, RXCS3 };
static int rxdb[] = { RXDB, RXDB1, RXDB2, RXDB3 };
static int txcs[] = { TXCS, TXCS1, TXCS2, TXCS3 };
static int txdb[] = { TXDB, TXDB1, TXDB2, TXDB3 };
#else	VAX8200
static int rxcs[] = { RXCS };
static int rxdb[] = { RXDB };
static int txcs[] = { TXCS };
static int txdb[] = { TXDB };
#endif	VAX8200

#ifdef VAX8600
static int conson[] = { LOCAL_CONS, REMOTE_PORT, EMM_PORT, LOGICAL_CONS };
static int consid[] = { LOCAL_CONS_ID, REMOTE_PORT_ID, EMM_ID, LOGIC_CONS_ID };
#endif
#ifdef VAX8800
static int consid8800[] = {N_LCL_CONS,N_LCL_NOLOG,N_RMT_CONS};
#endif 



/*ARGSUSED*/
cnopen(dev, flag)
	dev_t dev;
{
	register int whichcons = minor(dev);
	register int which = minor(dev);
	register struct tty *tp;
	register int s;

	switch (cpu) {
#if defined(VAX8600) || defined(VAX8200)	
	case VAX_8600:
		whichcons=0;

	case VAX_8200:
		if (which > 3 ) return(ENODEV);
		break;
#endif	
#ifdef VAX8800
	case VAX_8800:
		if (which > 2 ) return(ENODEV);
		tty8800rec[which] = 0;
		whichcons=0;
		break;
#endif
	default:
		if (which) return(ENODEV);
		break;
	}	
	tp = &cons[which];
	tp->t_oproc = cnstart;
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		tp->t_state = TS_ISOPEN|TS_CARR_ON;
		tp->t_flags = EVENP|ECHO|XTABS|CRMOD;
	}
	if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
		return (EBUSY);

#ifdef VAX8600
	if(cpu == VAX_8600) {
		if(consid[which] == EMM_ID) /* No EMM allowed */
			return(ENODEV);
		
	/* enable that console based on dev */
		mtpr(txcs[0], (mfpr(txcs[0])|WMASKNOW|TXCS_IE|conson[which]));
		mtpr(rxcs[0], (mfpr(rxcs[0])|conson[which]));
	}
#endif
 	s = spl5();	
	if (cpu != VAX_8800 || txcs8800ie == 0) {
		txcs8800ie=1;
		mtpr(txcs[whichcons], (mfpr(txcs[whichcons])|TXCS_IE));
	}
	splx(s);
	mtpr(rxcs[whichcons], (mfpr(rxcs[whichcons])|RXCS_IE));

	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*ARGSUSED*/
cnclose(dev)
	dev_t dev;
{
	register int which = minor(dev);
	register struct tty *tp = &cons[which];

	(*linesw[tp->t_line].l_close)(tp);


	switch (cpu) {
#if defined(VAX8800) || defined(VAX780)
	case VAX_8800:
	case VAX_780:
		ttyclose(tp);
		break;
#endif
#ifdef VAX8600
	case VAX_8600:
		ttyclose(tp); 
		/* Mask the line out */
		mtpr(txcs[0], mfpr(txcs[0])&~conson[which]|WMASKNOW);
		mtpr(rxcs[0], (mfpr(rxcs[0])&~conson[which]));
		break;
#endif

	default:
		mtpr(rxcs[which], mfpr(rxcs[which])&~RXCS_IE);
		ttyclose(tp);
#ifdef VAX8200
		if(cpu == VAX_8200) {
			tp->t_ispeed = tp->t_ospeed = DEFAULTSPEED;
			cnparam(which);
		}
#endif
		/* disable interrupts till line is opened */
		mtpr(txcs[which], mfpr(txcs[which])&~TXCS_IE);
		break;
	}
}

/*ARGSUSED*/
cnread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &cons[minor(dev)];

	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

/*ARGSUSED*/
cnwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &cons[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

/*
 * Got a level-20 receive interrupt -
 * the LSI wants to give us a character.
 * Catch the character, and see who it goes to.
 */
/*ARGSUSED*/
cnrint(dev)
	dev_t dev;
{
	int chrqueue();
	register int c;
	register int which = minor(dev);
	register struct tty *tp = &cons[which];

	switch (cpu) {

#ifdef VAX8800
	case VAX_8800:
		c = mfpr(rxdb[0]);
		ka8800requeue(c);
		return;
#endif
#ifdef VAX8600
	case VAX_8600:
		c = mfpr(rxdb[0]);
		/* Based on Id field, setup who the interrupt goes to */
		/* Valid values are 0,1,2,3 */
		if((c&RXDB_ID) == RXDB_ID) /* could be handled better */
			return;
		tp = &cons[(c>>8)&0x3];
		break;
#endif
#ifdef VAX780
	case VAX_780:	
		c = mfpr(rxdb[which]);
		if (c&RXDB_ID) {
			int cnrfl();
			chrqueue(cnrfl, c, 0);
			return;
		}
		break;
#endif VAX780

	default:
		c = mfpr(rxdb[which]);


	}
	if (tp->t_state&TS_ISOPEN) 
		chrqueue(linesw[tp->t_line].l_rint, c, tp);

}

/*ARGSUSED*/
cnioctl(dev, cmd, addr, flag)
	dev_t dev;
	caddr_t addr;
{
	register int which = minor(dev);
	register struct tty *tp = &cons[which];
	int error;

	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (error <= 0);
		error = ttioctl(tp, cmd, addr, flag);
#ifdef VAX8200
	if (error >= 0) {
		if (cmd == TIOCSETP || cmd == TIOCSETN)
			cnparam(which);
	}
#endif
	if (error < 0)
		error = ENOTTY;
	return (error);
}

/*
 * Got a level-20 transmission interrupt -
 * the LSI wants another character.  First,
 * see if we can send something to the typewriter.
 * If not, try the floppy.
 */
/*ARGSUSED*/
int didsendit;

cnxint(dev)
	dev_t dev;
{
	register int which = minor(dev);
	register struct tty *tp = &cons[which];
	register int i;

#ifdef VAX8600
	if(cpu == VAX_8600) {	 /* see who interrupted us */
		int k=mfpr(txcs[0]);
		tp = &cons[(k>>8)&0x3];
		which = tp - &cons[0];
		if((k&NO_LINES) == NO_LINES) 
			return;
	}
#endif
#ifdef VAX8800
	if( cpu == VAX_8800) {
		txcs8800ie=0;
		mtpr(TXCS, 0); /* disable transmit interrupts */

		if (rx8800ie) {
			int rx8800_trans();
			chrqueue(rx8800_trans,0, 0);
			return;
		}
		for(i=0;i < 3;i++) {
		    if (tty8800rec[i] == 0 ) {
			tp = &cons[i];
			if (tp->t_state&TS_ISOPEN) {
				tp->t_state &= ~TS_BUSY;
				if (tp->t_line)
					(*linesw[tp->t_line].l_start)(tp);
				else
					cnstart(tp);
				if (txcs8800ie) return;
			}		
		    }
		}
	} else 
		{
#endif
		/*If the line is open & no chance of it being the 780 floppy*/
		if((tp->t_state&TS_ISOPEN) || (cpu == VAX_780)) {
			tp->t_state &= ~TS_BUSY;
			if (tp->t_line)
				(*linesw[tp->t_line].l_start)(tp);
			else
				cnstart(tp);
		}
#ifdef VAX8800
	}
#endif VAX8800
}

cnstart(tp)
	register struct tty *tp;
{
	register int c, s;
	register int which;
	int cnoutput();

	/* Dont queue it up if we don't have to */
	if (cpu == VAX_8800 && txcs8800ie) return;


	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
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

	which = tp - &cons[0];
#ifdef VAX8600
	if(cpu == VAX_8600) {
		if(consid[which] == LOGIC_CONS_ID) {
			char *cp = tp->t_outq.c_cf;
		/* Hunt for any valid command else throw them away */
			while(*cp < 0x30 || *cp > 0x32) {
				if(getc(&tp->t_outq) == NULL)
					return;
			/* Peek at next character */
				cp = tp->t_outq.c_cf;
			}
		}
		if((mfpr(txcs[0])&conson[which]) != conson[which]) {
			mtpr(txcs[0], mfpr(txcs[0])|WMASKNOW|conson[which]);	
			return;
		}
	}
#endif VAX8600
	txcs8800ie=1;
	chrqueue(cnoutput, tp, which);
	return;

out:
#ifdef VAX8600
	if (cpu == VAX_8600) {
		which = tp - &cons[0];
		if (tp->t_state & (TS_TIMEOUT|TS_BUSY))
			if((tp->t_state&TS_TTSTOP) == 0)
				return;
		mtpr(txcs[0], (mfpr(txcs[0])|WMASKNOW)&~conson[which]);
	}
#endif
#ifdef VAX780
	/* I think this is impossible right here but I'm making sure */
	if ((cpu == VAX_780) && ((tp->t_state&TS_BUSY) == 0)) {	/* floppy */
		conxfl();	/* so start floppy xfer */
	}
#endif
	;
}

cnoutput(tp, which)
register struct tty *tp;
register int which;
{
	register int c,s,whichcons = which;

#if defined(VAX8600) || defined(VAX8800)
	if(cpu == VAX_8600 || cpu == VAX_8800) 
		whichcons = 0;

#endif
 	s = spl5();
	if (cpu == VAX_8800) {
		txcs8800ie=1;
		mtpr(TXCS,TXCS_IE);
	}

	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
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
#ifdef VAX8600
	if(cpu == VAX_8600) {
		if((mfpr(txcs[0])&conson[which]) != conson[which]) {
			mtpr(txcs[0], mfpr(txcs[0])|WMASKNOW|conson[which]);	
			splx(s);
			return;
		}
	}
#endif VAX8600
	c = getc(&tp->t_outq);
	if (tp->t_flags&(RAW|LITOUT)) {
		tp->t_state |= TS_BUSY;
		c &= 0xff;
#ifdef VAX8800
		if (cpu == VAX_8800) c |= consid8800[which];
#endif VAX8800	
		mtpr(txdb[whichcons], c);
	} else if (c <= 0177) {
		if (cpu != VAX_8800) {
			c |= partab[c] & 0x80;
		}
		tp->t_state |= TS_BUSY;
		c &= 0xff;
#ifdef VAX8800
		if (cpu == VAX_8800) c |= consid8800[which];
#endif VAX8800
		mtpr(txdb[whichcons], c);
	} else {
		tp->t_state |= TS_TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp, (c&0177));
	}
out:
#ifdef VAX8600
	if (cpu == VAX_8600) {
		which = tp - &cons[0];
		if (tp->t_state & (TS_TIMEOUT|TS_BUSY)) {
			if((tp->t_state&TS_TTSTOP) == 0) {
				splx(s);
				return;
			}
		}
		mtpr(txcs[0], (mfpr(txcs[0])|WMASKNOW)&~conson[which]);
	}
#endif
#ifdef VAX780
	if ((cpu == VAX_780) && ((tp->t_state&TS_BUSY) == 0)) {	/* floppy */
		conxfl();	/* so start floppy xfer */
	}
#endif
	splx(s);
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 */
cnputc(c)
	register int c;
{
	register struct tty *tp;
	register int loopcnt,savetxcs,s, timo,data;
#ifdef VAX8600
	int oldtxcs;
#endif
#ifdef MVAX
	extern (*v_consputc)();
#endif MVAX

	s=spl7();
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
#ifdef MVAX
	if( v_consputc ) {
		(*v_consputc)( c );
		if( c == '\n' )
			(*v_consputc)( '\r' );
	} else {
#endif MVAX
#ifdef VAX8800
		if(cpu==VAX_8800) {
			timo = 600;
			while (((mfpr(RXCS) & RXCS_DONE) == 0) &&
				(timo != 0)) --timo;

			loopcnt=0;
			while (((mfpr(RXCS) & RXCS_DONE) != 0) || tty8800rec[0]==1) {
				if (loopcnt++ > 20) {
					tty8800rec[0] = 0;
					break;
				}
				if (mfpr(RXCS) & RXCS_DONE) {
					data = mfpr(RXDB);
					if (cold==0) ka8800requeue(data);
					if ((data&0xf7f) == CSTOP) 
							tty8800rec[0] = 1;
					if ((data&0xf7f) == CSTART) 
							tty8800rec[0] = 0;
				}
				timo = 10000;
				while((--timo)!= 0){
					if ((mfpr(RXCS) & RXCS_DONE) != 0) 
						break;
				}

			}
		}
#endif
#ifdef VAX8600
		if(cpu == VAX_8600) {
			timo = 30000;
			while ((mfpr(TXCS)&TXCS_RDY) == 0)
				if(--timo == 0)
					break;
			oldtxcs = mfpr(TXCS);
			mtpr(TXCS, (oldtxcs|WMASKNOW|conson[0])&~TXCS_IE);
			timo = 30000;
		}
#endif VAX8600
			
		timo = 30000;
		while ((mfpr(TXCS)&TXCS_RDY) == 0)
			if(--timo == 0)
				break; 
		if (c == 0) 
			goto out;
		if (cpu != VAX_8800) {
			savetxcs = mfpr(TXCS);
			mtpr(TXCS, 0);
		}
		mtpr(TXDB, c&0xff);
		if (c == '\n')
			cnputc('\r');
		cnputc(0);

		if (cpu != VAX_8800) mtpr(TXCS, savetxcs);
out:
		;
#ifdef VAX8600
		if(cpu == VAX_8600)
			mtpr(TXCS, oldtxcs);
#endif
#ifdef MVAX
	}
#endif MVAX
	splx(s);

}
#ifdef VAX8200
#define INVALSP B4800
static int cn_speeds[] = { INVALSP, INVALSP, INVALSP, INVALSP, INVALSP,
		    0x00000000, INVALSP, 0x00000200, 0x00000400, 0x00000600,
		    INVALSP, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, INVALSP};

cnparam(which)
int which;
{
	register struct tty *tp = &cons[which];

	if (cpu != VAX_8200)
		return;
	if (which == 0)
		return; 	/* Not brave enough to try this on the console right now! */

	if(tp->t_ispeed <= 0 || tp->t_ispeed > 16)
		tp->t_ispeed = tp->t_ospeed = DEFAULTSPEED;

	if(cn_speeds[tp->t_ispeed] != INVALSP)
		mtpr(txcs[which], ((mfpr(txcs[which])&~0xf00)|(cn_speeds[tp->t_ispeed]|TXCS_BRE)));

}
#endif



