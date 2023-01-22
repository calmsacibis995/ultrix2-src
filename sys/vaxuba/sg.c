#ifndef lint
static char *sccsid = "@(#)sg.c	1.8	ULTRIX	3/3/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985, 86 by			*
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


/***********************************************************************
 *
 * Modification History:
 *  02-Mar-87  rafiey (Ali Rafieymehr)
 *	The video being turned off wasn't fixed correctly before.
 *
 *  11-Feb-87  rafiey (Ali Rafieymehr)
 *	Fixed a bug which was not turning the video on. Cleaned the code
 *	to include ioctls for the color support, and changed the chip
 *	select due to change of the hardware document and more cleanup.
 *
 *  07-Jan-87  rafiey (Ali Rafieymehr)
 *	Added the cursor to the driver and did some cleanup.
 *
 *  13-Dec-86  rafiey (Ali Rafieymehr)
 *	Converted the first pass color driver in to a semi-working
 *	version, which runs of the prototype VAXstar color board.
 *
 *   3-Sep-86  fred (Fred Canter)
 *	Make sure probe fails if CPU not a VAXstation 2200.
 *	Select video interrupt source (instead of sh driver).
 *
 *   2-Jul-86  rafiey (Ali Rafieymehr)
 *	Changed SGMAJOR to 50 and removed unused code.
 *
 *  18-Jun-86  rafiey (Ali Rafieymehr)
 *	Created this VAXstar color driver.
 *	Derived from qd.c.
 *
 **********************************************************************/

#if NSG > 0 || defined(BINARY)
#include "../data/sg_data.c"	/* include external references to data file */

/*
 * Following allow sgputc to function in
 * the CPU in physical mode (during crash dump).
 * One way transition, can't go back to virtual.
 */
 
#define	VS_PHYSNEXUS	0x20080000
#define	VS_PHYSBITMAP	0x30000000
#define	VS_PHYSCURSOR	0x3c000400

int	sg_physmode = 0;
/*
 * Definitions needed to access the VAXstar SLU.
 * Couldn't include sreg.h (too many compiler errors).
 */
#define	sscsr	nb_sercsr
#define	ssrbuf	nb_serrbuf_lpr
#define	sslpr	nb_serrbuf_lpr
#define	sstcr	nb_sertcr.c[0]
#define	sstbuf	nb_sermsr_tdr.c[0]
#define	SS_TRDY		0x8000		/* Transmit ready */
#define	SS_RDONE	0x80		/* Receiver done		*/
#define SS_PE		0x1000		/* Parity error			*/
#define SS_FE		0x2000		/* Framing error		*/
#define SS_DO		0x4000		/* Data overrun error		*/
#define	SINT_ST		0100		/* SLU xmit interrupt bit	*/

/*
 * VAXstar (color option) register address offsets from start of VAXstar (color)
 * address space.
 */

#define	ADDER	0x0000    /* ADDER chip address */
#define	FCC	0x0200    /* Fifo Compression Chip address */
#define	VDAC	0x0300    /* Video DAC address */
#define	CUR	0x0400    /* CURsor chip address */
#define	VRBACK	0x0500    /* Video ReadBACK address */
#define	FIFORAM	0x8000    /* FIFO/template RAM */

/*
 * general defines
 */

#define SGPRIOR (PZERO-1)               /* must be negative */

#define FALSE	0
#define TRUE	1
#define CHAR_S	0xc7
#define CHAR_Q	0xc1


struct	uba_device *sgdinfo[NSG];
struct	mouse_report last_rep;
extern	struct	mouse_report current_rep;	/* now in ss.c */
extern	struct	tty	sm_tty;			/* now in ss.c */
extern	struct	tty	ss_tty[];
struct	tty sg_tty[NSG*4];

int	nsg = NSG*4;

/*
 * macro to create a system virtual page number from system virtual adrs
 */

#define VTOP(x)  (((int)x & ~0xC0000000) >> PGSHIFT) /* convert address */
						     /* to system page # */

/*
 * Definition of the driver for the auto-configuration program.
 */
int	sgprobe(), sgattach(), sgfint(), sgaint(), sgiint();

u_short	sgstd[] = { 0 };
struct	uba_driver sgdriver =
	{ sgprobe, 0, sgattach, 0, sgstd, "sg", sgdinfo };

struct	sg_fifo_space	SG_bufmap[];
/*
 * v_consputc is the switch that is used to redirect the console cnputc to the
 * virtual console vputc.
 * v_consgetc is the switch that is used to redirect the console getchar to the
 * virtual console vgetc.
 */
extern (*v_consputc)();
extern (*v_consgetc)();

#define	CONSOLEMAJOR	0

/*
 * Keyboard state
 */
struct sg_keyboard {
	int shift;			/* state variables	*/
	int cntrl;
	int lock;
	int hold;
	char last;			/* last character	*/
} sg_keyboard;

short sg_divdefaults[15] = { LK_DOWN,	/* 0 doesn't exist */
	LK_AUTODOWN, LK_AUTODOWN, LK_AUTODOWN, LK_DOWN,
	LK_UPDOWN,   LK_UPDOWN,   LK_AUTODOWN, LK_AUTODOWN, 
	LK_AUTODOWN, LK_AUTODOWN, LK_AUTODOWN, LK_AUTODOWN, 
	LK_DOWN, LK_AUTODOWN };

short sg_kbdinitstring[] = {		/* reset any random keyboard stuff */
	LK_AR_ENABLE,			/* we want autorepeat by default */
	LK_CL_ENABLE,			/* keyclick */
	0x84,				/* keyclick volume */
	LK_KBD_ENABLE,			/* the keyboard itself */
	LK_BELL_ENABLE,			/* keyboard bell */
	0x84,				/* bell volume */
	LK_LED_DISABLE,			/* keyboard leds */
	LED_ALL };
#define KBD_INIT_LENGTH	sizeof(sg_kbdinitstring)/sizeof(short)

#define TOY ((time.tv_sec * 100) + (time.tv_usec / 10000))

int	sg_ipl_lo = 1;			/* IPL low flag			*/

extern	u_short	sm_pointer_id;	/* id of pointer device (mouse,tablet)-ss.c */
u_short	sg_mouseon = 0;                	/* Mouse is enable when 1 */
u_short	sg_open = 0;			/* graphics device is open when 1 */
u_short	sg_cur_reg = 0;  	  /* Register to keep track of cursor register bits*/
u_short	sgdebug = 0;			/* Debug is enable when #0 */

char	*sg_fifo_addr;
char	*sg_ptr;
u_short	req_length;
u_short int_flag = -1;
u_short nbytes_req, nbytes_left;

struct proc *rsel;			/* process waiting for select */

int	sgstart(), sgputc(), sggetc(), ttrstrt();

/*
 * Keyboard translation and font tables
 */
extern  char q_key[],q_shift_key[],*q_special[],q_font[];

extern	struct	nexus	nexus[];

/*
 * Default cursor (plane A and plane B)
 *
 */

unsigned  short def_cur[32] = { 

/* plane A */ 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF,
 	      0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF,
/* plane B */ 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF,
              0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF

	};

/*
 * Default colors for the VDAC
 */

	unsigned short def_vdac_colors[16] = {

/* BLACK   */		VDAC_BLACK,
/* BLUE    */		VDAC_BLUE,
/* GREEN   */		VDAC_GREEN,
/* CYAN    */		VDAC_CYAN,
/* RED     */		VDAC_RED,
/* MAGENTA */		VDAC_MAGENTA,
/* YELLOW  */		VDAC_YELLOW,
/* WHITE   */		VDAC_WHITE,
/* GREY_1  */		VDAC_GREY_1,
/* GREY_2  */		VDAC_GREY_2,
/* GREY_3  */		VDAC_GREY_3,
/* GREY_4  */		VDAC_GREY_4,
/* GREY_5  */		VDAC_GREY_5,
/* GREY_6  */		VDAC_GREY_6,
/* GREY_7  */		VDAC_GREY_7,
/* GREY_8  */		VDAC_GREY_8

	};

/******************************************************************
 **                                                              **
 ** Routine to see if the graphic device will interrupt.         **
 **                                                              **
 ******************************************************************/

sgprobe(reg)
	caddr_t reg;
{
	register struct nb_regs *sgaddr = (struct nb_regs *)nexus;
	register struct nb3_regs *sgaddr3 = (struct nb3_regs *)sgmem;
	register struct fcc *sgfcc;
	register struct vdac *sgvdac;

/*
 * Only on a VAXstation 2200 (not MicroVAX 2000)
 */
	if ((cpu != MVAX_II) ||
	    (cpu_subtype != ST_VAXSTAR) ||
	    (vs_cfgtst&VS_MULTU))
		return(0);
/*
 * Only if color option present
 */
	if ((vs_cfgtst&VS_VIDOPT) == 0)
	    return(0);

	sgaddr->nb_vdc_sel = 1;		/* select color option interrupts */
	sgaddr->nb_int_reqclr = SINT_VF;
	sgaddr->nb_int_msk |= SINT_VF;


	if (v_consputc != sgputc) {
	    sgbase = (caddr_t) ((u_long)sgaddr3);
	    sgmap.adder = sgbase + ADDER;
	    sgmap.fcc = sgbase + FCC;
	    sgmap.vdac = sgbase + VDAC;
	    sgmap.cur = sgbase + CUR;
	    sgmap.vrback = sgbase + VRBACK;
	    sgmap.fiforam = sgbase + FIFORAM;
	    sgvdac = (struct vdac *) sgmap.vdac;
	    sgvdac-> mode = 0x47;
	    cursor.x = 0;
	    cursor.y = 0;
	    sg_init_shared();		/* init shared memory */
	}
	sgfcc = (struct fcc *) sgmap.fcc;
/*
 * Initialize the FCC by issuing HALT command (bits 9, 10 cbcsr to zero)
 */

	*(u_long *) &sgfcc->cbcsr = (u_long) HALT;

/* Enable FIFO compression interrupt */

/* set fcc to display list mode */

	sgaddr->nb_int_msk |= SINT_VS;
	sgfcc->cbcsr &= ~FLUSH;
	sgfcc->thresh = 0x100;
	*(unsigned long *) &sgfcc->cbcsr |= (unsigned long)(((sgfcc->icsr|ENTHRSH) << 16)|DL_ENB);
	DELAY(20000);  /* wait  */

	if (cvec && cvec != 0x200)
	    cvec -= 4;
	return(8);
}


/******************************************************************
 **                                                              **
 ** Routine to attach to the graphic device.                     **
 **                                                              **
 ******************************************************************/

sgattach(ui)
	struct uba_device *ui;
{
	register int *pte;
	int	i;


        sg_keyboard.hold = 0; /* "Hold Screen" key is pressed if 1 */

/*
 * init "sgflags"
 */

	sgflags.inuse = 0;		/* init inuse variable EARLY! */
	sgflags.mapped = 0;
	sgflags.kernel_loop = 0;
	sgflags.user_fifo = 0;
	sgflags.curs_acc = ACC_OFF;
	sgflags.curs_thr = 128;
	sgflags.tab_res = 2;		/* default tablet resolution factor */
	sgflags.duart_imask = 0;	/* init shadow variables */
	sgflags.adder_ie = 0;

/*
 * init structures used in kbd/mouse interrupt routine.	This code must
 * come after the "sg_init_shared()" routine has run since that routine inits
 * the eq_header structure used here.
 */

/* init the "latest mouse report" structure */

	last_rep.state = 0;
	last_rep.dx = 0;
	last_rep.dy = 0;
	last_rep.bytcnt = 0;

/* init the event queue (except mouse position) */

	eq_header->header.events = (struct _vs_event *)
				  ((int)eq_header + sizeof(struct sginput));

	eq_header->header.size = MAXEVENTS;
	eq_header->header.head = 0;
	eq_header->header.tail = 0;

/* init single process access lock switch */

	sg_open = 0;


/*
 * Do the following only for the color display.
 */
	if (v_consputc == sgputc) {
/*
 * Map the bitmap for use by users.
 */

	    pte = (int *)(SGMEMmap[0]);
	    for( i=0 ; i<192 ; i++, pte++ )
		*pte = (*pte & ~PG_PROT) | PG_UW | PG_V;

	}
}





