/*	@(#)ccr.h	1.2	(ULTRIX)	8/19/86	*/

/*
 *
 * Copyright (C) 1985 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 *
 */


/* Definitions for CCR flags word */

#define CCR_VERSION		01
#define CCR_FUNCTION	02
#define CCR_CONUSER		04
#define	CCR_RESTIMER	010
#define	CCR_CMDSIZE		020
#define	CCR_RSPSIZE		040

/* Definitions needed for parsing MOP system id message */

#define sysid_VERSION	1
#define	sysid_FUNCTION	2
#define sysid_CONUSER	3
#define sysid_RESTIMER 	4
#define sysid_CMDSIZE	5
#define sysid_RSPSIZE	6

#define BYTLEN_FUNC		2
#define BYTLEN_CONUSER	6
#define BYTLEN_RESTIMER	2
#define BYTLEN_CMDSIZE	2
#define BYTLEN_RSPSIZE	2

#define MX_IMGFLD_LEN	17

/* Bit Map for FUNCTION word of the MOP system id message */

#define	FUNC_LOOP		01
#define FUNC_DUMP		02
#define	FUNC_PRI_LOAD	04
#define FUNC_MULT_LOAD	010
#define FUNC_BOOT		020
#define FUNC_CONS_CARR	040
#define FUNC_DATLNK_COU	0100
#define FUNC_CC_RESERVE	0200

/* Bit Map for CONTROL FLAG word of the Console Response and Ack Message */

#define FLG_COMND_LOST	02

/*
 * Definitions needed by subroutine nice_to_dev(), came from Network 
 * Management
 */

#define MAXDEV	4


/* Codes for MOP messages */

#define MOP_CCR_REQID	5
#define MOP_CCR_BOOT	6
#define MOP_CCR_SYSID	7
#define MOP_CCR_RESCON	13
#define MOP_CCR_RELCON	15
#define MOP_CCR_CCPOLL	17
#define MOP_CCR_CCRESP	19

/* Length of MOP messages */

#define CCR_REQID_LEN	4
#define CCR_RESCON_LEN	9
#define CCR_RELCON_LEN	1
	
/* Index into CCR MOP messages */

#define RESCON_PASSWD_IDX	3		/* Index to service password in reserve
									   console message. */
#define CONTRL_FLG_IDX		3		/* Index to control flags byte in console
									   command and poll message. */
#define MOP_LEN_OVRHD		2		/* Overhead for length field in MOP 
									   message */


/* Maximum length of a buffers */

#define MAXL_RECV_BUF	255
#define MAXL_CMD_SIZE	80


/* Timeout value for console command and poll messages 
 * and receives */

#define CCPOLL_TIMEOUT	14000
#define CCR_RECV_TIMOUT	15


/* Structure for information obtained when parsed MOP system id message */

struct sys_id_info {
					short	functions;
					u_char	console_user[6];
					short	max_restimer;
					short	max_cmd_size;
					short 	max_rsp_size;
};


/* Maximum lengths for storage of CCR command line information */

#define MXBYT_NODNAM	6
#define MXBYT_SERCIR	16
#define MXBYT_SERPAS	8
#define MXBYT_HARADD_ASCII	17	
#define MXBYT_HARADD_HEX	6	

/* Control characters which have special meaning for CCR */

#define CTRL_B	2
#define CTRL_D	4


#define no_eintr(x) while ((x) == -1 && errno == EINTR)
