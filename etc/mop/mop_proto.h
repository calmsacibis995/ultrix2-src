/* 	@(#)mop_proto.h	1.4	(ULTRIX)	12/4/86	*/

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
 * Miscellaneous
 */
#define MAX_SECONDARY_SIZE	1488		/* secondary load module cannot be larger */
#define MOP_MAXLD_SIZ		1498		/* max buf size in each memory load msg*/
#define MOP_RMD_SIZ		7		/* size of request memory dump msg*/
#define MOP_RETRY		12		/* number of retransmissions allowed */
#define MOP_TERIMG		0		/* command to load_ter_or_sys */
#define MOP_SYSIMG		1		/* command to load_ter_or_sys */
#define MOP_TX_POSTRX		1		/* command to read_mop */
#define MOP_TX_NOPOSTRX		0		/* command to read_mop */

/*
 * MOP protocol types
 */
#define PROTO_LOOPBACK	0x9000			/* loopback protocol type */
#define PROTO_DUMP_LOAD	0x6001			/* dump/load protocol type */
#define PROTO_REM_CONS	0x6002			/* remote console protocol type */

/*
 * MOP multicast addresses
 */
#define MCAST_LOOPBACK	\
		{ 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define MCAST_DUMP_LOAD	\
		{ 0xab, 0x00, 0x00, 0x01, 0x00, 0x00 }
#define MCAST_NULL	\
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }


/*
 * supported MOP CODEs
 */
#define FORWARD_DATA	0x2			/* forward data (loopback msgs) */
#define MOP_BOOT_MSG	6			/* Boot msg */
#define MOP_RQST_PGM	8			/* Program request msg */
#define MOP_RQST_MEM	10			/* Memory load request msg */
#define MOP_VOLASSIST	3			/* Volunteer assistance msg */
#define MOP_MLOAD_XFER	0			/* Memory Load with transfer address msg */
#define MOP_PLOAD_XFER	20			/* Parameter Load with transfer address msg */
#define MOP_MLOAD	2			/* Memory Load msg */
#define MOP_RQST_DMPSRV	12			/* Dump Service Request code */
#define MOP_RQST_MEMDMP	4			/* Memory Dump Request code */
#define MOP_MEMDMP_DATA	14			/* Memory Dump Data code */
#define MOP_DUMP_DONE	1			/* Memory Dump Complete code */


/*
 * supported MOP Device type literals - from appendix A of MOP spec.
 */
#define MOP_DEVTYP_UNA	1
#define MOP_DEVTYP_QNA	5
#define MOP_DEVTYP_BNT	23

/*
 * MOP Format version
 */
#define MOP_FORMAT_VSN	1			/* protocol format version */

/*
 * MOP program type
 */
#define MOP_SECONDARY	0			/* secondary loader */
#define MOP_TERTIARY	1			/* tertiary loader */
#define MOP_SYSTEM	2			/* system loader */

/*
 * MOP software id
 */
#define MOP_SWID_NONE	0			/* no software id */
#define MOP_SWID_SOS	-1			/* standard operating system */
#define MOP_SWID_MS	-2			/* maintenance system */

/*
 * MOP processor field
 */
#define MOP_SYS_PROC	0			/* system processor */
#define MOP_COM_PROC	1			/* communication processor */

/*
 * Supported MOP System Id literals
 */
#define MOP_SYSP_PDP	1			/* system processor is a PDP-11 (UNIBUS) */
#define MOP_SYSP_SER	2			/* system processor is a Communication Server */
#define MOP_SYSP_PRO	1			/* system processor is a PRO */

/*
 * MOP Data link buffer size
 */
#define MOP_DLBF_DEF	262			/* default data link buffer size */

/*
 * MOP Info Type literals
 */
#define SYSID_SYSPROC	300			/* system processor */
#define SYSID_HWADDR	7			/* hardware address */
#define SYSID_DLBUFSIZ	401			/* data link buffer size */

/*
 * MOP Parameter values for Load Parameters message.
 */
#define MOP_PLOAD_TSNAME	1		/* Target name code */
#define MOP_PLOAD_TSADDR	2		/* Target DECnet address */
#define MOP_PLOAD_HSNAME	3		/* Host name code */
#define MOP_PLOAD_HSADDR	4		/* Host DECnet address */
#define MOP_PLOAD_HSTIME	5		/* Host time code */
#define MOP_PLOAD_HSTIMSIZ	10		/* size of host time field */
#define MOP_PLOAD_CENTURY	19		/* 20th century passed in time field */
#define MOP_PLOAD_DLBSDEF	262		/* default data link buffer size of comm dev */

/*
 * MOP control values for control field in BOOT message.
 */
#define MOP_TRIGCTL	0			/* issue trigger - use multicast*/
#define MOP_LOADCTL	1			/* issue load - use local node */

/*
 * indexes into MOP messages
 */
#define MOP_CODE_IDX	2			/* index to mop code */
#define PRQ_DEVTYPE_IDX	3			/* index to device type */
#define PRQ_FRMTVER_IDX	4			/* index to format version */
#define PRQ_PGMTYPE_IDX	5			/* index to program type */
#define MOP_MLDNUM_IDX	3			/* index to memory load load number */
#define MOP_MLDERR_IDX	4			/* index to memory load error */
#define MOP_MLDSIZ_IDX	0			/* index to memory load siz */
#define MOP_MLDADR_IDX	4			/* index to memory load load number */
#define MOP_MLDIMG_IDX	8			/* index to memory load image */
#define MOP_RMDADDR_IDX	3			/* index to request memory dump address field */
#define MOP_RMDCNT_IDX	7			/* index to request memory dump count field */
#define MOP_MDIMAGE_IDX	7			/* index to memory dump image field */
#define MOP_BTVER_IDX	3			/* index to verification field in boot msg */
#define MOP_BTPRC_IDX	11			/* index to processor field in boot msg */
#define MOP_BTCTL_IDX	12			/* index to control field in boot msg */
#define MOP_BTDEV_IDX	13			/* index to device id field in boot msg */

/*
 * MOP Time out values.
 */
#define MOP_RTIMOUT	5			/* timeout on reads */
#define MOP_BTIMOUT	30			/* timeout on boot msgs */

/*
 * MOP data base routine commands
 */
#define MOP_LDSYS	0			/* fetch system image */
#define MOP_LDTER	1			/* fetch tertiary image */
#define MOP_LDSEC	2			/* fetch secondary image */
#define MOP_LDDIA	3			/* fetch diagnostic image */
#define MOP_DUMP	4			/* fetch dump file */
#define MOP_SWID	5			/* fetch software id */
