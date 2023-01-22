
#ifndef lint
static	char	*sccsid = "@(#)st.c	1.1	(ULTRIX)	7/3/86";
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/***********************************************************************
 *
 * Modification History:
 *
 *  18-Jun-86  -- darrell (Darrell Dunnuck)
 *	Created this VAXstar TZK50 device driver.
 *
 **********************************************************************/


#include "st.h"
#if NST > 0 || defined(BINARY)

#include "../data/st_data.c"

u_short ststd[] = { TMSTD, 0 };
int	stprobe(), stslave(), stattach(), stdgo(), stintr(), sttimer();

struct	uba_driver stdriver = { stprobe, stslave, stattach, stdgo,
				ststd, "st", stdinfo, "st",
				stminfo, 0 };

/*
 * Determine if there is a controller for
 * a tm at address reg.  Our goal is to make the
 * device interrupt.
 */

extern	int	cpu;
extern	int	cpu_subtype;

stprobe(reg)
	caddr_t reg;
{

	/*
	 * ONLY on a VAXstar/TEAMmate
	 */
	if((cpu != MVAX_II) || (cpu_subtype != ST_VAXSTAR))
		return(0);
#ifdef lint
	stintr(0);
#endif lint

/* FARKLE */
	if(1)
		return(0);
/* FARKLE */
	((struct stdevice *)reg)->tmcs = TM_IE;
	/*
	 * If this is a tm11, it ought to have interrupted
	 * by now, if it isn't (ie: it is a ts04) then we just
	 * hope that it didn't interrupt, so autoconf will ignore it.
	 * Just in case, we will reference one
	 * of the more distant registers, and hope for a machine
	 * check, or similar disaster if this is a ts.
	 *
	 * Note: on an 11/780, BADADDR will just generate
	 * a uba error for a ts; but our caller will notice that
	 * so we won't check for it.
	 */
	if (BADADDR((caddr_t)&((struct stdevice *)reg)->tmrd, 2))
		return (0);
	return (sizeof (struct stdevice));
}

/*
 * Due to a design flaw, we cannot ascertain if the tape
 * exists or not unless it is on line - ie: unless a tape is
 * mounted. This is too servere a restriction to bear,
 * so all units are assumed to exist.
 */
stslave(ui, reg)
	struct uba_device *ui;
	caddr_t reg;
{

	if (ui->ui_slave)	/* non-zero slave not allowed */
		return(0);
	return (1);
}

/*
 * Record attachment of the unit to the controller.
 */
stattach(ui)
	struct uba_device *ui;
{
	register struct st_softc *sc = &st_softc[ui->ui_unit];
	/*
	 * Tetotm is used to index the cstbuf and rtmbuf
	 * arrays given a te unit number.
	 */
	sttotm[ui->ui_unit] = ui->ui_mi->um_ctlr;
	bcopy(DEV_UNKNOWN,sc->sc_device,DEV_SIZE);
}

/*
 * Open the device.  Tapes are unique open
 * devices, so we refuse if it is already open.
 * We also check that a tape is available, and
 * don't block waiting here; if you want to wait
 * for a tape you should timeout in user code.
 */
