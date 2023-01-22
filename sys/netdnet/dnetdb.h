/*	@(#)dnetdb.h	1.1	(ULTRIX)	9/30/85	*/

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
 * 1.01 08-Aug-1985
 *      DECnet-ULTRIX   V1.1
 *
 */

/*
 * Structures returned by DECnet network
 * data base library.  All addresses
 * are in host order.
 */
struct	nodeent {
	char	*n_name;		/* name of node */
	int	n_addrtype;		/* node address type */
	int	n_length;		/* length of address */
	char	*n_addr;		/* address */
	unsigned char	*n_params;	/* node parameters */
};


#define ND_MAXNODE	6		/* max length of node name */
#define ND_VOLATILE	0		/* use the volatile database */
#define ND_PERMANENT	1		/* use the permanent database */
#define ND_VERSION	1		/* database version number */

/*
 * Additional node parameters can be placed after dnet_nodeent structure.
 * The format is as follows:
 *	parameter type code = 2 byte
 *	length = 2 bytes
 *	parameter data = variable number of bytes
 */
struct dnet_nodeent {
	unsigned short	dn_esiz; /* size of node entry i.e., structure + parameters */
	unsigned short	dn_addr; /* node address */	
	char	dn_name[ND_MAXNODE]; /* node name */
};

struct db_version {
	unsigned short	vs_esiz; 	/* size of version entry */
	unsigned short	vrsn_number; 	/* version number */	
	unsigned short	reserved; 	/* reserved */	
};


struct objectent {
	unsigned short	o_flags;	/* object flags */
	unsigned char	o_objnum;	/* object number */
	char		*o_objname;	/* pointer to object name */
	char		*o_defuser;	/* default user to run program under */
	char		*o_file;	/* execute file name */
};

#define OF_DEFER	0x01		/* deferred connection */
#define OF_STREAM	0x02		/* stream socket */

#define OB_MAXFILE	128		/* max length of file name */
#define OB_MAXNAME	16		/* max length of object name */
#define OB_MAXUSER	20		/* max length of user name */

struct dnet_objectent {
	unsigned short	do_flags;	/* object flags */
	unsigned char	do_objnum;	/* object number */
	char		do_objname[OB_MAXNAME+1];
	char		do_defuser[OB_MAXUSER+1];
	char		do_file[OB_MAXFILE+1];
};

struct dnet_nodeent *next_nodeent();
struct nodeent	*getnodebyname(), *getnodebyaddr(), *getnodeent();
struct objectent *getobjectbyname(), *getobjectbynumber();
