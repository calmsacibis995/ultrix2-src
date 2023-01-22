/* @(#)errlog.h	1.25	 (ULTRIX)	8/28/86 */

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************

 *			Modification History				*
 *									*
 * 04-Dec-86 -- pmk added address int to stack structure 	 	*
 *									*
 * 28-Aug-86 -- bjg added sxerr to el_rec union (shadowfax)		*
 *									*
 * 09-Jul-86 -- bjg added vaxstar memory controller define		*
 *									*
 * 11-Jun-86 -- bjg added support for tmscp controller and device 	*
 *		logging							*
 *									*
 * 10-Jun-86 -- map added support for mscp controller and device	*
 *		logging							*
 *									*
 * 03-Jun-86 -- bjg added lynx support					*
 *									*
 * 30-May-86 -- pmk  changed stack dump structure and used in el_pnc.	*
 *	also added two new ELMETYP defines.				*
 *									*
 * 14-May-86 -- pmk  added bvp structure, cleaned up bier,bigen,mniadp.	*
 *	added bvp defines, mscp disk defines & tmscp tape defines	*
 *									*
 * 29-Apr-86 -- jaw add error logging for nmi faults.			*
 *									*
 * 23-Apr-86 -- pmk   changed el_bua to el_bigen, added 630 memory	*
 *	type, and added bla device type.				*
 *									*
 * 02-Apr-86 -- pmk   changed EVALID selwakeup to schedeldaemon         *
 *									*
 * 01-Apr-86 -- map   added UQ controller type definitions		*
 *									*
 * 12-Mar-86	bjg   moved uba and sbi macro definitions to errlog.h 	*
 *			from kern_errlog.c				*
 *									*
 * 05-Mar-86 -- bjg   added new SIZE defines (APPEND, DUMP, 2048);	*
 *				added tmslg struct defn;		*
 *				added elbuf and defines 		*
 *									*
 * 21-Feb-86 -- bjg   added 8600, 8650 defines;remove pid,uid from      *
 *				block dev structure;removed ci_rev      *
 *				struct from ci dattn packet             *
 *									*
 * 19-Feb-86 -- pmk   added trailer definition.				*
 *									*
 * 14-Feb-86 -- map   add uq definition					*
 *									*
 * 12-Feb-86 -- pmk   changed 8800 nautilus, added ps/psl to bier & bua	*
 *									*
 * 04-Feb-86 -- jaw   hard link to /sys/vaxbi changed.			*
 *									*
 * 20-Jan-86 -- pmk							*
 *	Initial Creation for Error Logging				*
 *									*
 ************************************************************************/
#ifndef __ERRLOG__
#define __ERRLOG__ 

#ifndef BI_INCLUDE
#include "../vaxbi/bireg.h"
#endif BI_INCLUDE

/* Error Log Packet Structures */

/* class types (el_sub_id.subid_class) */

/* hardware detected errors 100-199 */
#define ELCT_MCK	100			/* machine check */
#define ELCT_MEM	101			/* mem. crd/rds */
#define ELCT_DISK	102			/* disk errs */
#define ELCT_TAPE	103			/* tape errs */
#define ELCT_DCNTL	104			/* device controller errs */
#define ELCT_ADPTR	105			/* adapter errs */
#define ELCT_BUS	106			/* bus errs */
#define ELCT_SINT	107			/* stray intr. */
#define ELCT_AWE	108			/* async. write err */
#define ELCT_EXPTFLT	109			/* panic exception/fault */
#define ELCT_NMIEMM	110			/* 8800 emm exception */
#define ELCT_CTE	111			/* console timeout entry */
#define ELCT_STKDMP	112			/* stack dump */

/* software detected errors 200-249 */
#define ELSW_PNC 	200			/* panic (bug check) */

/* informational ascii message 250-299 */
#define ELMSGT_INFO	250			/* info. type msg */
#define ELMSGT_SNAP8600 251			/* 8600 snapshot taken */

/* Operational Class 300-400 */

/* reload/restart 300-350 */
#define ELMSGT_SU	300			/* start up msg */
#define ELMSGT_SD	301			/* shutdown msg */

/* time stamp 310 */
#define ELMSGT_TIM	310			/* time stamp */


/* usage 311-315 */

/* statistics 316-319 */

/* maintenance 350-399 */
#define ELMSGT_DIAG	350			/* diag. info. type msg */
#define ELMSGT_REPAIR   351			/* repair */

/* mchk type frames (el_sub_id.subid_type)*/
#define ELMCKT_780	1			/* 780 machine chk frame */
#define ELMCKT_750	2			/* 750 machine chk frame */
#define ELMCKT_730	3			/* 730 machine chk frame */
#define ELMCKT_8600	4			/* 8600 machin chk frame */
#define ELMCKT_8200	5			/* 8200 machin chk frame */
#define ELMCKT_8800	6			/* 8800 machin chk frame */
#define ELMCKT_UVI	7			/* uvax-1 mach chk frame */
#define ELMCKT_UVII	8			/* uvax-2 mach chk frame */