stopen(dev, flag)
	register dev_t dev;
	register int flag;
{
	register struct uba_device *ui = stdinfo[UNIT(dev)];
	register struct st_softc *sc = &st_softc[UNIT(dev)];
	register struct stdevice *tmaddr = (struct stdevice *)ui->ui_addr;
	register int sel = SEL(dev);
	int unit = UNIT(dev);
	int s;

	if (unit >= nNST || sc->sc_openf ||
	    (ui == 0) || ui->ui_alive == 0)
		return (ENXIO);

	if ((tmaddr->tmer & TMER_EOT) && (dis_eot_st[unit] != DISEOT)) {
		sc->sc_flags &= DEV_EOM;
	} else {
		sc->sc_flags = 0;
	}
	sc->sc_category_flags = 0;

	sc->sc_dens = (((sel == MTHR) || (sel == MTHN)) ?
			      (TM_IE | TM_GO | (ui->ui_slave << 8)):
			      (TM_IE | TM_GO | (ui->ui_slave << 8) |
			       TM_D800));
	if((sel == MTHR) || (sel == MTHN)) {
		sc->sc_category_flags |= DEV_1600BPI;
	} else {
		sc->sc_category_flags |= DEV_800BPI;
	}
get:
	stcommand(dev, TM_SENSE, 1);
	if (sc->sc_erreg & TMER_SDWN) {
		sleep((caddr_t)&lbolt, PZERO+1);
		goto get;
	}

	if ((sc->sc_erreg & (TMER_SELR|TMER_TUR)) !=
	    (TMER_SELR|TMER_TUR)) {
		sc->sc_flags |= DEV_OFFLINE;
		if(!(flag & FNDELAY)) {
			DEV_UGH(sc->sc_device,unit,"offline");
			return(EIO);
		}
	}

	if ((flag & FWRITE) && (sc->sc_erreg & TMER_WRL)) {
		sc->sc_flags |= DEV_WRTLCK;
		if(!(flag & FNDELAY)) {
			DEV_UGH(sc->sc_device,unit,"write locked");
			return(EIO);
		}
	}

	sc->sc_openf = 1;
	sc->sc_blkno = (daddr_t)0;
	sc->sc_nxrec = INF;
	s = spl6();
	if (sc->sc_tact == 0) {
		sc->sc_timo = INF;
		sc->sc_tact = 1;
		timeout(sttimer, (caddr_t)dev, 5*hz);
	}
	splx(s);
	return (0);
}

/*
 * Close tape device.
 *
 * If tape was open for writing or last operation was
 * a write, then write two EOF's and backspace over the last one.
 * Unless this is a non-rewinding special file, rewind the tape.
 * Make the tape available to others.
 */
stclose(dev, flag)
	register dev_t dev;
	register int flag;
{
	register struct st_softc *sc = &st_softc[UNIT(dev)];
	register int unit = UNIT(dev);
	register int sel = SEL(dev);

	sc->sc_flags &= ~DEV_EOM;

	if (flag == FWRITE || (flag & FWRITE) &&
	    (sc->sc_flags & DEV_WRITTEN)) {
		stcommand(dev, TM_WEOF, 1);
		sc->sc_flags &= ~DEV_EOM;
		stcommand(dev, TM_WEOF, 1);
		sc->sc_flags &= ~DEV_EOM;
		stcommand(dev, TM_SREV, 1);
		sc->sc_flags &= ~DEV_EOM;
	}

	if ((sel == MTLR) || (sel == MTHR)) {
		/*
		 * 0 count means don't hang waiting for rewind complete
		 * rather cstbuf stays busy until the operation
		 * completes preventing further opens from completing
		 * by preventing a TM_SENSE from completing.
		 */
		stcommand(dev, TM_REW, 0);
	}

	sc->sc_openf = 0;

	if ((sc->sc_erreg & TMER_EOT) && (dis_eot_st[unit] != DISEOT)) {
		sc->sc_flags |= DEV_EOM;
	}
}

/*
 * Execute a command on the tape drive
 * a specified number of times.
 */
stcommand(dev, com, count)
	register dev_t dev;
	register int com;
	register int count;
{
	register struct buf *bp = &cstbuf[TMUNIT(dev)];
	register int s = spl5();

	while (bp->b_flags&B_BUSY) {
		/*
		 * This special check is because B_BUSY never
		 * gets cleared in the non-waiting rewind case.
		 */
		if (bp->b_repcnt == 0 && (bp->b_flags&B_DONE))
			break;
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO);
	}
	bp->b_flags = B_BUSY|B_READ;
	splx(s);
	bp->b_dev = dev;
	bp->b_repcnt = -count;
	bp->b_command = com;
	bp->b_blkno = 0;
	ststrategy(bp);
	/*
	 * In case of rewind from close, don't wait.
	 * This is the only case where count can be 0.
	 */
	if (count == 0)
		return;

	iowait(bp);

	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags &= B_ERROR;
}

