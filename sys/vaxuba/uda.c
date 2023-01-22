#ifndef lint
static char *sccsid = "@(#)uda.c	1.75    ULTRIX  3/3/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
 ************************************************************************

/*
 * uda.c
 *
 * Modification history
 *
 * MSCP/RA??/RC25/RD??/RRD??/RX?? disk driver
 *
 * 02-Mar-1987 - woodward
 *	fixed for runout of timeout slots in generic kernel
 *
 * 22-Jan-1987 - robin
 *	fixed some printf, turned off DEBUG, and fixed slave to work as
 *	per the MSCP spec. The slave code worked by accident,  it was checking
 *	the wrong value as what it checked is not valid if the drive is not 
 *	there.
 *
 * 14-Jan-87 - rr
 *	added test for credits to avoid uballoc if no credits and
 *	modified udgetcp() to zero faster
 *
 * 14-Jan-87 - Robin
 *	Added rrd50 and rx35.  Fixed the media id to use mscp media id
 *	as spec says.
 *
 * 06-Jan-87 - Robin
 *	When a request for map registers fails, the request is moved to the
 *	end of the request Q and the controller is moved to the end of the
 *	controler Q.  This will allow other requests to process while a large
 *	map register request waits for resources.  This allows a process to
 *	hang and not the entire system if the resources are never there.
 *
 * 04-Dec-86 - Robin
 *	Fixed a problem which caused a panic.  If a dual ported drive
 *	generated an attention mesg for a drive that is unknown to
 *	the system the system would crash.
 *
 * 16-Oct-86 - Ricky Palmer
 *	Corrected a missing test in the strategy routine for a drive
 *	that is really offline so that the PT_VALID panic won't occur.
 *
 * 17-Sep-86 - Ricky Palmer
 *	Removed EIO returns from open on NDELAY so that driver works
 *	like all the other disk drivers.
 *
 * 29-Aug-86 - Robin Lewis
 *	Changed the error message to report logical block numbers (LBN)
 *	and not sector numbers (sn).  That will make the documentation
 *	and radisk conform.
 *
 * 26-Aug-86 - rsp (Ricky Palmer)
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 * 15-Aug-86 - prs
 *	Moved the declaration of the uda structure udad[], to
 *	uda_data.c in uddump. This fixes the problem of not
 *	being able to dump to a disk on the second UNIBUS.
 *
 * 14-Aug-86 - map
 *	Clear cmdref field when issuing STCON in uqintr.
 *	Change call to bbr_rsp to continue if response was not
 *	intended for bbr.
 *	Fix loop in udreset to use nNUQ as loop variable.
 *
 * 22-Jul-86 - robin
 *	Fixed a problem in the ioctl routine which allowed the ui structure
 *	to be accessed passed the end of the struct. Added code to handle
 *	duplicate unit attention packets and access path attention packets.
 *
 * 22-Jul-86 - map
 *	Fix problem with 2 consecutive uq devices. The udadevice structure
 *	contains an extra word for the kdb. udprobe must return the actual
 *	size of the structure, in all but the kdb this is:
 *	sizeof(struct udadevice) - sizeof(short).
 *
 * 11-Jul-86 - map
 *	Fix problem with buffered data path when doing ONLINE recovery
 *
 * 11-Jul-86 - ricky palmer
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
 * 26-Jun-86 - map for robin
 *	Fix in udslave for generic boot hang.
 *
 * 25-Jun-86 - map
 *	Clear cmdref number in udgetcp. Needed for BBR.
 *
 * 13-Jun-86   -- jaw	fix to uba reset and drivers.
 *
 * 11-Jun-86 - bjg
 *	Change subid_type in uderror() for controller error
 *
 * 10-Jun-86 - map
 *	Added support for Dynamic Bad Block Replacement.
 *	Added code to release buffered data path if online fails.
 *	Added support for v2.0 error logging.
 *	Fix in ud_auto for wild-carded controllers (woodward)
 *	Added support for radisk utility.
 *
 * 5-Jun-86   -- jaw	changes to config.
 *
 * 27-May-86 - robin
 *	Added error return in open if unit is offline.
 *
 * 22-May-86 - prs
 *	Added partial dump code support, and removed common dump
 *	code.
 *
 * 14-May-86 - afd
 *	Changed CTRLNAME from rqd/uda to uqssp.
 *	In udreset() call TYPE macro to get controller name.
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added gtunt code in open to detect offline condition
 *	correctly. Also added code to fill in actual device name
 *	in devioctl structure. V2.0
 *
 *  9-apr-86  -- jaw	 must save bi error vector info on bisst.
 *
 *  8-Apr-86 -- ricky palmer
 *
 *	Added new DEVIOCGET ioctl request code. V2.0
 *	Added end-of-media (EOM) detection. V2.0
 *
 * 18-mar-86  -- jaw	 br/cvec changed to NOT use registers.
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 18-Nov-85 -- lp
 *	Added ud_burst to STEP3 init to set burst rate. The array
 *	ud_burst is in uda_data.c and each uda50 gets its own burst
 *	rate. Would ultimately like this to be a feature of config.
 *
 * 11-nov-85 -- jaw
 *	fix bug to get rid of uqssp message.  fix to bda dump code.
 *
 * 24-sep-85 -- reilly
 *	Added new ioctl request that will return the default paritition table
 *
 * 19-sep-85 -- jaw
 *	fix to old bug on 750 (also 8200).  Under heavey loads it
 *	is possible for the BDP allocation code to release it before
 *	the transation queues are empty.
 *	Also fix bda dump code.  Interrupt vector must be zero when
 *	resetting the device.
 *
 * 11-sep-85 -- jaw
 *	fix broken dump code for uda and bda.
 *
 * 29-Aug-85 -- jaw
 *	fix a bug in the 750 and 8200 buffer datapath handling.  The
 *	resource was being release on a none empty wait queue.
 *
 * 09-Aug-85 -- rich
 *	Fixed the RA's offline problem and some bugs in
 *	  the timeout code.
 *
 * 26-jul-85 -- jaw
 *	reset of uda drive done wrong in two places.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19 Jun 1985 -- reilly
 *	Remove the switch statement that determined the partition, based
 *	on media type, to an array that can be modified in the uda_data.c
 *	file.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 12 Jun 1985 -- jaw
 *	Added in "ud_auto" routine to allow selecting a device after
 *	booting.
 *
 * 05 Jun 1985 -- rich, jaw
 *	Added BDA support.  This thing is really a uqssp driver and
 *	I added some thinks to clean it up for that reason.
 *
 * 30 Apr 1985 -- reilly
 *	Forgot to initialize a local variable.	This is a fix to a
 *	problem that causes the rc25 to hang during autoconf.
 *
 * 13 Apr 1985 -- rich
 *	Added a timer and requeue logic to detect halted or lost controllers
 *
 * 04 Apr 1985 -- reilly
 *	add supported for 32 logical devices and set no limits on the
 *	slave number except those imposed in the mscp protocol.
 *
 * 20 Mar 1985 -- jaw
 *	 add support for 8200 and bua.
 *
 *
 * 13 Mar. 1985 -- reilly
 *	The udsize routine did not know how to handle the -1 length.  I
 *	have fixed it to do the right things.
 *
 *  7 Mar. 1985 -- darrell
 *	Added bad block reporting code
 *
 *  9 Mar 85 -- rjl
 *	Changed hard error message to put out rqd controller type for MicroVAX
 *	and changed size of rd53 swap partition in data file.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 *  1 Jan. 1985 -- rjl
 *	Moved the defination of the ra_info structure to udareg.h so that
 *	it would be possible to include it in autoconf for the MicroVAX
 *	floppy boots
 *
 * 21 Dec. 1984 -- rjl
 *	Fixed harderr printout to give the right unit and added support
 *	for the rd53 drive.
 *
 * 30 Nov  1984 -- reilly
 *	Fixed up an error message
 *
 * 20 Nov  1984 -- reilly
 *	Fixed a bug in the ioctl call that was passing the wrong partition
 *	size if size was equal to -1.
 *
 * 22 Oct  1984 -- reilly
 * 001 - Added code to start handling the disk partitioning scheme
 *
 * 14 Apr  1984 -- rjl
 *	Added code to print out the geometry information under control
 *	of the debug flag
 *
 *  3 Mar, 1984 -- rjl
 *	Integration of Ultrix-32 version for the RQDX1/RD51/RX50
 *
 * Date:	Jan  30 1984
 *
 * This thing has been beaten beyound belief.
 *
 * 1) When this device is on the same unibus as another DMA device
 * like a versatec or a rk07. the Udstrat routine complains that it still
 * has a buffered data path that it shouldn't.	I don't know why.
 *
 * decvax!rich.
 *
 */

#define UDADEVNUM	(9)		/* entry in bdevsw */
#include "ra.h"
#if NUQ > 0 || defined(BINARY)
/*
 * UQSSP driver
 *
 * Restrictions:
 *
 */

#include "../data/uda_data.c"
#include "../h/proc.h"
#include "../vaxbi/bdareg.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/dump.h"
#include "../h/errlog.h"


/*
 * Controller states
 */
#define S_IDLE	0		/* hasn't been initialized */
#define S_STEP1 1		/* doing step 1 init */
#define S_STEP2 2		/* doing step 2 init */
#define S_STEP3 3		/* doing step 3 init */
#define S_SCHAR 4		/* doing "set controller characteristics" */
#define S_RUN	5		/* running */

char	*uq_name[] = {
		"uda50", /* 0 */
#define 	UDA_TYPE	0
		"rc25", /* 1 */
#define 	RC25_TYPE	1
		"maya", /* 2 */
#define 	MAYA_TYPE	2
		"rux",	/* 3 */
#define 	RUX_TYPE	3
		"UNKNOWN", /* 4 */

		"tu81", /* 5 */
#define 	TU81_TYPE	5
		"uda50a", /* 6 */
#define 	UDA50A_TYPE	6
		"rqdx", /* 7 */
#define 	RQDX_TYPE	7
		"uqssp", /* 8 */
		"uqssp", /* 9 */
		"uqssp", /* 10 */
		"uqssp", /* 11 */
		"uqssp", /* 12 */
		"kda50", /* 13 */
#define 	KDA50_TYPE	13
		"uqssp", /* 14 */
		"uqssp", /* 15 */
		"rrd50", /* 16 */
#define		RRD50_TYPE	16
		"uqssp", /* 17 */
		"kdb50", /* 18 */
#define 	BDA_TYPE	18
		"rqdx3", /* 19 */
#define 	RQDX3_TYPE	19
		"rqdx4", /* 20 */
#define 	RQDX4_TYPE	20
#define 	MAXUQNAME	20
		0
};

#define TYPE(sc)	( (sc)->sc_type < 0 || (sc)->sc_type > MAXUQNAME ? "unknown" : uq_name[(sc)->sc_type])
#define PHYS(addr)	((long) \
		((long)ptob(Sysmap[btop((long)(addr)&0x7fffffff)].pg_pfnum)  \
			| (long)( PGOFSET & (long)(addr) ) ) )

/* this mess decodes the Media type identifier */
static char mscpdev_temp[DEV_SIZE];
#define F_to_C(x,i)	( ((x)->mscp_mediaid) >> (i*5+7) & 0x1f ? ( ( (((x)->mscp_mediaid) >>( i*5 + 7)) & 0x1f) + 'A' - 1): ' ')

int	udaerror = 0;			/* causes hex dump of packets */
int	udadebug = 0;
int	uda_cp_wait = 0;		/* Something to wait on for command */
					/* packets and or credits. */
int	wakeup();
extern	int	hz;			/* Should find the right include */
extern	struct	timeval time;

extern	int	bbr_start();
extern	int	bbr_requeue();
extern	int	bbr_unlink();
extern	int	bbr_rsp();
extern	int	bbr_res_wait();
extern	int	bbr_res_need;
extern	struct el_rec	*ealloc();
extern	int	nNUBA;

#ifdef	DEBUG
#define printd	if (udadebug) printf
#define printd1  if (udadebug > 1) printf
#define printd10	if(udadebug >= 10) printf
#endif

#define SC_TIMERON	0x100
#define SC_TIMERESET	0x200

