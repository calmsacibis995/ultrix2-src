#ifndef lint
static	char	*sccsid = "@(#)const.c	1.2	(ULTRIX)	3/18/86";
#endif lint

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
/************************************************************************
 *			Modification History
 *
 * 001	David L Ballenger, 18-Mar-1986
 *	Change use of const as identifier, since it is now a reserved word
 *
 * Based on:  const.c	1.2 (Berkeley) 3/7/81
 *
 ************************************************************************/

/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"
#include "tree.h"

STATIC	int constcnt = -1;

/*
 * The const declaration part
 */
constbeg(l, cline)
	int l, cline;
{

	line = l;
	if (nodecl)
		printoff();
	puthedr();
	putcm();
	ppnl();
	indent();
	ppkw("const");
	ppgoin(DECL);
	constcnt = 0;
	setline(cline);
}

const_setup(cline, cid, cdecl)
	int cline;
	char *cid;
	int *cdecl;
{

	if (constcnt)
		putcm();
	setline(cline);
	ppitem();
	ppid(cid);
	ppsep(" = ");
	gconst(cdecl);
	ppsep(";");
	constcnt++;
	setinfo(cline);
	putcml();
}

constend()
{

	if (constcnt == -1)
		return;
	if (nodecl)
		return;
	if (constcnt == 0)
		ppid("{const decls}");
	ppgoout(DECL);
	constcnt = -1;
}

/*
 * A constant in an expression
 * or a declaration.
 */
gconst(r)
	int *r;
{
	register *cn;

	cn = r;
loop:
	if (cn == NIL) {
		ppid("{constant}");
		return;
	}
	switch (cn[0]) {
		default:
			panic("gconst");
		case T_PLUSC:
			ppop("+");
			cn = cn[1];
			goto loop;
		case T_MINUSC:
			ppop("-");
			cn = cn[1];
			goto loop;
		case T_ID:
			ppid(cn[1]);
			return;
		case T_CBINT:
		case T_CINT:
		case T_CFINT:
			ppnumb(cn[1]);
			if (cn[0] == T_CBINT)
				ppsep("b");
			return;
		case T_CSTRNG:
			ppstr(cn[1]);
			return;
	}
}
