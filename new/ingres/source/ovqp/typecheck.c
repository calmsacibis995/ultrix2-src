#ifndef lint
static	char	*sccsid = "@(#)typecheck.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>
# include	<symbol.h>
# include	"../decomp/globs.h"


/*
** TYPECHECK
**
**	Performs typechecking and conversions where appropriate .
**	Prohibits mixed type expressions.
*/

typecheck(pp1, pp2, opval)
SYMBOL	*pp1, *pp2;
int	opval;
{
	register int	i;
	register SYMBOL	*p1, *p2;

	p1 = pp1;
	p2 = pp2;

	i = (p1->type == CHAR & p2->type == CHAR);	/* c is true only if both are chars */
	switch (opval)
	{

	  case opCONCAT:
		if (!i)
			ov_err(NUMERIC);	/* numeric in a char operator */
		return;	/* else no further checking is needed */

	  case opADD:
	  case opSUB:
	  case opMUL:
	  case opDIV:
	  case opPOW:
	  case opMOD:
		if (i)
			ov_err(BADCHAR);	/* arithmetic operation on two character fields */
	}

	/* first check for identical types of symbols */
	if (p1->type == p2->type)
	{
		if (p1->len == p2->len)
			return;
		/* lengths are different. make p2 point to the smaller one */
		if (p1->len < p2->len)
		{
			p2 = p1;
			p1 = pp2;
		}

		switch (p2->type)
		{
		  case INT:
			if (p1->len == 2)
				p2->len = 2;
			else if (p1->len == 4)
				i2toi4(p2);

		  case CHAR:
			return;	/* done if char or int */

		  case FLOAT:
			f8tof4(p1);
			return;
		}
	}

	/* at least one symbol is an INT or FLOAT. The other can't be a CHAR */
	if (p1->type == CHAR || p2->type == CHAR)
		ov_err(BADMIX);	/* attempting binary operation on one CHAR field with a numeric */

	/* one symbol is an INT and the other a FLOAT */
	if (p2->type == INT)
	{
		/* exchange so that p1 is an INT and p2 is a FLOAT */
		p1 = p2;
		p2 = pp1;
	}

	/* p1 is an INT and p2 a FLOAT */
	itof(p1);
	if (p2->len == 4)
		f8tof4(p1);
}
/*
**	Coerce the top of stack symbol to the
**	specified type and length. If the current
**	value is a character then it must be converted
**	to numeric. A user error will occure is the
**	char is not syntaxtically correct.
*/

typecoerce(tosx, ntype, nlen)
SYMBOL	*tosx;
int	ntype;
int	nlen;
{
	register SYMBOL	*tos;
	register char	*cp;
	register int	*val;
	int		ret;
	char		temp[256];

	tos = tosx;

	if (tos->type == CHAR)
	{
		val = (int *) &tos->value.sym_data;
		cp = temp;
		bmove(tos->value.sym_data.cptype, cp, tos->len & I1MASK);
		cp[tos->len & I1MASK] = '\0';
		if (ntype == FLOAT)
			ret = atof(cp, val);
		else
		{
			if (nlen == 4)
				ret = atol(cp, val);
			else
				*val = atoi(cp);
		}
		if (ret < 0)
			ov_err(CHARCONVERT);
		tos->type = ntype;
		tos->len = nlen;
	}
	else
		rcvt(tos, ntype, nlen);
}


i2toi4(pp)
SYMBOL *pp;
{
	register SYMBOL	*p;

#	ifdef xOTR3
	if (tTf(87, 0))
	{
		printf("i2toi4: ");
		prsym(pp);
	}
#	endif
	p = pp;

	*(i4type *)&p->value = *(i2type *)&p->value;
	p->len = 4;
#	ifdef xOTR3
	if (tTf(87, 0))
	{
		printf("i2toi4 rets: ");
		prsym(p);
	}
#	endif
}


i4toi2(pp)
SYMBOL *pp;
{
	register SYMBOL	*p;

#	ifdef xOTR3
	if (tTf(87, 1))
	{
		printf("i4toi2: ");
		prsym(pp);
	}
#	endif
	p = pp;

	*(i2type *)&p->value = *(i4type *)&p->value;
	p->len = 2;
#	ifdef xOTR3
	if (tTf(87, 1))
	{
		printf("i4toi2 rets: ");
		prsym(p);
	}
#	endif
}


itof(p)
register SYMBOL *p;
{

#	ifdef xOTR3
	if (tTf(87, 2))
	{
		printf("itof: ");
		prsym(p);
	}
#	endif

	if  (p->len == 4)
		p->value.sym_data.f8type = p->value.sym_data.i4type;
	else
		p->value.sym_data.f8type = p->value.sym_data.i2type;
	p->type = FLOAT;
	p->len= 8;
#	ifdef xOTR3
	if (tTf(87, 2))
	{
		printf("itof rets: ");
		prsym(p);
	}
#	endif
}


ftoi2(p)
register SYMBOL *p;
{
#	ifdef xOTR3
	if (tTf(87, 3))
	{
		printf("ftoi: ");
		prsym(p);
	}
#	endif

	p->value.sym_data.i2type = p->value.sym_data.f8type;
	p->type = INT;
	p->len = 2;
#	ifdef xOTR3
	if (tTf(87, 3))
	{
		printf("ftoi rets: ");
		prsym(p);
	}
#	endif
}


ftoi4(p)
register SYMBOL *p;
{
#	ifdef xOTR3
	if (tTf(87, 4))
	{
		printf("ftoi4: ");
		prsym(p);
	}
#	endif

	p->value.sym_data.i4type = p->value.sym_data.f8type;
	p->type = INT;
	p->len = 4;
#	ifdef xOTR3
	if (tTf(87, 4))
	{
		printf("ftoi4 rets: ");
		prsym(p);
	}
#	endif
}


f8tof4(pp)
SYMBOL	*pp;
{
	pp->len = 4;
}