int	udprobe(), udslave(), udattach(), uqintr();
struct	mscp *udgetcp();
#define CTRLNAME "uq"

u_short udstd[] = { 0772150, 0772550, 0777550, 0 };
struct	uba_driver uqdriver =
{ udprobe, udslave, udattach, 0, udstd, "ra", uddinfo, CTRLNAME, udminfo, 0};

#define b_qsize 	b_resid 	/* queue size per drive, in udutab */
#define b_ubinfo	b_resid 	/* Unibus mapping info, per buffer */

#define isbda(sc)	((sc)->sc_type == BDA_TYPE)
#define SA_W(sc,addr)	*(isbda(sc)? &((addr)->udasaw): &((addr)->udasa) )
#define UQSSP_RESET(sc,addr)	{ if(isbda(sc)){\
		struct bda_regs *bdar= (struct bda_regs *)((long)(addr) - 0xf2); \
		int	dest,vec,errvec; \
		vec  = bdar->bda_biic.biic_int_ctrl; \
		dest = bdar->bda_biic.biic_int_dst; \
		errvec = bdar->bda_biic.biic_err; \
		bisst(&bdar->bda_biic.biic_ctrl);\
		while(bdar->bda_biic.biic_ctrl&BICTRL_BROKE); \
		bdar->bda_biic.biic_int_ctrl = vec& 0x0ffff; \
		bdar->bda_biic.biic_int_dst = dest; \
		bdar->bda_biic.biic_err = errvec; \
		} else { (addr)->udaip = 0; } }

#define DELAYTEN 1000

udprobe(reg, ctlr)
	caddr_t reg;
	int ctlr;
{
	register struct uda_softc *sc = &uda_softc[ctlr];
	struct udadevice *udaddr;
	struct uba_ctlr *um;
	int count;
#ifdef lint
	reg = reg;
	udreset(0); uqintr(0);
#endif
	udaddr = (struct udadevice *) reg;

	if (uba_hd[numuba].uba_type & UBABDA)
		sc->sc_type = BDA_TYPE;
	sc->sc_ivec = (uba_hd[numuba].uh_lastiv -= 4);

	UQSSP_RESET(sc,udaddr);

	count=0;
	while(count < DELAYTEN){	/* wait for at most 10 secs */
		if((udaddr->udasa & UDA_STEP1) != 0)
			break;
		DELAY(10000);
		count=count+1;
	}
	if (count == DELAYTEN) return(0);

	sc->sc_state = S_STEP1;
	SA_W(sc,udaddr)=
		UDA_ERR|(NCMDL2<<11)|(NRSPL2<<8)|UDA_IE|(sc->sc_ivec/4);

	count=0;
	while(count < DELAYTEN && sc->sc_state == S_STEP1) {
		if((udaddr->udasa&UDA_STEP2) != 0)
			break;			/* intr should have */

		DELAY(10000);
		count = count+1;
	}

	if (count == DELAYTEN)	return(0);

	return( isbda(sc)? sizeof(struct udadevice) : sizeof(struct udadevice) - sizeof(short));
}

udslave(ui, reg)
	struct uba_device *ui;
	caddr_t reg;
{
	register struct uba_ctlr *um = udminfo[ui->ui_ctlr];
	register struct uda_softc *sc = &uda_softc[ui->ui_ctlr];
	struct udadevice *udaddr;
	struct	mscp	*mp;
	int	i, x;			/* Something to write into to start */
					/* the uda polling */

#ifdef lint
	ui = ui; reg = reg; i = i;
#endif
	udaddr = (struct udadevice *)um->um_addr;
	if((sc->sc_state != S_RUN) && (sc->sc_state != S_SCHAR)){
		if(!udinit(ui->ui_ctlr))
			return(0);
	}
	/* Here we will wait for the controller */
	/* to come into the run state or go idle.  If we go idle we are in */
	/* touble and I don't yet know what to do so I will punt */

	while(sc->sc_state != S_RUN && sc->sc_state != S_IDLE); /* spin */
	if(sc->sc_state == S_IDLE){	/* The Uda failed to initialize */
		printf("%s%d failed to init\n",TYPE(sc),ui->ui_ctlr);
		return(0);
	}
	/*
	 *	Search for an empty slot based in the controllers space
	 *	and initialize the slave number and ui address so that
	 *	the udrsp routine can handle the request
	 */
	for( x = 0; x < NDPUQ; x++ ) {
		if ( udip[ui->ui_ctlr].ra[x].uda_ui == 0 ) {
			udip[ui->ui_ctlr].ra[x].uda_ui = ui;
			udip[ui->ui_ctlr].ra[x].uda_slave = ui->ui_slave;
			break;
		}
	}
	if( x >= NDPUQ) return(0);
	/* The controller is up so let see if the drive is there! */
	if(0 == (mp = udgetcp(um))){	/* ditto */
		printf("%s%d can't get command packet\n",TYPE(sc),ui->ui_ctlr);
		return(0);
	}
	mp->mscp_opcode = M_OP_GTUNT;	/* This should give us the drive type*/
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_cmdref = (long) ui->ui_slave;
#ifdef	DEBUG
	printd1("%s%d Get unit status slave %d\n",TYPE(sc),ui->ui_ctlr,ui->ui_slave);
#endif
	ra_info[ui->ui_unit].rastatus = 0;	/* set to zero */

	*((long *) mp->mscp_dscptr ) |= UDA_OWN | UDA_INT;/* maybe we should poll*/
	i = udaddr->udaip;
	while(!ra_info[ui->ui_unit].rastatus);	/* Wait for some status */
	if(ra_info[ui->ui_unit].rastatus == M_ST_OFFLN) {
		if (x < NDPUQ) {
			/* don't attach drive if not there */
			udip[ui->ui_ctlr].ra[x].uda_ui = 0;
		}
		return(0);		/* Failed No such drive */
	} else
		return(1);		/* Got it and it is there */
}

udattach(ui)
	register struct uba_device *ui;
{

	if (ui->ui_dk >= 0) {
		dk_mspw[ui->ui_dk] = 1.0 / (60 * 31 * 256);	/* approx */
		bcopy(mscpdev_temp,ra_info[ui->ui_unit].ra_device,
		      strlen(mscpdev_temp));
		ra_info[ui->ui_unit].ra_softcnt = 0;	/* set to zero */
		ra_info[ui->ui_unit].ra_hardcnt = 0;	/* set to zero */
	}
	ui->ui_flags = 0;
}

/*
 * Open a UDA.	Initialize the device and
 * set the unit online.
 */
udopen(dev, flag)
	dev_t dev;
	int flag;
{
	register int unit;
	register struct uba_device *ui;
	register struct uda_softc *sc;
	register struct mscp *mp;
	register struct uba_ctlr *um;
	struct udadevice *udaddr;
	int s,i;
	int udstrategy(),ud_timer(),x;			/* 001 */
	extern quota;

#ifdef lint
	flag = flag; i = i;
#endif
	unit = minor(dev) >> 3;
#ifdef DEBUG
	printd1("udopen unit %d\n",unit);
	if(udadebug>1)DELAY(10000);
#endif
	if (unit >= nNRA || (ui = uddinfo[unit]) == 0 || ui->ui_alive == 0)
		return (ENXIO);

	sc = &uda_softc[ui->ui_ctlr];
	ra_info[unit].ra_flags = 0;		     /* set to zero */
	ra_info[unit].ra_category_flags = 0;	     /* set to zero */
	s = spl5();
	if (sc->sc_state != S_RUN) {
		if (sc->sc_state == S_IDLE)
			if(!udinit(ui->ui_ctlr)){
				printf("%s%d: Controller failed to init\n",
					TYPE(sc),ui->ui_ctlr);
				(void) splx(s);
				return(ENXIO);
			}
		/* wait for initialization to complete */
		timeout(wakeup,(caddr_t)ui->ui_mi,11*hz); /* to be sure*/
		sleep((caddr_t)ui->ui_mi, 0);
		untimeout(wakeup,(caddr_t)ui->ui_mi);	/* to be sure*/
		if (sc->sc_state != S_RUN) {
			(void) splx(s);
			return (EIO);
		}
	}
	/* Bring the drive online */

	um = ui->ui_mi;
	udaddr = (struct udadevice *) um->um_addr;
	(void) splx(s);
	/*
	 * Driver may believe that disk drive is still online, however disk
	 *   could have been unloaded, so do a Get Unit Status
	 *   to see if its really online.  If not clear ui_flags.
	 * We also want to do a Get Unit Status for the drive type.
	 */
	s = spl5();
	while(0 ==(mp = udgetcp(um)))
		{
		uda_cp_wait++;
		sleep(&uda_cp_wait,PSWP+1);
		uda_cp_wait--;
		}
	(void) splx(s);
	mp->mscp_opcode = M_OP_GTUNT;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_cmdref = (long) & ra_info[ui->ui_unit].ratype;
				    /* need to sleep on something */
	ra_info[ui->ui_unit].rastatus = 0;   /* set to zero */
	*((long *) mp->mscp_dscptr ) |= UDA_OWN | UDA_INT;
	i = udaddr->udaip;
	timeout(wakeup,(caddr_t) mp->mscp_cmdref,10 * hz);
	sleep((caddr_t) mp->mscp_cmdref,PSWP+1);
	untimeout(wakeup, (caddr_t) mp->mscp_cmdref);
	if ((ra_info[ui->ui_unit].rastatus & M_ST_MASK) == M_ST_OFFLN) {
		ui->ui_flags = 0;	/* its really offline */
	}

	if( (ui->ui_flags == 0 ) || ( ra_part[unit].pt_valid == 0 ) ){
		/*
		 *	If device volume was turned off
		 *	put it back online
		 */

		if ( ui->ui_flags == 0 ) {	/* 001 */
			s = spl5();
			while(0 ==(mp = udgetcp(um))){
				uda_cp_wait++;
				sleep(&uda_cp_wait,PSWP+1);
				uda_cp_wait--;
			}
			(void) splx(s);

			mp->mscp_opcode = M_OP_ONLIN;
			mp->mscp_unit = ui->ui_slave;
			mp->mscp_cmdref = (long) & ra_info[ui->ui_unit].ratype;
				/* need to sleep on something */
#ifdef	DEBUG
			printd("uda: bring unit %d online\n",ui->ui_unit);
#endif
			s = spl5();
			*((long *) mp->mscp_dscptr ) |= UDA_OWN | UDA_INT ;
			i = udaddr->udaip;
			timeout(wakeup,(caddr_t) mp->mscp_cmdref,10 * hz);
			/* make sure we wake up */
			sleep((caddr_t) mp->mscp_cmdref,PSWP+1); /*wakeup in udrsp() */
			untimeout(wakeup,(caddr_t) mp->mscp_cmdref);
		/* Check if BBR was in progress the last time the drive */
		/* went offline.					*/

			if( bbr_start(ui, -1, 0) == BBR_RET_SUCC ) {
				while(ra_info[ui->ui_unit].bbr_ip == BBR_IN_PROGRESS)
					sleep(&ra_info[ui->ui_unit],PSWP+1);
			}
			(void) splx(s);
		}

		/*
		 *			001
		 *	Make sure that we were able to init the disk. If so,
		 *	we must see if the partition tables are on the pack
		 *	and do the appropriate actions which are to set the
		 *	packs partitions table structure.
		 */

		if ( ui->ui_flags != 0 ) {
			/*
			 *	Set up the default partition tables
			 *	because rsblk will call the strategy
			 *	which requires a valid partition table
			 */

			for( x = 0; x <= 7; x++ ) {
				ra_part[unit].pt_part[x].pi_nblocks =
				    ra_info[unit].ra_sizes[x].nblocks;
				ra_part[unit].pt_part[x].pi_blkoff =
				    ra_info[unit].ra_sizes[x].blkoffs;
			}
			/*
			 *	Indicate that the partition is now valid
			 */

			ra_part[unit].pt_valid = PT_VALID;
			/*
			 *	Go see if the partition tables are on the pack
			 */

			rsblk( udstrategy, dev, &ra_part[unit] );

		}

	}

	if(ui->ui_flags == 0) {
		ra_info[unit].ra_flags |= DEV_OFFLINE;
	}

	if(ra_info[unit].ra_flags & DEV_WRTLCK) {
		ra_info[unit].ra_flags |= DEV_WRTLCK;
	}

	if ((sc->sc_timer & SC_TIMERON) == 0) {
		sc->sc_timer = SC_TIMERON;	/* turn on timer */
		timeout(ud_timer,um->um_ctlr,sc->sc_rate); /* schedule timer starting*/
	}
	return (0);
}