/* stray intr types (el_sub_id.subid_type) */
#define ELSI_SCB	1			/* interrupt scb */
#define ELSI_UNI	2			/* interrupt unibus */

/* console timeout entry types (el_sub_id.subid_type) */
#define ELCTE_8600	1			/* for 8600 */
#define ELCTE_8800	2			/* for 8800 */

/* device error types (el_sub_id.subid_type) */
#define ELDEV_MSCP	1
#define ELDEV_REGDUMP	2

/* device controller error types (el_sub_id.subid_type) */
#define ELCI_ATTN	1
#define ELCI_LPKT	2
#define	ELUQ_ATTN	3
#define ELBI_BLA	4
#define ELBI_BVP	5
#define ELMSCP_CNTRL	6
#define ELTMSCP_CNTRL	7

/* adapter error types (el_sub_id.subid_type) */
#define ELADP_UBA	1
#define ELADP_BUA	2
#define ELADP_NBI	3

/* bus error types (el_sub_id.subid_type) */
#define ELBUS_SBI780	1			/*  780 sbi fault type */
#define ELBUS_SBI8600	2			/* 8600 sbi faults */
#define ELBUS_BIER	3			/* 8200 bi errors */
#define ELBUS_NMIFLT	4			/* 8800 nmi fault */

/* stack dump error types (el_sub_id.subid_type) */
#define ELSTK_KER	1
#define ELSTK_INT	2
#define ELSTK_USR	3

/* mem cntl types (el_sub_id.subid_ctldevtyp) */
#define ELMCNTR_780C	1			/* 780C mem. cntl */
#define ELMCNTR_780E	2			/* 780E mem. cntl */
#define ELMCNTR_750	3			/* 750/730 mem. cntl. */
#define ELMCNTR_730	4			/* 750/730 mem. cntl. */
#define ELMCNTR_8600	5			/* 8600 mem. cntl */
#define ELMCNTR_BI	6			/* 8200 BI mem. cntl. */
#define ELMCNTR_NMI	7			/* 8800 NMI mem. cntl. */
#define ELMCNTR_630	8			/* 630 mem. cntl. */
#define ELMCNTR_VAXSTAR	9			/* vaxstar mem. cntl. */

/* ci hardware port types (el_sub_id.subid_ctldevtyp) */
#define	ELCIHPT_CI780	2
#define	ELCIHPT_CI750	3
#define	ELCIHPT_CIBCI	5
#define	ELCIHPT_CIBCA	11

/* bvp hardware port types (el_sub_id.subid_ctldevtyp) */
#define	ELBVP_AIE	1
#define	ELBVP_AIE_TK	2
#define	ELBVP_AIO	3
#define ELBVP_ACP	4
#define ELBVP_SHDWFAX	5

/* uq hardware port types (el_sub_id.subid_ctldevtyp) */
#define	ELUQHPT_UDA50	0		
#define	ELUQHPT_RC25	1
#define	ELUQHPT_RUX50	2		
#define	ELUQHPT_TK50	3		
#define	ELUQHPT_TU81	5
#define	ELUQHPT_UDA50A	6		
#define	ELUQHPT_RQDX	7
#define	ELUQHPT_KDA50	13
#define	ELUQHPT_TK70	14
#define	ELUQHPT_RV80	15
#define	ELUQHPT_RRD50	16
#define	ELUQHPT_KDB50	18
#define	ELUQHPT_RQDX3	19

/* controller types mscp (el_sub_id.subid_ctldevtyp) */
#define	ELMPCT_HSC50	1		
#define	ELMPCT_UDA50	2		
#define	ELMPCT_RC25	3
#define	ELMPCT_VMS	4
#define	ELMPCT_TU81	5
#define	ELMPCT_UDA50A	6		
#define	ELMPCT_RQDX	7
#define	ELMPCT_TOPS	8
#define	ELMPCT_TK50	9		
#define	ELMPCT_RUX50	10		
#define	ELMPCT_KFBTA	12
#define	ELMPCT_KDA50	13
#define	ELMPCT_TK70	14
#define	ELMPCT_RV80	15
#define	ELMPCT_RRD50	16
#define	ELMPCT_KDB50	18
#define	ELMPCT_RQDX3	19
#define	ELMPCT_HSC70	32
#define	ELMPCT_HSB50	64
#define	ELMPCT_ULTRIX	248