/*
 * Queue a tape operation.
 */
ststrategy(bp)
	register struct buf *bp;
{
	register struct uba_ctlr *um;
	register struct st_softc *sc = &st_softc[UNIT(bp->b_dev)];
	register struct buf *dp;
	register int s;
	register int unit = UNIT(bp->b_dev);

	if ((sc->sc_flags & DEV_EOM) && !((sc->sc_flags & DEV_CSE) ||
	    (dis_eot_st[unit] & DISEOT))) {
		bp->b_resid = bp->b_bcount;
		bp->b_error = ENOSPC;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Put transfer at end of unit queue
	 */
	dp = &stutab[unit];
	bp->av_forw = NULL;
	s = spl5();
	um = stdinfo[unit]->ui_mi;
	if (dp->b_actf == NULL) {
		dp->b_actf = bp;
		/*
		 * Transport not already active...
		 * put at end of controller queue.
		 */
		dp->b_forw = NULL;
		if (um->um_tab.b_actf == NULL)
			um->um_tab.b_actf = dp;
		else
			um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
	} else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	/*
	 * If the controller is not busy, get
	 * it going.
	 */
	if (um->um_tab.b_active == 0)
		ststart(um);
	splx(s);
}

/*
 * Start activity on a tm controller.
 */
ststart(um)
	register struct uba_ctlr *um;
{
	register struct stdevice *addr = (struct stdevice *)um->um_addr;
	register struct buf *bp, *dp;
	register struct st_softc *sc;
	register struct uba_device *ui;
	int cmd, tmunit, unit;
	daddr_t blkno;

	/*
	 * Look for an idle transport on the controller.
	 */
loop:
	if ((dp = um->um_tab.b_actf) == NULL)
		return;

	if ((bp = dp->b_actf) == NULL) {
		um->um_tab.b_actf = dp->b_forw;
		goto loop;
	}

	tmunit = TMUNIT(bp->b_dev);
	unit = UNIT(bp->b_dev);
	ui = stdinfo[unit];
	sc = &st_softc[unit];
	addr->tmcs = (ui->ui_slave << 8);
	sc->sc_dsreg = addr->tmcs;
	sc->sc_erreg = addr->tmer;
	sc->sc_resid = addr->tmbc;

	if ((sc->sc_flags & DEV_EOM) && !((sc->sc_flags & DEV_CSE) ||
	    (dis_eot_st[unit] & DISEOT))) {
		bp->b_resid = bp->b_bcount;
		bp->b_error = ENOSPC;
		bp->b_flags |= B_ERROR;
		goto next;
	}

	/*
	 * Default is that last command was NOT a write command;
	 * if we do a write command we will notice this in tmintr().
	 */
	sc->sc_flags &= ~DEV_WRITTEN;

	if (sc->sc_openf < 0 || (addr->tmcs & TM_CUR) == 0) {
		/*
		 * Have had a hard error on a non-raw tape
		 * or the tape unit is now unavailable
		 * (e.g. taken off line).
		 */
		bp->b_flags |= B_ERROR;
		goto next;
	}
	if (bp == &cstbuf[tmunit]) {
		/*
		 * Execute control operation with the specified count.
		 */
		if (bp->b_command == TM_SENSE)
			goto next;
		/*
		 * Set next state; give 5 minutes to complete
		 * rewind, or 10 seconds per iteration (minimum 60
		 * seconds and max 5 minutes) to complete other ops.
		 */
		if (bp->b_command == TM_REW) {
			um->um_tab.b_active = SREW;
			sc->sc_timo = 5 * 60;
		} else {
			um->um_tab.b_active = SCOM;
			sc->sc_timo =
			    imin(imax(10*(int)-bp->b_repcnt,60),5*60);
		}
		if (bp->b_command == TM_SFORW ||
		    bp->b_command == TM_SREV)
			addr->tmbc = bp->b_repcnt;
		goto dobpcmd;
	}

	if (sc->sc_blkno != bdbtofsb(bp->b_blkno) &&
	    !um->um_tab.b_errcnt)
		sc->sc_blkno = bdbtofsb(bp->b_blkno);

	sc->sc_nxrec = bdbtofsb(bp->b_blkno) + 1;

	/*
	 * If the data transfer command is in the correct place,
	 * set up all the registers except the csr, and give
	 * control over to the UNIBUS adapter routines, to
	 * wait for resources to start the i/o.
	 */
	if ((blkno = sc->sc_blkno) == bdbtofsb(bp->b_blkno)) {
		addr->tmbc = -bp->b_bcount;
		if ((bp->b_flags&B_READ) == 0) {
			if (um->um_tab.b_errcnt)
				cmd = TM_WIRG;
			else
				cmd = TM_WCOM;
		} else
			cmd = TM_RCOM;
		um->um_tab.b_active = SIO;
		um->um_cmd = sc->sc_dens|cmd;
		sc->sc_timo = 60;
		(void) ubago(ui);
		return;
	}

	/*
	 * Tape positioned incorrectly;
	 * set to seek forwards or backwards to the correct spot.
	 * This happens for raw tapes only on error retries.
	 */
	um->um_tab.b_active = SSEEK;
	if (blkno < bdbtofsb(bp->b_blkno)) {
		bp->b_command = TM_SFORW;
		addr->tmbc = blkno - bdbtofsb(bp->b_blkno);
	} else {
		bp->b_command = TM_SREV;
		addr->tmbc = bdbtofsb(bp->b_blkno) - blkno;
	}

	sc->sc_timo = imin(imax(10 * -addr->tmbc, 60), 5 * 60);

dobpcmd:
	/*
	 * Do the command in bp.
	 */
	addr->tmcs = (sc->sc_dens | bp->b_command);
	return;

next:
	/*
	 * Done with this operation due to error or
	 * the fact that it doesn't do anything.
	 * Release UBA resources (if any), dequeue
	 * the transfer and continue processing this slave.
	 */
	if (um->um_ubinfo)
		ubadone(um);
	um->um_tab.b_errcnt = 0;
	dp->b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

/*
 * The bus resources we needed have been
 * allocated to us; start the device.
 */
stdgo(um)
	register struct uba_ctlr *um;
{
	register struct stdevice *addr = (struct stdevice *)um->um_addr;

	addr->tmba = um->um_ubinfo;
	addr->tmcs = um->um_cmd | ((um->um_ubinfo >> 12) & 0x30);
}

/*
 * TM interrupt routine.
 */
stintr(tm11)
	int tm11;
{
	register struct buf *bp;
	register struct uba_ctlr *um = stminfo[tm11];
	register struct stdevice *addr;
	register struct st_softc *sc;
	register int state;
	register struct buf *dp;
	int tmunit, unit;

	if ((dp = um->um_tab.b_actf) == NULL)
		return;

	bp = dp->b_actf;
	tmunit = TMUNIT(bp->b_dev);
	unit = UNIT(bp->b_dev);
	addr = (struct stdevice *)stdinfo[unit]->ui_addr;
	sc = &st_softc[unit];

	/*
	 * If last command was a rewind, and tape is still
	 * rewinding, wait for the rewind complete interrupt.
	 */
	if (um->um_tab.b_active == SREW) {
		um->um_tab.b_active = SCOM;
		if (addr->tmer&TMER_RWS) {
			sc->sc_timo = 5*60;		/* 5 minutes */
			return;
		}
	}

	/*
	 * An operation completed... record status
	 */
	sc->sc_timo = INF;
	sc->sc_dsreg = addr->tmcs;
	sc->sc_erreg = addr->tmer;
	sc->sc_resid = addr->tmbc;
	if ((bp->b_flags & B_READ) == 0)
		sc->sc_flags |= DEV_WRITTEN;
	state = um->um_tab.b_active;
	um->um_tab.b_active = 0;

	/*
	 * Check for errors.
	 */
	if (addr->tmcs & TM_ERR) {
		while (addr->tmer & TMER_SDWN)
			;			/* await settle down */
		/*
		 * If we hit end-of-tape (EOT),
		 * just return.
		 */
		if (addr->tmer & TMER_EOT) {
			sc->sc_flags |= DEV_EOM;
			goto opdone;
		}

		/*
		 * If we hit the end of the tape file, update position.
		 */
		if (addr->tmer & TMER_EOF) {
			sc->sc_category_flags |= DEV_TPMARK;
			if (bp == &cstbuf[tmunit]) {
				if (sc->sc_blkno >
				    bdbtofsb(bp->b_blkno)) {
					/* reversing */
					sc->sc_nxrec =
					       bdbtofsb(bp->b_blkno) -
					       addr->tmbc;
					sc->sc_blkno = sc->sc_nxrec;
				} else {
					/* spacing forward */
					sc->sc_blkno =
						bdbtofsb(bp->b_blkno) +
						addr->tmbc;
					sc->sc_nxrec = sc->sc_blkno - 1;
				}
				goto seteof;
			}
			/* eof on read */
			sc->sc_nxrec = bdbtofsb(bp->b_blkno);
seteof:
			state = SCOM;		/* force completion */
			/*
			 * Stuff bc so it will be unstuffed correctly
			 * later to get resid.
			 */
			addr->tmbc = -bp->b_bcount;
			goto opdone;
		}

		/*
		 * If we were reading raw tape and the only error was
		 * that the record was too long, then we don't consider
		 * this an error.
		 */
		if (bp != &cstbuf[tmunit] && (bp->b_flags&B_READ) &&
		    (addr->tmer&(TMER_HARD|TMER_SOFT)) == TMER_RLE)
			goto ignoreerr;

		/*
		 * If error is not hard, and this was an i/o operation
		 * retry up to 8 times.
		 */
		if ((addr->tmer&TMER_HARD)==0 && state==SIO) {
			if (++um->um_tab.b_errcnt < 7) {
				sc->sc_blkno++;
				ubadone(um);
				goto opcont;
			}
		} else
			/*
			 * Hard or non-i/o errors on non-raw tape
			 * cause it to close.
			 */
			if (sc->sc_openf>0 && bp == &cstbuf[tmunit])
				sc->sc_openf = -1;
		/*
		 * Couldn't recover error
		 */
		sc->sc_flags |= DEV_HARDERR;
		mprintf("%s: unit# %d: hard error block# %d\n",
			sc->sc_device, unit, bp->b_blkno);
		mprintf("tmer=%b\n", sc->sc_erreg, TMER_BITS);
		bp->b_flags |= B_ERROR;
		goto opdone;
	}

	/*
	 * Advance tape control FSM.
	 */
ignoreerr:
	switch (state) {

	case SIO:
		/*
		 * Read/write increments tape block number
		 */
		sc->sc_blkno++;
		goto opdone;

	case SCOM:
		/*
		 * For forward/backward space record update current
		 * position.
		 */
		if (bp == &cstbuf[tmunit])
		switch (bp->b_command) {

		case TM_SFORW:
			sc->sc_blkno -= bp->b_repcnt;
			break;

		case TM_SREV:
			sc->sc_blkno += bp->b_repcnt;
			break;
		}
		goto opdone;

	case SSEEK:
		sc->sc_blkno = bdbtofsb(bp->b_blkno);
		goto opcont;

	default:
		panic("tmintr");
	}
opdone:
	/*
	 * Reset error count and remove
	 * from device queue.
	 */
	sc->sc_flags |= DEV_DONE;
	um->um_tab.b_errcnt = 0;
	dp->b_actf = bp->av_forw;
	/*
	 * Check resid; watch out for resid >32767 (tmbc not negative).
	 */
	bp->b_resid = ((int) -addr->tmbc) & 0xffff;
	ubadone(um);
	iodone(bp);
	/*
	 * Circulate slave to end of controller
	 * queue to give other slaves a chance.
	 */
	um->um_tab.b_actf = dp->b_forw;
	if (dp->b_actf) {
		dp->b_forw = NULL;
		if (um->um_tab.b_actf == NULL)
			um->um_tab.b_actf = dp;
		else
			um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
	}
	if (um->um_tab.b_actf == 0)
		return;
opcont:
	ststart(um);
}

sttimer(dev)
	register int dev;
{
	register struct st_softc *sc = &st_softc[UNIT(dev)];
	register int tmunit = TMUNIT(dev);
	register int unit = UNIT(dev);
	register short x;

	if (sc->sc_timo != INF && (sc->sc_timo -= 5) < 0) {
		mprintf("%s: unit# %d: lost interrupt\n",
			sc->sc_device, unit);
		sc->sc_timo = INF;
		x = spl5();
		stintr(tmunit);
		(void) splx(x);
	}
	timeout(sttimer, (caddr_t)dev, 5*hz);
}

stread(dev, uio)
	register dev_t dev;
	register struct uio *uio;
{
	register int tmunit = TMUNIT(dev);

	return (physio(ststrategy, &rstbuf[tmunit], dev, B_READ,
		minphys, uio));
}

stwrite(dev, uio)
	register dev_t dev;
	register struct uio *uio;
{
	register int tmunit = TMUNIT(dev);

	return (physio(ststrategy, &rstbuf[tmunit], dev, B_WRITE,
		minphys, uio));
}

stioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register struct uba_device *ui = stdinfo[TMUNIT(dev)];
	register struct st_softc *sc = &st_softc[UNIT(dev)];
	register struct buf *bp = &cstbuf[TMUNIT(dev)];
	register callcount;
	register int fcount;
	struct mtop *mtop;
	struct mtget *mtget;
	struct devget *devget;
	int unit = UNIT(dev);

	/* we depend of the values and order of the MT codes here */
	static tmops[] = { TM_WEOF,TM_SFORW,TM_SREV,TM_SFORW,
			   TM_SREV,TM_REW,TM_OFFL,TM_SENSE };

	switch (cmd) {

	case MTIOCTOP:				/* tape operation */
		mtop = (struct mtop *)data;
		switch (mtop->mt_op) {

		case MTWEOF:
			callcount = mtop->mt_count;
			fcount = 1;
			break;

		case MTFSF: case MTBSF:
			callcount = mtop->mt_count;
			fcount = INF;
			break;

		case MTFSR: case MTBSR:
			callcount = 1;
			fcount = mtop->mt_count;
			break;

		case MTREW: case MTOFFL:
			sc->sc_flags &= ~DEV_EOM;
			callcount = 1;
			fcount = 1;
			break;

		case MTNOP: case MTCACHE: case MTNOCACHE:
		case MTCLX: case MTCLS:
			return(0);

		case MTCSE:
			sc->sc_flags |= DEV_CSE;
			return(0);

		case MTENAEOT:
			dis_eot_st[unit] = 0;
			return(0);

		case MTDISEOT:
			dis_eot_st[unit] = DISEOT;
			sc->sc_flags &= ~DEV_EOM;
			return(0);

		default:
			return (ENXIO);
		}
		if (callcount <= 0 || fcount <= 0)
			return (EINVAL);
		while (--callcount >= 0) {
			stcommand(dev, tmops[mtop->mt_op], fcount);
			if ((mtop->mt_op == MTFSR || mtop->mt_op ==
			    MTBSR) && bp->b_resid)
				return (EIO);
			if ((bp->b_flags & B_ERROR) ||
			    sc->sc_erreg & TMER_BOT)
				break;
		}
		return (geterror(bp));

	case MTIOCGET:				/* tape status */
		mtget = (struct mtget *)data;
		mtget->mt_dsreg = sc->sc_dsreg;
		mtget->mt_erreg = sc->sc_erreg;
		mtget->mt_resid = sc->sc_resid;
		mtget->mt_type = MT_ISTM;
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		devget->category = DEV_TAPE;
		devget->bus = DEV_UB;
		bcopy(DEV_UNKNOWN,devget->interface,DEV_SIZE);
		bcopy(sc->sc_device,devget->device,DEV_SIZE);
		devget->bus_num = ui->ui_ubanum;	/* which UBA	*/
		devget->ctlr_num = ui->ui_ctlr; 	/* which interf.*/
		devget->slave_num = ui->ui_slave;	/* which plug	*/
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,DEV_SIZE);	/* Ultrix "te"	*/
		devget->unit_num = unit;		/* which te??	*/
		devget->soft_count = sc->sc_softcnt;	/* soft er. cnt.*/
		devget->hard_count = sc->sc_hardcnt;	/* hard er. cnt.*/
		devget->stat = sc->sc_flags;		/* status	*/
		devget->category_stat = sc->sc_category_flags;	/* c.st.*/
		break;

	default:
		return (ENXIO);
	}
	return (0);
}

