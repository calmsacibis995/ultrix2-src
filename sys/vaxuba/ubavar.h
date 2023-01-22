/*
 *	@(#)ubavar.h	1.20	(ULTRIX)	12/16/86
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984, 85, 86 by			*
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
 * Modification history:
 *
 *  13-Dec-86 -- fred (Fred Canter)
 *	Added shmem and SHMEMmap for MicroVAX 2000 8 line SLU.
 *
 *   5-Aug-86 -- darrell (Darrell Dunnuck)
 *	Added definitions for VAXstar disk and TZK50 drivers
 *	sharing common disk buffer.
 *
 *   2-Jul-86 -- fred (Fred Canter)
 *	Added mapping for TEAMmate 8 line SLU registers.
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 18-mar-86  -- jaw     br/cvec changed to NOT use registers.
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 08-Aug-85 -- darrell
 *	Add constants for zero vector timer fix, and an integer
 *	definition.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06-Jun-85 -jaw
 *	added support for BDA.
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 */
/*
 * This file contains definitions related to the kernel structures
 * for dealing with the unibus adapters.
 *
 * Each uba has a uba_hd structure.
 * Each unibus controller which is not a device has a uba_ctlr structure.
 * Each unibus device has a uba_device structure.
 */

#ifndef LOCORE
/*
 * Per-uba structure.
 *
 * This structure holds the interrupt vector for the uba,
 * and its address in physical and virtual space.  At boot time
 * we determine the devices attached to the uba's and their
 * interrupt vectors, filling in uh_vec.  We free the map
 * register and bdp resources of the uba into the structures
 * defined here.
 *
 * During normal operation, resources are allocated and returned
 * to the structures here.  We watch the number of passive releases
 * on each uba, and if the number is excessive may reset the uba.
 * 
 * When uba resources are needed and not available, or if a device
 * which can tolerate no other uba activity (rk07) gets on the bus,
 * then device drivers may have to wait to get to the bus and are
 * queued here.  It is also possible for processes to block in
 * the unibus driver in resource wait (mrwant, bdpwant); these
 * wait states are also recorded here.
 */
struct	uba_hd {
	int	uba_type;		/* see defines below. */
	struct	uba_regs *uh_uba;	/* virt addr of uba */
	struct	uba_regs *uh_physuba;	/* phys addr of uba */
	int	(**uh_vec)();		/* interrupt vector */
	struct	uba_device *uh_actf;	/* head of queue to transfer */
	struct	uba_device *uh_actl;	/* tail of queue to transfer */
	short	uh_mrwant;		/* someone is waiting for map reg */
	short	uh_bdpwant;		/* someone awaits bdp's */
	int	uh_bdpfree;		/* free bdp's */
	int	uh_hangcnt;		/* number of ticks hung */
	int	uh_zvcnt;		/* number of 0 vectors */
	int	uh_zvflg;		/* flag for timing zero vectors */
	int	uh_errcnt;		/* number of errors */
	int	uh_lastiv;		/* last free interrupt vector */
	short	uh_users;		/* transient bdp use count */
	short	uh_xclu;		/* an rk07 is using this uba! */
#define	UAMSIZ	100
	struct	map *uh_map;		/* buffered data path regs free */
};
#define UBA780	0x1
#define UBA750	0x2
#define UBA730	0x4
#define UBAUVI	0x8
#define UBAUVII	0x10
#define UBABUA	0x20
#define UBABDA	0x40
#define UBABLA	0x80


#ifndef LOCORE
/*
 * Per-controller structure.
 * (E.g. one for each disk and tape controller, and other things
 * which use and release buffered data paths.)
 *
 * If a controller has devices attached, then there are
 * cross-referenced uba_drive structures.
 * This structure is the one which is queued in unibus resource wait,
 * and saves the information about unibus resources which are used.
 * The queue of devices waiting to transfer is also attached here.
 */