/* disks types mscp (el_sub_id.subid_ctldevtyp) */
#define ELDT_RA80	1
#define ELDT_RC25	2
#define ELDT_RCF25	3
#define ELDT_RA60	4
#define ELDT_RA81	5
#define ELDT_RD51	6
#define ELDT_RX50	7
#define ELDT_RD52	8
#define ELDT_RD53	9
#define ELDT_RX33	10
#define ELDT_RA82	11
#define ELDT_RD31	12
#define ELDT_RD54	13
#define ELDT_RRD50	14
#define ELDT_RV80	15
#define ELDT_RX18	17
#define ELDT_RA70	18
#define ELDT_RA90	19

/* tapes types tmscp (el_sub_id.subid_ctldevtyp) */
#define ELTT_TA78	1
#define ELTT_TU81	2
#define ELTT_TK50	3
#define ELTT_TA81	4
#define ELTT_TK70	14
#define ELTT_TRV80	15

/* panic exception/fault errcodes (el_sub_id.subid_type) */
#define ELEF_RAF	1			/* reserved addr fault */
#define ELEF_PIF	2			/* privileged instr fault */
#define ELEF_ROF	3			/* reserved operand fault */
#define ELEF_BPT	4			/* bpt instr fault */
#define ELEF_XFC	5			/* xfc instr fault */
#define ELEF_SYSCALL	6			/* system call exception/fault*/
#define ELEF_AT		7			/* arithmetic exception/fault */
#define ELEF_AST	8			/* ast exception/fault */
#define ELEF_SEG	9			/* segmentation fault */
#define ELEF_PRO	10			/* protection fault */
#define ELEF_TRACE	11			/* trace exception/fault */
#define ELEF_CMF	12			/* compatibility mode fault */
#define ELEF_PF		13			/* page fault  */
#define ELEF_PTF	14			/* page table fault */

/* elmemerr.type */
#define ELMETYP_CRD	1
#define ELMETYP_RDS	2
#define ELMETYP_CNTL	3
#define ELMETYP_WMASK	4
#define ELMETYP_PAR	5
#define ELMETYP_NXM	6

/* el_sub_id.subid_num  for machine check is cpu number
 * 780,750,730  num = 0
 * MVAX		num = 0
 * 8600		num = 0
 * 8200 	num = bi node number
 * 8800	naut.	num = 0 or 1, right or left cpu
 */

/* 780/8600 sbi error codes (el_sub_id.subid_errcode) */
#define SBI_ERROR 0
#define SBI_WTIME 1		/* 780 only */
#define SBI_ALERT 2
#define SBI_FLT   3
#define SBI_FAIL  4

#define Elsbi elrp->el_body.elsbia8600
#define Eluba elrp->el_body.eluba780
#define Sbi8600 elrp->el_body.elsbia8600
#define Sbi780 elrp->el_body.elsbiaw780

/* ci error codes definition (el_sub_id.subid_errcode)
 * 3 3 2 2 2 2 2 2 2 2 2 2 1 2 1 1 1 1 1 1 1 1 
 * 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +---------------+-----------------------------------------------+
 * | R      | E |G|                                                |
 * | S      | T |E|                                                |
 * | V      | Y |N|                  ERROR_CODE                    |
 * | D      | P |E|                                                |
 * |        | E |R|                                                |
 * |        |   |R|                                                |
 * +---------------+-----------------------------------------------+
 *		
 *  Bits		     Function
 * -----		-----------------
 *  0-23		ERROR_CODE
 *    24		GENERR - Error is common to all SCA ports
 * 25-26		ETYPE  - Error Type Code
 *		        Informational		0x00
 * 		        Virtual Path Crash	0x01
 * 		        Local Port Crash	0x02
 * 28-29		RSVD   - Reserved
 * 30-31		RSVD   - Reserved
 *		
 * Error codes ( ERROR_CODE ) are densely assigned for each possable combination
 * of ETYPE and GENERR.
 */
/*  elbd_flags (el_bdhdr) */
/* 0x00 = write */
/* 0x01 = read */
/* 0x02 = done */
/* 0x04 = error */

#define EL_INVALID 	0
#define EL_VALID 	1
#define EL_UNDEF	-1
#define EL_PRISEVERE	1
#define EL_PRIHIGH	3
#define EL_PRILOW	5
#define EL_STKDUMP	512	
#define EL_SIZE256	256
#define EL_SIZE128	128
#define EL_SIZE64	64
#define EL_SIZE16	16
#define EL_SIZE12	12
#define EL_SIZE2048	2048
#define EL_SIZEAPPND	1400
#define EL_DUMPSIZE 	10240
#define EL_BSIZE	(8192 - 3 * sizeof(char *))
#define EL_SSIZE	(2048 - sizeof(char *))
#define EL_BEG elbuf.kqueue
#define EL_END elbuf.kqueue + EL_BSIZE
#define EL_SBEG elbuf.squeue
#define EL_SEND elbuf.squeue + EL_SSIZE