streset(uban)
	register int uban;
{
	register struct uba_ctlr *um;
	register struct uba_device *ui;
	register struct buf *dp;
	register int tm11;
	register int unit;
	struct st_softc *sc;

	for (tm11 = 0; tm11 < nNST; tm11++) {
		if ((um = stminfo[tm11]) == 0 || um->um_alive == 0 ||
		   um->um_ubanum != uban)
			continue;
		um->um_tab.b_active = 0;
		um->um_tab.b_actf = um->um_tab.b_actl = 0;
		if (um->um_ubinfo) {
			mprintf("tm reset");
			mprintf("<%d>", (um->um_ubinfo>>28)&0xf);
			um->um_ubinfo = 0;
		}
		((struct stdevice *)(um->um_addr))->tmcs = TM_DCLR;
		for (unit = 0; unit < nNST; unit++) {
			if ((ui = stdinfo[unit]) == 0 || ui->ui_mi !=
			    um || ui->ui_alive == 0)
				continue;
			sc = &st_softc[unit];
			dp = &stutab[unit];
			dp->b_active = 0;
			dp->b_forw = 0;
			if (um->um_tab.b_actf == NULL)
				um->um_tab.b_actf = dp;
			else
				um->um_tab.b_actl->b_forw = dp;
			um->um_tab.b_actl = dp;
			if (sc->sc_openf > 0)
				sc->sc_openf = -1;
		}
		ststart(um);
	}
}

