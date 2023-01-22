#ifndef lint
static char *sccsid = "@(#)uipc_port.c	1.5	ULTRIX	10/3/86";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *
 *   Modification history:
 *
 * 11 Sep 86 -- koehler
 * 	gnode name change
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 16 Apr 85 -- depp
 *	Insured that if an error occurs in "openp" routine, that
 *	it will be properly passed back to calling routine.
 *
 *  04 Oct 85 -- LeRoy Fundingsland
 *	This file ported from System V and modified to	
 *	handle the very different Ultrix port approach.	
 */


#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/errno.h"
#include "../h/gnode.h"
#include "../h/file.h"

/*
 * Open a port
 * Check read and write counts, delay as necessary
 */

openp(gp, mode)
register struct gnode *gp;
register mode;
{
	if (mode&FREAD) {
		if (gp->g_frcnt++ == 0)
			wakeup((caddr_t)&gp->g_frcnt);
	}
	if (mode&FWRITE) {
		if (mode&FNDELAY && gp->g_frcnt == 0) 
			return(ENXIO);
		if (gp->g_fwcnt++ == 0)
			wakeup((caddr_t)&gp->g_fwcnt);
	}
	if (mode&FREAD) {
		while (gp->g_fwcnt == 0) {
			if (mode&FNDELAY || gp->g_size)
				return(0);
			sleep(&gp->g_fwcnt, PPIPE);
		}
	}
	if (mode&FWRITE) {
		while (gp->g_frcnt == 0)
			sleep(&gp->g_frcnt, PPIPE);
	}
	return(0);
}

/*
 * Close a port
 * Update counts and cleanup
 *
 * If there is unread data in the port then don't close
 * the read side or the write side in case it is opened
 * later for re-use. If this is the last close on the inode
 * then IRELE will close any sockets which are still open.
 */

closep(gp, mode)
	register struct gnode *gp;
	register mode;
{

	if(mode & FREAD){
		if(--gp->g_frcnt == 0  &&  gp->g_size == 0){
			soclose(gp->g_rso);
			gp->g_rso = 0;
		}
	}
	if(mode & FWRITE){
		if(--gp->g_fwcnt == 0){
			soclose(gp->g_wso);
			gp->g_wso = 0;
		}
	}
}
