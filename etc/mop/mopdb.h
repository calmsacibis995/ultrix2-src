/*	@(#)mopdb.h	1.1	(ULTRIX)	9/30/85	*/

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
 * Structures returned by MOP data base library. 
 */
#define MDB_FN_MAXL	256			/* maximum size of load path/file name */
#define MDB_NN_MAXL	6			/* max siz of node name */
#define MDB_NN_LEN	MDB_NN_MAXL + 1		/* max siz of node name array */
#define MDB_NA_MAXL	6			/* max size of hw address */


/*
 * nodes data base parameters
 */
struct ndb_parameters
{
	u_char	node_name[MDB_NN_LEN];		/* name of node */
	u_short	node_addr;			/* DECnet address */
	char node_physaddr[nm_MAXETHER];	/*  physical address */
	struct r_netmgt node_parm;		/* nodes data base parameters */
};


/*
 * valid option values.
 */
#define LOPTION_NODE	0			/* load node option */
#define LOPTION_CKT	3			/* load circuit option */
/*
 * downline load parameters bit mask
 */
#define LPAR_SCKT	0x1	/* service circuit */
#define LPAR_SPSWD	0x2	/* service password */
#define LPAR_SDEV	0x4	/* service device */
#define LPAR_CPU	0x8	/* cpu type */
#define LPAR_HWADDR	0x10	/* hardware address */
#define LPAR_SNVER	0x20	/* service version number */
#define LPAR_LFILE	0x40	/* load file */
#define LPAR_SFILE	0x80	/* secondary loader */
#define LPAR_TFILE	0x100	/* tertiary loader */
#define LPAR_DGFILE	0x200	/* diagnostic file */
#define LPAR_SWTYPE	0x400	/* software type */
#define LPAR_SWID	0x800	/* software id */
#define LPAR_DUFILE	0x1000	/* dump file */
#define LPAR_HOST	0x2000	/* host id */
#define LPAR_SERVICE	0x4000	/* host id */
#define LPAR_PHYSADDR	0x8000	/* physical address */
#define LPAR_NAME	0x10000	/* target name */
#define LPAR_ADDR	0x20000	/* target address */


struct load_parameters
{
	u_char option;				/* option */
	u_char load_node_name[MDB_NN_LEN];	/* target node name */
	u_short load_node_addr;			/* target node name */
	u_char load_node_ckt[17];		/* circuit to reach target */
	int parm_present;			/* bit map of parameters present in structure */
	int parm_delete;			/* bit map of parameters deleted in structure */
	struct ndb_parameters db_parm;		/* data base parameters */
};