/******************************************************************
 **                                                              **
 ** Routine to open the graphic device.                          **
 **                                                              **
 ******************************************************************/

extern struct pdma sspdma[];
extern	int ssparam();

/*ARGSUSED*/
sgopen(dev, flag)
	dev_t dev;
{
	register int unit = minor(dev);
	register struct tty *tp;
	register struct nb_regs *sgiaddr = (struct nb_regs *)nexus;
	register struct vdac *sgvdac;
	register struct fcc *sgfcc;

	sgvdac = (struct vdac *)sgmap.vdac;
	sgfcc = (struct fcc *) sgmap.fcc;

/*
 * The graphics device can be open only by one person 
 */
	if (unit == 1) {
	    if (sg_open != 0)
		return(EBUSY);
	    else
		sg_open = 1;
            sgflags.inuse |= GRAPHIC_DEV;  /* graphics dev is open */
	} else {
            sgflags.inuse |= CONS_DEV;  /* mark console as open */
	}
	if ((unit == 2) && (major(dev) == CONSOLEMAJOR))
	    tp = &sm_tty;
	else
	    tp = &ss_tty[unit];

	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
	    return (EBUSY);
	tp->t_addr = (caddr_t)&sspdma[unit];
	tp->t_oproc = sgstart;

	if ((tp->t_state&TS_ISOPEN) == 0) {
	    ttychars(tp);
	    tp->t_state = TS_ISOPEN|TS_CARR_ON;
	    tp->t_ispeed = B4800;
	    tp->t_ospeed = B4800;
	    if( unit == 0 )
		tp->t_flags = XTABS|EVENP|ECHO|CRMOD;
	    else 
		tp->t_flags = RAW;
	    if(tp != &sm_tty)
		ssparam(unit);
	}
	sgiaddr->nb_int_msk |= SINT_VS;		/* enable interrupts */
/*
 * Process line discipline specific open if its not the mouse.
 */
	if (unit != 1)
	    return ((*linesw[tp->t_line].l_open)(dev, tp));
	else {
	    sg_mouseon = 1;
	    return(0);
	}
}


/******************************************************************
 **                                                              **
 ** Routine to close the graphic device.                         **
 **                                                              **
 ******************************************************************/

/*ARGSUSED*/
sgclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit = minor(dev);

	unit = minor(dev);
	if ((unit == 2) && (major(dev) == CONSOLEMAJOR))
	    tp = &sm_tty;
	else
	    tp = &ss_tty[unit];

/*
 * If unit is not the mouse call the line disc. otherwise clear the state
 * flag, and put the keyboard into down/up.
 */
	if( unit != 1 ){
	    (*linesw[tp->t_line].l_close)(tp);
	    ttyclose(tp);
	    sgflags.inuse &= ~CONS_DEV;
	} else {
	    sg_mouseon = 0;
	    if (sg_open != 1)
		return(EBUSY);
	    else
		sg_open = 0;	 /* mark the graphics device available */
	    sgflags.inuse &= ~GRAPHIC_DEV;
	}
	tp->t_state = 0;
}



/******************************************************************
 **                                                              **
 ** Routine to read from the graphic device.                     **
 **                                                              **
 ******************************************************************/

extern sg_strategy();

sgread(dev, uio)
dev_t dev;
struct uio *uio;
{
	register struct tty *tp;
	register int minor_dev;
	register int unit;

	minor_dev = minor(dev);
	unit = (minor_dev >> 2) & 0x07;

/* If this is the console... */

        if ((minor_dev & 0x03) != 1  &&
             sgflags.inuse & CONS_DEV) {
	    if ((minor_dev == 2) && (major(dev) == CONSOLEMAJOR))
	    	tp = &sm_tty;
	    else
            	tp = &ss_tty[minor_dev];
            return ((*linesw[tp->t_line].l_read)(tp, uio));
        }

/*
 * else this must be a FIFO xfer from user space
 */

        else if (sgflags.inuse & GRAPHIC_DEV) {
           return (physio(sg_strategy, &sgbuf[unit],
                           dev, B_READ, minphys, uio));
        }

}


/******************************************************************
 **                                                              **
 ** Routine to write to the graphic device.                      **
 **                                                              **
 ******************************************************************/

extern sg_strategy();

sgwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	register int minor_dev;
	register int unit;

	minor_dev = minor(dev);
	unit = (minor_dev >> 2) & 0x07;

/* If this is the console... */

        if ((minor_dev & 0x03) != 1  &&
             sgflags.inuse & CONS_DEV) {
	    if ((minor_dev == 2) && (major(dev) == CONSOLEMAJOR))
	    	tp = &sm_tty;
	    else
            	tp = &ss_tty[minor_dev];
            return ((*linesw[tp->t_line].l_write)(tp, uio));
        }

/*
 * else this must be a FIFO xfer from user space
 */

        else if (sgflags.inuse & GRAPHIC_DEV) {
           return (physio(sg_strategy, &sgbuf[unit],
                           dev, B_WRITE, minphys, uio));
        }
}





/******************************************************************
 **                                                              **
 ** Strategy routine to do FIFO                                  **
 **                                                              **
 ******************************************************************/

sg_strategy(bp)
register struct buf *bp;
{

	register int npf, o;
	register struct pte *pte;
	register struct pte *mpte;
	register struct proc *rp;
	register struct fcc *sgfcc;
	register struct adder *sgaddr;
	register u_short *bufp;
	int	s;
	int	unit, i;
	unsigned v;

	unit = (minor(bp->b_dev) >> 2) & 0x07;

	s = spl5();

/*
 * following code figures out the proper ptes to
 * remap into system space so interrupt routine can
 * copy into buf structure.
 */ 
	v = btop(bp->b_un.b_addr);
	o = (int)bp->b_un.b_addr & PGOFSET;
	npf = btoc(bp->b_bcount + o);
	rp = bp->b_flags&B_DIRTY ? &proc[2] : bp->b_proc;
	if ((bp->b_flags & B_PHYS) == 0)
	{
		sg_fifo_addr = bp->b_un.b_addr;			
	}
	else {
		if (bp->b_flags & B_UAREA)
			pte = &rp->p_addr[v];
		else if (bp->b_flags & B_PAGET)
			pte = &Usrptmap[btokmx((struct pte *)bp->b_un.b_addr)];
		else if ((bp->b_flags & B_SMEM)  &&	/* SHMEM */
					((bp->b_flags & B_DIRTY) == 0))
			pte = ((struct smem *)rp)->sm_ptaddr + v;
		else {
			pte = vtopte(rp, v);
		}


		sg_fifo_addr = (char *)((int)SG_bufmap + (int)o); 
		mpte = (struct pte *)sgbufmap; 

		for (i = 0; i< npf; i++) {
			if(pte->pg_pfnum == 0)
				panic("sg: zero pfn in pte");
			*(int *)mpte++ = pte++->pg_pfnum | PG_V | PG_KW;
			mtpr(TBIS, (char *) SG_bufmap + (i*NBPG)); 
		}
		*(int *)mpte = 0;
		mtpr(TBIS, (char *)SG_bufmap + (i * NBPG));
	}

	sgfcc = (struct fcc *) sgmap.fcc;
	sgaddr = (struct adder *) sgmap.adder;
	sg_ptr = sgmap.fiforam;
	sgflags.user_fifo = -1;
	if (!(bp->b_flags & B_READ)) {
	    nbytes_req = bp->b_bcount;
	    bcopy(sg_fifo_addr, sg_ptr, 0x020);
	    sgfcc->put += 0x010;
	    sg_fifo_addr += 0x020;
	    sg_ptr += 0x020;
	    nbytes_left = nbytes_req - 0x020;
	}
	sgfcc->icsr |= ENTHRSH;
	while (sgflags.user_fifo) {
            sleep((caddr_t)&sgflags.user_fifo, SGPRIOR);
        }

        splx(s);
	iodone(bp);
}






/******************************************************************
 **                                                              **
 ** Mouse activity select routine.                               **
 **                                                              **
 ******************************************************************/

sgselect(dev, rw)
dev_t dev;
{

	register int s = spl5();
	register int unit = minor(dev);

	if (unit == 1)
	    switch(rw) {
	    case FREAD:					/* event available */

			if(!(ISEMPTY(eq_header))) {
			    splx(s);
			    return(1);
			}
			rsel = u.u_procp;
			sgflags.selmask |= SEL_READ;
			splx(s);
			return(0);
	    case FWRITE:		/* FIFO done? */

                	if (DMA_ISEMPTY(FIFOheader)) {
                    	    splx(s);
                    	    return(1);          /* return "1" if FIFO is done */
			}
			rsel = u.u_procp;
			sgflags.selmask |= SEL_WRITE;
			splx(s);
			return(0);
	    }
	else
	    return( ttselect(dev, rw) );
}






/******************************************************************
 **                                                              **
 ** Graphic device ioctl routine.                                **
 **                                                              **
 ******************************************************************/

