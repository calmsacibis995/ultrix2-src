
/*
 *	@(#)udareg.h	1.8	(ULTRIX)	7/11/86
 */

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
 ************************************************************************/

/*
 * udareg.h
 *
 * Modification history
 *
 * MSCP registers structure and definitions
 *
 * 11-Jan-85 - rjl
 *
 *	Moved the defination of the ra_info structure from the driver to
 *	here so that it could be included in autoconf for the purpose of
 *	autoselecting a rootdevice for the MicroVAX floppy boots.
 *
 *  4-Apr-85 - map
 *
 *	Added word udasaw in struct udadevice for BDA write-only SA.
 *	Added definitions for BDA.
 *
 *  5-Jun-85 - jaw
 *
 *	More junk for BDA.
 *
 * 10-Jun-86 - map
 *	
 *	Added fields to ra_info for support of BBR.
 *
 */

/* Register device structure */
struct udadevice {
	short	udaip;			/* Initialization and polling	*/
	short	udasa;			/* Status and address		*/
#define udasar	udasa
	short	udasaw; 		/* Write register for some	*/
};

/* Register device definitions */
#define UDA_ERR 	0100000 	/* Error bit			*/
#define UDA_STEP4	0040000 	/* Step 4 has started		*/
#define UDA_STEP3	0020000 	/* Step 3 has started		*/
#define UDA_STEP2	0010000 	/* Step 2 has started		*/
#define UDA_STEP1	0004000 	/* Step 1 has started		*/
#define UDA_NV		0002000 	/* So host settable int. vector */
#define UDA_QB		0001000 	/* Controller supports Q22 bus	*/
#define UDA_DI		0000400 	/* Controller implements diags. */
#define UDA_IE		0000200 	/* Interrupt enable		*/
#define UDA_PI		0000001 	/* Host req. adapter purge int. */
#define UDA_GO		0000001 	/* Start operation, after init. */

#define BDA_IP		0xF2		/* BIIC offset of BDA IP reg.	*/
#define BDA_SA		0xF4		/* BIIC off. of BDA r/o. SA reg.*/
#define BDA_SAW 	0xF6		/* BIIC off. of BDA w/o. SA reg.*/

/* MSCP Communications Area */
struct udaca {
	short	ca_xxx1;		/* Unused			*/
	char	ca_xxx2;		/* Unused			*/
	char	ca_bdp; 		/* BDP to purge 		*/
	short	ca_cmdint;		/* Command queue trans. int.flg.*/
	short	ca_rspint;		/* Response queue trans.int.flg.*/
	long	ca_rspdsc[NRSP];	/* Response descriptors 	*/
	long	ca_cmddsc[NCMD];	/* Command descriptors		*/
};
#define ca_ringbase	ca_rspdsc[0]

/* MSCP communications area definitions */
#define UDA_OWN 0x80000000		/* Port owns descrpt, else host */
#define UDA_INT 0x40000000		/* Allow interrupt on ring tran.*/
#define UDA_MAP 0x80000000		/* Mapped buffer modifier descr.*/


/* Software state per controller */
struct uda_softc {
	short	sc_state;		/* State of controller		*/
	short	sc_mapped;		/* Bus map allocated for uda[]	*/
	int	sc_ubainfo;		/* Bus mapping info.		*/
	struct uda *sc_uda;		/* Bus address of uda[] 	*/
	int	sc_ivec;		/* Interrupt vector address	*/
	short	sc_credits;		/* Transfer credits		*/
	short	sc_lastcmd;		/* Pointer into command ring	*/
	short	sc_lastrsp;		/* Pointer into response ring	*/
	short	sc_timer;		/* Counter for watch dog timer	*/
	struct	timeval sc_progress;	/* Last time progress occurred	*/
	int	sc_rate;		/* Number of hz between checks	*/
	int	sc_type;		/* What we are			*/
	int	sc_bbr_ip;		/* Number of BBRs in progress	*/
					/* on this controller		*/
	u_long	sc_act_vec;		/* Active device vector		*/
};