#define EL_MAXRECSIZE	1536
#define EL_MAXAPPSIZE	(EL_MAXRECSIZE - EL_MISCSIZE)
#define EL_FULL (struct el_rec *) 0

#define EL_RHDRSIZE	sizeof(struct el_rhdr)
#define EL_SUBIDSIZE	sizeof(struct el_sub_id)
#define EL_TRAILERSIZE  4
#define EL_MISCSIZE  	(EL_RHDRSIZE + EL_SUBIDSIZE + EL_TRAILERSIZE)
#define EL_CILPKTSUBTRACT  EL_MISCSIZE + sizeof(struct elci_common)+ \
			sizeof(struct ci_lcommon) + sizeof(struct ci_packet)
#define EL_EXPTFLTSIZE	sizeof(struct el_exptflt)
#define EL_UBASIZE	sizeof(struct el_uba780)
#define EL_PNCSIZE	sizeof(struct el_pnc)
#define EL_MEMSIZE	sizeof(struct el_mem)

#define EL_REGMASK	0x012f3f19

#define trailer "%~<^"

extern struct proc *elprocp;
extern int schedeldaemon();
extern struct callout *callfree;
#define EVALID(eptr) \
	eptr->elrhdr.rhdr_valid = EL_VALID; \
	if (callfree != 0) { \
		(void)schedeldaemon(); \
	}

#define LSUBID(eptr, class, type, ctldev, num, unitnum, errcode) \
	eptr->elsubid.subid_class  = class; \
	eptr->elsubid.subid_type = type; \
	eptr->elsubid.subid_ctldevtyp = ctldev; \
	eptr->elsubid.subid_num = num; \
	eptr->elsubid.subid_unitnum = unitnum; \
	eptr->elsubid.subid_errcode = errcode;

/* Error Logger Buffer */
#ifdef KERNEL
/* kqueue full implies |in-out| = 1 */
struct elbuf {
	char *in;		/* input ptr */
	char *out;		/* output ptr */
	char *le;		/* logical end */
	char kqueue[EL_BSIZE];
	char *sin;		/* input ptr for severe area */
	char squeue[EL_SSIZE];
} elbuf;
#endif

struct el_rhdr {			/* errlog header */
	u_short rhdr_reclen;		/* errlog record length */
	u_short rhdr_seqnum; 		/* seq. number */
	u_long rhdr_time;		/* time in sec */
	u_long rhdr_sid;		/* system id, filled in by elcs */
	u_char rhdr_valid;		/* valid error record */
	u_char rhdr_pri;		/* priority hi - low */
	u_char rhdr_elver[2];		/* errlog version,filled in by elcs */
	char rhdr_hname[EL_SIZE12];	/* host name, filled in by elcs */
};

struct el_sub_id {			/* sub id packet */
	u_short subid_class;		/* class type, ELCT_DISK */
	u_char subid_type;		/* error type, ELDEV_MSCP */
	u_char subid_ctldevtyp;		/* controller/device type,  ELDT_RA60 */
	u_short subid_num;		/* cpu number - mchk errors */
					/* bus number - bus/adptr/cntr errors */
					/* cntl number - device errors */
	u_short subid_unitnum;		/* adpt number - adapter errors */
					/* cntl number - controller errors */
					/* unit number - device errors */
	u_long subid_errcode;		/* ci,mck summ.,cte,emm,exptflt */
};

struct el_mem {				/* mem. crd/rds packet */
	short elmem_cnt;		/* num. of mem. err structures */
	struct el_memerr {
		short cntl;		/* cntl. number 1-? */
		u_char type;		/* type err 1-crd,2-rds,3-cntl,4-wmask*/
		u_char numerr;		/* num. of errors on this address */
		int regs[4];		/* mem. regs */
	} elmemerr;			/* mem. err structure */
};