ud_timer(ctlr)
int	ctlr;
{
	register struct uba_ctlr *um = udminfo[ctlr];
	register struct udadevice *udaddr = (struct udadevice *)um->um_addr;
	register struct uda_softc *sc = &uda_softc[ctlr];
	register struct uda *ud = &uda[ctlr];
	struct uba_device *ui;
	int	i, s, zs, z, unit;
	struct	buf	*bp;

#ifdef DEBUG
	printd1("tick %d",ctlr);
#endif DEBUG
	s = spl5();
	if(sc->sc_timer & SC_TIMERESET){
		panic("UQSSP controller failed to reinit\n");
	}
	if(udwtab[ctlr].av_forw == &udwtab[ctlr]) {
		sc->sc_timer = SC_TIMERON;
#ifdef DEBUG
		printd1(" No work\n");
#endif DEBUG
	} else {
		zs = ((z=time.tv_usec-sc->sc_progress.tv_usec)< 0)?z+=1000000,
			time.tv_sec - sc->sc_progress.tv_sec -1:
			time.tv_sec - sc->sc_progress.tv_sec ;
		z /= 10000;
		if((zs > (sc->sc_rate/hz)) ||
			(zs == (sc->sc_rate/hz)) && (z > (sc->sc_rate % hz))){
			/* reset the stupid controller */
#ifdef DEBUG
			printd("%d %d %d %d %d\n",time.tv_sec,time.tv_usec,
				sc->sc_progress.tv_sec,sc->sc_progress.tv_usec,
				sc->sc_rate);
#endif DEBUG
			UQSSP_RESET(sc,udaddr);
			udaerror++;		/* get last fail packet */
			ud_requeue(ctlr);
			udinit(ctlr);
			sc->sc_timer = SC_TIMERON|SC_TIMERESET;
			timeout(ud_timer, ctlr, 15*hz);
			splx(s);
			return;
		} else {
#ifdef DEBUG
				printd1("inc\n%d %d %d %d %d\n",time.tv_sec,time.tv_usec,
				sc->sc_progress.tv_sec,sc->sc_progress.tv_usec,
				sc->sc_rate);
#endif DEBUG
		}
	}

	for (unit = 0; unit < nNRA; unit++) {
		if ((ui = uddinfo[unit]) == 0)
			continue;
		if (ui->ui_alive == 0 || ui->ui_mi != um)
			continue;
			/* found a drive is if it has any request */
		if(udutab[unit].b_actf){
			/* find if drive is on controller queue */
			for(bp=um->um_tab.b_actf;bp!=NULL;
			    bp=bp->b_forw){
				if(bp == &udutab[unit])
					break;
			}
			if(bp == NULL){
				if(udutab[unit].b_active) {
				   udutab[unit].b_active = 0;
#ifdef DEBUG
				   printd("Marking unit %d inactive\n");
#endif DEBUG
				}
			}

			/*
			 * Only link drive to controller if bbr is not
			 * in progress.
			 */
			if( ra_info[ui->ui_unit].bbr_lock ) {
				timeout(ud_timer, ctlr, sc->sc_rate);
				splx(s);
				return;
			}

			if (udutab[unit].b_active == 0) {
				udutab[unit].b_forw = NULL;
				if (um->um_tab.b_actf == NULL)
				    um->um_tab.b_actf = &udutab[unit];
				else
				    um->um_tab.b_actl->b_forw =
					 &udutab[unit];
				um->um_tab.b_actl = &udutab[unit];
				udutab[unit].b_active = 1;
#ifdef DEBUG
				printd("queuing drive\n");
#endif DEBUG
			}
		}
	}
	udstart(um);	/* start up drives for good measure */
	i = udaddr->udaip;	/* tell controller to poll just in case */
	timeout(ud_timer, ctlr, sc->sc_rate);
	splx(s);
}




/*
 * Initialize a UDA.  Set up UBA mapping registers,
 * initialize data structures, and start hardware
 * initialization sequence.
 */
udinit(d)
	int d;
{
	register struct uda_softc *sc;
	register struct uda *ud;
	struct udadevice *udaddr;
	struct uba_ctlr *um;

	sc = &uda_softc[d];
	um = udminfo[d];
	um->um_tab.b_active++;
	ud = &uda[d];
	udwtab[um->um_ctlr].av_forw = &udwtab[um->um_ctlr];
	udaddr = (struct udadevice *)um->um_addr;
		/* bda is not mapped */
	if(sc->sc_mapped == 0) {
	    if(sc->sc_type == BDA_TYPE) {
		sc->sc_uda =(struct uda *) (sc->sc_ubainfo = PHYS(ud));

	    } else {
		/*
		 * Map the communications area and command
		 * and response packets into Unibus address
		 * space.
		 */
		sc->sc_ubainfo = uballoc(um->um_ubanum, (caddr_t)ud,
		    sizeof (struct uda), 0);
		if(uba_hd[um->um_ubanum].uba_type & UBAUVI) {
			sc->sc_uda = (struct uda *)(sc->sc_ubainfo & 0x3fffff);
		} else
			sc->sc_uda = (struct uda *) (sc->sc_ubainfo & 0x3ffff);
	   }
	   sc->sc_mapped = 1;
	}


	/*
	 * Start the hardware initialization sequence.
	 */
	UQSSP_RESET(sc,udaddr);

#ifdef DEBUG
	printd1("udinit 0x%x\n",udaddr);
#endif DEBUG
	while((udaddr->udasa & UDA_STEP1) == 0){
#ifdef	DEBUG
		printd1("udasa 0%o\n",udaddr->udasa);
		if(udadebug > 1)DELAY(100000);
#endif
		if(udaddr->udasa & UDA_ERR)
			return(0);	/* CHECK */
	}
	sc->sc_state = S_STEP1;
	sc->sc_credits = 0;
	SA_W(sc,udaddr)=UDA_ERR|(NCMDL2<<11)|(NRSPL2<<8)|UDA_IE|(sc->sc_ivec/4);
	/*
	 * Initialization continues in interrupt routine.
	 */
	return(1);
}

udstrategy(bp)
	register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	register int unit;
	register struct pt    *rasizes; 	/* 001 */
	int xunit = minor(bp->b_dev) & 07;
	daddr_t sz, maxsz;
	int s;

	sz = (bp->b_bcount+511) >> 9;
	unit = dkunit(bp);
	if (unit >= nNRA)
		goto bad;
	rasizes = &ra_part[unit];		/* 001 */
	ui = uddinfo[unit];
/* Please don't change the following conditional. It must be in here in order for
the open routine to always work correctly. - rsp */
	if ( ra_part[unit].pt_valid != PT_VALID ) {
		if(ra_info[ui->ui_unit].ra_flags & DEV_OFFLINE) {
			goto bad;
		} else {
			panic("udstrategy: invalid partition table ");
		}
	}
#ifdef DEBUG
	printd1("udstrat unit %d\n",unit);
	if(udadebug > 1)DELAY(10000);
#endif
	um = ui->ui_mi;
	if (ui == 0 || ui->ui_alive == 0)
		goto bad;
	if ((maxsz = rasizes->pt_part[xunit].pi_nblocks) < 0)	/* 001 */
		maxsz = ra_info[unit].radsize - rasizes->pt_part[xunit].pi_blkoff; /* 001 */
	if (bp->b_blkno < 0 || bp->b_blkno+sz > maxsz ||
	    rasizes->pt_part[xunit].pi_blkoff >= ra_info[unit].radsize) {  /* 001 */
		ra_info[ui->ui_unit].ra_flags |= DEV_EOM;
		goto bad;
	}
#ifdef DISKLOGGING
	disklog(bp->b_blkno + rasizes->pt_part[minor(bp->b_dev)&7].pi_blkoff,sz,unit,
		bp->b_flags&B_READ == B_READ? 1: 0); /* 001 */
#endif
	s = spl5();
	/*
	 * Link the buffer onto the drive queue
	 */
	dp = &udutab[ui->ui_unit];
	if (dp->b_actf == 0)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	bp->av_forw = 0;

	/*
	 * Link the drive onto the controller queue
	 */
	if( ra_info[ui->ui_unit].bbr_lock ){ /* Only link if bbr not active */
		splx(s);
		return;
	}

	if (dp->b_active == 0) {
		dp->b_forw = NULL;
		if (um->um_tab.b_actf == NULL)
			um->um_tab.b_actf = dp;
		else
			um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
		dp->b_active = 1;
	}
	if (um->um_tab.b_active == 0) {
		if ((um->um_hd->uba_type & (UBABUA|UBA750))
		    && udwtab[um->um_ctlr].av_forw == &udwtab[um->um_ctlr]) {
			if (um->um_ubinfo != 0)
				mprintf("udastrat: ubinfo 0x%x\n",um->um_ubinfo);
			else{
				um->um_ubinfo =
				   uballoc(um->um_ubanum, (caddr_t)0, 0,
					UBA_NEEDBDP);
			}
		}


		(void) udstart(um);
	}

	splx(s);
	return;

bad:
	bp->b_flags |= B_ERROR;
	if(ra_info[ui->ui_unit].ra_flags & DEV_EOM) {
		bp->b_error = ENOSPC;
	}
	iodone(bp);
	return;
}