/*ARGSUSED*/
sgioctl(dev, cmd, data, flag)
	dev_t dev;
	int cmd;
	register caddr_t data;
{
	register int *ptep;		/* page table entry pointer */
	register int mapix;		/* QMEMmap[] page table index */
	register struct _vs_event *vep;
	register struct tty *tp;
	register struct color_cursor *pcc;
	register struct adder *sgaddr;
	register struct nb_regs *nbaddr = (struct nb_regs *)nexus;

	struct sgmap *sg;		/* pointer to device map struct */
	struct adder *adder;		/* ADDER reg structure pointer */

	struct prgkbd *cmdbuf;
	struct prg_cursor *curs;
	struct _vs_cursor *pos;

	struct devget *devget;
	register int unit = minor(dev);

	int error;
	int s;

	int i;				/* SIGNED index */
	int sbr;			/* SBR variable (you silly boy) */
	u_int ix;

	short status;
	short *shortp;			/* generic pointer to a short */
	char *chrp;			/* generic character pointer */

	short *temp;			/* a pointer to template RAM */

/*
 * service the VAXstar color device ioctl commands
 */

	switch (cmd) {


	    case QD_MAPDEVICE:
		sg = (struct sgmap *) &sgmap;
		bcopy(sg, data, sizeof(struct sgmap));
		    break;

	    case QD_MAPCOLOR:

		sgflags.mapped |= MAPCOLOR;
		ptep = (int *) ((VTOP(color_buf) * 4)
				+ (mfpr(SBR) | 0x80000000));

		/* allow user write to color map write buffer */

		*ptep++ = (*ptep & ~PG_PROT) | PG_UW | PG_V;
		*ptep = (*ptep & ~PG_PROT) | PG_UW | PG_V;

		mtpr(TBIA, 0);			/* clr CPU translation buf */

		sgaddr = (struct adder *) sgmap.adder;

		sgflags.adder_ie |= VSYNC;
		sgaddr->interrupt_enable = sgflags.adder_ie;

		*(int *)data = (int) color_buf;
		break;


	    /*--------------------------------------------------------------
	    * unmap shared color map write buffer and kill VSYNC intrpts */

	    case QD_UNMAPCOLOR:

		if (sgflags.mapped & MAPCOLOR) {

		    sgflags.mapped &= ~MAPCOLOR;

		    ptep = (int *) ((VTOP(color_buf) * 4)
				    + (mfpr(SBR) | 0x80000000));

		    /* re-protect color map write buffer */

		    *ptep++ = (*ptep & ~PG_PROT) | PG_KW | PG_V;
		    *ptep = (*ptep & ~PG_PROT) | PG_KW | PG_V;

		    mtpr(TBIA, 0);	/* smash CPU's translation buf */

		    sgaddr = (struct adder *) sgmap.adder;

		    sgflags.adder_ie &= ~VSYNC;
		    sgaddr->interrupt_enable = sgflags.adder_ie;
		}
		break;


	    case QD_MAPSCROLL:

		sgflags.mapped |= MAPSCR;
		ptep = (int *) ((VTOP(scroll) * 4)
				+ (mfpr(SBR) | 0x80000000));

		/* allow user write to scroll area */

		*ptep = (*ptep & ~PG_PROT) | PG_UW | PG_V;

		mtpr(TBIA, 0);			/* clr CPU translation buf */

		scroll->status = 0;

		sgaddr = (struct adder *) sgmap.adder;

		sgflags.adder_ie |= FRAME_SYNC;
		sgaddr->interrupt_enable = sgflags.adder_ie;

		*(int *)data = (int) scroll;
		break;


	    /*-------------------------------------------------------------
	    * unmap shared scroll param area and disable scroll intrpts */

	    case QD_UNMAPSCROLL:

		if (sgflags.mapped & MAPSCR) {

		    sgflags.mapped &= ~MAPSCR;

		    ptep = (int *) ((VTOP(scroll) * 4)
				    + (mfpr(SBR) | 0x80000000));

		    /* re-protect 512 scroll param area */

		    *ptep = (*ptep & ~PG_PROT) | PG_KW | PG_V;

		    mtpr(TBIA, 0);	/* smash CPU's translation buf */

		    sgaddr = (struct adder *) sgmap.adder;
		    sgflags.adder_ie &= ~FRAME_SYNC;
		    sgaddr->interrupt_enable = sgflags.adder_ie;
		}
		break;



           /*---------------------------------------------
            * give user write access to the event queue */

            case QD_MAPEVENT:

		sgflags.mapped |= MAPEQ;
                ptep = (int *) ((VTOP(eq_header) * 4)
                                + (mfpr(SBR) | 0x80000000));

                /* allow user write to 1K event queue */

                *ptep++ = (*ptep & ~PG_PROT) | PG_UW | PG_V;
                *ptep = (*ptep & ~PG_PROT) | PG_UW | PG_V;

                mtpr(TBIA, 0);                  /* clr CPU translation buf */

                /* return event queue address */

                *(int *)data = (int) eq_header;
                break;


           /*-------------------------------------
            * do setup for FIFO by user process  */

            case QD_MAPIOBUF:

               /*------------------------------------------------
                * set 'user write enable' bits for FIFO buffer  */

                sgflags.mapped |= MAPFIFO;

                ptep = (int *) ((VTOP(FIFOheader) * 4)
                               + (mfpr(SBR) | 0x80000000));

                for (i = (FIFObuf_size >> PGSHIFT); i > 0; --i)
                    *ptep++ = (*ptep & ~PG_PROT) | PG_UW | PG_V;

                mtpr(TBIA, 0);                  /* clr CPU translation buf */

                /*----------------------
               * return I/O buf adr */

                *(int *)data = (int) FIFOheader;
                break;


	    case QD_RDCONFIG:
		break;

	    case QD_GETEVENT:	/* extract the oldest event from event queue */

		if (ISEMPTY(eq_header)) {
		    vep = (struct _vs_event *) data;
		    vep->vse_device = VSE_NULL;
		    break;
		}

		vep = (struct _vs_event *) GETBEGIN(eq_header);
		s = spl5();
		GETEND(eq_header);
		splx(s);
		bcopy(vep, data, sizeof(struct _vs_event));
		break;


	    case QD_RESET:    /* init the dragon, DUART, and driver variables */

		sg_init_shared();		/* init shared memory */
		sg_setup_dragon();	/* init the ADDER/VIPER stuff */
		sg_clr_screen();
		sg_load_cursor( def_cur);	/* load default cursor map */
		sg_ld_font();			/* load the console font */
		break;


	    case QD_SET:	/* init the DUART and driver variables */

		sg_init_shared();
		break;


	    case QD_CLRSCRN:	/* clear the screen. This reinits the dragon */

		sg_setup_dragon();
		sg_clr_screen();
		break;


	    case QD_WTCURSOR:	/* load a cursor into template RAM */

		sg_load_cursor(data);
		break;

/*	    case QD_RDCURSOR:

		break;
*/


	    case QD_POSCURSOR:		/* position the mouse cursor */

		pos = (struct _vs_cursor *) data;
		pcc = (struct color_cursor *) sgmap.cur;
		s = spl5();
	        pcc->xpos = CURS_MIN_X + pos->x;
	        pcc->ypos = CURS_MIN_Y + pos->y;
		eq_header->curs_pos.x = pos->x;
		eq_header->curs_pos.y = pos->y;
		splx(s);
		break;

	    /*--------------------------------------
	    * set the cursor acceleration factor */

	    case QD_PRGCURSOR:

		curs = (struct prg_cursor *) data;
		s = spl5();
		sgflags.curs_acc = curs->acc_factor;
		sgflags.curs_thr = curs->threshold;
		splx(s);
		break;


	    /*--------------------------------------
   	     * pass caller's programming commands to LK201 */

	    case QD_PRGKBD:

		cmdbuf = (struct prgkbd *)data;     /* pnt to kbd cmd buf */

		sg_key_out (cmdbuf->cmd);
		
/*
 * Send param1?
 */
		if (cmdbuf->cmd & LAST_PARAM)
		    break;
		sg_key_out (cmdbuf->param1);

/*
 * Send param2?
 */
		if (cmdbuf->param1 & LAST_PARAM)
		    break;
		sg_key_out (cmdbuf->param2);
		break;


	    /*--------------------------------------
   	     * pass caller's programming commands to mouse */

	    case QD_PRGMOUSE:

		break;



	    case QD_KERN_LOOP:		/* redirect kernel messages */
		sgflags.kernel_loop = -1;
		break;

	    case QD_KERN_UNLOOP:	/* don't redirect kernel messages */

		sgflags.kernel_loop = 0;
		break;


	    case QD_PRGTABRES:      /* program the tablet resolution factor*/

		sgflags.tab_res = *(short *)data;
		break;

	    case DEVIOCGET:			    /* device status */
		    devget = (struct devget *)data;
		    bzero(devget,sizeof(struct devget));
		    devget->category = DEV_TERMINAL;
		    devget->bus = DEV_QB;
		    bcopy(DEV_VCB02,devget->interface,
			  strlen(DEV_VCB02));
		    if(unit == 0)
		    	bcopy(DEV_VR290,devget->device,
			  strlen(DEV_VR290));		    /* terminal */
		    else if(sm_pointer_id == MOUSE_ID)
		    	bcopy(DEV_MOUSE,devget->device,
			  strlen(DEV_MOUSE));
		    else if(sm_pointer_id == TABLET_ID)
		    	bcopy(DEV_TABLET,devget->device,
			  strlen(DEV_TABLET));
		    else
		    	bcopy(DEV_UNKNOWN,devget->device,
			  strlen(DEV_UNKNOWN));
		    devget->adpt_num = 0;          	    /* no adapter*/
		    devget->nexus_num = 0;           	    /* fake nexus 0 */
		    devget->bus_num = 0;            	    /* No bus   */
		    devget->ctlr_num = 0;    	    	    /* cntlr number */
		    devget->slave_num = unit;		    /* which line   */
		    bcopy("sg", devget->dev_name, 3);	    /* Ultrix "sg" */
		    devget->unit_num = unit;		    /* sg line?     */
		    devget->soft_count = 0;		    /* soft er. cnt.*/
		    devget->hard_count = 0;		    /* hard er cnt. */
		    devget->stat = 0;           	    /* status	    */
		    devget->category_stat = 0;		    /* cat. stat.   */
		    break;

	    default:


		    tp = &ss_tty[unit];
		    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
		    if (error >= 0)
			return(error);

		    error = ttioctl(tp, cmd, data, flag);
		    if (error >= 0)
			return(error);
		    break;
	}

	return(0);
}





/******************************************************************
 **                                                              **
 ** ADDER interrupt routine.                                     **
 **                                                              **
 ******************************************************************/

sgaint(sg)
register int sg;
{
	register struct	adder	*sgaddr;
	register struct fcc	*sgfcc;
	register struct nb_regs *nbaddr = (struct nb_regs *)nexus;
	register struct	vdac	*sgvdac;
	struct color_buf *cbuf;

	short status;
	int i;
	register struct rgb *rgbp;

	spl4(); 			/* allow interval timer in */

	sgaddr = (struct adder *) sgmap.adder;
	sgfcc = (struct fcc *) sgmap.fcc;

/*
 * service the vertical blank interrupt (VSYNC bit) by loading any pending
 * color map load request
 */

	if (sgaddr->status & ADDRESS_COMPLETE) {
	    if (sgflags.adder_ie & ADDRESS_COMPLETE) {
	    	sgfcc->cbcsr |= FLUSH;
	    	if (sgfcc->fwused > 0) {
	    	    bcopy(sg_ptr, sg_fifo_addr, sgfcc->fwused*2);
	    	    sgfcc->get += sgfcc->fwused;
	    	    sg_fifo_addr += (sgfcc->fwused * 2);
	    	    sg_ptr += (sgfcc->fwused * 2);
	    	}
	    	sgflags.adder_ie &= ~ADDRESS_COMPLETE;
	    	sgaddr->interrupt_enable = sgflags.adder_ie;
	    	while (!(sgfcc->cbcsr & IDLE));
	    	if (sgfcc->fwused > 0) {
	    	    bcopy(sg_ptr, sg_fifo_addr, sgfcc->fwused*2);
	    	    sgfcc->get += sgfcc->fwused;
	    	    sg_fifo_addr += (sgfcc->fwused * 2);
	    	    sg_ptr += (sgfcc->fwused * 2);
	    	}
		if (sgflags.user_fifo) {
		    sgflags.user_fifo = 0;
		    wakeup((caddr_t)&sgflags.user_fifo);
		}
	    	sgfcc->cbcsr = (short) HALT;
	    }
	}
	if (sgaddr->status & VSYNC) {
	    sgaddr->status &= ~VSYNC;	/* clear the interrupt */

	    cbuf = color_buf;
	    if (cbuf->status & LOAD_COLOR_MAP) {
		sgvdac = (struct vdac *) sgmap.vdac;

		for (i = cbuf->count, rgbp = cbuf->rgb; --i >= 0; rgbp++) {

		    status = rgbp->green << 8;
		    status |= (rgbp->blue << 4);
		    status |= rgbp->red;
		    sgvdac->a_color_map[rgbp->offset] = status;
		}

		cbuf->status &= ~LOAD_COLOR_MAP;
	    }
	}

/*
 * service the scroll interrupt (FRAME_SYNC bit)
 */

	if (sgaddr->status & FRAME_SYNC) {
	    sgaddr->status &= ~FRAME_SYNC; /* clear the interrupt */

	    if (scroll->status & LOAD_REGS) {

		for ( i = 1000, sgaddr->status = 0
		    ; i > 0  &&  !((status = sgaddr->status)
			 & ID_SCROLL_READY)

		    ; --i);

		if (i == 0) {
		    cprintf("\nsg: sgaint: timeout on ID_SCROLL_READY");
		    return;
		}

		sgaddr->ID_scroll_data = scroll->viper_constant;
		sgaddr->ID_scroll_command = ID_LOAD | SCROLL_CONSTANT;

		sgaddr->y_scroll_constant = scroll->y_scroll_constant;
		sgaddr->y_offset_pending = scroll->y_offset;

		if (scroll->status & LOAD_INDEX) {

		    sgaddr->x_index_pending = scroll->x_index_pending;
		    sgaddr->y_index_pending = scroll->y_index_pending;
		}

	    scroll->status = 0x00;
	    }
	}
}





/******************************************************************
 **                                                              **
 ** FCC (FIFO) interrupt routine.                                **
 **                                                              **
 ******************************************************************/