struct el_devhdr {			/* device header packet */
	dev_t devhdr_dev;		/* dev. major/minor numbers */
	long devhdr_flags;		/* buffer flags */
	long devhdr_bcount;		/* byte count of transfer */
	daddr_t devhdr_blkno;		/* logical block number */
	short devhdr_retrycnt;		/* retry count */
	short devhdr_herrcnt;		/* hard err count total */
	short devhdr_serrcnt;		/* soft err count total */
	short devhdr_csr;		/* device csr */
};
/* MSCP uq packet header */
struct mslg_header {
	short	uda_msglen;		/* Length of MSCP packet	*/
	char	uda_credits;		/* 0-3: credits, 4-7: msgtype	*/
	char	uda_vcid;		/* Virtual circuit id (con. id.)*/
};
/* disk mscp structure */
struct mslg {
	struct	mslg_header mslg_header;/* device specific header */
	long	mslg_cmdref;		/* command reference number */
	short	mslg_unit;		/* unit number */
	short	mslg_seqnum;		/* sequence number */
	u_char	mslg_format;		/* format */
	u_char	mslg_flags;		/* error log message flags */
	short	mslg_event;		/* event code */
	quad	mslg_cntid;		/* controller id */
	u_char	mslg_cntsvr;		/* controller software version */
	u_char	mslg_cnthvr;		/* controller hardware version */
	short	mslg_multunt;		/* multi-unit code */
	quad	mslg_unitid;		/* unit id */
	u_char	mslg_unitsvr;		/* unit software version */
	u_char	mslg_unithvr;		/* unit hardware version */
	short	mslg_group;		/* group; retry + level */
	long	mslg_volser;		/* volume serial number */
	long	mslg_hdr;		/* header */
	char	mslg_sdistat[12];	/* SDI status information */
};
/* TMSCP packet info (same as MSCP) */
struct tmslg_header {
	short	tmscp_msglen;		/* Length of MSCP packet	*/
	char	tmscp_credits;		/* 0-3: credits, 4-7: msgtype	*/
	char	tmscp_vcid;		/* Virtual circuit id (con. id.)*/
};
/* tape mscp structure */
struct tmslg {
	struct	tmslg_header mslg_header;/* device specific header */
	long	mslg_cmdref;		/* command reference number */
	short	mslg_unit;		/* unit number */
	short	mslg_seqnum;		/* sequence number */
	u_char	mslg_format;		/* format */
	u_char	mslg_flags;		/* error log message flags */
	short	mslg_event;		/* event code */
	quad	mslg_cntid;		/* controller id */
	u_char	mslg_cntsvr;		/* controller software version */
	u_char	mslg_cnthvr;		/* controller hardware version */
	short	mslg_multunt;		/* multi-unit code */
	quad	mslg_unitid;		/* unit id */
	u_char	mslg_unitsvr;		/* unit software version */
	u_char	mslg_unithvr;		/* unit hardware version */
	short	mslg_group;		/* group; retry + level */
	long	mslg_position;		/* position (object count) */
	u_char	mslg_fmtsvr;		/* formatter software version */
	u_char	mslg_fmthvr;		/* formatter hardware version */
	short	mslg_xxx2;		/* unused */
	char	mslg_stiunsucc[62];	/* STI status information */
};
struct el_bdev {			/* block device packet disk/tape */
	struct el_devhdr eldevhdr;	/* device header packet */
	union {
		u_int devreg[22];	/* device regs. non mscp drivers */
		struct mslg mslg;	/* mscp datagram packet */
		struct tmslg tmslg;	/* tmscp datagram packet */
	} eldevdata;
};


struct el_sbi_aw780 {			/* 780 sbi fault/async. write packet */
	int sbiaw_er;			/* sbi error reg */
	int sbiaw_toa;			/* time out address */
	int sbiaw_fs;			/* sbi fault status */
	int sbiaw_sc;			/* sbi silo compare */
	int sbiaw_mt;			/* sbi maint. reg. */
	int sbiaw_silo[EL_SIZE16];	/* sbi silo 16 regs */
	int sbiaw_csr[EL_SIZE16];	/* sbi csr's  num. nexi */
	int sbiaw_pc;
	int sbiaw_psl;
};

struct el_sbia8600 {			/* 8600 sbi fault/alert/error packet */
	int sbia_ioaba;			/* ioa baseaddress */
	int sbia_dmacid;		/* dmac id reg */
	int sbia_dmacca;		/* dmac cmd-addr reg */
	int sbia_dmabid;		/* dmab id reg */
	int sbia_dmabca;		/* dmab cmd-addr reg */
	int sbia_dmaaid;		/* dmaa id reg */
	int sbia_dmaaca;		/* dmaa cmd-addr reg */
	int sbia_dmaiid;		/* dmai id reg */
	int sbia_dmaica;		/* dmai cmd-addr reg */
	int sbia_ioadc;			/* ioa diag cntl reg */
	int sbia_ioaes;			/* ioa error summary reg */
	int sbia_ioacs;			/* ioa cntl-status reg */
	int sbia_ioacf;			/* ioa config reg */
	int sbia_er;			/* sbi error reg */
	int sbia_to;			/* sbi time out address */
	int sbia_fs;			/* sbi fault status */
	int sbia_sc;			/* sbi silo compare */
	int sbia_mr;			/* sbi maint. reg */
	int sbia_silo[EL_SIZE16];	/* silo regs 16 */
	int sbia_csr[EL_SIZE16];	/* sbi csr's  num nexi */
	int sbia_pc;
	int sbia_psl;
};