udstart(um)
	register struct uba_ctlr *um;
{
	register struct buf *bp, *dp, *tp;
	register struct mscp *mp;
	register struct uda_softc *sc;
	register struct uba_device *ui;
	struct	pt  *rasizes;		/* 001 */
	struct udadevice *udaddr;
	struct	uda	*ud = &uda[um->um_ctlr];
	int i,tempi;
	int v;

	sc = &uda_softc[um->um_ctlr];

	um->um_tab.b_active = 1;

   for(;;){
#ifdef DEBUG
	printd1("udstart top %d\n",um->um_ctlr);
	if(udadebug > 1)DELAY(10000);
#endif
	if ((dp = um->um_tab.b_actf) == NULL) {
		/*
		 * All done look for responses and call it quits
		 */
		/*
		 * Only mark controller not active and release buffered
		 * data path if no BBR in progress and no drive active
		 */
		if( (sc->sc_bbr_ip == 0) && (sc->sc_act_vec == 0) ){
			/*
			 * Release Buffered Data Path if we have one
			 */
			if ( um->um_ubinfo ) {
			    if ((um->um_hd->uba_type &(UBA750|UBABUA))
				&& (udwtab[um->um_ctlr].av_forw==&udwtab[um->um_ctlr])) {
					ubarelse(um->um_ubanum, &um->um_ubinfo);
			    }
			}
			um->um_tab.b_active = 0;
		}
		/* Check for response ring transitions lost in the
		 * Race condition
		 */
				break;
	}
	if ((bp = dp->b_actf) == NULL) {
		/*
		 * No more requests for this drive, remove
		 * from controller queue and look at next drive.
		 * We know we're at the head of the controller queue.
		 */
		dp->b_active = 0;
		um->um_tab.b_actf = dp->b_forw;
		continue;
	}
	udaddr = (struct udadevice *)um->um_addr;
	if ((udaddr->udasa&UDA_ERR) || sc->sc_state != S_RUN) {
		if(sc->sc_timer & SC_TIMERESET)
			return;
		printf("Reseting %s%d udasa %o, state %d\n",TYPE(sc)
			  ,um->um_ctlr , udaddr->udasa&0xffff, sc->sc_state);
		ud_requeue(um->um_ctlr);
		udinit(um->um_ctlr);
		return;
	}
	ui = uddinfo[dkunit(bp)];
	rasizes = &ra_part[ui->ui_unit];		/* 001 */
	if (ui->ui_flags == 0) {	/* not online */
		if ((mp = udgetcp(um)) == NULL){
			break;
		}
		mp->mscp_opcode = M_OP_ONLIN;
		mp->mscp_unit = ui->ui_slave;
		sc->sc_act_vec |= (1<<ui->ui_unit);
		dp->b_active = 2;
		um->um_tab.b_actf = dp->b_forw; /* remove from controller q */
#ifdef DEBUG
		printd("%s%d: bring unit %d online\n",TYPE(sc),
			um->um_ctlr, ui->ui_slave);
#endif DEBUG
		*((long *)mp->mscp_dscptr) |= UDA_OWN|UDA_INT;
		i = udaddr->udaip;
		continue;
	}
	if (sc->sc_credits < 2) {	/* avoid lots of work for nothing */
		break;
	}
	if (um->um_hd->uba_type&UBABDA)
			i = UBA_CANTWAIT;
	else if (um->um_hd->uba_type&UBA780)
			i = UBA_NEEDBDP|UBA_CANTWAIT;
	else if (um->um_hd->uba_type&(UBABUA|UBA750))
			i = um->um_ubinfo|UBA_HAVEBDP|UBA_CANTWAIT;
	else if (um->um_hd->uba_type&UBA730)
			i = UBA_CANTWAIT;
	else if (um->um_hd->uba_type&(UBAUVI|UBAUVII))
			i = UBA_CANTWAIT|UBA_MAPANYWAY;

/*	get the buffer mapped */
	if ((i = ubasetup(um->um_ubanum, bp, i)) == 0) {
		if(dp->b_actf != 0)		/* move bp to end of Q */
		{				/* Not last or only entry so move it to end*/
			if(dp->b_actf == bp) 	/* first in Q moved to last */
			{
				dp->b_actl->av_forw = bp;
				dp->b_actl = bp;
				dp->b_actf = bp->av_forw;
				dp->b_actl->av_forw = 0;
			}else			/* bp is in the Q somewhere */
			{
				dp->b_actl->av_forw = bp;
				tp = bp;
						/* find buffer in ahead on bp */
				while ((tp->av_forw != bp ) && (tp->av_forw != 0))
				{
					tp = tp->av_forw;
				}
				if(tp->av_forw != 0)
					tp->av_forw = bp->av_forw;
				bp->av_forw = 0;
				dp->b_actl = bp;
			}
			/*
			 * Move drive to the end of the controller queue
			 */
			if (dp->b_forw != NULL) {
				um->um_tab.b_actf = dp->b_forw;
				um->um_tab.b_actl->b_forw = dp;
				um->um_tab.b_actl = dp;
				dp->b_forw = NULL;
			}
		}
		break;
	}
	if (um->um_hd->uba_type&(UBABUA|UBA750|UBABDA))
		tempi = i & 0xfffffff;	       /* mask off bdp */
	else
		tempi = i;

/*	get a command packet */
	if ((mp = udgetcp(um)) == NULL) {
		ubarelse(um->um_ubanum,&tempi);
		break;
	}
	mp->mscp_cmdref = (long)bp;	/* pointer to get back */
	mp->mscp_opcode = bp->b_flags&B_READ ? M_OP_READ : M_OP_WRITE;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_lbn = bp->b_blkno + rasizes->pt_part[minor(bp->b_dev)&7].pi_blkoff;	/* 001 */
	mp->mscp_bytecnt = bp->b_bcount;
	if( uba_hd[um->um_ubanum].uba_type&UBAUVI) {
		mp->mscp_buffer = (i & 0x3ffff) | UDA_MAP;
		mp->mscp_mapbase = (long)
			&(uba_hd[um->um_ubanum].uh_physuba->uba_map[0]);
	} else if (uba_hd[um->um_ubanum].uba_type&UBABDA) {
		mp->mscp_buffer = (i & 0x3ffff) | UDA_MAP;
		mp->mscp_mapbase = (long)
			PHYS(&(uba_hd[um->um_ubanum].uh_uba->uba_map[0]));

	}
	else
		mp->mscp_buffer = (i & 0x3ffff) | (((i>>28)&0xf)<<24);

	bp->b_ubinfo = tempi;		    /* save mapping info */
	*((long *)mp->mscp_dscptr) |= UDA_OWN|UDA_INT;
#ifdef DEBUG
	printd1("opcode 0%o unit %d lbn %d cnt %d\n",mp->mscp_opcode,mp->mscp_unit,mp->mscp_lbn,mp->mscp_bytecnt);
	if(udadebug>1)DELAY(100000);
#endif
	i = udaddr->udaip;		/* initiate polling */
	dp->b_qsize++;
	if (ui->ui_dk >= 0) {
		dk_busy |= 1<<ui->ui_dk;
		dk_xfer[ui->ui_dk]++;
		dk_wds[ui->ui_dk] += bp->b_bcount>>6;
	}

	/*
	 * Move drive to the end of the controller queue
	 */
	if (dp->b_forw != NULL) {
		um->um_tab.b_actf = dp->b_forw;
		um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
		dp->b_forw = NULL;
	}
	sc->sc_progress = time;/* log progess */
	/*
	 * Move buffer to I/O wait queue
	 */
	dp->b_actf = bp->av_forw;
	dp = &udwtab[um->um_ctlr];
	bp->av_forw = dp;
	bp->av_back = dp->av_back;
	dp->av_back->av_forw = bp;
	dp->av_back = bp;
   }
   for (i = sc->sc_lastrsp;; i++) {
	i %= NRSP;
	if (ud->uda_ca.ca_rspdsc[i]&UDA_OWN)
		break;
	udrsp(um, ud, sc, i);
	ud->uda_ca.ca_rspdsc[i] |= UDA_OWN;
    }
    sc->sc_lastrsp = i;
}

/*
 * UDA interrupt routine.
 */
uqintr(d)
	int d;
{
	register struct uba_ctlr *um = udminfo[d];
	register struct udadevice *udaddr = (struct udadevice *)um->um_addr;
	struct buf *bp;
	register int i;
	register struct uda_softc *sc = &uda_softc[d];
	register struct uda *ud = &uda[d];
	int x;
	struct uda *uud;
	struct mscp *mp;

#ifdef	DEBUG
	printd10("uqintr: state %d, udasa %o\n", sc->sc_state, udaddr->udasa);
#endif
	switch (sc->sc_state) {
	case S_IDLE:
		printf("%s%d: random interrupt ignored\n",TYPE(sc), d);
		return;

	case S_STEP1:
#define STEP1MASK	0174377
#define STEP1GOOD	(UDA_STEP2|UDA_IE|(NCMDL2<<3)|NRSPL2)
		if ((udaddr->udasa&STEP1MASK) != STEP1GOOD) {
			int count = 0;
			while((udaddr->udasa&STEP1MASK) != STEP1GOOD) {
				DELAY(100);
				count++;
				if(count > 10 ) break;
			}
			if(count > 10 ) {
				sc->sc_state = S_IDLE;
				printf("failed to initialize step1: sa %x",
					udaddr->udasa);
				wakeup((caddr_t)um);
				return;
			}
		}
		SA_W(sc,udaddr) = ((int)&sc->sc_uda->uda_ca.ca_ringbase)|
		    (((um->um_hd->uba_type & UBA780) != 0) ? UDA_PI : 0);
		sc->sc_state = S_STEP2;
		return;

	case S_STEP2:
#define STEP2MASK	0174377
#define STEP2GOOD	(UDA_STEP3|UDA_IE|(sc->sc_ivec/4))
		if ((udaddr->udasa&STEP2MASK) != STEP2GOOD) {
			int count = 0;
			while((udaddr->udasa&STEP2MASK) != STEP2GOOD) {
				DELAY(100);
				count++;
				if(count > 10 ) break;
			}
			if(count > 10 ) {
				sc->sc_state = S_IDLE;
				printf("failed to initialize step2: sa %x",
					udaddr->udasa);
				wakeup((caddr_t)um);
				return;
			} 
		}
		SA_W(sc,udaddr) = ((int)&sc->sc_uda->uda_ca.ca_ringbase)>>16;
		sc->sc_state = S_STEP3;
		return;

	case S_STEP3:
#define STEP3MASK	0174000
#define STEP3GOOD	UDA_STEP4
		if ((udaddr->udasa&STEP3MASK) != STEP3GOOD) {
			int count = 0;
			while((udaddr->udasa&STEP3MASK) != STEP3GOOD) {
				DELAY(100);
				count++;
				if(count > 10 ) break;
			}
			if(count > 10 ) {
				sc->sc_state = S_IDLE;
				printf("failed to initialize step3: sa %x",
					udaddr->udasa);
				wakeup((caddr_t)um);
				return;
			} 
		}
		udamicro[d] = udaddr->udasa;
		if(!sc->sc_type)
			sc->sc_type = (udamicro[d]>>4) & 0x7f ;
#ifdef	DEBUG
		printd("Uda%d Version %d model %d\n",d,udamicro[d]&0xF,
			(udamicro[d]>>4) & 0x7F);
#endif
		SA_W(sc,udaddr) = UDA_GO | (udaerror? 2 : 0)
				| ((ud_burst[d] > 0 ? ud_burst[d]%64 : 0) << 2);
		sc->sc_state = S_SCHAR;

		/*
		 * Initialize the data structures.
		 */
		uud = sc->sc_uda;
		for (i = 0; i < NRSP; i++) {
			ud->uda_ca.ca_rspdsc[i] = UDA_OWN|UDA_INT|
				(long)&uud->uda_rsp[i].mscp_cmdref;
			ud->uda_rsp[i].mscp_dscptr = &ud->uda_ca.ca_rspdsc[i];
			ud->uda_rsp[i].mscp_header.uda_msglen = mscp_msglen;
		}
		for (i = 0; i < NCMD; i++) {
			ud->uda_ca.ca_cmddsc[i] = UDA_INT|
				(long)&uud->uda_cmd[i].mscp_cmdref;
			ud->uda_cmd[i].mscp_dscptr = &ud->uda_ca.ca_cmddsc[i];
			ud->uda_cmd[i].mscp_header.uda_msglen = mscp_msglen;
		}
		bp = &udwtab[d];
		bp->av_forw = bp->av_back = bp;
		sc->sc_lastcmd = 1;
		sc->sc_lastrsp = 0;
		mp = &uda[um->um_ctlr].uda_cmd[0];
#ifdef DEBUG
		printd("mp = 0x%x\n",mp);
#endif DEBUG
		mp->mscp_unit = mp->mscp_modifier = 0;
		mp->mscp_flags = mp->mscp_cmdref = 0;
		mp->mscp_bytecnt = mp->mscp_buffer = 0;
		mp->mscp_errlgfl = mp->mscp_copyspd = 0;
		mp->mscp_opcode = M_OP_STCON;
		mp->mscp_cntflgs = M_CF_ATTN|M_CF_MISC|M_CF_THIS;
		*((long *)mp->mscp_dscptr) |= UDA_OWN|UDA_INT;
#ifdef DEBUG
		printd("opcode 0x%x *= 0x%x\n",&mp->mscp_opcode,mp->mscp_opcode);
		DELAY(10000);
		printd("desc 0x%x val 0x%x\n",mp->mscp_dscptr,*mp->mscp_dscptr);
		DELAY(10000);
#endif DEBUG
		i = udaddr->udaip;	/* initiate polling */
#ifdef DEBUG
		printd("opcode 0x%x *= 0x%x\n",&mp->mscp_opcode,mp->mscp_opcode);
#endif DEBUG
		DELAY(10000);
		return;

	case S_SCHAR:
		sc->sc_timer &= ~ SC_TIMERESET;
	case S_RUN:
		break;

	default:
		printf("%s%d: interrupt in unknown state %d ignored\n",
			TYPE(sc), d, sc->sc_state);
		return;
	}

	if (udaddr->udasa&UDA_ERR) {
		printf("%s(%d): fatal error (%o)\n", TYPE(sc),
			d, udaddr->udasa&0xffff);
		UQSSP_RESET(sc,udaddr);
		wakeup((caddr_t)um);
	}

	/*
	 * Check for a buffer purge request.
	 */
	if (ud->uda_ca.ca_bdp) {
		/*
		 * THIS IS A KLUDGE.
		 * Maybe we should change the entire
		 * UBA interface structure.
		 */
		int s = spl6(); 	/* was spl7 but I don't like turning */
					/* off machine checks */
		i = um->um_ubinfo;
#ifdef	DEBUG
		printd1("uda: purge bdp %d\n", ud->uda_ca.ca_bdp);
#endif
		um->um_ubinfo = ud->uda_ca.ca_bdp<<28;
		ubapurge(um);
		um->um_ubinfo = i;
		(void) splx(s);
		ud->uda_ca.ca_bdp = 0;
		SA_W(sc,udaddr) = 0;	  /* signal purge complete */
	}

	/*
	 * Check for response ring transition.
	 */
	ud->uda_ca.ca_rspint = 0;
	for (i = sc->sc_lastrsp;; i++) {
		i %= NRSP;
		if (ud->uda_ca.ca_rspdsc[i]&UDA_OWN)
			break;
		udrsp(um, ud, sc, i);
		ud->uda_ca.ca_rspdsc[i] |= UDA_OWN;
	}
	sc->sc_lastrsp = i;

	/*
	 * Check for command ring transition.
	 */
	if (ud->uda_ca.ca_cmdint) {
#ifdef	DEBUG
		printd1("uda: command ring transition\n");
#endif
		ud->uda_ca.ca_cmdint = 0;
	}
	if(uda_cp_wait)
		wakeup(&uda_cp_wait);

	if ( sc->sc_state == S_RUN )
	(void) udstart(um);
	if( bbr_res_need ) {	/* Some BBR process needs a resource	*/
		struct	uba_device	*ui;
		for( x = 0; x < NDPUQ; x++ ) {
			if ( (ui = udip[um->um_ctlr].ra[x].uda_ui) != 0 ) {
				if( ra_info[ui->ui_unit].res_wait )
					bbr_res_wait( &ra_info[ui->ui_unit] );
			}
		}
	}
}