sgfint(sg)
	int sg;
{
	register struct fcc *sgfcc;
	register struct adder *sgaddr;
	register struct FIFOreq_header *header;
	register struct FIFOreq *request;
	int	unit;
	u_short	csr;
	u_short	*sgfifo;
	u_short	*temp;

	spl4();
	unit = sg<<2;

	header = FIFOheader;
	sgfcc = (struct fcc *) sgmap.fcc;
	sgaddr = (struct adder *) sgmap.adder;

	if (int_flag == -1) {    
	    if (DMA_ISIGNORE(header)) {
	    	DMA_CLRIGNORE(header);
	    	return;
	    }
	}

	if (sgflags.user_fifo == -1) {
	    csr = sgfcc->cbcsr;
	    if ((csr & PTB_ENB) == PTB_ENB) {
	    	if (nbytes_left > 0) {
		    bcopy(sg_fifo_addr, sg_ptr, nbytes_left);
		    sgfcc->put += (nbytes_left / 2);
		    sg_fifo_addr += nbytes_left;
	    	    sg_ptr += nbytes_left;
		    nbytes_left -= nbytes_left;
		    if (sgfcc->fwused < sgfcc->thresh)
		    	sgfcc->thresh = sgfcc->fwused;
		    sgfcc->icsr &= ~ENTHRSH;
		    sgfcc->icsr |= ENIDLE;
	    	    sgfcc->cbcsr |= FLUSH;
	    	} else {
		    sg_wait_status(sgaddr, ADDRESS_COMPLETE);
	    	    if (sgflags.user_fifo) {
			sgflags.user_fifo = 0;
			wakeup((caddr_t)&sgflags.user_fifo);
	    	    }
	    	    sgfcc->cbcsr = (short) HALT;
	        }
	    } else {
		if ((csr & BTP_ENB) == BTP_ENB) {
loop:
		    if (sgfcc->fwused > 512) {
	    		bcopy(sg_ptr, sg_fifo_addr, 1024);
	    		sgfcc->get += 512;
			sg_fifo_addr += 1024;
	    		sg_ptr += 1024;
		    }
		    else {
			bcopy(sg_ptr, sg_fifo_addr, sgfcc->fwused*2);
			sgfcc->get += sgfcc->fwused;
			sg_fifo_addr += (sgfcc->fwused * 2);
			sg_ptr += (sgfcc->fwused * 2);
		    }
		    if (sgfcc->fwused > sgfcc->thresh) goto loop;
		    sgflags.adder_ie |= ADDRESS_COMPLETE;
		    sgaddr->interrupt_enable = sgflags.adder_ie;
		}
	    }
	} else {
		if (int_flag == -1) {
		    if (DMA_ISFULL(header)) {
		    	if (rsel && sgflags.selmask & SEL_WRITE) {
			    selwakeup(rsel, 0);
			    rsel = 0;
			    sgflags.selmask &= ~SEL_WRITE;
		    	}
		    }

		    if (DMA_ISEMPTY(header))
		    	return;
		}

		request = FIFO_GETBEGIN(header);
		if (request->FIFOtype == DISPLIST) {
		    temp = (u_short *)request->bufp;
		    req_length = request->length;
	    	    if (req_length > 0) {
			bcopy(temp, sgmap.fiforam, request->length);
			sgfcc->put += (request->length / 2);
			if (sgfcc->fwused < sgfcc->thresh)
		    	    sgfcc->thresh = sgfcc->fwused;
			sgfcc->icsr &= ~ITHRESH;
			request->length = 0;
			req_length = 0;
			int_flag = 0;
			return;
	    	    } else {
/*		    	sg_wait_status(sgaddr, ADDRESS_COMPLETE);*/
		    	sgfcc->icsr &= ~ENTHRSH;
			sgfcc->icsr &= ~ITHRESH;
			int_flag = -1;
	    	    }
		} else {
			mprintf("\nsg: sgfint: illegal FIFOtype\n");
			DMA_CLRACTIVE(header);
			return;
		}
	    DMA_GETEND(header);
	    if (DMA_ISEMPTY(header)) {
	    	if (rsel && sgflags.selmask & SEL_WRITE) {
		    selwakeup(rsel, 0);
		    rsel = 0;
		    sgflags.selmask &= ~SEL_WRITE;
	    	}
	        DMA_CLRACTIVE(header);
	        return;
	    }
        *(unsigned long *) &sgfcc->cbcsr = (unsigned long) 0;
        *(unsigned long *) &sgfcc->put = (unsigned long ) 0;
        sgfcc->thresh = 0x0000;
*(unsigned long *)&sgfcc->cbcsr |= (unsigned long)(((sgfcc->icsr|ENTHRSH)<<16)|DL_ENB);
	}
}





/******************************************************************
 **                                                              **
 ** Graphic device input interrupt Routine.                      **
 **                                                              **
 ******************************************************************/

sgiint(ch)
register int ch;
{
	register struct _vs_event *vep;
	register struct sginput *eqh;
	register struct color_cursor *sgcursor;
	struct mouse_report *new_rep;
	struct tty *tp;
	register int unit;
	register c;
	register int i, j;
	u_short data;
	char	wakeup_flag = 0;	/* flag to do a select wakeup call */
	int	cnt;
/*
 * Mouse state info
 */
	static char temp, old_switch, new_switch;

	eqh = eq_header;
	unit = (ch>>8)&03;
	new_rep = &current_rep;
	tp = &ss_tty[unit];

/*
 * If graphic device is turned on
 */

   if (sg_mouseon == 1) {
  
	cnt = 0;
	while (cnt++ == 0) {

/*
 * Pick up LK-201 input (if any)
 */

	    if (unit == 0) {

/* event queue full ? */

		if (ISFULL(eqh) == TRUE) {
		    cprintf("\nsg0: sgiint: event queue overflow");
		    return(0);
		}
/*
 * Get a character.
 */

		data = ch & 0xff;

/*
 * Check for various keyboard errors
 */

		if( data == LK_POWER_ERROR || data == LK_KDOWN_ERROR ||
	    	    data == LK_INPUT_ERROR || data == LK_OUTPUT_ERROR) {
			cprintf("\nsg0: sgiint: keyboard error, code = %x",data);
			return(0);
		}

		if (data < LK_LOWEST) 
		    	return(0);
		++wakeup_flag;		/* request a select wakeup call */

		vep = PUTBEGIN(eqh);
		PUTEND(eqh);
/*
 * Check for special case in which "Hold Screen" key is pressed. If so, treat is
 * as if ^s or ^q was typed.
 */
		if (data == HOLD) {
			vep->vse_direction = VSE_KBTRAW;
			vep->vse_type = VSE_BUTTON;
			vep->vse_device = VSE_DKB;
			vep->vse_x = eqh->curs_pos.x;
			vep->vse_y = eqh->curs_pos.y;
			vep->vse_time = TOY;
			vep->vse_key = CNTRL;    	/* send CTRL */
			vep = PUTBEGIN(eqh);
			PUTEND(eqh);
			vep->vse_direction = VSE_KBTRAW;
			vep->vse_type = VSE_BUTTON;
			vep->vse_device = VSE_DKB;
			vep->vse_x = eqh->curs_pos.x;
			vep->vse_y = eqh->curs_pos.y;
			vep->vse_time = TOY;
			if( sg_keyboard.hold  == 0) {
			    if((tp->t_state & TS_TTSTOP) == 0) {
		            	vep->vse_key = CHAR_S;  /* send character "s" */
			    	sg_key_out( LK_LED_ENABLE );
				sg_key_out(LED_4);
				sg_keyboard.hold = 1;
				tp->t_state |= TS_TTSTOP;
			    }
			    else {
		            	vep->vse_key = CHAR_Q;  /* send character "q" */
				tp->t_state &= ~TS_TTSTOP;
			    }
			}
			else {
		            vep->vse_key = CHAR_Q;     /* send character "q" */
			    sg_key_out( LK_LED_DISABLE );
			    sg_key_out( LED_4 );
			    sg_keyboard.hold = 0;
			    tp->t_state &= ~TS_TTSTOP;
			}
			vep = PUTBEGIN(eqh);
			PUTEND(eqh);
			vep->vse_direction = VSE_KBTRAW;
			vep->vse_type = VSE_BUTTON;
			vep->vse_device = VSE_DKB;
			vep->vse_x = eqh->curs_pos.x;
			vep->vse_y = eqh->curs_pos.y;
			vep->vse_time = TOY;
			vep->vse_key = ALLUP;
		}
		else {
			if (sg_keyboard.cntrl == 1) {
			    switch (data) {
			    case CHAR_S:
					tp->t_state |= TS_TTSTOP;
					break;
			    case CHAR_Q:
			    		sg_key_out( LK_LED_DISABLE );
			    		sg_key_out( LED_4 );
			    		sg_keyboard.hold = 0;
					tp->t_state &= ~TS_TTSTOP;
					break;
			    default:
					sg_keyboard.cntrl = 0;
			    }
			}
			vep->vse_direction = VSE_KBTRAW;
			vep->vse_type = VSE_BUTTON;
			vep->vse_device = VSE_DKB;
			vep->vse_x = eqh->curs_pos.x;
			vep->vse_y = eqh->curs_pos.y;
			vep->vse_time = TOY;
			vep->vse_key = data;
			if (data == CNTRL)
			    sg_keyboard.cntrl = 1;
		}
	    }

/*
 * Pick up the mouse input (if any)
 */

	    if ((unit == 1) && (sm_pointer_id == MOUSE_ID)) {

/* event queue full ? */

		if (ISFULL(eqh) == TRUE) {
		    cprintf("\nsg0: sgiint: event queue overflow");
		    return(0);
		}

/*
 * see if mouse position has changed
 */
		if( new_rep->dx != 0 || new_rep->dy != 0) {

/*
 * Check to see if we have to accelerate the mouse
 *
 */
		    if (sgflags.curs_acc > ACC_OFF) {
			if (new_rep->dx >= sgflags.curs_thr)
			    new_rep->dx +=
				(new_rep->dx - sgflags.curs_thr) * sgflags.curs_acc;
			if (new_rep->dy >= sgflags.curs_thr)
			    new_rep->dy +=
				(new_rep->dy - sgflags.curs_thr) * sgflags.curs_acc;
		    }

/*
 * update mouse position
 */
		    if( new_rep->state & X_SIGN) {
			eqh->curs_pos.x += new_rep->dx;
			if( eqh->curs_pos.x > MAX_CUR_X )
			    eqh->curs_pos.x = MAX_CUR_X;
		    }
		    else {
			eqh->curs_pos.x -= new_rep->dx;
			if( eqh->curs_pos.x < -15 )
			    eqh->curs_pos.x = -15;
		    }
		    if( new_rep->state & Y_SIGN) {
			eqh->curs_pos.y -= new_rep->dy;
			if( eqh->curs_pos.y < -15 )
			    eqh->curs_pos.y = -15;
		    }
		    else {
			eqh->curs_pos.y += new_rep->dy;
			if( eqh->curs_pos.y > MAX_CUR_Y )
			    eqh->curs_pos.y = MAX_CUR_Y;
		    }
		    if( tp->t_state & TS_ISOPEN ) {
			sgcursor = (struct color_cursor *) sgmap.cur;
			sgcursor->xpos = CURS_MIN_X + eqh->curs_pos.x;
			sgcursor->ypos = CURS_MIN_Y + eqh->curs_pos.y;
		    }
		    if (eqh->curs_pos.y <= eqh->curs_box.bottom &&
			eqh->curs_pos.y >=  eqh->curs_box.top &&
			eqh->curs_pos.x <= eqh->curs_box.right &&
			eqh->curs_pos.x >=  eqh->curs_box.left) goto mbuttons;
		    vep = PUTBEGIN(eqh);
		    PUTEND(eqh);
		    ++wakeup_flag;	/* request a select wakeup call */

/*
 * Put event into queue and do select
 */

		    vep->vse_x = eqh->curs_pos.x;
		    vep->vse_y = eqh->curs_pos.y;
		    vep->vse_device = VSE_MOUSE;	/* mouse */
		    vep->vse_type = VSE_MMOTION;	/* position changed */
		    vep->vse_time = TOY;		/* time stamp */
		    vep->vse_direction = 0;
		    vep->vse_key = 0;
		}

/*
 * See if mouse buttons have changed.
 */

mbuttons:

		new_switch = new_rep->state & 0x07;
		old_switch = last_rep.state & 0x07;

		temp = old_switch ^ new_switch;
		if( temp ) {
		    for (j = 1; j < 8; j <<= 1) {/* check each button */
			if (!(j & temp))  /* did this button change? */
			    continue;
/* event queue full? */

			if (ISFULL(eqh) == TRUE) {
		    	    cprintf("\nsg0: sgiint: event queue overflow");
		    	    return(0);
			}

			vep = PUTBEGIN(eqh);	/* get new event */
			PUTEND(eqh);

			++wakeup_flag;      /* request a select wakeup call */

/* put event into queue */

			switch (j) {
			case RIGHT_BUTTON:
					vep->vse_key = VSE_RIGHT_BUTTON;
					break;

			case MIDDLE_BUTTON:
					vep->vse_key = VSE_MIDDLE_BUTTON;
					break;

			case LEFT_BUTTON:
					vep->vse_key = VSE_LEFT_BUTTON;
					break;

			}
			if (new_switch & j)
				vep->vse_direction = VSE_KBTDOWN;
			else
				vep->vse_direction = VSE_KBTUP;
			vep->vse_type = VSE_BUTTON;
			vep->vse_device = VSE_MOUSE;	/* mouse */
			vep->vse_time = TOY;
		    	vep->vse_x = eqh->curs_pos.x;
		    	vep->vse_y = eqh->curs_pos.y;
		    }

/* update the last report */

		    last_rep = current_rep;
		}
	    } /* Pick up mouse input */

	    else if ((unit == 1) && (sm_pointer_id == TABLET_ID)) {

/* event queue full? */

		    if (ISFULL(eqh) == TRUE) {
		    	cprintf("\nsg0: sgiint: event queue overflow");
		    	return(0);
		    }


/* update cursor position coordinates */

		    new_rep->dx /= sgflags.tab_res;
		    new_rep->dy = (2200 - new_rep->dy) / sgflags.tab_res;
		    if( new_rep->dx > MAX_CUR_X )
			new_rep->dx = MAX_CUR_X;
		    if( new_rep->dy > MAX_CUR_Y )
			new_rep->dy = MAX_CUR_Y;

/*
 * see if the puck/stylus has moved
 */
		    if( eqh->curs_pos.x != new_rep->dx ||
			eqh->curs_pos.y != new_rep->dy) {

/*
 * update cursor position
 */
		 	eqh->curs_pos.x = new_rep->dx;
		 	eqh->curs_pos.y = new_rep->dy;

		    	if( tp->t_state & TS_ISOPEN ) {
			    sgcursor = (struct color_cursor *) sgmap.cur;
			    sgcursor->xpos = CURS_MIN_X + eqh->curs_pos.x;
			    sgcursor->ypos = CURS_MIN_Y + eqh->curs_pos.y;
			}
		    	if (eqh->curs_pos.y < eqh->curs_box.bottom &&
			    eqh->curs_pos.y >=  eqh->curs_box.top &&
			    eqh->curs_pos.x < eqh->curs_box.right &&
			    eqh->curs_pos.x >=  eqh->curs_box.left) goto tbuttons;

			vep = PUTBEGIN(eqh);
			PUTEND(eqh);
			++wakeup_flag;	/* request a select wakeup call */

/* Put event into queue */

/*
 * The type should be "VSE_TMOTION" but, the X doesn't know this type, therefore
 * until X is fixed, we fake it to be "VSE_MMOTION".
 *
 */
		    	vep->vse_type = VSE_MMOTION;
		    	vep->vse_device = VSE_TABLET;	/* tablet */
			vep->vse_direction = 0;
		    	vep->vse_x = eqh->curs_pos.x;
		    	vep->vse_y = eqh->curs_pos.y;
			vep->vse_key = 0;
		    	vep->vse_time = TOY;
		    }

/*
 * See if tablet buttons have changed.
 */

tbuttons:

		new_switch = new_rep->state & 0x1e;
		old_switch = last_rep.state & 0x1e;
		temp = old_switch ^ new_switch;
		if( temp ) {

/* event queue full? */

		    if (ISFULL(eqh) == TRUE) {
		    	cprintf("\nsg0: sgiint: event queue overflow");
		    	return(0);
		    }

		    vep = PUTBEGIN(eqh);
		    PUTEND(eqh);
		    ++wakeup_flag;	/* request a select wakeup call */

/* put event into queue */

		    vep->vse_device = VSE_TABLET;	/* tablet */
		    vep->vse_type = VSE_BUTTON;
		    vep->vse_x = eqh->curs_pos.x;
		    vep->vse_y = eqh->curs_pos.y;
		    vep->vse_time = TOY;

/* define the changed button and if up or down */

		    for (j = 1; j <= 0x10; j <<= 1) {/* check each button */
			if (!(j & temp))  /* did this button change? */
			    continue;
			switch (j) {
			case T_RIGHT_BUTTON:
					vep->vse_key = VSE_T_RIGHT_BUTTON;
					break;

			case T_FRONT_BUTTON:
					vep->vse_key = VSE_T_FRONT_BUTTON;
					break;

			case T_BACK_BUTTON:
					vep->vse_key = VSE_T_BACK_BUTTON;
					break;

			case T_LEFT_BUTTON:
					vep->vse_key = VSE_T_LEFT_BUTTON;
					break;

			}
		    	if (new_switch & j)
			    vep->vse_direction = VSE_KBTDOWN;
		    	else
			    vep->vse_direction = VSE_KBTUP;
		    }

/* update the last report */

		    last_rep = current_rep;
		}
	    } /* Pick up tablet input */
	} /* While input available */

/*
 * If we have proc waiting, and event has happened, wake him up
 */
	if(rsel && wakeup_flag && sgflags.selmask & SEL_READ) {
	    selwakeup(rsel,0);
	    rsel = 0;
	    sgflags.selmask &= ~SEL_READ;
	    wakeup_flag = 0;
	}
   }

/*
 * If the graphic device is not turned on, this is console input
 */

   else {

/*
 * Get a character from the keyboard.
 */

	if (ch & 0100000) {
	    data = ch & 0xff;

/*
 * Check for various keyboard errors
 */

	    if( data == LK_POWER_ERROR || data == LK_KDOWN_ERROR ||
		data == LK_INPUT_ERROR || data == LK_OUTPUT_ERROR) {
			cprintf("sg0: sgiint: Keyboard error, code = %x\n",data);
			return(0);
	    }
	    if( data < LK_LOWEST ) return(0);

/*
 * See if its a state change key
 */

	    switch ( data ) {
	    case LOCK:
			sg_keyboard.lock ^= 0xffff;	/* toggle */
			if( sg_keyboard.lock )
				sg_key_out( LK_LED_ENABLE );
			else
				sg_key_out( LK_LED_DISABLE );
			sg_key_out( LED_3 );
			return;

	    case SHIFT:
			sg_keyboard.shift ^= 0xffff;
			return;	

	    case CNTRL:
			sg_keyboard.cntrl ^= 0xffff;
			return;

	    case ALLUP:
			sg_keyboard.cntrl = sg_keyboard.shift = 0;
			return;

	    case REPEAT:
			c = sg_keyboard.last;
			break;

	    case HOLD:
/*
 * "Hold Screen" key was pressed, we treat it as if ^s or ^q was typed.
 */
			if (sg_keyboard.hold == 0) {
			    if((tp->t_state & TS_TTSTOP) == 0) {
			    	c = q_key[CHAR_S];
			    	sg_key_out( LK_LED_ENABLE );
			    	sg_key_out( LED_4 );
				sg_keyboard.hold = 1;
			    } else
				c = q_key[CHAR_Q];
			}
			else {
			    c = q_key[CHAR_Q];
			    sg_key_out( LK_LED_DISABLE );
			    sg_key_out( LED_4 );
			    sg_keyboard.hold = 0;
			}
			if( c >= ' ' && c <= '~' )
			    c &= 0x1f;
		    	(*linesw[tp->t_line].l_rint)(c, tp);
			return;

	    default:

/*
 * Test for control characters. If set, see if the character
 * is elligible to become a control character.
 */
			if( sg_keyboard.cntrl ) {
			    c = q_key[ data ];
			    if( c >= ' ' && c <= '~' )
				c &= 0x1f;
			} else if( sg_keyboard.lock || sg_keyboard.shift )
				    c = q_shift_key[ data ];
				else
				    c = q_key[ data ];
			break;	

	    }

	    sg_keyboard.last = c;

/*
 * Check for special function keys
 */
	    if( c & 0x80 ) {

		register char *string;

		string = q_special[ c & 0x7f ];
		while( *string )
		    (*linesw[tp->t_line].l_rint)(*string++, tp);
	    } else
		    (*linesw[tp->t_line].l_rint)(c, tp);
	    if (sg_keyboard.hold &&((tp->t_state & TS_TTSTOP) == 0)) {
		    sg_key_out( LK_LED_DISABLE );
		    sg_key_out( LED_4 );
		    sg_keyboard.hold = 0;
	    }
	}
   }

	return(0);
}