struct el_uba780 {	 		/* 780 uba fault/error packet */
	int uba_cf;			/* uba config reg */
	int uba_cr;			/* uba control reg */
	int uba_sr;			/* status reg */
	int uba_dcr;			/* diag. cntl. reg. */
	int uba_fmer;			/* failed map entry reg */
	int uba_fubar;			/* failed unibus addr. reg */
	int uba_pc;
	int uba_psl;
};

struct el_bigen {			/* bi fault/error packet bua/bla */
	int bigen_dev;			/* device type reg */
	int bigen_bicsr;		/* bi csr reg */
	int bigen_ber;			/* bi err reg */
	int bigen_csr;			/* control & status reg */
	int bigen_fubar;		/* failed unibus addr. reg */
	int bigen_pc;
	int bigen_psl;
};

struct el_bier  {				/* bi bus err packet */
	short bier_nument;			/* number of entries */
	struct bi_regs biregs[EL_SIZE16];	/* bi err reg struct */
						/* 16 nodes possible */
	int bier_pc;
	int bier_psl;
};

struct el_uq {				/* uq device attention information */
	u_long	sa_contents;		/* sa register contents		   */
};

struct el_bvp {
	u_long bvp_biic_typ;		/* port biic type reg */
	u_long bvp_biic_csr;		/* port biic csr reg */
	u_long bvp_pcntl;		/* port control reg */
	u_long bvp_pstatus;		/* port status reg */
	u_long bvp_perr;		/* port error reg */
	u_long bvp_pdata;		/* port data reg */
};

struct el_sxerr {
	int bi_csr;			/* BIIC error info */
	int bi_buserr;			/* valid ONLY if bi_csr bus err bit */
	int ACP_status;			/* subsystem status */
	int diag_reg;			/* powerup self-test error register */
	int subtest_num;		/* number of failing subtest */
	int error_info[10];		/* error-specific info */
};

struct elci_dattn {			/* ci device attention information */
	struct ci_regs {
	    u_long ci_pcnf;		/* port configuration register */
	    u_long ci_pmcsr;		/* port maint control & status reg */
	    u_long ci_psr;		/* port status register */
	    u_long ci_pfaddr;		/* port failing address reg */
	    u_long ci_per;		/* port error register */
	    u_long ci_ppr;		/* port parameter register */
	} ciregs;
	struct bi_regs cibiregs;	/* optional biic device regs */
	union ci_dattnopt {		/* optional device attention info */
		struct ci_ucode {
			u_long ci_addr;		/* faulty ucode address */
			u_long ci_bvalue;	/* bad ucode value */
			u_long ci_gvalue;	/* correct ucode value */
		} ciucode;
	} cidattnopt;
};

struct elci_lpkt {			/* ci logged packet information */
	struct ci_lcommon {
		u_char ci_lsaddr[ 6 ];		/* local station address */
		u_char ci_lsysid[ 6 ];		/* local station id number */
		u_char ci_lname[ 8 ];		/* local system node name */
		u_char ci_rsaddr[ 6 ];		/* remote station address */
		u_char ci_rsysid[ 6 ];		/* remote station id number */
		u_char ci_rname[ 8 ];		/* remote system node name */
	} cilcommon;
	union ci_lpktopt {		/* optional logged packet info */
		struct ci_packet {		/* logged packet information */
			u_char ci_port;		/* destination port */
			u_char ci_status;	/* status */
			u_char ci_opcode;	/* command operation code */
			u_char ci_flags;	/* port command flags */
		} cipacket;
		u_short	ci_rreason;		/* remote reason path crash*/
		struct ci_protocol {		/* ci ppd protocol info */
			u_char	ci_local;	/* local ci ppd version */
			u_char	ci_remote;	/* remote ci ppd version */
		} ciprotocol;
		struct ci_dbcoll {		/* database collision info */
			u_char ci_ksysid[ 6 ];  /* known system id number */
			u_char ci_kname[ 8 ];	/* known system node name */
		} cidbcoll;
	} cilpktopt;
};


struct el_ci {					/* ci error packet */
	struct elci_common {
		u_short ci_nerrs;			/* number of errors */
		u_short ci_nreinits;			/* num port reinits */
	} cicommon;
	union {
	    struct elci_dattn elcidattn;	/* device attention info */
	    struct elci_lpkt elcilpkt;		/* logged packet info */
	} elci_types;
};

struct el_nmiflt {
	int nmiflt_nmifsr;		/*  primary cpu */
	int nmiflt_nmiear;		/*  primary cpu */
	int nmiflt_memcsr0;		/*  memory fault bits */
	int nmiflt_nbia0;		/*  nbia0 fault bits */
	int nmiflt_nbia1;		/*  nbia1 fault bits */
	int nmiflt_nmisilo[EL_SIZE256];	/*  silo data */
};