/*
 * Process a response packet
 */
udrsp(um, ud, sc, i)
	register struct uba_ctlr *um;
	register struct uda *ud;
	register struct uda_softc *sc;
	int i;
{
	register struct mscp *mp;
	struct uba_device *ui = 0;
	struct buf *dp, *bp, nullbp;
	int st, x;
	struct	uba_device *ud_auto();
	int fnd = 0, type;
	struct rapt *rp = radfpt;	/* Initialize the default part table */
	int itemp;

	mp = &ud->uda_rsp[i];
	mp->mscp_header.uda_msglen = mscp_msglen;

	sc->sc_credits += mp->mscp_header.uda_credits & 0xf;  /* just 4 bits?*/
	if ((mp->mscp_header.uda_credits & 0xf0) > 0x10)	/* Check */
		return;
#ifdef	DEBUG
	printd10("udarsp, opcode 0x%x status 0x%x\n",mp->mscp_opcode,mp->mscp_status);
#endif
	/*
	 * If it's an error log message (datagram),
	 * pass it on for more extensive processing.
	 */
	if ((mp->mscp_header.uda_credits & 0xf0) == 0x10) {	/* check */
		uderror(um, (struct mslg *)mp);
		return;
	}

	/*
	 * If it is an end message check to see if anything is set in
	 * the mscp_flags field.  Bad blocks and other media errors are
	 * reported in the mscp_flags field.
	 */

	if(mp->mscp_cmdref == BBR_CMD_REF)
	    {
		/*
		 *	Get uba_device address of device based on slave
		 *	number and controller number.
		 */
		for( x = 0; x < NDPUQ; x++ ) {
			if ( udip[um->um_ctlr].ra[x].uda_slave == mp->mscp_unit &&
			     (ui = udip[um->um_ctlr].ra[x].uda_ui) != 0 ) {
				break;
			}
		}
		if( ui == 0 )
			return;
		if( bbr_rsp( ui, mp ) == NULL) /* If not NULL then not a    */
			return; 	       /* BBR response. Pass it on. */
	    }

	if(((mp->mscp_opcode & M_OP_END) == M_OP_END) && mp->mscp_flags != 0) {
		if ((mp->mscp_flags & M_EF_BBLKR) == M_EF_BBLKR) {
			mprintf("%s%d: unit %d, ",TYPE(sc),um->um_ctlr, mp->mscp_unit);
			mprintf("Bad Block Reported at LBN %d\n",
				mp->mscp_lbn);
			if ((mp->mscp_flags & M_EF_BBLKU) == M_EF_BBLKU)
				mprintf("\t\tMore Bad Blocks NOT reported!\n");
		   /*
		    *	Get uba_device address of device based on slave
		    *	number and controller number.
		    */
		    for( x = 0; x < NDPUQ; x++ ) {
			if ( udip[um->um_ctlr].ra[x].uda_slave == mp->mscp_unit &&
			(ui = udip[um->um_ctlr].ra[x].uda_ui) != 0 ) {
				break;
		}
		    }
		    if( ui == 0 )
			return;
		/*
		 * If unit is write protected fall through to response
		 * processing.
		 */

		    if( bbr_start( ui, mp->mscp_lbn, 0 ) != BBR_RET_WRTPRT) {
			    bbr_unlink( ui );	/* Unlink drive from ctlr queue */
		/*
		 * If not an ACCESS command then requeue the buffer and
		 * return. Else continue with response processing. ACCESS
		 * is only issued by radisk program for online disk
		 * scanning. No bp is associated with the command so a
		 * requeue is not necessary. More response processing is
		 * done in the ACCESS case of the switch statement below.
		 */
			    if( mp->mscp_opcode != (M_OP_ACCES|M_OP_END) ) {
				   bbr_requeue( ui, mp );
				   return;
			     }
		    }
		}
		else
			if ((mp->mscp_flags & M_EF_BBLKU) == M_EF_BBLKU){
				mprintf("%s%d: unit %d, ",TYPE(sc),um->um_ctlr, mp->mscp_unit);
				mprintf("bad blocks detected -- no LBNs available\n");
			}

	}

	st = mp->mscp_status&M_ST_MASK;
	/* The controller interrupts as drive 0 */
	/* this means that you must check for controller interrupts */
	/* before you check to see if there is a drive 0 */
	if((M_OP_STCON|M_OP_END) == mp->mscp_opcode){
		if (st == M_ST_SUCC)
			sc->sc_state = S_RUN;
		else
			sc->sc_state = S_IDLE;
		wakeup((caddr_t)um);
		if(sc->sc_rate == 0 ) {
	  /* this is a the rate at which the watch dog timer for the controller
		will run.  */
			sc->sc_rate = 30 * hz;
		}
		return;
	}
	/*
	 *	Get uba_device address of device based on slave number and
	 *	controller number.
	 */
	for( x = 0; x < NDPUQ; x++ ) {
		if ( udip[um->um_ctlr].ra[x].uda_slave == mp->mscp_unit &&
		     (ui = udip[um->um_ctlr].ra[x].uda_ui) != 0 ) {
			break;
		}
	}

	/*
	 *	If address is zero then we don't have the data structures
	 *	ready for this request.
	 */
	if ( ui == 0 && mp->mscp_opcode != M_OP_AVATN)
		return;

	if ((mp->mscp_unitflgs & M_UF_WRTPH) && (ui != 0)) {
		ra_info[ui->ui_unit].ra_flags |= DEV_WRTLCK;
	}

	switch (mp->mscp_opcode) {

	case M_OP_ONLIN|M_OP_END:
		ra_info[ui->ui_unit].rastatus = mp->mscp_status;
		ra_info[ui->ui_unit].ratype =  mp->mscp_mediaid;
		ra_info[ui->ui_unit].raunitflgs =  mp->mscp_unitflgs;
		dp = &udutab[ui->ui_unit];

		if (st == M_ST_SUCC) {
			/*
			 * Link the drive onto the controller queue
			 */
			dp->b_forw = NULL;
			if (um->um_tab.b_actf == NULL)
				um->um_tab.b_actf = dp;
			else
				um->um_tab.b_actl->b_forw = dp;
			um->um_tab.b_actl = dp;
			ui->ui_flags = 1;	/* mark it online */
			ra_info[ui->ui_unit].radsize=(daddr_t)mp->mscp_untsize;
#ifdef	DEBUG
			printd("uda: unit %d online\n", mp->mscp_unit);
			printd("uda: unit %d online %x %c%c %c%c%c%d\n"
				,mp->mscp_unit, mp->mscp_mediaid
				,F_to_C(mp,4),F_to_C(mp,3),F_to_C(mp,2)
				,F_to_C(mp,1),F_to_C(mp,0)
				,mp->mscp_mediaid & 0x7f);
#endif DEBUG
			/*
			 *	Get media type and then search through
			 *	the default partition table looking for
			 *	a media type that matches. If we find one
			 *	then we know the partition table for that
			 *	media type.
			 */
			type = mp->mscp_mediaid;
			for (; rp->ra_type != 0; rp++ ) {
				if ( type == rp->ra_type ) {
					ra_info[ui->ui_unit].ra_sizes =
						rp->ra_sizes;
					fnd++;	/* indicate we found one */
					break;
				}
			}

			if ( !fnd ) {
				ui->ui_flags = 0;	/* mark it offline */
				ra_info[ui->ui_unit].ratype = 0;
				printf("Don't have a partition table for ");
				printf("a %c%c %c%c%c%d\n"
				,F_to_C(mp,4),F_to_C(mp,3),F_to_C(mp,2)
				,F_to_C(mp,1),F_to_C(mp,0)
				,mp->mscp_mediaid & 0x7f);
				while (bp = dp->b_actf) {
					dp->b_actf = bp->av_forw;
					bp->b_flags |= B_ERROR;
					iodone(bp);
				}
			}
			dp->b_active = 1;
		} else {
			if(dp->b_actf){
				harderr(dp->b_actf,"ra");
				mprintf("status = 0x%x\n",mp->mscp_status);
			}
			ra_info[ui->ui_unit].ra_flags |= DEV_OFFLINE;
			while (bp = dp->b_actf) {
				dp->b_actf = bp->av_forw;
				bp->b_flags |= B_ERROR;
				iodone(bp);
			}
		}
		if(mp->mscp_cmdref!=NULL){/* Seems to get lost sometimes */
			wakeup((caddr_t *) mp->mscp_cmdref);
		}
		sc->sc_act_vec &= ~(1<<ui->ui_unit);
		break;
/*
 * The AVAILABLE ATTENTION messages occurs when the
 * unit becomes available after spinup,
 * marking the unit offline will force an online command
 * prior to using the unit.  Configure it in if we haven't seen it before
 */
	case M_OP_AVATN:
		mprintf("%s(%d): unit %d attention\n",
			 TYPE(sc), um->um_ctlr, mp->mscp_unit);
		if (ui == 0) {
			if (ui = ud_auto(um,mp->mscp_unit,mp)) {
				if (mp->mscp_unitflgs & M_UF_WRTPH) 
					ra_info[ui->ui_unit].ra_flags |= DEV_WRTLCK;
				return;
			}
			else {
				mprintf("drive not configured.");
				break;
			}
		}

		ui->ui_flags = 0;  /* it went offline and we didn't notice */
		ra_info[ui->ui_unit].ratype =  mp->mscp_mediaid;
		break;

	case M_OP_END:
/*
 * An endcode without an opcode (0200) is an invalid command.
 * The mscp specification states that this would be a protocol
 * type error, such as illegal opcodes. The mscp spec. also
 * states that parameter error type of invalid commands should
 * return the normal end message for the command. This does not appear
 * to be the case. An invalid logical block number returned an endcode
 * of 0200 instead of the 0241 (read) that was expected.
 */

		printf("endcd=%o, stat=%o\n", mp->mscp_opcode, mp->mscp_status);
		break;
	case M_OP_READ|M_OP_END:
	case M_OP_WRITE|M_OP_END:
		sc->sc_progress = time;/* log progess */
		bp = (struct buf *)mp->mscp_cmdref;
		ubarelse(um->um_ubanum, (int *)&bp->b_ubinfo);
		/*
		 * Unlink buffer from I/O wait queue.
		 */
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
		if ((um->um_hd->uba_type &(UBA750|UBABUA))){
			if(udwtab[um->um_ctlr].av_forw==&udwtab[um->um_ctlr]
			     && (um->um_tab.b_active == 0)) {
				if (um->um_ubinfo == 0)
					printf("uqintr: um_ubinfo == 0\n");
				else {
					ubarelse(um->um_ubanum, &um->um_ubinfo);
				}
			}
			else ubapurge(um);
		}

		dp = &udutab[ui->ui_unit];
		dp->b_qsize--;
		if (ui->ui_dk >= 0)
			if (dp->b_qsize == 0)
				dk_busy &= ~(1<<ui->ui_dk);
		if (st == M_ST_OFFLN || st == M_ST_AVLBL) {
			ui->ui_flags = 0;	/* mark unit offline */
			/*
			 * Link the buffer onto the front of the drive queue
			 */
			if ((bp->av_forw = dp->b_actf) == 0)
				dp->b_actl = bp;
			dp->b_actf = bp;
			/*
			 * Link the drive onto the controller queue
			 */
			if (dp->b_active == 0) {
				dp->b_forw = NULL;
				if (um->um_tab.b_actf == NULL)
					um->um_tab.b_actf = dp;
				else
					um->um_tab.b_actl->b_forw = dp;
				um->um_tab.b_actl = dp;
				dp->b_active = 1;
			}
			if ((um->um_hd->uba_type & (UBA750|UBABUA))
			    &&	um->um_ubinfo == 0) {
					um->um_ubinfo = uballoc(um->um_ubanum, (caddr_t)0, 0,UBA_NEEDBDP);

			}
#ifdef	DEBUG
			printd("Drive %d status = 0x%x\n",ui->ui_unit,st);
#endif
			break;
		}
		if (st != M_ST_SUCC) {
			if(st == M_ST_WRTPR) {
				ra_info[ui->ui_unit].ra_flags |= DEV_WRTLCK;
			} else {
				ra_info[ui->ui_unit].ra_flags &= ~DEV_WRTLCK;
				prterrmsg(mp, bp);
				harderr(bp, "ra");
				mprintf("status 0%o\n", mp->mscp_status);
			}
			bp->b_flags |= B_ERROR;
		}
		bp->b_resid = bp->b_bcount - mp->mscp_bytecnt;
		iodone(bp);
		break;

	case M_OP_GTUNT|M_OP_END:
		ra_info[ui->ui_unit].rastatus = mp->mscp_status;
		ra_info[ui->ui_unit].ratype =  mp->mscp_mediaid;
		ra_info[ui->ui_unit].raunitflgs =  mp->mscp_unitflgs;
		ra_info[ui->ui_unit].lbnstrk =	mp->mscp_track;
		ra_info[ui->ui_unit].nrct   =  mp->mscp_rctcpys;
		ra_info[ui->ui_unit].rctsize =	mp->mscp_rctsize;
		ra_info[ui->ui_unit].rbnstrk =	mp->mscp_rbns;

		switch(st) {
		case M_ST_OFFLN:
		case M_ST_AVLBL:
		case M_ST_CNTLR:
		case M_ST_DRIVE:
			if (mp->mscp_unitflgs & M_UF_WRTPH) {
				ra_info[ui->ui_unit].ra_flags |= DEV_WRTLCK;
			}
			mscpdev_temp[0] = F_to_C(mp,2);
			mscpdev_temp[1] = F_to_C(mp,1);
			mscpdev_temp[2] = F_to_C(mp,0);
			if(mscpdev_temp[2] == ' ') {
				itemp = 2;
			} else {
				itemp = 3;
			}
			mscpdev_temp[itemp] = ((mp->mscp_mediaid & 0x7f)/10) + '0';
			if(mscpdev_temp[itemp] != '0') {
				itemp++;
			}
			mscpdev_temp[itemp++] = ((mp->mscp_mediaid & 0x7f)%10) + '0';
			mscpdev_temp[itemp] = '\0';
			if(ui->ui_flags){
				mprintf("%c%c%c%d - %d OFFLINE 0x%x\n"
				    ,F_to_C(mp,2),F_to_C(mp,1),F_to_C(mp,0)
				    ,mp->mscp_mediaid & 0x7f
				    ,mp->mscp_unit,mp->mscp_status);
				ui->ui_flags = 0;
			}
		}
		/*
		 * Wakeup needed for GTUNT that is done in open routine
		 */
		if(mp->mscp_cmdref != NULL)
			wakeup((caddr_t *) mp->mscp_cmdref);
		break;

	case M_OP_ACCES|M_OP_END:
		ra_info[ui->ui_unit].acc_status = mp->mscp_status;
		ra_info[ui->ui_unit].acc_flags =  mp->mscp_flags;
		ra_info[ui->ui_unit].acc_badlbn =  mp->mscp_lbn;
		ra_info[ui->ui_unit].acc_bytecnt =  mp->mscp_bytecnt;
		wakeup((caddr_t *) mp->mscp_cmdref);
		break;

	case M_OP_ACPTH:
		break;	/* Ignore this as it was caused by VMS dual-porting. */

	case M_OP_DUPUN:
		printf("%s%d: Duplicate unit number %d\n",TYPE(sc),um->um_ctlr,mp->mscp_unit);
		break;

	default:
		printf("%s%d unknown packet\n",TYPE(sc),um->um_ctlr);
		uderror(um, (struct mslg *)mp);
	}
}