/******************************************************************
 **                                                              **
 ** Routine to start transmission.                               **
 **                                                              **
 ******************************************************************/

sgstart(tp)
	register struct tty *tp;
{
	register int unit, c;
	register struct tty *tp0;
	int s;

	unit = minor(tp->t_dev);
	tp0 = &sm_tty;
	unit &= 03;
	s = spl5();
/*
 * If it's currently active, or delaying, no need to do anything.
 */
	if (tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
	    goto out;
/*
 * Display chars until the queue is empty, if the second subchannel is open
 * direct them there. Drop characters from any lines other than 0 on the floor.
 */

	while( tp->t_outq.c_cc ) {
	    c = getc(&tp->t_outq);
	    if (unit == 0) {
		if (tp0->t_state & TS_ISOPEN)
		    (*linesw[tp0->t_line].l_rint)(c, tp0);
		else
	    	    sg_blitc( c & 0xff );
	    }
	}

/*
 * If there are sleepers, and output has drained below low
 * water mark, wake up the sleepers.
 */
	if ( tp->t_outq.c_cc<=TTLOWAT(tp) )
	    if (tp->t_state&TS_ASLEEP){
		tp->t_state &= ~TS_ASLEEP;
		wakeup((caddr_t)&tp->t_outq);
	    }
	tp->t_state &= ~TS_BUSY;
out:
	splx(s);
}


/******************************************************************
 **                                                              **
 ** Routine to stop output on the graphic device, e.g. for ^S/^Q **
 ** or output flush.                                             **
 **                                                              **
 ******************************************************************/

/*ARGSUSED*/
sgstop(tp, flag)
	register struct tty *tp;
{
	register int s;

/*
 * Block interrupts while modifying the state.
 */
	s = spl5();
	if (tp->t_state & TS_BUSY)
	    if ((tp->t_state&TS_TTSTOP)==0)
		tp->t_state |= TS_FLUSH;
	    else
		tp->t_state &= ~TS_BUSY;
	splx(s);
}





/******************************************************************
 **                                                              **
 ** Routine to output a character to the screen                  **
 **                                                              **
 ******************************************************************/

sg_blitc( c )
register char c;
{

	register struct adder *sgaddr;
	register struct color_cursor *sgcursor;
	register int i;

/*
 * initialize ADDER
 */

	sgaddr = (struct adder *) sgmap.adder;
	sgcursor = (struct color_cursor *) sgmap.cur;

	c &= 0x7f;

	switch ( c ) {
	case '\t':				/* tab		*/
		    for (i = 8 - ((cursor.x >> 3) & 0x07); i > 0; --i) {
		    	sg_blitc( ' ' );
		    }
		    return(0);

	case '\r':				/* return	*/
		    cursor.x = 0;
		    sgcursor->xpos = CURS_MIN_X + cursor.x;
		    return(0);

	case '\b':				/* backspace	*/
		    if (cursor.x > 0) {
		    	cursor.x -= CHAR_WIDTH;
		    	sg_blitc( ' ' );
		    	cursor.x -= CHAR_WIDTH;
			sgcursor->xpos = CURS_MIN_X + cursor.x;
		    }
		    return(0);

	case '\n':				/* linefeed	*/
		    if ((cursor.y += CHAR_HEIGHT) > (MAX_CUR_Y - CHAR_HEIGHT)) {
		    	if (sg_mouseon == 1)
			    cursor.y = 0;
		    	else {
			    cursor.y -= CHAR_HEIGHT;
			    sg_scroll_up(sgaddr);
		    	}
		    }
		    sgcursor->ypos = CURS_MIN_Y + cursor.y;
		    return(0);

	default:
		    if( c < ' ' || c > '~' )
			return(0);
	}

/*
 * setup VIPER operand control registers
 */

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0001);  /* select plane #0 */
	sg_write_id(sgaddr, SRC1_OCR_B,
			EXT_NONE | INT_SOURCE | ID | BAR_SHIFT_DELAY);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x000E);  /* select other planes */
	sg_write_id(sgaddr, SRC1_OCR_B,
			EXT_SOURCE | INT_NONE | NO_ID | BAR_SHIFT_DELAY);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x000F);  /* select all planes */
	sg_write_id(sgaddr, DST_OCR_B,
			EXT_NONE | INT_NONE | NO_ID | NO_BAR_SHIFT_DELAY);

	sg_write_id(sgaddr, MASK_1, 0xFFFF);
	sg_write_id(sgaddr, VIPER_Z_LOAD | FOREGROUND_COLOR_Z, 1);
	sg_write_id(sgaddr, VIPER_Z_LOAD | BACKGROUND_COLOR_Z, 0);

