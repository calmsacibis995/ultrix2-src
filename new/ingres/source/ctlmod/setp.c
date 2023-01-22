#ifndef lint
static	char	*sccsid = "@(#)setp.c	1.1	(ULTRIX)	1/8/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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

# include	"ctlmod.h"
# include	<ingres.h>
# include	<tree.h>
# include	<aux.h>


/*
**  SETP -- set parameter
**
**	Sets a parameter, to be later sent by 'call' to whomever.
**
**	Parameters:
**		type -- parameter type.
**			PV_STRING -- a string, 'len' is ignored.
**			PV_TUPLE -- a tuple of length 'len'.
**			PV_QTREE -- a query tree pointer, 'len'
**				is ignored.
**			PV_INT -- an integer, 'len' is ignored.
**		val -- the value (real value if PV_INT, pointer
**			otherwise).
**		len -- the length of the tuple in PV_TUPLE mode.
**
**	Returns:
**		none
**
**	Side Effects:
**		Adjusts Ctx.ctx_pc & Ctx.ctx_pv.
**
**	Trace Flags:
**		4.8 - 4.15
*/

setp(type, val, len)
register int	type;
char		*val;
register int	len;
{
	register PARM	*pp;
	register char	*newp;
	extern char	*need();

	/*
	**  Check the magic bounds.
	*/

	if (!Ctx.ctx_init)
/*
		pp = &Resp.resp_rval;
*/		syserr("setp: no initp");
	else if (Ctx.ctx_pc >= PV_MAXPC)
		syserr("setp: overflow");
	else
		pp = &Ctx.ctx_pv[Ctx.ctx_pc++];

	/*
	**  Figure out the length from the type.
	*/

	switch (type)
	{
	  case PV_STR:
		len = length(val) + 1;
		newp = need(Qbuf, len);
		bmove(val, newp, len);
		pp->pv_val.pv_str = newp;
		break;
	
	  case PV_TUPLE:
		pp->pv_val.pv_tuple = (char *) val;
		break;
	
	  case PV_QTREE:
		len = sizeof pp->pv_val.pv_qtree;
		pp->pv_val.pv_qtree = (QTREE *) val;
		break;

	  case PV_INT:
		len = sizeof (short);
		pp->pv_val.pv_int = (int) val;
		break;

	
	  default:
		syserr("setp: type %d", type);
	}

	/*
	**  Set up the parameter.
	*/

	pp->pv_type = type;
	pp->pv_len = len;

# ifdef xCTR1
	if (tTf(4, 8))
	{
		lprintf("setp: ");
		pr_parm(pp);
	}
# endif
}
