/* 	@(#)node_params.h	1.1	(ULTRIX)	9/30/85	*/

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
 *  Additional node parameters can be placed after each dnet_nodeent
 *  structure.  The format is as follows:
 *
 *	parameter type code = 1 byte
 *	length = 2 bytes
 *	paramter data = variable number of bytes
 *
 */

#define NODE_MAX_PSIZ		0x8000	/* Maximum size of parameter block */
#define NODE_PARM_HDRSIZ	4	/* Size of parameter block header */
#define NODE_SSIZ		2	/* Size of parameter block size variable */
#define NODE_CSIZ		2	/* Size of parameter code */


/*
 *  parameter codes from DECnet Network Mgt Spec.
 */
#define	NODE_END_PARAM		0	/* End of parameter list marker */
#define	NODE_SERVICE_CKT	110	/* Service Circuit */
#define	NODE_SERVICE_PSWD	111	/* Service Password */
#define	NODE_SERVICE_DEV	112	/* Service Device */
#define	NODE_CPU		113	/* CPU */
#define	NODE_HARDWARE_ADDR	114	/* Default Hardware Address */
#define	NODE_SERVICE_NVER	115	/* Service Node Version */
#define	NODE_LOAD_FILE		120	/* Load File, i.e., system image */
#define	NODE_SEC_LOADER		121	/* Secondary Load File */
#define	NODE_TER_LOADER		122	/* Tertiary Load File */
#define	NODE_DIAG_FILE		123	/* Diagnostic File */
#define	NODE_SW_TYPE		125	/* Software type */
#define	NODE_SW_ID		126	/* Software identification */
#define	NODE_DUMP_FILE		130	/* Dump File */
#define	NODE_HOST		140	/* Host node id (address only) */
#define	NODE_SERVICE		1750	/* Service disable */