/*
 * load DESTINATION origin and vectors
 */

	sgaddr->fast_dest_dy = 0;
	sgaddr->slow_dest_dx = 0;
	sgaddr->error_1 = 0;
	sgaddr->error_2 = 0;

	sgaddr->rasterop_mode = DST_WRITE_ENABLE | NORMAL;

	sg_wait_status(sgaddr, RASTEROP_COMPLETE);

	sgaddr->destination_x = cursor.x;
	sgaddr->fast_dest_dx = CHAR_WIDTH;

	sgaddr->destination_y = cursor.y;
	sgaddr->slow_dest_dy = CHAR_HEIGHT;

/*
 * load SOURCE origin and vectors
 */

	sgaddr->source_1_x = FONT_X + ((c - ' ') * CHAR_WIDTH);
	sgaddr->source_1_y = FONT_Y;

	sgaddr->source_1_dx = CHAR_WIDTH;
	sgaddr->source_1_dy = CHAR_HEIGHT;

	sg_write_id(sgaddr, LU_FUNCTION_R1, FULL_SRC_RESOLUTION | LF_SOURCE);
	sgaddr->cmd = RASTEROP | OCRB | 0 | S1E | DTE;

/*
* update console cursor coordinates */

	cursor.x += CHAR_WIDTH;
	sgcursor->xpos = CURS_MIN_X + cursor.x;

	if (cursor.x > (MAX_CUR_X - CHAR_WIDTH)) {
	    sg_blitc( '\r' );
	    sg_blitc( '\n' );
	}
}





/********************************************************************
 **                                                                **
 ** Routine to direct kernel console output to display destination **
 **                                                                **
 ********************************************************************/

sgputc( c )
register char c;
{

	register struct tty *tp0;


/*
 * This routine may be called in physical mode by the dump code
 * so we change the driver into physical mode.
 * One way change, can't go back to virtual mode.
 */
	if( (mfpr(MAPEN) & 1) == 0 ) {
		sg_physmode = 1;
		sg_mouseon = 0;
		sg_blitc(c & 0xff);
		return;
	}

/*
 * direct kernel output char to the proper place
 */

	tp0 = &sm_tty;

	if (sgflags.kernel_loop != 0  &&  tp0->t_state & TS_ISOPEN) {
	    (*linesw[tp0->t_line].l_rint)(c, tp0);
	} else {
	    sg_blitc(c & 0xff);
	}

}






/******************************************************************
 **                                                              **
 ** Routine to get a character from LK201.                       **
 **                                                              **
 ******************************************************************/

sggetc()
{
	int	c;
	u_short	data;

/*
 * Get a character from the keyboard,
 */

loop:
	data = ssgetc();

/*
 * Check for various keyboard errors
 */

	if( data == LK_POWER_ERROR || data == LK_KDOWN_ERROR ||
            data == LK_INPUT_ERROR || data == LK_OUTPUT_ERROR) {
		cprintf(" sg0: Keyboard error, code = %x\n",data);
		return(0);
	}
	if( data < LK_LOWEST ) return(0);

/*
 * See if its a state change key
 */

	switch ( data ) {
	case LOCK:
		sg_keyboard.lock ^= 0xffff;	/* toggle */
		if( sg_keyboard.lock )
			sg_key_out( LK_LED_ENABLE );
		else
			sg_key_out( LK_LED_DISABLE );
		sg_key_out( LED_3 );
		goto loop;

	case SHIFT:
		sg_keyboard.shift ^= 0xffff;
		goto loop;

	case CNTRL:
		sg_keyboard.cntrl ^= 0xffff;
		goto loop;

	case ALLUP:
		sg_keyboard.cntrl = sg_keyboard.shift = 0;
		goto loop;

	case REPEAT:
		c = sg_keyboard.last;
		break;

	default:

/*
 * Test for control characters. If set, see if the character
 * is elligible to become a control character.
 */
		if( sg_keyboard.cntrl ) {
		    c = q_key[ data ];
		    if( c >= ' ' && c <= '~' )
			c &= 0x1f;
		} else if( sg_keyboard.lock || sg_keyboard.shift )
			    c = q_shift_key[ data ];
		       else
			    c = q_key[ data ];
		break;	

	}

	sg_keyboard.last = c;

/*
 * Check for special function keys
 */
	if( c & 0x80 )
	    return (0);
	else
	    return (c);
}






/*********************************************************************
 **                                                                 **
 ** Routine to initialize virtual console. This routine sets up the **
 ** graphic device so that it can be used as the system console. It **
 ** is invoked before autoconfig and has to do everything necessary **
 ** to allow the device to serve as the system console.             **
 **                                                                 **
 *********************************************************************/

extern	(*vs_gdopen)();
extern	(*vs_gdclose)();
extern	(*vs_gdread)();
extern	(*vs_gdwrite)();
extern	(*vs_gdselect)();
extern	(*vs_gdkint)();
extern	(*vs_gdioctl)();
extern	(*vs_gdstop)();

sgcons_init()
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register struct nb3_regs *sgaddr = (struct nb3_regs *)sgmem;
	register struct nb1_regs *sgaddr1 = (struct nb1_regs *)qmem;
	register struct vdac *sgvdac;
	register struct color_cursor *sgcursor;


/*
 * Set the line parameters on SLU line 0 for
 * the LK201 keyboard: 4800 BPS, 8-bit char, 1 stop bit, no parity.
 */
	ssaddr->sslpr = (SER_RXENAB | SER_KBD | SER_SPEED | SER_CHARW);

/*
 * Load sgmap structure with the virtual addresses of the VAXstar (color).
 */

	sgbase = (caddr_t) ((u_long)sgaddr);

	sgmap.adder = sgbase + ADDER;
	sgmap.fcc = sgbase + FCC;
	sgmap.vdac = sgbase + VDAC;
	sgmap.cur = sgbase + CUR;
	sgmap.vrback = sgbase + VRBACK;
	sgmap.fiforam = sgbase + FIFORAM;

	sgvdac = (struct vdac *) sgmap.vdac;
	sgvdac-> mode = 0x47;
	sgcursor = (struct color_cursor *)sgmap.cur;
	sg_cur_reg = (VBHI | ENRG1 | ENPA | ENPB);
	sgcursor->cmdr = sg_cur_reg;
	sgcursor->xmin1 = 0;
	sgcursor->xmax1 = MAX_CUR_X;
	sgcursor->ymin1 = 0;
	sgcursor->ymax1 = MAX_CUR_Y;

/*
 * Initialize the VAXstar (color)
 */

	cursor.x = 0;
	cursor.y = 0;
	sg_init_shared();		/* init shared memory */
	sg_setup_dragon();		/* init ADDR/VIPER */
	sg_clr_screen();		/* clear the screen */
	sg_ld_font();			/* load the console font */
	sg_load_cursor(def_cur);	/* load default cursor */
        sg_input();			/* init the input devices */
	v_consputc = sgputc;
	v_consgetc = sggetc;
	vs_gdopen = sgopen;
	vs_gdclose = sgclose;
	vs_gdread = sgread;
	vs_gdwrite = sgwrite;
	vs_gdselect = sgselect;
	vs_gdkint = sgiint;
	vs_gdioctl = sgioctl;
	vs_gdstop = sgstop;
}






sgreset()
{
}






/******************************************************************
 **                                                              **
 ** Routine to setup the input devices                           **
 **  (keyboard, mouse, and tablet).                              **
 **                                                              **
 ******************************************************************/

sg_input()
{

	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int	lpr;
	int	i;
	int	status;
	char	id_byte;

/*
 * Set the line parameters on SLU line 0 for
 * the LK201 keyboard: 4800 BPS, 8-bit char, 1 stop bit, no parity.
 */
	ssaddr->sslpr = (SER_RXENAB | SER_KBD | SER_SPEED | SER_CHARW);
/*
 * Reset the keyboard to the default state
 */

	sg_key_out(LK_DEFAULTS);

/*
 * Set SLU line 1 parameters for mouse communication.
 */
	lpr = SER_POINTER | SER_CHARW | SER_PARENB | SER_ODDPAR
		| SER_SPEED | SER_RXENAB;
	ssaddr->sslpr = lpr;

/*
 * Perform a self-test
 */
	sg_putc(SELF_TEST);
/*
 * Wait for the first byte of the self-test report
 *
 */
	status = sg_getc();
	if (status < 0) {
	    cprintf("\nsg: Timeout on 1st byte of self-test report\n");
	    goto OUT;
	}
/*
 * Wait for the hardware ID (the second byte returned by the self-test report)
 *
 */
	id_byte = sg_getc();
	if (id_byte < 0) {
	    cprintf("\nsg: Timeout on 2nd byte of self-test report\n");
	    goto OUT;
	}
/*
 * Wait for the third byte returned by the self-test report)
 *
 */
	status = sg_getc();
	if (status != 0) {
	    cprintf("\nsg: Timeout on 3rd byte of self-test report\n");
	    goto OUT;
	}
/*
 * Wait for the forth byte returned by the self-test report)
 *
 */
	status = sg_getc();
	if (status != 0) {
	    cprintf("\nsg: Timeout on 4th byte of self-test report\n");
	    goto OUT;
	}

/*
 * Wait to be sure that the self-test is done (documentation indicates that
 * it requires 1 second to do the self-test).
 */

	DELAY(1000000);

/*
 * Set the operating mode
 *
 *   We set the mode for both mouse and the tablet to "Incremental stream mode".
 *
 */
	if ((id_byte & 0x0f) == MOUSE_ID)
		sm_pointer_id = MOUSE_ID;
	else
		sm_pointer_id = TABLET_ID;
	sg_putc(INCREMENTAL);

OUT:
	return(0);
}







/******************************************************************
 **                                                              **
 ** Routine to get 1 character from the mouse (SLU line 1).      **
 ** Return an error on timeout or faulty character.              **
 **                                                              **
 ** NOTE:                                                        **
 **	This routine will be used just during initialization     **
 **     (during system boot).                                    **
 **                                                              **
 ******************************************************************/

sg_getc()
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int timo;
	register int c;


	for(timo=50; timo > 0; --timo) {
		DELAY(1000);
		if(ssaddr->sscsr&SS_RDONE) {
			c = ssaddr->ssrbuf;
			if(((c >> 8) & 03) != 1)
				continue;
			if(c&(SS_DO|SS_FE|SS_PE))
				continue;
			return(c & 0xff);
		}
	}
	return(-1);
}






/******************************************************************
 **                                                              **
 ** Routine to send a character to the mouse using SLU line 1.   **
 **                                                              **
 ** NOTE:                                                        **
 **	This routine will be used just during initialization     **
 **     (during system boot).                                    **
 **                                                              **
 ******************************************************************/

sg_putc(c)
	register int c;
{
	register struct nb_regs *ssaddr = (struct nb_regs *)nexus;
	register int timo;

	ssaddr->sstcr |= 0x2;
	timo = 60000;
	while ((ssaddr->sscsr&SS_TRDY) == 0)
		if (--timo == 0)
			break;
	ssaddr->sstbuf = c&0xff;
	DELAY(50000);		/* ensure character transmit completed */
	ssaddr->sstcr &= ~0x2;
}




/******************************************************************
 **                                                              **
 ** Routine to load the cursor.                                  **
 **                                                              **
 ******************************************************************/

sg_load_cursor(data)
unsigned short data[32];
{

	register struct color_cursor *cur;
	register struct vdac *sgvdac;
	register int	i;

	cur = (struct color_cursor *) sgmap.cur;
	sgvdac = (struct vdac *) sgmap.vdac;
	sg_cur_reg |= LODSA;
	cur->cmdr = sg_cur_reg;

	for (i = 0; i < 32; ++i)
	    cur->cmem = data[i];

	sg_cur_reg &= ~LODSA;
	cur->cmdr = sg_cur_reg;
}