struct el_nmiadp {
	int nmiadp_nbiacsr0;		/* nmi bi adp csr0   */
	int nmiadp_nbiacsr1;		/* nmi bi adp csr1   */
	int nmiadp_nbib0err;		/* bi bus 0 err reg  */
	int nmiadp_nbib1err;		/* bi bus 1 err reg  */
};

struct el_strayintr {			/* stray intr. packet */
	u_char stray_ipl;		/* ipl level */
	short stray_vec;		/* vector */
};

struct el_exptflt {			/* panic exception/fault packet */
	int exptflt_va;			/* va. if appropriate else zero */
	int exptflt_pc;			/* pc at time of exception/fault */
	int exptflt_psl;		/* psl at time of exception/fault */
};

struct el_stkdmp {
        int addr;
	int size;
	int stack[EL_SIZE128];		/* stack dump of kernel/interpt/user */
};

struct el_pnc {				/* panic packet (bug check) */
	char pnc_asc[EL_SIZE64];	/* ascii panic string */
	int pnc_sp;
	int pnc_ap;
	int pnc_fp;
	int pnc_pc;
	struct pncregs {
	    int pnc_ksp;
	    int pnc_usp;
	    int pnc_isp;
	    int pnc_p0br;
	    int pnc_p0lr;
	    int pnc_p1br;
	    int pnc_p1lr;
	    int pnc_sbr;
	    int pnc_slr;
	    int pnc_pcbb;
	    int pnc_scbb;
	    int pnc_ipl;
	    int pnc_astlvl;
	    int pnc_sisr;
	    int pnc_iccs;
	} pncregs;
	struct el_stkdmp kernstk;		/* dump of kernel stack */
	struct el_stkdmp intstk;		/* dump of interrupt stack */
};

struct el_msg {				/* msg packet */
	short msg_len;			/* length */
	char msg_asc[EL_SIZE256];	/* ascii string */
};

struct el_timchg {
	struct timeval timchg_time;	/* time in sec and usec */
	struct timezone timchg_tz;	/* time zone data */
	char timchg_version[EL_SIZE16];	/* ultrix vx.x */
};

struct el_mc8600frame {
	int	mc8600_bytcnt;		/* machine check stack frame byte cnt */
	int	mc8600_ehm_sts; 	/* ehm.sts */
	int	mc8600_evmqsav; 	/* ebox vmq sav */
	int	mc8600_ebcs;		/* ebox control status register */
	int	mc8600_edpsr;		/* ebox data path status register */
	int	mc8600_cslint;		/* ebox console/interrupt register */
	int	mc8600_ibesr;		/* ibox error and status register */
	int	mc8600_ebxwd1;		/* ebox write data 1 */
	int	mc8600_ebxwd2;		/* ebox write data 1 */
	int	mc8600_ivasav;		/* ibox va sav */
	int	mc8600_vibasav; 	/* ibox viba */
	int	mc8600_esasav;		/* ibox esa */
	int	mc8600_isasav;		/* ibox isa */
	int	mc8600_cpc;		/* ibox cpc */
	int	mc8600_mstat1;		/* mbox status reg#1 */
	int	mc8600_mstat2;		/* mbox status reg#2 */
	int	mc8600_mdecc;		/* mbox data ecc register */
	int	mc8600_merg;		/* mbox error generator register */
	int	mc8600_cshctl;		/* mbox cache control register */
	int	mc8600_mear;		/* mbox error address register */
	int	mc8600_medr;		/* mbox error data register */
	int	mc8600_accs;		/* accelerator status register */
	int	mc8600_cses;		/* control store error status reg */
	int	mc8600_pc;		/* pc */
	int	mc8600_psl;		/* psl */
};

struct el_mc8800frame {
	int	mc8800_bcnt;			/* byte count */
	int	mc8800_mcsts;			/* cpu error status */
	int	mc8800_ipc;			/* istream pc */
	int	mc8800_vaviba;			/* va/viba register */
	int	mc8800_iber;			/* i reg */
	int	mc8800_cber;			/* c reg */
	int	mc8800_eber;			/* e reg */
	int	mc8800_nmifsr;			/* nmi  */
	int	mc8800_nmiear;			/* nmi  */
	int	mc8800_pc; 			/* macro pc */
	int	mc8800_psl;			/* psl */
};

struct el_mc8200frame {
	int	mc8200_bcnt;
	int	mc8200_summary;
	int	mc8200_parm1;
	int	mc8200_va;
	int	mc8200_vap;
	int	mc8200_mar;
	int	mc8200_stat;
	int	mc8200_pcfail;
	int	mc8200_upcfail;
	int	mc8200_pc;
	int	mc8200_psl;
};

