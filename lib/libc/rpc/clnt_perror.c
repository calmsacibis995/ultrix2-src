#ifndef lint
static	char	*sccsid = "@(#)clnt_perror.c	1.3	(ULTRIX)	2/13/87";
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
 *	Portions of this software have been licensed to 
 *	Digital Equipment Company, Maynard, MA.
 *	Copyright (c) 1986 Sun Microsystems, Inc.  ALL RIGHTS RESERVED.
 */

/*
 * clnt_perror.c
 *
 */

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <stdio.h>

extern char *sys_errlist[];

/*
 * Print reply error info
 */
void
clnt_perror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	struct rpc_err e;
	void clnt_perrno();

	CLNT_GETERR(rpch, &e);
	fprintf(stderr, "%s: ", s);
	switch (e.re_status) {
		case RPC_SUCCESS:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_TIMEDOUT:
		case RPC_PROGUNAVAIL:
		case RPC_PROCUNAVAIL:
		case RPC_CANTDECODEARGS:
			clnt_perrno(e.re_status);
			break;
		case RPC_CANTSEND:
			clnt_perrno(e.re_status);
			fprintf(stderr, "; errno = %s",
			    sys_errlist[e.re_errno]);
			break;
	
		case RPC_CANTRECV:
			clnt_perrno(e.re_status);
			fprintf(stderr, "; errno = %s",
			    sys_errlist[e.re_errno]);
			break;
	
		case RPC_VERSMISMATCH:
			clnt_perrno(e.re_status);
			fprintf(stderr, "; low version = %lu, high version = %lu", e.re_vers.low, e.re_vers.high);
			break;
	
		case RPC_AUTHERROR:
			clnt_perrno(e.re_status);
			fprintf(stderr, "; why = ");
			switch (e.re_why) {
			case AUTH_OK:
				fprintf(stderr, "auth ok");
				break;
	
			case AUTH_BADCRED:
				fprintf(stderr, "bad credentials");
				break;
	
			case AUTH_REJECTEDCRED:
				fprintf(stderr, "rejected credentials");
				break;
	
			case AUTH_BADVERF:
				fprintf(stderr, "bad verifier");
				break;
	
			case AUTH_REJECTEDVERF:
				fprintf(stderr, "rejected verifier");
				break;
	
			case AUTH_TOOWEAK:
				fprintf(stderr, "auth too weak (remote error)");
				break;
	
			case AUTH_INVALIDRESP:
				fprintf(stderr, "invalid auth response");
				break;
	
			default:
				fprintf(stderr, "unknown auth failure");
				break;
			}
			break;
	
		case RPC_PROGVERSMISMATCH:
			clnt_perrno(e.re_status);
			fprintf(stderr, "; low version = %lu, high version = %lu", e.re_vers.low, e.re_vers.high);
			break;
	
		default:
			fprintf(stderr, "unknown rpc failure; s1 = %lu, s2 = %lu", e.re_lb.s1, e.re_lb.s2);
			break;
	}
	fprintf(stderr, "\n");
}

/*
 * This interface for use by clntrpc
 */
void
clnt_perrno(num)
	enum clnt_stat num;
{
	switch (num) {
		case RPC_SUCCESS:
			fprintf(stderr, "rpc successful");
			break;
	
		case RPC_CANTENCODEARGS:
			fprintf(stderr, "can't encode rpc args");
			break;
	
		case RPC_CANTDECODERES:
			fprintf(stderr, "can't decode rpc results");
			break;
	
		case RPC_CANTSEND:
			fprintf(stderr, "can't send rpc");
			break;
	
		case RPC_CANTRECV:
			fprintf(stderr, "can't receive rpc");
			break;
	
		case RPC_TIMEDOUT:
			fprintf(stderr, "rpc timed out");
			break;
	
		case RPC_VERSMISMATCH:
			fprintf(stderr, "rpc version mismatch");
			break;
	
		case RPC_AUTHERROR:
			fprintf(stderr, "rpc authentification error");
			break;
	
		case RPC_PROGUNAVAIL:
			fprintf(stderr, "remote rpc program unavailable");
			break;
	
		case RPC_PROGVERSMISMATCH:
			fprintf(stderr, "rpc program mismatch");
			break;
	
		case RPC_PROCUNAVAIL:
			fprintf(stderr, "unknown rpc procedure");
			break;
	
		case RPC_CANTDECODEARGS:
			fprintf(stderr, "can't decode rpc args");
			break;
		case RPC_UNKNOWNHOST:
			fprintf(stderr, "unknown rpc host");
			break;
		case RPC_PMAPFAILURE:
			fprintf(stderr, "port mapper failure");
			break;
		case RPC_PROGNOTREGISTERED:
			fprintf(stderr, "rpc program not registered");
			break;
		case RPC_SYSTEMERROR:
			fprintf(stderr, "rpc system error");
			break;
	}
}

/*
 * A handle on why an rpc creation routine failed (returned NULL.)
 */
struct rpc_createerr rpc_createerr;

clnt_pcreateerror(s)
	char *s;
{

	fprintf(stderr, "%s: ", s);
	clnt_perrno(rpc_createerr.cf_stat);
	switch (rpc_createerr.cf_stat) {
		case RPC_PMAPFAILURE:
			fprintf(stderr, " - ");
			clnt_perrno(rpc_createerr.cf_error.re_status);
			break;

		case RPC_SYSTEMERROR:
			fprintf(stderr, " - %s", sys_errlist[rpc_createerr.cf_error.re_errno]);
			break;

	}
	fprintf(stderr, "\n");
}