/******************************************************************
 **                                                              **
 ** Routine to output a character to the LK201 keyboard. This	 **
 ** routine polls the tramsmitter on the keyboard to output a    **
 ** code. The timer is to avaoid hanging on a bad device.        **
 **                                                              **
 ******************************************************************/

sg_key_out( c )
char c;
{
	register struct nb_regs *ssaddr;
	register int timo = 30000;
	int s;
	int tcr, ln;

	if (v_consputc != sgputc)
		return;

	if(sg_physmode)
		ssaddr = (struct nb_regs *)VS_PHYSNEXUS;
	else
		ssaddr = (struct nb_regs *)nexus;

	if(sg_physmode == 0)
		s = spl5();
	tcr = 0;
	ssaddr->sstcr |= 1;
	while (1) {
		while ((ssaddr->sscsr&SS_TRDY) == 0 && timo--) ;
		ln = (ssaddr->sscsr>>8) & 3;
		if (ln != 0) {
			tcr |= (1 << ln);
			ssaddr->sstcr &= ~(1 << ln);
			continue;
		}
		ssaddr->sstbuf = c&0xff;
		while (1) {
			while ((ssaddr->sscsr&SS_TRDY) == 0) ;
			ln = (ssaddr->sscsr>>8) & 3;
			if (ln != 0) {
				tcr |= (1 << ln);
				ssaddr->sstcr &= ~(1 << ln);
				continue;
			}
			break;
		}
		ssaddr->sstcr &= ~0x1;
		if (tcr == 0)
			ssaddr->nb_int_reqclr = SINT_ST;
		else
			ssaddr->sstcr |= tcr;
		break;
	}
	if(sg_physmode == 0)
		splx(s);
}








/******************************************************************
 **                                                              **
 ** Routine to write to the ID bus                               **
 **                                                              **
 ******************************************************************/

sg_write_id(sgaddr, adrs, data)
register struct adder *sgaddr;
register short adrs;
register short data;
{
	int i;
	short status;

	for ( i = 100000, sgaddr->status = 0
	    ; i > 0  &&  !((status = sgaddr->status) &
		 ADDRESS_COMPLETE)
	    ; --i);

	if (i == 0)
	    goto ERR;

	for ( i = 100000, sgaddr->status = 0
	    ; i > 0  &&  !((status = sgaddr->status) &
		 TX_READY)
	    ; --i);

	if (i > 0) {
	    sgaddr->id_data = data;
	    sgaddr->command = ID_LOAD | adrs;
	    return(0);
	}

ERR:
	cprintf("\nsg_write_id: timeout trying to write to VIPER");
	return(-1);
}





/******************************************************************
 **                                                              **
 ** Routine to delay for at least one display frame time         **
 **                                                              **
 ******************************************************************/

sg_wait_status(sgaddr, mask)
register struct adder *sgaddr;
register int mask;
{
	register short status;
	int i;

	for ( i = 10000, sgaddr->status = 0
	    ; i > 0  &&  !((status = sgaddr->status) & mask)
	    ; --i);

	if (i == 0) {
	    cprintf("\nsg_wait_status: timeout polling for 0x%x in sgaddr->status", mask);
	    return(-1);
	}

	return(0);

}





/******************************************************************
 **                                                              **
 ** Routine to initialize the shared memory                      **
 **                                                              **
 ******************************************************************/

sg_init_shared()
{

	register struct color_cursor *sgcursor;

	sgcursor = (struct color_cursor *) sgmap.cur;
/*
 * initialize the event queue pointers and header
 */

	eq_header = (struct sginput *)
			  (((int)event_shared & ~(0x01FF)) + 512);
	eq_header->curs_pos.x = 0;
	eq_header->curs_pos.y = 0;

	sgcursor->xpos = CURS_MIN_X + eq_header->curs_pos.x;
	sgcursor->ypos = CURS_MIN_Y + eq_header->curs_pos.y;

	eq_header->curs_box.left = 0;
	eq_header->curs_box.right = 0;
	eq_header->curs_box.top = 0;
	eq_header->curs_box.bottom = 0;

/*
 * assign a pointer to the FIFO I/O buffer for this QDSS. 
 */

	FIFOheader = (struct FIFOreq_header *)
			  ((int)(&FIFO_shared[0] + 512) & ~0x1FF);

	FIFOheader->FIFOreq = (struct FIFOreq *) ((int)FIFOheader
				  + sizeof(struct FIFOreq_header));

	FIFOheader->shared_size = FIFObuf_size;
	FIFOheader->used = 0;
	FIFOheader->size = 10;	/* default = 10 requests */
	FIFOheader->oldest = 0;
	FIFOheader->newest = 0;

/*
 * assign a pointer to the scroll structure.
 */

	scroll = (struct scroll *)
			 ((int)(&scroll_shared[0] + 512) & ~0x1FF);
	scroll->status = 0;
	scroll->viper_constant = 0;
	scroll->y_scroll_constant = 0;
	scroll->y_offset = 0;
	scroll->x_index_pending = 0;
	scroll->y_index_pending = 0;

/*
 * assign a pointer to the color map write buffer
 */

	color_buf = (struct color_buf *)
			   ((int)(&color_shared[0] + 512) & ~0x1FF);
	color_buf->status = 0;
	color_buf->count = 0;

}




/******************************************************************
 **                                                              **
 ** Routine to initialize the ADDER, VIPER, bitmaps, and color   **
 ** map.                                                         **
 **                                                              **
 ******************************************************************/

sg_setup_dragon()
{

	register struct adder *sgaddr;
	register struct	vdac  *sgvdac;
	register short *color;


	int i;			/* general purpose variables */
	int status;

	short top;		/* clipping/scrolling boundaries */
	short bottom;
	short right;
	short left;

	short *red;		/* color map pointers */
	short *green;
	short *blue;

/*
 * init for setup
 */

	sgaddr = (struct adder *) sgmap.adder;
	sgvdac = (struct vdac *) sgmap.vdac;
	sgvdac-> mode = 0x47;
	sgaddr->command = CANCEL;

/*
 * set monitor timing
 */

	sgaddr->x_scan_count_0 = 0x2800;
	sgaddr->x_scan_count_1 = 0x1020;
/*	sgaddr->x_scan_count_2 = 0x003E;*/
	sgaddr->x_scan_count_2 = 0x003A;
	sgaddr->x_scan_count_3 = 0x38F0;
	sgaddr->x_scan_count_4 = 0x6128;
/*	sgaddr->x_scan_count_5 = 0x093B;*/
	sgaddr->x_scan_count_5 = 0x093A;
	sgaddr->x_scan_count_6 = 0x313C;
	sgaddr->x_scan_conf = 0x00C8;

/*
 * got a bug in secound pass ADDER! lets take care of it
 */

/* normally, just use the code in the following bug fix code, but to
 * make repeated demos look pretty, load the registers as if there was
 * no bug and then test to see if we are getting sync
 */

	sgaddr->y_scan_count_0 = 0x135F;
	sgaddr->y_scan_count_1 = 0x3363;
	sgaddr->y_scan_count_2 = 0x2366;
	sgaddr->y_scan_count_3 = 0x0388;

/* if no sync, do the bug fix code */

	if (sg_wait_status(sgaddr, FRAME_SYNC) == -1) {

/*
 * first load all Y scan registers with very short frame and
 * wait for scroll service. This guarantees at least one SYNC
 * to fix the pass 2 Adder initialization bug (synchronizes
 * XCINCH with DMSEEDH).
 */

	    sgaddr->y_scan_count_0 = 0x01;
	    sgaddr->y_scan_count_1 = 0x01;
	    sgaddr->y_scan_count_2 = 0x01;
	    sgaddr->y_scan_count_3 = 0x01;

	    sg_wait_status(sgaddr, FRAME_SYNC);
	    sg_wait_status(sgaddr, FRAME_SYNC);

/* now load the REAL sync values (in reverse order just to
 *  be safe.
 */

	    sgaddr->y_scan_count_3 = 0x0388;
	    sgaddr->y_scan_count_2 = 0x2366;
	    sgaddr->y_scan_count_1 = 0x3363;
	    sgaddr->y_scan_count_0 = 0x135F;
	}


/*
 * zero the index registers
 */

	sgaddr->x_index_pending = 0;
	sgaddr->y_index_pending = 0;
	sgaddr->x_index_new = 0;
	sgaddr->y_index_new = 0;
	sgaddr->x_index_old = 0;
	sgaddr->y_index_old = 0;

	sgaddr->pause = 0;

/*
 * set rasterop mode to normal pen down
 */

	sgaddr->rasterop_mode = DST_WRITE_ENABLE | DST_INDEX_ENABLE | NORMAL;

/*
 * set the rasterop registers to a default values
 */

	sgaddr->source_1_dx = 1;
	sgaddr->source_1_dy = 1;
	sgaddr->source_1_x = 0;
	sgaddr->source_1_y = 0;
	sgaddr->destination_x = 0;
	sgaddr->destination_y = 0;
	sgaddr->fast_dest_dx = 1;
	sgaddr->fast_dest_dy = 0;
	sgaddr->slow_dest_dx = 0;
	sgaddr->slow_dest_dy = 1;
	sgaddr->error_1 = 0;
	sgaddr->error_2 = 0;

/*
 * scale factor = unity
 */

	sgaddr->fast_scale = UNITY;
	sgaddr->slow_scale = UNITY;

/*
 * set the source 2 parameters
 */

	sgaddr->source_2_x = 0;
	sgaddr->source_2_y = 0;
	sgaddr->source_2_size = 0x0022;

/*
 * initialize plane addresses for four vipers
 */

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0001);
	sg_write_id(sgaddr, PLANE_ADDRESS, 0x0000);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0002);
	sg_write_id(sgaddr, PLANE_ADDRESS, 0x0001);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0004);
	sg_write_id(sgaddr, PLANE_ADDRESS, 0x0002);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0008);
	sg_write_id(sgaddr, PLANE_ADDRESS, 0x0003);

/* initialize the external registers. */

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x000F);
	sg_write_id(sgaddr, CS_SCROLL_MASK, 0x000F);

/* initialize resolution mode */

	sg_write_id(sgaddr, MEMORY_BUS_WIDTH, 0x000C);     /* bus width = 16 */
	sg_write_id(sgaddr, RESOLUTION_MODE, 0x0000);      /* one bit/pixel */

/* initialize viper registers */

	sg_write_id(sgaddr, SCROLL_CONSTANT, SCROLL_ENABLE|VIPER_LEFT|VIPER_UP);
	sg_write_id(sgaddr, SCROLL_FILL, 0x0000);

/*
 * set clipping and scrolling limits to full screen
 */

	for ( i = 1000, sgaddr->status = 0
	    ; i > 0  &&  !((status = sgaddr->status) &
		 ADDRESS_COMPLETE)
	    ; --i);

	if (i == 0)
	    cprintf("\nsg%d: sg_setup_dragon: timeout on ADDRESS_COMPLETE");

	top = 0;
	bottom = 2048;
	left = 0;
	right = 1024;

	sgaddr->x_clip_min = left;
	sgaddr->x_clip_max = right;
	sgaddr->y_clip_min = top;
	sgaddr->y_clip_max = bottom;

	sgaddr->scroll_x_min = left;
	sgaddr->scroll_x_max = right;
	sgaddr->scroll_y_min = top;
	sgaddr->scroll_y_max = bottom;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->x_index_pending = left;
	sgaddr->y_index_pending = top;
	sgaddr->x_index_new = left;
	sgaddr->y_index_new = top;
	sgaddr->x_index_old = left;
	sgaddr->y_index_old = top;

	for ( i = 1000, sgaddr->status = 0
	    ; i > 0  &&  !((status = sgaddr->status) &
		 ADDRESS_COMPLETE)
	    ; --i);

	if (i == 0)
	    cprintf("\nsg%d: sg_setup_dragon: timeout on ADDRESS_COMPLETE");

	sg_write_id(sgaddr, LEFT_SCROLL_MASK, 0x0000);
	sg_write_id(sgaddr, RIGHT_SCROLL_MASK, 0x0000);