struct el_mc780frame {
	int	mc8_bcnt;			/* byte count == 0x28 */
	int	mc8_summary;			/* summary parameter */
	int	mc8_cpues;			/* cpu error status */
	int	mc8_upc;			/* micro pc */
	int	mc8_vaviba;			/* va/viba register */
	int	mc8_dreg;			/* d register */
	int	mc8_tber0;			/* tbuf error reg 0 */
	int	mc8_tber1;			/* tbuf error reg 1 */
	int	mc8_timo;			/* timeout address */
	int	mc8_parity;			/* parity */
	int	mc8_sbier;			/* sbi error register */
	int	mc8_pc; 			/* trapped pc */
	int	mc8_psl;			/* trapped psl */
};

struct el_mc750frame {
	int	mc5_bcnt;			/* byte count == 0x28 */
	int	mc5_summary;			/* summary parameter */
	int	mc5_va; 			/* virtual address register */
	int	mc5_errpc;			/* error pc */
	int	mc5_mdr;
	int	mc5_svmode;			/* saved mode register */
	int	mc5_rdtimo;			/* read lock timeout */
	int	mc5_tbgpar;			/* tb group parity error reg */
	int	mc5_cacherr;			/* cache error register */
	int	mc5_buserr;			/* bus error register */
	int	mc5_mcesr;			/* machine check status reg */
	int	mc5_pc; 			/* trapped pc */
	int	mc5_psl;			/* trapped psl */
};

struct el_mc730frame {
	int	mc3_bcnt;			/* byte count == 0xc */
	int	mc3_summary;			/* summary parameter */
	int	mc3_parm[2];			/* parameter 1 and 2 */
	int	mc3_pc; 			/* trapped pc */
	int	mc3_psl;			/* trapped psl */
};

struct el_mcUVIframe {
	int	mc1_bcnt;			/* byte count == 0xc */
	int	mc1_summary;			/* summary parameter */
	int	mc1_parm[2];			/* parameter 1 and 2 */
	int	mc1_pc; 			/* trapped pc */
	int	mc1_psl;			/* trapped psl */
};

struct el_mcUVIIframe {
	int	mc1_bcnt;			/* byte count == 0xc */
	int	mc1_summary;			/* summary parameter */
	int	mc1_vap;			/* most recent virtual addr */
	int	mc1_internal_state;		/* internal state ? */
	int	mc1_pc; 			/* trapped pc */
	int	mc1_psl;			/* trapped psl */
};

struct el_mck {					/* machine check packet */
	union {
		struct el_mc8800frame el8800mcf;
		struct el_mc8600frame el8600mcf;	/* mck frame that is */
		struct el_mc8200frame el8200mcf;	/* pushed on intr.   */
		struct el_mc780frame el780mcf;		/* stack by micro    */
		struct el_mc750frame el750mcf;		/* code              */
		struct el_mc730frame el730mcf;
		struct el_mcUVIIframe elUVIImcf;
		struct el_mcUVIframe elUVImcf;
	} elmck_frame;
};

struct el_rec {					/* errlog record packet */ 
	struct el_rhdr elrhdr;			/* record header */
	struct el_sub_id elsubid;		/* subsystem id packet */
	union {
		struct el_mck elmck;			/* machine check     */
		struct el_mem elmem;	       		/* memory crd errors */
		struct el_bdev elbdev;	  		/* device errors     */
		struct el_sbi_aw780 elsbiaw780;		/* sbi faults, async */
							/* writes            */
		struct el_sbia8600 elsbia8600;		/* sbi alerts 8600   */
		struct el_uba780 eluba780;		/* uba errors        */
		struct el_bier elbier;			/* bi errors         */
		struct el_uq eluq;			/* uq port errors    */
		struct el_bvp elbvp;			/* bvp port errors   */
		struct el_sxerr elsxerr;		/* shadowfax errors  */
		struct el_bigen	elbigen;		/* bi adp/cntl errors*/
		struct el_strayintr elstrayintr;	/* stray interrupts  */
		struct el_exptflt elexptflt;		/* panic exception & */
							/* fault             */
		struct el_nmiflt elnmiflt;		/* 8800 nmi faults   */
		struct el_nmiadp elnmiadp;		/* 8800 nmi adapter  */
		struct el_ci elci;			/* ci errors         */
		struct el_pnc elpnc;			/* panic frame       */
		struct el_stkdmp elstkdmp;		/* stack dump        */
		struct el_msg elmsg;			/* ascii text msg    */
		struct el_timchg eltimchg;		/* time change  &    */
							/* startup/shutdown  */
	} el_body;
	char eltrailer[EL_TRAILERSIZE];			/* asc trailer code  */
};

struct el_rec *ealloc();
#endif __ERRLOG__
