/*	@(#)nmgtdb.h	1.1	(ULTRIX)	9/30/85	*/

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
 * Structures used by Network Mgt wrt to data base library. 
 */

#include <netdnet/nm_defines.h>

struct cm_node 
{
	unsigned short n_addr;
	unsigned char n_name[nm_MAXNNAME + 1];	/*  optional - ascii image  */
};

struct r_netmgt					/*  Net Mgt Layer  */
{
	char rnm_sercirc[nm_MAXLINE + 1];   	/*  SERVICE CIRCUIT */
	char rnm_serpasswd[9];			/*  SERVICE PASSWORD */
	u_char rnm_serdev;			/*  SERVICE DEVICE */
	u_char rnm_cpu;				/*  CPU  */
	char rnm_hwaddr[nm_MAXETHER];		/*  HARDWARE ADDRESS  */
	u_char rnm_sernodever;			/*  SERVICE NODE VERSION  */
	char rnm_loadfile[nm_MAXFIELD + 1];	/*  LOAD FILE  */
	char rnm_secload[nm_MAXFIELD + 1];	/*  SECONDARY LOADER */
	char rnm_terload[nm_MAXFIELD + 1];	/*  TERTIARY LOADER */
	char rnm_diagfile[nm_MAXFIELD + 1]; 	/*  DIAGNOSTIC FILE */
	u_char rnm_swtype;			/*  SOFTWARE TYPE */
	char rnm_swident[nm_MAXIDLEN/2 + 1];	/*  SOFTWARE IDENTIFICATION */
	char rnm_dumpfile[nm_MAXFIELD + 1];	/*  DUMP FILE  */
	struct cm_node rnm_host;		/*  HOST  */ 
	u_char rnm_service;			/*  SERVICE DISABLE */
};


/*
 * end of stolen NML section
 */				