/*
 * set source and the mask register to all ones (ie: white)
 */

	sg_write_id(sgaddr, SOURCE, 0xFFFF);
	sg_write_id(sgaddr, MASK_1, 0xFFFF);
	sg_write_id(sgaddr, VIPER_Z_LOAD | FOREGROUND_COLOR_Z, 15);
	sg_write_id(sgaddr, VIPER_Z_LOAD | BACKGROUND_COLOR_Z, 0);

/*
 * initialize Operand Control Register banks for fill command
 */

	sg_write_id(sgaddr, SRC1_OCR_A, EXT_NONE | INT_M1_M2  | NO_ID | WAIT);
	sg_write_id(sgaddr, SRC2_OCR_A, EXT_NONE | INT_SOURCE | NO_ID | NO_WAIT);
	sg_write_id(sgaddr, DST_OCR_A, EXT_NONE | INT_NONE | NO_ID | NO_WAIT);

	sg_write_id(sgaddr, SRC1_OCR_B, EXT_NONE | INT_SOURCE | NO_ID | WAIT);
	sg_write_id(sgaddr, SRC2_OCR_B, EXT_NONE | INT_M1_M2  | NO_ID | NO_WAIT);
	sg_write_id(sgaddr, DST_OCR_B, EXT_NONE | INT_NONE | NO_ID | NO_WAIT);

/*
 * init Logic Unit Function registers, (these are just common values,
 * and may be changed as required). 
 */

	sg_write_id(sgaddr, LU_FUNCTION_R1, FULL_SRC_RESOLUTION | LF_SOURCE);
	sg_write_id(sgaddr, LU_FUNCTION_R2, FULL_SRC_RESOLUTION | LF_SOURCE | INV_M1_M2);
	sg_write_id(sgaddr, LU_FUNCTION_R3, FULL_SRC_RESOLUTION | LF_D_OR_S);
	sg_write_id(sgaddr, LU_FUNCTION_R4, FULL_SRC_RESOLUTION | LF_D_XOR_S);

/*
 * load the color map 
 */

/*	sg_init_vdac();*/
/*	sgvdac->mode = 0x42;*/
	for ( i = 0, sgaddr->status = 0
	    ; i < 10000  &&  !((status = sgaddr->status) &
		 FRAME_SYNC)
	    ; ++i);

	if (i == 0)
	    cprintf("\nsg%d: sg_setup_dragon: timeout on FRAME_SYNC");

	sgvdac->a_color_map[0] = 0x0000;	/* black */
	sgvdac->a_color_map[1] = 0x0FFF;	/* white */

/*
 * set color map for mouse cursor
 */

	sgvdac->b_cur_colorA = 0x0000;		/* black */
	sgvdac->b_cur_colorB = 0x0FFF;		/* white */

	return(0);

}






/******************************************************************
 **                                                              **
 ** Routine to load eight basic colors into the color map and    **
 ** eight grey scales as well                                    **
 **                                                              **
 **                                                              **
 **                     red    green   blue     map location     **
 **                    --------------------     ------------     **
 **     black           00   |  00   |  00            0          **
 **     blue            00   |  00   |  FF            1          **
 **     green           00   |  FF   |  00            2          **
 **     cyan            00   |  FF   |  FF            3          **
 **     red             FF   |  00   |  00            4          **
 **     magenta         FF   |  00   |  FF            5          **
 **     yellow          FF   |  FF   |  00            6          **
 **     white           FF   |  FF   |  FF            7          **
 **                                                              **
 **     grey_1          00   |  00   |  00            8          **
 **     grey_2          24   |  24   |  24            9          **
 **     grey_3          48   |  48   |  48           10          **
 **     grey_4          6C   |  6C   |  6C           11          **
 **     grey_5          90   |  90   |  90           12          **
 **     grey_6          B4   |  B4   |  B4           13          **
 **     grey_7          D8   |  D8   |  D8           14          **
 **     grey_8          FF   |  FF   |  FF           15          **
 **                                                              **
 ******************************************************************/

sg_init_vdac()
{

	register struct vdac *sgvdac;
	register struct adder *sgaddr;
	register int	i;

	sgvdac = (struct vdac *) sgmap.vdac;
	sgaddr = (struct adder *) sgmap.adder;

	for (i = 0; i < 16; i++)
	    sgvdac->a_color_map[i] = def_vdac_colors[i];


/*	for (i = 0; i < 16; i++)
	    sgvdac->b_color_map[i] = def_vdac_colors[i];

*/
	sgvdac->a_cur_colorA = VDAC_GREY_3;
	sgvdac->a_cur_colorB = VDAC_GREY_6;
	sgvdac->a_cur_colorC = VDAC_GREY_8;

	sgvdac->b_cur_colorA = VDAC_GREY_3;
	sgvdac->b_cur_colorB = VDAC_GREY_6;
	sgvdac->b_cur_colorC = VDAC_GREY_8;

/*	sgvdac->dadj_sync = 0x001A;
	sgvdac->dadj_blank = 0x001A;
	sgvdac->dadj_active = 0x001A;
*/

	sg_wait_status(sgaddr, FRAME_SYNC);

/* for now */
	sgvdac->mode = 0x0056;
}






/******************************************************************
 **                                                              **
 ** Routine to clear the screen                                  **
 **                                                              **
 **			     >>> NOTE <<<                        **
 **                                                              **
 ** This code requires that certain adder initialization be      **
 ** valid. To assure that this requirement is satisfied, this    **
 ** routine should be called only after calling the              **
 ** "sg_setup_dragon()" function.                                **
 ** Clear the bitmap a piece at a time. Since the fast scroll    **
 ** clear only clears the current displayed portion of the       **
 ** bitmap put a temporary value in the y limit register so we   **
 ** can access whole bitmap.                                     **
 ******************************************************************/

sg_clr_screen()
{
	register struct adder *sgaddr;

	sgaddr = (struct adder *) sgmap.adder;

	sgaddr->x_limit = 1024;
	sgaddr->y_limit = 2048 - CHAR_HEIGHT;
	sgaddr->y_offset_pending = 0;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_scroll_constant = SCROLL_ERASE;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_offset_pending = 864;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_scroll_constant = SCROLL_ERASE;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_offset_pending = 1728;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_scroll_constant = SCROLL_ERASE;

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->y_offset_pending = 0;	/* back to normal */

	sg_wait_status(sgaddr, FRAME_SYNC);
	sg_wait_status(sgaddr, FRAME_SYNC);

	sgaddr->x_limit = MAX_SCREEN_X;
	sgaddr->y_limit = MAX_SCREEN_Y + FONT_HEIGHT;

}




/******************************************************************
 **                                                              **
 ** Routine to put the console font in the off-screen memory     **
 ** of the color VAXstar.                                        **
 **                                                              **
 ******************************************************************/

sg_ld_font()
{
	register struct adder *sgaddr;

	int i;		/* scratch variables */
	int j;
	int k;
	short packed;


	sgaddr = (struct adder *) sgmap.adder;

/* setup VIPER operand control registers  */

	sg_write_id(sgaddr, MASK_1, 0xFFFF);
	sg_write_id(sgaddr, VIPER_Z_LOAD | FOREGROUND_COLOR_Z, 255);
	sg_write_id(sgaddr, VIPER_Z_LOAD | BACKGROUND_COLOR_Z, 0);

	sg_write_id(sgaddr, SRC1_OCR_B,
			EXT_NONE | INT_NONE | ID | BAR_SHIFT_DELAY);
	sg_write_id(sgaddr, SRC2_OCR_B,
			EXT_NONE | INT_NONE | ID | BAR_SHIFT_DELAY);
	sg_write_id(sgaddr, DST_OCR_B,
			EXT_SOURCE | INT_NONE | NO_ID | NO_BAR_SHIFT_DELAY);

	sgaddr->rasterop_mode = DST_WRITE_ENABLE |
			 DST_INDEX_ENABLE | NORMAL;

/* load destination data  */

	sg_wait_status(sgaddr, RASTEROP_COMPLETE);

	sgaddr->destination_x = FONT_X;
	sgaddr->destination_y = FONT_Y;
	sgaddr->fast_dest_dx = FONT_WIDTH;
	sgaddr->slow_dest_dy = CHAR_HEIGHT;

/* setup for processor to bitmap xfer  */

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x0001);
	sgaddr->cmd = PBT | OCRB | 2 | DTE | 2;

/* iteratively do the processor to bitmap xfer */

	for (i = 0; i < ROWS; ++i) {

	    /* PTOB a scan line */

	    for (j = 0, k = i; j < 48; ++j) {

		/* PTOB one scan of a char cell */

		packed = q_font[k];
		k += ROWS;
		packed |= ((short)q_font[k] << 8);
		k += ROWS;

		sg_wait_status(sgaddr, TX_READY);
		sgaddr->id_data = packed;
	    }
	}
}







/******************************************************************
 **                                                              **
 ** Routine to scroll up the screen one character height         **
 **                                                              **
 ******************************************************************/

sg_scroll_up()
{

	register struct adder *sgaddr;

/*
 * setup VIPER operand control registers
 */

	sg_wait_status(sgaddr, ADDRESS_COMPLETE);

	sg_write_id(sgaddr, CS_UPDATE_MASK, 0x000F);  /* select all planes */
	sg_write_id(sgaddr, CS_SCROLL_MASK, 0x000F);  /* select all planes */

	sg_write_id(sgaddr, MASK_1, 0xFFFF);
	sg_write_id(sgaddr, VIPER_Z_LOAD | FOREGROUND_COLOR_Z, 15);
	sg_write_id(sgaddr, VIPER_Z_LOAD | BACKGROUND_COLOR_Z, 0);

	sg_write_id(sgaddr, SRC1_OCR_B,
			EXT_NONE | INT_SOURCE | ID | BAR_SHIFT_DELAY);
	sg_write_id(sgaddr, DST_OCR_B,
			EXT_NONE | INT_NONE | NO_ID | NO_BAR_SHIFT_DELAY);

/*
 * load DESTINATION origin and vectors
 */

	sgaddr->fast_dest_dy = 0;
	sgaddr->slow_dest_dx = 0;
	sgaddr->error_1 = 0;
	sgaddr->error_2 = 0;

	sgaddr->rasterop_mode = DST_WRITE_ENABLE | NORMAL;

	sgaddr->destination_x = 0;
	sgaddr->fast_dest_dx = 1024;

	sgaddr->destination_y = 0;
	sgaddr->slow_dest_dy = 864 - CHAR_HEIGHT;

/*
 * load SOURCE origin and vectors
 */

	sgaddr->source_1_x = 0;
	sgaddr->source_1_dx = 1024;

	sgaddr->source_1_y = CHAR_HEIGHT;
	sgaddr->source_1_dy = 864 - CHAR_HEIGHT;

	sg_write_id(sgaddr, LU_FUNCTION_R1, FULL_SRC_RESOLUTION | LF_SOURCE);
	sgaddr->cmd = RASTEROP | OCRB | 0 | S1E | DTE;

/*
 * do a rectangle clear of last screen line
 */

	sg_write_id(sgaddr, MASK_1, 0xffff);
	sg_write_id(sgaddr, SOURCE, 0xffff);
	sg_write_id(sgaddr,DST_OCR_B,
		(EXT_NONE | INT_NONE | NO_ID | NO_BAR_SHIFT_DELAY));
	sg_write_id(sgaddr, VIPER_Z_LOAD | FOREGROUND_COLOR_Z, 0);
	sgaddr->error_1 = 0;
	sgaddr->error_2 = 0;
	sgaddr->slow_dest_dx = 0;		  /* set up the width of */
	sgaddr->slow_dest_dy = CHAR_HEIGHT;	/* rectangle */

	sgaddr->rasterop_mode = (NORMAL | DST_WRITE_ENABLE) ;
	sg_wait_status(sgaddr, RASTEROP_COMPLETE);
	sgaddr->destination_x = 0;
	sgaddr->destination_y = 864 - CHAR_HEIGHT;

	sgaddr->fast_dest_dx = 1024;	/* set up the height */
	sgaddr->fast_dest_dy = 0;	/* of rectangle */

	sg_write_id(sgaddr, LU_FUNCTION_R2, (FULL_SRC_RESOLUTION | LF_SOURCE));
	sgaddr->cmd = (RASTEROP | OCRB | LF_R2 | DTE ) ;

}

#endif