/*
 * Process an error log message
 *
 * Format the errorlog packet and send it to the error log buffer
 *
 */
uderror(um, mp)
	register struct uba_ctlr *um;
	register struct mslg *mp;
{
	register	i;
	struct	el_rec	*elp;
	int		class, type, devtype, unitnum, subidnum;

	if( (elp = ealloc( (sizeof(struct el_bdev)), EL_PRIHIGH)) == EL_FULL)
		return;

	switch (mp->mslg_format) {
	case M_FM_CNTERR:
	case M_FM_BUSADDR:

		class = ELCT_DCNTL;	/* Device controller class	*/
		type = ELMSCP_CNTRL;	/* MSCP controller type 	*/
		unitnum = um->um_ctlr;	/* Controller number		*/
		devtype = (mp->mslg_cntid.val[1] >> 16) & 0xFF;
		if( um->um_ubanum >= nNUBA)
			subidnum = um->um_adpt; /* BI controller	*/
		else
			subidnum = um->um_ubanum; /* Unibus/Qbus controller */
		break;

	case M_FM_DISKTRN:
	case M_FM_SDI:
	case M_FM_SMLDSK:

		class = ELCT_DISK;	/* Disk class			*/
		type = ELDEV_MSCP;	/* MSCP disk type		*/
		devtype = (mp->mslg_unitid.val[1] >> 16) & 0xF;
		unitnum = mp->mslg_unit;/* Unit number			*/
		subidnum = um->um_ctlr; /* Controller number		*/
		break;

	default:

		class = EL_UNDEF;	/* Unknown			*/
		type = EL_UNDEF;	/* Unknown			*/
		devtype = EL_UNDEF;	/* Unknown			*/
		unitnum = EL_UNDEF;	/* Unknown			*/
		subidnum = EL_UNDEF;	/* Unknown			*/
		break;
	}

	LSUBID(elp,
	       class,
	       type,
	       devtype,
	       subidnum,
	       unitnum,
	       (u_long)mp->mslg_format)

	elp->el_body.elbdev.eldevhdr.devhdr_dev = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_flags = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_bcount = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_blkno = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_retrycnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_herrcnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_serrcnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_csr = PHYS(um->um_addr);
	elp->el_body.elbdev.eldevdata.mslg = *(mp);
	EVALID(elp);


}


/*
 * Find an unused command packet
 */
struct mscp *
udgetcp(um)
	register struct uba_ctlr *um;
{
	register struct mscp *mp;
	register struct udaca *cp;
	register struct uda_softc *sc;
	register int i;
	register int s;

	s = spl5();
	cp = &uda[um->um_ctlr].uda_ca;
	sc = &uda_softc[um->um_ctlr];
	/*
	 * If no credits, can't issue any commands
	 * until some outstanding commands complete.
	 */
	i = sc->sc_lastcmd;
	if(((cp->ca_cmddsc[i]&(UDA_OWN|UDA_INT))==UDA_INT)&&
	    (sc->sc_credits >= 2)) {
		sc->sc_credits--;	/* committed to issuing a command */
		cp->ca_cmddsc[i] &= ~UDA_INT;
		mp = &uda[um->um_ctlr].uda_cmd[i];

		/* zero out part of the mscp packet */
		mp->mscp_cmdref = 0;
		mp->mscp_unit = 0;
/*		mp->mscp_xxx1 = 0;			*/
		mp->mscp_opcode = 0;
		mp->mscp_flags = 0;
		mp->mscp_modifier = 0;

		/* zero out part of the mscp_generic structure */
		mp->mscp_bytecnt = 0;
		mp->mscp_buffer = 0;
		mp->mscp_mapbase = 0;
/*		mp->mscp_xxx2 = 0;			*/
		mp->mscp_lbn = 0;

		/* zero out the shadow status */
		mp->mscp_shdwsts = 0;

		sc->sc_lastcmd = (i + 1) % NCMD;
		(void) splx(s);
		return(mp);
	}
	(void) splx(s);
	return(NULL);
}

udread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = minor(dev) >> 3;

	if (unit >= nNRA)
		return (ENXIO);
	return (physio(udstrategy, &rudbuf[unit], dev, B_READ, minphys, uio));
}

udwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = minor(dev) >> 3;

	if (unit >= nNRA)
		return (ENXIO);
	return (physio(udstrategy, &rudbuf[unit], dev, B_WRITE, minphys, uio));
}

