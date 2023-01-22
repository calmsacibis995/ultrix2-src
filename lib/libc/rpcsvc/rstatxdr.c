#ifndef lint
static char *sccsid = "@(#)rstatxdr.c	1.1      (ULTRIX)        4/10/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/rstat.h>

rstat(host, statp)
	char *host;
	struct stats *statp;
{
	return callrpc(host, RSTATPROG, RSTATVERS, RSTATPROC_STATS,
	    xdr_void, 0, xdr_stats, statp);
}

havedisk(host)
	char *host;
{
	long have;

	if (callrpc(host, RSTATPROG, RSTATVERS, RSTATPROC_HAVEDISK,
	    xdr_void, 0, xdr_long,  &have) < 0)
		return -1;
	else
		return have;
}

xdr_stats(xdrs, statp)
	XDR *xdrs;
	struct stats *statp;
{
	int i;

	for (i = 0; i < CPUSTATES; i++)
		if (xdr_int(xdrs, &statp->cp_time[i]) == 0)
			return 0;
	for (i = 0; i < DK_NDRIVE; i++)
		if (xdr_int(xdrs, &statp->dk_xfer[i]) == 0)
			return 0;
	if (xdr_int(xdrs, &statp->v_pgpgin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pgpgout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_intr) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ipackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ierrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_oerrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_collisions) == 0)
		return 0;
	return 1;
}
xdr_statsswtch(xdrs, statp)		/* version 2 */
	XDR *xdrs;
	struct statsswtch *statp;
{
	int i;

	for (i = 0; i < CPUSTATES; i++)
		if (xdr_int(xdrs, &statp->cp_time[i]) == 0)
			return 0;
	for (i = 0; i < DK_NDRIVE; i++)
		if (xdr_int(xdrs, &statp->dk_xfer[i]) == 0)
			return 0;
	if (xdr_int(xdrs, &statp->v_pgpgin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pgpgout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_intr) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ipackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ierrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_oerrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_collisions) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_swtch) == 0)
		return 0;
	for (i = 0; i < 3; i++)
		if (xdr_long(xdrs, &statp->avenrun[i]) == 0)
			return 0;
	if (xdr_timeval(xdrs, &statp->boottime) == 0)
		return 0;
	return 1;
}

xdr_timeval(xdrs, tvp)
	XDR *xdrs;
	struct timeval *tvp;
{
	if (xdr_long(xdrs, &tvp->tv_sec) == 0)
		return 0;
	if (xdr_long(xdrs, &tvp->tv_usec) == 0)
		return 0;
	return 1;
}
