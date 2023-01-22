/*	@(#)ccr_mesg_text.h	1.1	(ULTRIX)	9/30/85	*/

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
 */


/*
 * Text for Console Carrier Messages (including errors)
 */

char *ccr_msglist[] = {
	"FAIL",								/* Fail */
	"SUCCESS",							/* Success */
	"Remote console reserved",			/* ccr_REMCONRES */
	"Remote console released",			/* ccr_REMCONREL */
	"Permission denied",				/* ccr_PRIV */
	"Hardware address required",		/* ccr_HARDADDR */
	"No node entry in database",			/* ccr_NONODE */
	"Invalid function code in system id message",	/* ccc_INVFUNCODE */
	"Message format error",				/* ccr_MESFORMERR */
	"Incompatible version number",		/* ccr_INCVERSION */
	"Remote console already in use",	/* ccr_REMCONINUSE */
	"Remote console not reserved",		/* ccr_NOTRESERV */
	"Programming error",				/* ccr_PROGERR */
	"",						
	"",						
	"",
	"",
	"",
	"",
	"",
	"Fatal error - no physical address for device",	/* ccr_NOWAY */
};

int num_ccr_mesgs = {sizeof ccr_msglist/sizeof ccr_msglist[0]};	