/*ARGSUSED*/
udioctl(dev, cmd, data, flag)			/* 001 */
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register struct uba_device *ui;
	register struct uda_softc *sc;
	register struct pt *pt = (struct pt *)data;
	struct dkacc *dkacc = (struct dkacc *)data;
	struct udadevice *udaddr;
	struct mscp *mp;
	struct uba_ctlr *um;
	struct dkop *dkop;
	struct dkget *dkget;
	struct devget *devget;
	int unit;
	int i, s, error;

	unit = minor(dev) >> 3;
	if (unit >= nNRA)
		return (ENXIO);
	ui = uddinfo[unit];
	sc = &uda_softc[ui->ui_ctlr];
	um = ui->ui_mi;
	udaddr = (struct udadevice *) um->um_addr;

	switch (cmd) {

	case DKIOCHDR:	/* do header read/write */
		break;

	case DIOCGETPT:  /* 001 get partition table info */
	case DIOCDGTPT:  /* get default partition table info */

		if ( cmd == DIOCGETPT )
			/*
			 *	Do a structure copy into the user's data area
			 */
			*pt = ra_part[unit];
		else {
			/*
			 *	Get and store the default block count and
			 *	offset
			 */
			for( i = 0; i <= 7; i++ ) {
				pt->pt_part[i].pi_nblocks =
				    ra_info[unit].ra_sizes[i].nblocks;
				pt->pt_part[i].pi_blkoff =
				    ra_info[unit].ra_sizes[i].blkoffs;
			}
		}

		/*
		 *	Change all of the -1 nblocks to the actual value
		 */
		for( i = 0; i <= 7; i++ ) {
			if ( pt->pt_part[i].pi_nblocks == -1 )
				pt->pt_part[i].pi_nblocks =
				  ra_info[unit].radsize - pt->pt_part[i].pi_blkoff;
		}
		pt->pt_magic = PT_MAGIC;
		break;

	case DIOCSETPT: /* 001 set the driver partition tables */
		/*
		 *	Only super users can set the pack's partition
		 *	table
		 */

		if ( !suser() )
			return(EACCES);

		/*
		 *	Before we set the new partition tables make sure
		 *	that it will no corrupt any of the kernel data
		 *	structures
		 */
		if ( ( error = ptcmp( dev, &ra_part[unit], pt ) ) != 0 )
			return(error);

		/*
		 *	Using the user's data to set the partition table
		 *	for the pack
		 */

		ra_part[unit] = *pt;

		/*
		 *	See if we need to update the superblock of the
		 *	"a" partition of this disk
		 */
		ssblk(dev,pt);

		/*
		 *	Just make sure that we set the valid bit
		 */

		ra_part[unit].pt_valid = PT_VALID;
		break;

	case DKIOCDOP:
		if ( !suser() )
			return(EACCES);
		break;

	case DKIOCGET:
		break;

	case DEVIOCGET:
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));
		devget->category = DEV_DISK;
		devget->adpt_num = ui->ui_adpt; 	/* which adapter*/
		devget->nexus_num = ui->ui_nexus;	/* which nexus	*/

		if (ui->ui_hd->uba_type & (UBAUVI|UBAUVII)) {
			devget->bus = DEV_QB;
			devget->bus_num = ui->ui_ubanum;     /* which QB ? */

		} else if (ui->ui_hd->uba_type & (UBABDA)) {
			devget->bus = DEV_BI;
			devget->bus_num = ui->ui_adpt;	     /* which BI ? */
		} else {
			devget->bus = DEV_UB;
			devget->bus_num = ui->ui_ubanum;     /* which UBA ? */
		}

		switch (devget->bus) {

		case DEV_BI:

			switch (sc->sc_type) {
				case RC25_TYPE:
				    bcopy(DEV_KLESI,devget->interface,
					  strlen(DEV_KLESI));
				    break;
				case BDA_TYPE:
				    bcopy(DEV_KDB50,devget->interface,
					  strlen(DEV_KLESI));
				    break;
			}
			break;

		case DEV_CI:
/* Need code to determine if interface is HSC??, HSB??, etc. */
			break;
		case DEV_UB:
			switch (sc->sc_type) {
				case UDA_TYPE:
				    bcopy(DEV_UDA50,devget->interface,
					  strlen(DEV_UDA50));
				    break;
				case RC25_TYPE:
				    bcopy(DEV_KLESI,devget->interface,
					  strlen(DEV_KLESI));
				    break;
				case RUX_TYPE: case MAYA_TYPE:
				    bcopy(DEV_RUX50,devget->interface,
					  strlen(DEV_RUX50));
				    break;
				case UDA50A_TYPE:
				    bcopy(DEV_UDA50A,devget->interface,
					  strlen(DEV_UDA50A));
				    break;
			}
			break;

		case DEV_QB:
			switch (sc->sc_type) {
				case RQDX_TYPE:
				    if (ui->ui_hd->uba_type&UBAUVI) {
					bcopy(DEV_RQDX1,devget->interface,
					      strlen(DEV_RQDX1));
				    } else {
					bcopy(DEV_RQDX2,devget->interface,
					      strlen(DEV_RQDX2));
				    }
				    break;
				case RRD50_TYPE:
				    bcopy(DEV_RRD50,devget->interface,
					  strlen(DEV_RRD50));
				    break;
				case RQDX3_TYPE:
				    bcopy(DEV_RQDX3,devget->interface,
					  strlen(DEV_RQDX3));
				    break;
				case RQDX4_TYPE:
				    bcopy(DEV_RQDX4,devget->interface,
					  strlen(DEV_RQDX4));
				    break;
				case KDA50_TYPE:
				    bcopy(DEV_KDA50,devget->interface,
					  strlen(DEV_KDA50));
				    break;
			}
			break;
		}

		bcopy(ra_info[ui->ui_unit].ra_device,
		      devget->device,
		      strlen(ra_info[ui->ui_unit].ra_device)); /* ra81,rd52,etc..   */
		devget->ctlr_num = ui->ui_ctlr; 	/* which interface   */
		devget->slave_num = ui->ui_slave;	/* which plug	     */
		bcopy(ui->ui_driver->ud_dname,
		      devget->dev_name,
		      strlen(ui->ui_driver->ud_dname)); /* Ultrix "ra"	     */
		devget->unit_num = unit;		/* which ra??	     */
		devget->soft_count =
			ra_info[ui->ui_unit].ra_softcnt;/* soft error count  */
		devget->hard_count =
			ra_info[ui->ui_unit].ra_hardcnt;/* hard error count  */
		devget->stat =
			ra_info[ui->ui_unit].ra_flags;	/* general status    */
		devget->category_stat = DEV_DISKPART;	/* which partition   */
		break;

  case DKIOCACC:
		/*
		 *	Only super users can beat on the pack
		 */


		if ( !suser() )
			return(EACCES);

		ui = uddinfo[unit];
		/*
		 * setup unit info "ui"
		 */
		if (ui->ui_alive == 0)
			return(EACCES);
		switch (dkacc->dk_opcode){
		case ACC_REVEC:
			/*
			* Force LBN  to be re-vectored
			*/
			while(ra_info[ui->ui_unit].bbr_lock == 1)
				sleep(&ra_info[ui->ui_unit],PSWP+1);
			bbr_start(ui,dkacc->dk_lbn, 1);
			while(ra_info[ui->ui_unit].bbr_ip == BBR_IN_PROGRESS)
				sleep(&ra_info[ui->ui_unit],PSWP+1);
			/*
			* return 0 on success
			*/
			dkacc->dk_status = (ra_info[ui->ui_unit].bbr_ip);

		case ACC_SCAN:
			if((sc->sc_state != S_RUN) ){
				if(!udinit(ui->ui_ctlr)){
				return(0);
				}
			}
			if ( ui->ui_flags == 1 ){
				s = spl5();
				while(0 ==(mp = udgetcp(um))){
					uda_cp_wait++;
					sleep(&uda_cp_wait,PSWP+1);
					uda_cp_wait--;
				}
			(void) splx(s);
				mp->mscp_opcode = M_OP_ACCES;
				mp->mscp_unit = ui->ui_slave;
				mp->mscp_lbn = dkacc->dk_lbn;
				mp->mscp_bytecnt = dkacc->dk_length;
				mp->mscp_cmdref = (long) & ra_info[ui->ui_unit].acc_status;
			while(ra_info[ui->ui_unit].bbr_lock == 1)
				sleep(&ra_info[ui->ui_unit],PSWP+1);
			s = spl5();
			*((long *) mp->mscp_dscptr ) |= UDA_OWN | UDA_INT ;
			i = udaddr->udaip;
			sleep((caddr_t) mp->mscp_cmdref,PSWP+1);
			(void) splx(s);
			/*
			   if a bad block was reported wait for bbr and then return
			   so the user level process can reissue the access.
			*/
			if((ra_info[ui->ui_unit].acc_flags & M_EF_BBLKR) == M_EF_BBLKR)
			{
				sleep(&ra_info[ui->ui_unit],PSWP+1);
			}
			dkacc->dk_lbn = ra_info[ui->ui_unit].acc_badlbn;
			dkacc->dk_length = ra_info[ui->ui_unit].acc_bytecnt;
			dkacc->dk_status = ra_info[ui->ui_unit].acc_status;
			dkacc->dk_flags = ra_info[ui->ui_unit].acc_flags;
			}
			break;

		default:
			return(ENXIO);
		}
		break;

	default:
		return (ENXIO);
	}

	return (0);
}
udreset(uban)
	int uban;
{
	register struct uba_ctlr *um;
	register struct uba_device *ui;
	register struct buf *bp, *dp;
	register int unit;
	struct buf *nbp;
	int d;

	for (d = 0; d < nNUQ; d++) {
		if ((um = udminfo[d]) == 0 || um->um_ubanum != uban ||
		    um->um_alive == 0)
			continue;
		printf(" %s%d", TYPE(&uda_softc[d]),d);
		um->um_tab.b_active = 0;
		um->um_tab.b_actf = um->um_tab.b_actl = 0;
		uda_softc[d].sc_state = S_IDLE;
		uda_softc[d].sc_mapped = 0;	/* Rich */
		for (unit = 0; unit < nNRA; unit++) {
			if ((ui = uddinfo[unit]) == 0)
				continue;
				if (ui->ui_alive == 0 || ui->ui_mi != um)
					continue;
				udutab[unit].b_active = 0;
				udutab[unit].b_qsize = 0;
			}
			for (bp = udwtab[d].av_forw; bp != &udwtab[d]; bp = nbp) {
				nbp = bp->av_forw;
				bp->b_ubinfo = 0;
				/*
				 * Link the buffer onto the drive queue
				 */
				dp = &udutab[dkunit(bp)];
				if (dp->b_actf == 0)
					dp->b_actf = bp;
				else
					dp->b_actl->av_forw = bp;
				dp->b_actl = bp;
				bp->av_forw = 0;
				/*
				 * Link the drive onto the controller queue
				 */
				if (dp->b_active == 0) {
					dp->b_forw = NULL;
					if (um->um_tab.b_actf == NULL)
						um->um_tab.b_actf = dp;
					else
						um->um_tab.b_actl->b_forw = dp;
					um->um_tab.b_actl = dp;
					dp->b_active = 1;
				}
			}
			udinit(d);
		}
	}

	extern int dkn;

	struct	uba_device	*ud_auto(um,unit,mp)
	register	struct	uba_ctlr	*um;
	int	unit;
	struct mscp *mp;
	{
		register struct uba_device *ui;
		register int numuba,i,x,savectlr;
		struct uba_driver *udp;
		struct udadevice *udaddr;

		numuba = um->um_ubanum;
		udaddr = (struct udadevice *) um->um_addr;
		udp	=  um->um_driver;

		/* search for driver structure. */
		for (ui = ubdinit; ui->ui_driver; ui++) {
			if (ui->ui_driver != um->um_driver ||
			    ui->ui_alive ||
		    (ui->ui_ctlr != um->um_ctlr && ui->ui_ctlr != '?') ||
		    (um->um_ctlr == '?') ||
		    (ui->ui_ubanum != numuba && ui->ui_ubanum != '?') ||
		    (unit != ui->ui_slave))
			continue;
		savectlr = ui->ui_ctlr;
		ui->ui_ctlr= um->um_ctlr;

		/* found a drive structure...lets search for empty
		   controller slot */
		for( x = 0; x < NDPUQ; x++ ) {
			if ( udip[ui->ui_ctlr].ra[x].uda_ui == 0 ) {
				udip[ui->ui_ctlr].ra[x].uda_ui = ui;
				udip[ui->ui_ctlr].ra[x].uda_slave
							= ui->ui_slave;
				break;
			}
		}
		/* controller is full */
		if (x == NDPUQ){
			ui->ui_ctlr = savectlr;
			continue;
		}
		/* initialize the slave structure */
		ui->ui_alive = 1;
		ui->ui_ctlr = um->um_ctlr;
		ui->ui_ubanum = um->um_ubanum;
		ui->ui_hd = &uba_hd[numuba];
		ui->ui_mi = um;
		ui->ui_addr = um->um_addr;

		ui->ui_physaddr = (char *) PHYS(um->um_addr);
		if (ui->ui_dk && dkn < DK_NDRIVE)
			ui->ui_dk = dkn++;
		else
			ui->ui_dk = -1;

		udp->ud_dinfo[ui->ui_unit] = ui;

		if (ui->ui_dk >= 0)
			dk_mspw[ui->ui_dk] = 1.0 / (60 * 31 * 256);/*approx*/
		ui->ui_flags = 0;

		ra_info[ui->ui_unit].rastatus = 0;
		ra_info[ui->ui_unit].ratype =  mp->mscp_mediaid;

		mprintf("%s%d at %s%d slave %d\n",
			udp->ud_dname, ui->ui_unit,
			udp->ud_mname, um->um_ctlr, ui->ui_slave);

		return(ui);
	}
	return(0);
}


ud_requeue(ctlr)
int ctlr;
{
	register struct uba_ctlr *um = udminfo[ctlr];
	register struct uba_device *ui;
	register struct buf *bp, *dp;
	register int unit;
	struct buf *nbp;
	struct	uda	*ud = &uda[ctlr];
	struct	uda_softc	*sc = &uda_softc[ctlr];
	int	i;

	printf("uq%d being reset\n", ctlr);

	um->um_tab.b_actf = um->um_tab.b_actl = 0;
	uda_softc[ctlr].sc_state = S_IDLE;

	for (i = sc->sc_lastrsp;; i++) {
		i %= NRSP;
		if (ud->uda_ca.ca_rspdsc[i]&UDA_OWN)
			break;
		udrsp(um, ud, sc, i);
		ud->uda_ca.ca_rspdsc[i] |= UDA_OWN;
	}
	sc->sc_lastrsp=i;

	for (unit = 0; unit < nNRA; unit++) {
		if ((ui = uddinfo[unit]) == 0)
			continue;
		if (ui->ui_alive == 0 || ui->ui_mi != um)
			continue;
		udutab[unit].b_active = 0;
		udutab[unit].b_qsize = 0;
		ui->ui_flags = 0;
#ifdef DEBUG
		printd("unit= %d\n",unit);
#endif DEBUG
	}
	for (bp = udwtab[ctlr].av_forw; bp != &udwtab[ctlr]; bp = nbp) {
		nbp = bp->av_forw;
		ubarelse(um->um_ubanum, (int *)&bp->b_ubinfo);
		bp->b_ubinfo = 0;
		/*
		 * Link the buffer onto the drive queue
		 */
		dp = &udutab[dkunit(bp)];
		if (dp->b_actf == 0)
			dp->b_actf = bp;
		else
			dp->b_actl->av_forw = bp;
		dp->b_actl = bp;
		bp->av_forw = 0;
		/*
		 * Link the drive onto the controller queue
		 */
		if (dp->b_active == 0) {
			dp->b_forw = NULL;
			if (um->um_tab.b_actf == NULL)
				um->um_tab.b_actf = dp;
			else
				um->um_tab.b_actl->b_forw = dp;
			um->um_tab.b_actl = dp;
			dp->b_active = 1;
		}
	}
}

