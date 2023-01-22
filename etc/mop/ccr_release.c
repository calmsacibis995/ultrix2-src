#ifndef lint
static	char	*sccsid = "@(#)ccr_release.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * MODULE - CCR_RELEASE.C, sends a message to release the remote console
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

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"


u_char 	ccr_release_cons[] = { CCR_RELCON_LEN,0,MOP_CCR_RELCON };

extern int dli_tranmit();

/*
*	r e l e a s e _ c o n s o l e	
*
* Description:
*	This routine releases the remote console.
*
* Inputs:
*	sock	the socket handle
*
* Outputs:
*	None
*
* Notes:
*
*/

int release_console(sock)
int sock;
{

	/* 
	 * Call routine to tranmit a release console message.
	 */

	dli_transmit(sock,ccr_release_cons,sizeof(ccr_release_cons));
	fprintf(stderr,"\n");
	print_message(ccr_REMCONREL);


}