stdump()
{
	register struct uba_device *ui;
	register struct uba_regs *up;
	register struct stdevice *addr;
	register int blk;
	register int num = maxfree;
	register int start = 0;

	if (stdinfo[0] == 0)
		return (ENXIO);
	ui = PHYS(stdinfo[0], struct uba_device *);
	up = PHYS(ui->ui_hd, struct uba_hd *)->uh_physuba;
/*	ubainit(up);	FARKLE */
	DELAY(500000);
	addr = (struct stdevice *)ui->ui_physaddr;
	stwait(addr);
	addr->tmcs = TM_DCLR | TM_GO;
	while (num > 0) {
		blk = num > DBSIZE ? DBSIZE : num;
		stdwrite(start, blk, addr, up);
		start += blk;
		num -= blk;
	}
	steof(addr);
	steof(addr);
	stwait(addr);
	if (addr->tmcs&TM_ERR)
		return (EIO);
	addr->tmcs = TM_REW | TM_GO;
	stwait(addr);
	return (0);
}

stdwrite(dbuf, num, addr, up)
	register int dbuf;
	register int num;
	register struct stdevice *addr;
	register struct uba_regs *up;
{
	register struct pte *io;
	register int npf;

	stwait(addr);
	io = up->uba_map;
	npf = num+1;
	while (--npf != 0)
		 *(int *)io++ = (dbuf++ | (1<<UBAMR_DPSHIFT) |
				 UBAMR_MRV);
	*(int *)io = 0;
	addr->tmbc = -(num*NBPG);
	addr->tmba = 0;
	addr->tmcs = TM_WCOM | TM_GO;
}

stwait(addr)
	register struct stdevice *addr;
{
	register int s;

	do
		s = addr->tmcs;
	while ((s & TM_CUR) == 0);
}

steof(addr)
	register struct stdevice *addr;
{

	stwait(addr);
	addr->tmcs = TM_WEOF | TM_GO;
}
#endif