struct uba_ctlr {
	struct	uba_driver *um_driver;
	char	*um_ctlrname;	/* name of the controller */
	short	um_ctlr;	/* controller index in driver */
	caddr_t	ui_parent;
	int	um_adpt;	/* which i/o bus it is on */
	int	um_nexus;	/* which nexus on i/o bus */
	short	um_ubanum;	/* the uba it is on */
	short	um_alive;	/* controller exists */
	int	(**um_intr)();	/* interrupt handler(s) */
	caddr_t	um_addr;	/* address of device in i/o space */
	struct	uba_hd *um_hd;
/* the driver saves the prototype command here for use in its go routine */
	int	um_cmd;		/* communication to dgo() */
	int	um_ubinfo;	/* save unibus registers, etc */
	struct	buf um_tab;	/* queue of devices for this controller */
};

/*
 * Per ``device'' structure.
 * (A controller has devices or uses and releases buffered data paths).
 * (Everything else is a ``device''.)
 *
 * If a controller has many drives attached, then there will
 * be several uba_device structures associated with a single uba_ctlr
 * structure.
 *
 * This structure contains all the information necessary to run
 * a unibus device such as a dz or a dh.  It also contains information
 * for slaves of unibus controllers as to which device on the slave
 * this is.  A flags field here can also be given in the system specification
 * and is used to tell which dz lines are hard wired or other device
 * specific parameters.
 */
struct uba_device {
	struct	uba_driver *ui_driver;
	char	*ui_devname;	/* name of the device */
	short	ui_unit;	/* unit number on the system */
	caddr_t	ui_parent;
	int	ui_adpt;	/* which i/o bus it is on */
	int	ui_nexus;	/* which nexus on i/o bus */
	short	ui_ubanum;	/* the uba it is on */
	short	ui_ctlr;	/* mass ctlr number; -1 if none */
	short	ui_slave;	/* slave on controller */
	int	(**ui_intr)();	/* interrupt handler(s) */
	caddr_t	ui_addr;	/* address of device in i/o space */
	short	ui_dk;		/* if init 1 set to number for iostat */
	int	ui_flags;	/* parameter from system specification */
	short	ui_alive;	/* device exists */
	short	ui_type;	/* driver specific type information */
	caddr_t	ui_physaddr;	/* phys addr, for standalone (dump) code */
/* this is the forward link in a list of devices on a controller */
	struct	uba_device *ui_forw;
/* if the device is connected to a controller, this is the controller */
	struct	uba_ctlr *ui_mi;
	struct	uba_hd *ui_hd;
};
#endif

/*
 * Per-driver structure.
 *
 * Each unibus driver defines entries for a set of routines
 * as well as an array of types which are acceptable to it.
 * These are used at boot time by the configuration program.
 */
struct uba_driver {
	int	(*ud_probe)();		/* see if a driver is really there */
	int	(*ud_slave)();		/* see if a slave is there */
	int	(*ud_attach)();		/* setup driver for a slave */
	int	(*ud_dgo)();		/* fill csr/ba to start transfer */
	u_short	*ud_addr;		/* device csr addresses */
	char	*ud_dname;		/* name of a device */
	struct	uba_device **ud_dinfo;	/* backpointers to ubdinit structs */
	char	*ud_mname;		/* name of a controller */
	struct	uba_ctlr **ud_minfo;	/* backpointers to ubminit structs */
	short	ud_xclu;		/* want exclusive use of bdp's */
};
#endif

/*
 * Flags to UBA map/bdp allocation routines
 */
#define	UBA_NEEDBDP	0x01		/* transfer needs a bdp */
#define	UBA_CANTWAIT	0x02		/* don't block me */
#define	UBA_NEED16	0x04		/* need 16 bit addresses only */
#define	UBA_HAVEBDP	0x08		/* use bdp specified in high bits */
#define UBA_MAPANYWAY	0x10		/* map anyway on MicroVAX I */

/*
 * Macros to bust return word from map allocation routines.
 */
#define	UBAI_BDP(i)	((int)(((unsigned)(i))>>28))
#define	UBAI_NMR(i)	((int)((i)>>18)&0x3ff)
#define	UBAI_MR(i)	((int)((i)>>9)&0x1ff)
#define	UBAI_BOFF(i)	((int)((i)&0x1ff))

