#ifndef lint
static char *sccsid = "@(#)rusersxdr.c	1.1      (ULTRIX)        4/10/86";
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
#include <rpcsvc/rusers.h>
#include <utmp.h>

rusers(host, up)
	char *host;
	struct utmparr *up;
{
	return callrpc(host, RUSERSPROG, RUSERSVERS, RUSERSPROC_NAMES,
	    xdr_void, 0, xdr_utmparr, up);
}

rnusers(host, up)
	char *host;
	struct utmparr *up;
{
	int nusers;

	if (callrpc(host, RUSERSPROG, RUSERSVERS, RUSERSPROC_NUM,
	    xdr_void, 0, xdr_u_long, &nusers) < 0)
		return -1;
	else
		return (nusers);
}

xdr_utmp(xdrsp, up)
	XDR *xdrsp;
	struct utmp *up;
{
	int len;
	char *p;

	len = sizeof(up->ut_line);
	p = up->ut_line;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	len = sizeof(up->ut_name);
	p = up->ut_name;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	len = sizeof(up->ut_host);
	p = up->ut_host;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	if (xdr_long(xdrsp, &up->ut_time) == FALSE)
		return 0;
	return 1;
}

xdr_utmpptr(xdrsp, up)
	XDR *xdrsp;
	struct utmp **up;
{
	return (xdr_reference(xdrsp, up, sizeof (struct utmp), xdr_utmp));
}

xdr_utmparr(xdrsp, up)
	XDR *xdrsp;
	struct utmparr *up;
{
	return (xdr_array(xdrsp, &up->uta_arr, &(up->uta_cnt),
	    MAXUSERS, sizeof(struct utmp *), xdr_utmpptr));
}
