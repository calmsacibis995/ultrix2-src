/* 	@(#)mop_var.h	1.2	(ULTRIX)	6/20/86	*/

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

#define NULL_PATH	\
		{ 0x00 }
#define NULL_PROG	\
		{ 0x00 }
#define DUMP_LOAD	\
		{ 'm', 'o', 'p', '_', 'd', 'u', 'm', 'p', 'l', 'o', 'a', 'd' }
#define DUMP_LOAD_PATH	\
		{ '/', 'u', 's', 'r', '/', 'l', 'i', 'b', '/', 'd', 'n', 'e', 't', '/' }

#define MAX_SOCKETS	20
#define MAX_IFREQ	32
#define MOM_MSG_SIZE	1600
#define ASCII_EADDRSIZE	19		/* size of phy addr in ascii */
#define DMPLD_INDEX	0		/* index into proto_mcast table for dump/load protocol */
#define DB_PARM_LEN	256		/* length of each field in data base */

/*
 * define various file format flags
 */
#define FF_UNKNOWN      0               /* unknown load file format */
#define FF_AOUTOLD      1               /* old a.out load file format */
#define FF_AOUTNEW      2               /* new a.out load file format */
#define FF_VMS          3               /* VMS .EXE load file format */
#define FF_RSX          4               /* RSX TASK load file format */

/*
 * literals used in accessing task image label
 */
#define LABEL_TYPE_RSX	0		/* RSX task label */
#define LABEL_TYPE_RT11	1		/* RT11 task label */
#define TSK_LABEL_SIZE	0404		/* size of task label */
#define TSK_BASE_IDX	010		/* base address of task image */
#define TSK_HIGH_IDX	012		/* high address of task image */
#define TSK_SIZE_IDX	016		/* size of task image */
#define TSK_FLAGS_IDX	030		/* task flags word */
#define TSK_NHD		040000		/* flag = no header */
#define TSK_CHK		0100		/* flag = not check pointable */
#define TSK_HEADER_IDX	0360		/* offset of task image */
#define TSK_XFER_IDX	0350		/* transfer address of task image */
#define TSK_IBLKSIZE	64		/* block size of task image */
#define TSK_DBLKSIZE	512		/* disc block size */

/*
 * literals used in accessing VMS image label
 */
#define IHD_W_ALIAS     510             /* ??? */
#define IHD_B_HDRBLKCNT 16              /* header block count */
#define IHD_W_SIZE      0               /* header size */
#define IHD_W_ACTIVOFF  2               /* image activation offset */
#define ISD_W_PAGCNT    2               /* image section page count */
#define ISD_V_VPN       4               /* image section virtual page number */
#define IHA_L_TFRADR1   0               /* transfer address */

/*
 * Exit codes for dumper/loader, trigger/load
 */
#define EXIT_GOOD		0	/* good status */
#define EXIT_GOODSEC		1	/* secondary exited ok */
#define EXIT_GOODTER		2	/* tertiary exited ok */
#define EXIT_GOODSYS		3	/* system exited ok */
#define EXIT_GOODDIA		4	/* diagnostic exited ok */
#define EXIT_GOODDMP		5	/* dump exited ok */
#define EXIT_BADSWID		6	/* bad sw id in request msg */
#define EXIT_FOPENFAIL		7	/* file open failure */
#define EXIT_FREADFAIL		8	/* file read failure */
#define EXIT_FWRITEFAIL		9	/* file write failure */
#define EXIT_INVMSGSIZ		10	/* request message too large */
#define EXIT_INVPROGTYP		11	/* invalid program type in rqst msg */
#define EXIT_INVRQST		12	/* invalid request message */
#define EXIT_MCAST		13	/* multicast addr on rqst message */
#define EXIT_NOAUTHORIZATION	14	/* not authorized to honor request */
#define EXIT_NODIAGFILE		15	/* diagnostic load file not defined */
#define EXIT_NODUMPFILE		16	/* dump file not defined */
#define EXIT_NOSYSFILE		17	/* system load file not defined */
#define EXIT_NOTCHOSEN		18	/* not chosen to downline load */
#define EXIT_NOTSU		19	/* not super user */
#define EXIT_RCVFAIL		20	/* receive failure */
#define EXIT_SENDFAIL		21	/* transmit failure */

#define EXIT_PARSE_FAIL		22	/* could not parse command line */
#define EXIT_DBACCESSFAIL	23	/* failed to access or find entry in data base */
#define EXIT_SERDISABL		24	/* load service is disabled */
#define EXIT_NODBKEY		25	/* no data base handle to find entry */
#define EXIT_BADEVNAME		26	/* bad device name was given */
#define EXIT_DLICONFAIL		27	/* DLI connect failed  */
#define EXIT_SIGFAIL		28	/* unable to create signal catcher */
#define EXIT_CANTEXEC		29	/* cannot fork and exec program */
#define EXIT_WAITFAIL		30	/* a wait subr call failed */
#define EXIT_CANTDUP		31	/* a dup subr call failed */
#define EXIT_BADARG		32	/* bad argument passed to loader/dumper */

/*
 * used by forced load utility to determine whether load or trigger
 * is in order
 */
#define TRIGGER_NODE	1		/* issue trigger boot msg */
#define LOAD_NODE	2		/* issue load boot msg */

/*
 * used by MOM to associate a protocol type with a specific device.
 */
struct mom_sock_db
{
	int so;				/* socket handle */
	struct proto_mcast *pm;		/* protocol type and mcast addr */
	u_char devname[2];		/* device name */
	u_short devunit;		/* device unit number */
};


/*
 * used by MOM to associate a protocol type/multicast address
 * with a program that processes packets of this kind.
 */
struct proto_mcast
{
	u_short protype;		/* protocol type */
	u_char multicast[6];		/* multicast addr assoc with protocol */
	u_char pathname[50];		/* path to exec program */
	u_char program[15];		/* name of exec program */
};


/*
 * table constructed from a mop message with the dump/load
 * protocol type.
 */
struct mop_parse_dmpld
{
	u_char mop_dmpld_code;		/* mop code */
	u_char mop_devtyp;		/* device type */
	u_char mop_format_vsn;		/* format version */
	u_char mop_pgm_type;		/* program type */
	char mop_swid_form;		/* format of software id */
	u_char mop_swid_id[17];		/* software id in ASCII */
	u_char mop_processor;		/* processor */
	u_char mop_sys_processor;	/* which system processor */
	u_char mop_hw_addr[6];		/* hardware addr from info field */
	u_short mop_dl_bufsiz;		/* data link buffer size */
	u_int mop_dump_size;		/* size of memory to be dumped */
	u_char mop_compat_bits;		/* compatiblity bits in dump request */
};


/*
 * Pertinent task image information
 */
struct task_image
{
	u_long	base_addr;		/* base address of image */
	u_long	task_size;		/* size in units of 64 byte blocks unless its a AOUT image*/
	u_long	task_xferaddr;		/* task transfer address */
};

/*
 * Information in header of Console Carrier Server image, which
 * is in an Absolute Loader File Format.
 */
struct header_fmt
{	
	u_short null_word;		/* First 2 bytes always equal to 1 */
	u_short record_length;	/* length of record in bytes, including
								the first word */
	u_short load_address;	/* the load address in the server */
};

/*
 * node id structure containing target node info sent in PARAMTER 
 * LOAD with XFER ADDRESS message.
 */
struct node_id
{
	u_short node_dnaddr;		/* DECnet address */
	u_char node_name[DB_PARM_LEN];	/* node name */
};
