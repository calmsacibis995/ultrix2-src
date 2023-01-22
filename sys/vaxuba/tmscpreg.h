
/*
 *	1/30/87	(ULTRIX)	@(#)tmscpreg.h	1.6
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1986 by			*
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
 * tmscpreg.h
 *
 * Modification history
 *
 * TMSCP registers structure and definitions
 *
 * Robin  10/2/86
 *	Added tms_unitid to correct a problem in determining if a unit was
 *	known by a controler.
 *
 * Robin  1/30/87
 *	Added sc_timer which indicates if a timeout is running on a controller
 *
 */

/* Register device structure */
struct tmscpdevice {
	short	tmscpip;		/* Initialization and polling	*/
	short	tmscpsa;		/* Status and address		*/
};

/* Register device definitions */
#define TMSCP_ERR		0100000 /* Error bit			*/
#define TMSCP_STEP4	0040000 	/* Step 4 has started		*/
#define TMSCP_STEP3	0020000 	/* Step 3 has started		*/
#define TMSCP_STEP2	0010000 	/* Step 2 has started		*/
#define TMSCP_STEP1	0004000 	/* Step 1 has started		*/
#define TMSCP_NV		0002000 /* No host settable int. vector */
#define TMSCP_QB		0001000 /* Controller supports Q22 bus	*/
#define TMSCP_DI		0000400 /* Controller implements diags. */
#define TMSCP_OD		0000200 /* Port allows odd buf. addr.'s */
#define TMSCP_IE		0000200 /* Interrupt enable		*/
#define TMSCP_MP		0000100 /* Port supports address mapping*/
#define TMSCP_LF		0000002 /* Host req. last fail resp.	*/
#define TMSCP_PI		0000001 /* Host req. adapter purge int. */
#define TMSCP_GO		0000001 /* Start operation, after init. */

/* TMSCP Communications Area */
struct tmscpca {
	short	ca_xxx1;		/* Unused			*/
	char	ca_xxx2;		/* Unused			*/
	char	ca_bdp; 		/* BDP to purge 		*/
	short	ca_cmdint;		/* Command queue trans. int.flg.*/
	short	ca_rspint;		/* Response queue trans.int.flg.*/
	long	ca_rspdsc[NRSP];	/* Response descriptors 	*/
	long	ca_cmddsc[NCMD];	/* Command descriptors		*/
};
#define ca_ringbase	ca_rspdsc[0]

/* TMSCP communications area definitions */
#define TMSCP_OWN	0x80000000	/* Port owns descrpt. else host */
#define TMSCP_INT	0x40000000	/* Allow interrupt on ring tran.*/
#define TMSCP_MAP	0x80000000	/* Mapped buffer modifier descr.*/

/* TMSCP packet info (same as MSCP) */
struct mscp_header {
	short	tmscp_msglen;		/* Length of MSCP packet	*/
	char	tmscp_credits;		/* 0-3: credits, 4-7: msgtype	*/
	char	tmscp_vcid;		/* Virtual circuit id (con. id.)*/
};

/* Software state per controller */
struct tmscp_softc {
	short	sc_state;		/* State of controller		*/
	short	sc_mapped;		/* Bus map allocated for tmscp[]*/
	int	sc_ubainfo;		/* Bus mapping info.		*/
	struct	tmscp *sc_tmscp;	/* Bus address of tmscp[]	*/
	int	sc_ivec;		/* Interrupt vector address	*/
	short	sc_credits;		/* Transfer credits		*/
	short	sc_lastcmd;		/* Pointer into command ring	*/
	short	sc_lastrsp;		/* Pointer into response ring	*/
	short	sc_timer;		/* Flag to indicate timer is on */
};

/* Per drive-unit info */
struct tms_info {
	daddr_t 	tms_dsize;	/* Max. user size from onl. pkt.*/
	unsigned	tms_type;	/* Drive type int. field	*/
	quad		tms_unitid;	/* 0 = unit unknown by controler*/
	int		tms_resid;	/* Residual from last xfer.	*/
	u_char		tms_endcode;	/* Last command endcode 	*/
	unsigned	tms_status;	/* Command status, last command */
	char		tms_openf;	/* Lock against multiple opens	*/
	char		tms_serex;	/* Set on serious exception	*/
	char		tms_clserex;	/* Set on serex.cleared by nop. */
	short		tms_fmtmenu;	/* The unit's format/density	*/
	short		tms_unitflgs;	/* Unit flag parameters 	*/
	short		tms_format;	/* The unit's current form./den.*/
	long		tms_flags;	/* Last command end flags	*/
	long		tms_category_flags;	/* Category flags	*/
	u_long		tms_softcnt;	/* Soft error count		*/
	u_long		tms_hardcnt;	/* Hard error count		*/
	char		tms_device[DEV_SIZE];	/* Device type string	*/
	long		tms_position;	/* LBN position on tape 	*/
};