#ifndef LOCORE
#ifdef KERNEL
/*
 * UBA related kernel variables
 */
int	numuba;					/* number of uba's */
struct	uba_hd uba_hd[];
int tty_ubinfo[];			/* allocated unibus map for ttys */ 

/*
 * Ubminit and ubdinit initialize the mass storage controller and
 * device tables specifying possible devices.
 */
extern	struct	uba_ctlr ubminit[];
extern	struct	uba_device ubdinit[];

/*
 * UNIbus device address space is mapped by UMEMmap
 * into virtual address umem[][].
 */
extern	struct pte UMEMmap[][512];	/* uba device addr pte's */
extern	char umem[][512*NBPG];		/* uba device addr space */

/*
 * Qbus device address space is mapped by QMEMmap
 * into virtual address qmem[][].
 * NOTE: also used by VAXstar (no-bus).
 */
extern	struct pte QMEMmap[][512];	/* qbus device addr pte's */
extern	char qmem[][512*NBPG];		/* qbus device addr space */

/*
 * Part of VAXstar device address space is mapped by NMEMmap
 * into virtual address nmem[][].
 */
extern	struct pte NMEMmap[][512];	/* nobus device addr pte's */
extern	char nmem[][512*NBPG];		/* nobus device addr space */

/*
 * VAXstation 2000 color option address map
 */
extern	struct pte SGMEMmap[][512];	/* nobus device addr pte's */
extern	char sgmem[][512*NBPG];		/* nobus device addr space */

/*
 * MicroVAX 2000 serial line expander (8 line SLU) map.
 */
extern	struct pte SHMEMmap[][512];	/* nobus device addr pte's */
extern	char shmem[][512*NBPG];		/* nobus device addr space */

/*
 * Since some VAXen vector their unibus interrupts
 * just adjacent to the system control block, we must
 * allocate space there when running on ``any'' cpu.  This space is
 * used for the vectors for uba0 and uba1 on all cpu's.
 */
extern	int (*UNIvec[])();			/* unibus vec for uba0 */

/*
 * On 780's and 8600's, we must set the scb vectors for the nexus of the
 * UNIbus adaptors to vector to locore unibus adaptor interrupt dispatchers
 * which make 780's look like the other VAXen.
 */
#ifdef VAX8600
extern	Xua0int(), Xua1int(), Xua2int(), Xua3int(), Xua4int(), Xua5int(), Xua6int();
#else
#ifdef VAX780
extern	Xua0int(), Xua1int(), Xua2int(), Xua3int();
#endif VAX780
#endif VAX8600

extern int cvec;
extern int br;
#endif KERNEL
#endif !LOCORE

/*
 *  definitions for the zero vector timer
 */
#define	ZVINTVL		300	/* zero vector interval in seconds */
#define ZVTHRESH	100000	/* zero vector reporting threshold */

/* Stuff for common I/O buffer sharing between vaxstar disk and TK50 */
struct vsbuf {
	u_char	vs_active;	/* vaxstar buffer is being used		      */
	u_char	vs_id;		/* Device ID				      */
	u_char	vs_wants;	/* the device ID that wants the bus	      */
};

#define VS_IDLE		0	/* Buffer not being used		      */
#define VS_SDC		1	/* vaxstar disk driver			      */
#define VS_ST		2	/* vaxstar tape driver			      */

/*
 * Values returned to vs_bufctl
 */
#define VS_DONE		0	/* Done					      */
#define VS_IP		1	/* In progress				      */
#define VS_WANT		2	/* More requests, still want the buffer	      */

/*
 * Action values
 */
#define	VS_DEALLOC	0	/* Deallocate the vaxstar buffer	      */
#define VS_ALLOC	1	/* Allocate the vaxstar buffer		      */

/*
 * Stillwant values
 */
#define	VS_DONTWANT	0	/* Don't want the buffer back		      */
#define VS_STILLWANT	1	/* Still want the buffer for next request     */


