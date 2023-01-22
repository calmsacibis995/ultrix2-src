#ifndef lint
static char *sccsid = "@(#)mop_syslog.c	1.3	ULTRIX	10/3/86";
#endif lint

/*
 * Program mop_syslog.c,  Module MOP 
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
*		m o p _ s y s l o g
*
* Version:	1.0 - 4/9/85
*
*
* Description:
*	This subroutine logs error messages for MOP modules by using
*	the "syslog" subroutine.
*
* Inputs:
*	progid		= buffer containing name of calling program.
*	errmsg		= buffer containing message to be printed.
*	pr_errno	= if set, log errno variable.
*	log_priority	= syslog priority to be used in syslog().
*
* Outputs:
*		None.
*
* Notes:
*
*/
 
#include <stdio.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <syslog.h>

extern char *target_address;
extern int errno;
int mop_syslog_debug = 0;

mop_syslog(progid, errmsg, pr_errno, log_priority)
char *progid, *errmsg;
int pr_errno, log_priority;
{
	int save_errno = errno;
	char buf[4096];

	if ( log_priority == LOG_DEBUG && ! mop_syslog_debug )
		return;

	if ( target_address )
	{
		toupcase( target_address );
		sprintf(buf, "%s, (target node Ethernet address = %s)", errmsg, target_address);
	}
	else
		sprintf(buf, "%s", errmsg);

	openlog(progid, LOG_PID | LOG_TIME);

	syslog(log_priority, "%s", buf );
	errno = save_errno;
	if ( pr_errno )
		syslog(log_priority, "%m");


	closelog();

	return;
}