/* Per drive-unit info */
struct	ra_info {
	struct	size	*ra_sizes;	/* Partion tables for drive	*/
	daddr_t 	radsize;	/* Max user size from onl. pkt. */
	unsigned	ratype; 	/* Drive type int. field	*/
	unsigned	rastatus;	/* Command status, last command */
	unsigned	raunitflgs;	/* Unit status from last onl.	*/
	unsigned	ra_softcnt;	/* Soft error count		*/
	unsigned	ra_hardcnt;	/* Hard error count		*/
	long		ra_flags;	/* Status flags 		*/
	long		ra_category_flags;	/* Category flags	*/
	char		ra_device[DEV_SIZE];	/* Device type string	*/
	u_short		lbnstrk;	/* LBNS/track			*/
	u_char		rbnstrk;	/* RBNS/track			*/
	u_char		nrct;		/* Number of RCT copies		*/
	u_short		rctsize;	/* Size of one RCT copy		*/

/*
 *	BBR portion of structure
 */
	int		bbr_step;	/* BBR step			*/
	int		bbr_substep;	/* BBR sub_step			*/
	int		bbr_ip;		/* BBR in progress flag		*/
	int		bbr_lock;	/* Unit lock			*/
	int		bbr_reason;	/* Reason BBR was invoked 	*/
	u_long		bad_LBN;	/* LBN we are replacing		*/
	u_short		status;		/* MSCP status from BBR-related command */
	u_char		mscp_flags;	/* MSCP flags from BBR-related command */
	short		unit_flags;	/* MSCP unit flags for STUNT	*/
	short		bbr_mods;	/* Modifier field for MSCP command */
	struct	uba_device *ui;		/* ui structure for this device	*/
	struct	buf	*cur_buf;	/* Buffer involved in current I/O */
	struct	buf	*rct0_buf;	/* Buf structure containing copy */
					/* of RCT sector 0 		*/
	struct	buf	*multi_buf;	/* Buf structure to be used by	*/
					/* multi-read/write algoritms	*/
	struct	buf	*new_buf;	/* Current RCT block		*/
	struct	buf	*old_buf;	/* Block containing previous 	*/
					/* revector of this block	*/
	struct	buf	*orig_buf;	/* Buf structure for best attempt */
					/* read of user data (bad_LBN)	*/
	struct	buf	*test_buf;	/* Used for tests in step 7	*/
	struct	buf	*inv_buf;	/* One's complement of saved data */
	u_long		old_RBN;	/* RBN that bad_LBN was previously */
					/* revectored to.		*/
	u_long		new_RBN;	/* RBN that bad_LBN will be 	*/
					/* revectored to.		*/
	int		prev_repl;	/* Indicates if bad_LBN was	*/
					/* previously revectored. (1 = yes) */
	int		is_primary;	/* Is new_RBN the primary RBN for */
					/* bad_LBN (1 = yes)		*/
	int		bbr_cnt;	/* Generic counter		*/
	int		bbr_rep;	/* Repetition count for step 7	*/
	int		bbr_fe;		/* Is saved data valid (1 = no)	*/
	int		err;		/* Error indicator		*/
	int		suc;		/* Success indicator		*/
	int		recurs;		/* Recursion counter for step 9	*/
	int		result;		/* Result of ping-pong search	*/
	u_long		new_block;	/* Used for RCT block		*/
	u_long		old_block;	/* Used for RCT block		*/
	int		new_offset;	/* Offset into new_block	*/
	int		old_offset;	/* Offset into old_block	*/
	int		save_step;	/* Save step - for multi algorithms */
	int		save_substep;	/* Save substep - for multi algorithms */
	u_long		res_blk;	/* Resource wait - block number	*/
	u_long		res_op;		/* Resource wait - opcode	*/
	struct	buf	*res_bp;	/* Resource wait - buf pointer	*/
	int		res_wait;	/* 1 if we are in resource wait	*/
	struct	rct_search *search;	/* Pointer to rct_search structure */
/* Force revector and access area
 *
 */
	u_long		acc_badlbn;	/* first bad LBN found by access*/
	u_short		acc_status;	/* MSCP status from access command */
	u_long		acc_bytecnt;	/* access end packet byte count */
	u_short		acc_flags;	/* access end packet flags */
};

/* Disk type and default partition storage */
struct rapt {
	long	ra_type;		/* Disk type			*/
	struct size *ra_sizes;		/* Pointer to def. part. sizes	*/
};