#define DBSIZE 32

#define ca_Rspdsc	ca_rspdsc[0]
#define ca_Cmddsc	ca_rspdsc[1]
#define uda_Rsp 	uda_rsp[0]
#define uda_Cmd 	uda_cmd[0]

uddump(dev, dumpinfo)
	dev_t dev;
	struct dumpinfo dumpinfo;
{
	struct udadevice *udaddr;
	struct uda *ud_ubaddr;
	char *start;
	char *start_tmp;
	int num, blk, unit;
	register struct uba_regs *uba;
	register struct uba_device *ui;
	register struct uda *udp;
	register struct pte *io;
	register int i,ubatype;
	struct	uda_softc	*sc;
	struct bda_regs *bdar;

	unit = minor(dev) >> 3;
	if (unit >= nNRA)
		return (ENXIO);
#define phys(cast, addr) ((cast)((int)addr & 0x7fffffff))
	ui = phys(struct uba_device *, uddinfo[unit]);
	sc = phys(struct uda_softc *, &uda_softc[ui->ui_ctlr]);
	if (ui->ui_alive == 0)
		return (ENXIO);
	uba = phys(struct uba_hd *, ui->ui_hd)->uh_physuba;
	ubatype = phys(struct uba_hd *, ui->ui_hd)->uba_type;
	ubainit(uba,ubatype);
	/* need bda */
	udaddr = (struct udadevice *)ui->ui_physaddr;
	DELAY(2000000);
	udp = phys(struct uda *, &udad[ui->ui_ctlr]);

	num = btoc(sizeof(struct uda)) + 1;
	io = &uba->uba_map[NUBMREG-num];

	for(i = 0; i<num; i++)
		*(int *)io++ = UBAMR_MRV|(btop(udp)+i);
	if(ubatype&UBAUVI)
		ud_ubaddr = udp;
	else
		if (sc->sc_type == BDA_TYPE) ud_ubaddr = udp;
	else
		ud_ubaddr = (struct uda *)(((int)udp & PGOFSET)|((NUBMREG-num)<<9));

	bdar = (struct bda_regs *)((long)(udaddr) - 0xf2);
	if(isbda(sc)){
		bisst(&bdar->bda_biic.biic_ctrl);
		while(bdar->bda_biic.biic_ctrl&BICTRL_BROKE);
	}
	else udaddr->udaip = 0;

	while ((udaddr->udasa & UDA_STEP1) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	SA_W(sc,udaddr) = UDA_ERR;
	while ((udaddr->udasa & UDA_STEP2) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	SA_W(sc,udaddr) = (short)&ud_ubaddr->uda_ca.ca_ringbase;
	while ((udaddr->udasa & UDA_STEP3) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	SA_W(sc,udaddr) = (short)(((int)&ud_ubaddr->uda_ca.ca_ringbase) >> 16);
	while ((udaddr->udasa & UDA_STEP4) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	SA_W(sc,udaddr) = UDA_GO;
	udp->uda_ca.ca_Rspdsc = (long)&ud_ubaddr->uda_Rsp.mscp_cmdref;
	udp->uda_ca.ca_Cmddsc = (long)&ud_ubaddr->uda_Cmd.mscp_cmdref;
	udp->uda_Cmd.mscp_bytecnt = udp->uda_Cmd.mscp_buffer = 0;
	udp->uda_Cmd.mscp_cntflgs = 0;
	udp->uda_Cmd.mscp_version = 0;
	if (udcmd(M_OP_STCON, udp, udaddr, sc) == 0) {
		return(EFAULT);
	}
	udp->uda_Cmd.mscp_bytecnt = udp->uda_Cmd.mscp_buffer = 0;
	udp->uda_Cmd.mscp_unit = ui->ui_slave;
	if (udcmd(M_OP_ONLIN, udp, udaddr,sc) == 0) {
		return(EFAULT);
	}

	start = start_tmp = 0;
	if (dumplo < 0)
		return (EINVAL);
	dumpinfo.blkoffs += dumplo;

	/*
	 * If a full dump is being performed, then this loop
	 * will dump all of core. If a partial dump is being
	 * performed, then as much of core as possible will be
	 * dumped, leaving room for the u_area and error logger
	 * buffer. Please note that dumpsys predetermined what
	 * type of dump will be performed.
	 */

	while ((dumpinfo.size_to_dump > 0) || (dumpinfo.partial_dump)) {
		blk = dumpinfo.size_to_dump > DBSIZE ? DBSIZE : dumpinfo.size_to_dump;
		io = uba->uba_map;
		for (i = 0; i < blk; i++)
			*(int *)io++ = (btop(start)+i) | UBAMR_MRV;
		*(int *)io = 0;
		udp->uda_Cmd.mscp_lbn = btop(start_tmp) + dumpinfo.blkoffs;
		udp->uda_Cmd.mscp_unit = ui->ui_slave;
		udp->uda_Cmd.mscp_bytecnt = blk*NBPG;
		if(ubatype&UBAUVI)
			udp->uda_Cmd.mscp_buffer = (long) start_tmp;
		else
		if (sc->sc_type == BDA_TYPE)
			udp->uda_Cmd.mscp_buffer = (long) start_tmp;
		else
			udp->uda_Cmd.mscp_buffer = 0;

		if (udcmd(M_OP_WRITE, udp, udaddr,sc) == 0) {
			return(EIO);
		}
		start += blk*NBPG;
		start_tmp += blk*NBPG;
		dumpinfo.size_to_dump -= blk;
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
	return (0);
}


udcmd(op, udp, udaddr,sc)
	int op;
	register struct uda *udp;
	struct udadevice *udaddr;
	struct	uda_softc	*sc;
{
	int i;

#ifdef	lint
	i = i;
#endif

	udp->uda_Cmd.mscp_opcode = op;
	udp->uda_Rsp.mscp_header.uda_msglen = mscp_msglen;
	udp->uda_Cmd.mscp_header.uda_msglen = mscp_msglen;
	udp->uda_ca.ca_Rspdsc |= UDA_OWN|UDA_INT;
	udp->uda_ca.ca_Cmddsc |= UDA_OWN|UDA_INT;
	if (udaddr->udasa&UDA_ERR)
		printf("%s error udasa (%x)\n",TYPE(sc), udaddr->udasa&0xffff);
	i = udaddr->udaip;
	for (;;) {
		if (udaddr->udasa&UDA_ERR)
			return(0);
		if (udp->uda_ca.ca_cmdint)
			udp->uda_ca.ca_cmdint = 0;
		if (udp->uda_ca.ca_rspint)
			break;
	}
	udp->uda_ca.ca_rspint = 0;
	if (udp->uda_Rsp.mscp_opcode != (op|M_OP_END) ||
	    (udp->uda_Rsp.mscp_status&M_ST_MASK) != M_ST_SUCC) {
		printf("error: com %d opc 0x%x stat 0x%x\ndump ",
			op,
			udp->uda_Rsp.mscp_opcode,
			udp->uda_Rsp.mscp_status);
		return(0);
	}
	return(1);
}

udsize(dev)
	dev_t dev;
{
	int unit = minor(dev) >> 3;
	register struct pt *part_pt = &ra_part[unit];
	struct uba_device *ui;
	int xunit = minor(dev) & 07;
	int rasize;
	struct	size	*rasizes;

	if (unit >= nNRA || (ui = uddinfo[unit]) == 0 || ui->ui_alive == 0
		 || ui->ui_flags == 0)
		return (-1);
	/*
	 *	Sanity check		001
	 */
	if ( ra_part[unit].pt_valid != PT_VALID )
		panic("udsize: invalid partition table ");

	/*
	 *	If the length of the partition is -1 then we must do some
	 *	calculation to find the actual length.
	 */
	if ( ( rasize = part_pt->pt_part[xunit].pi_nblocks) != -1 )
		return ( rasize );
	else
		return ( ra_info[unit].radsize - rasize );
}
#ifdef	DISKLOGGING
extern struct timeval time;
#define MAXLOG 1000
struct dklog {
	struct	timeval tim;
	int	blkno;
	short	sz;
	char	unit;
	char	cmd;
} dklog[MAXLOG], *cur_dklog = dklog;

disklog(bk,sz,unit,cmd)
{
	cur_dklog->blkno = bk;
	cur_dklog->sz = sz;
	cur_dklog->unit = unit;
	cur_dklog->cmd = cmd;
	cur_dklog->tim = time;
	cur_dklog->tim.tv_usec += mfpr(ICR) + 1000000/hz;
	if(cur_dklog->tim.tv_usec >= 1000000) {
		cur_dklog->tim.tv_usec -= 1000000;
		cur_dklog->tim.tv_sec++;
	}
	if(cur_dklog == &dklog[MAXLOG-1] ) {
		cur_dklog = dklog;
	} else cur_dklog++;
}
#endif DISKLOGGING

/*
 *  Print the value of The Status or Event Sub-code field in
 *  an easy to read ENGLISH message.
 */

prterrmsg(mp, bp)
	register struct mscp *mp;
	struct buf *bp;
{
	int no_blk_num = 0;


	mprintf("ra%d%c: ", dkunit(bp), 'a'+(minor(bp->b_dev)&07));

	switch(mp->mscp_status) {
		case M_ST_WRTPR:
			printf("write protected!\n");
			no_blk_num++;
			break;
		case M_ST_AVLBL:
			mprintf("unit available -- but not ONLINE\n");
			no_blk_num++;
			break;
		case M_OL_DUPUNUM:
			printf("duplicate unit number\n");
			no_blk_num++;
			break;
		case M_DT_FEM:
			printf("Force Error Modifier set ");
			printf("LBN %d\n", bp->b_blkno + (mp->mscp_bytecnt/512) +
			ra_info[dkunit(bp)].ra_sizes[(minor(bp->b_dev)&07)].blkoffs);
			no_blk_num++;
			break;
		case M_DT_IVHDR:
			mprintf("Invalid Header ");
			break;
		case M_DT_DST:
			mprintf("Data Sync Timeout ");
			break;
		case M_DT_CECCFLD:
			mprintf("Correctable error in ECC field ");
			break;
		case M_DT_UCECC:
			mprintf("Uncorrectable ECC Error ");
			break;
		case M_DT_1SYMECC:
			mprintf("One Symbol ECC Error ");
			break;
		case M_DT_2SYMECC:
			mprintf("Two Symbol ECC Error ");
			break;
		case M_DT_3SYMECC:
			mprintf("Three Symbol ECC Error ");
			break;
		case M_DT_4SYMECC:
			mprintf("Four Symbol ECC Error ");
			break;
		case M_DT_5SYMECC:
			mprintf("Five Symbol ECC Error ");
			break;
		case M_DT_6SYMECC:
			mprintf("Six Symbol ECC Error ");
			break;
		case M_DT_7SYMECC:
			mprintf("Seven Symbol ECC Error ");
			break;
		case M_DT_8SYMECC:
			mprintf("Eight Symbol ECC Error ");
			break;

		default:
			mprintf("Error, Status and event Sub-code = 0x%x\n",
				mp->mscp_modifier);
	}
	if (!no_blk_num)
		mprintf("LBN %d\n", bp->b_blkno + (mp->mscp_bytecnt/512) +
				    ra_info[dkunit(bp)].ra_sizes[(minor(bp->b_dev)&07)].blkoffs);
}


#endif
