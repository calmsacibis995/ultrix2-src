#ifndef lint
static	char	*sccsid = "@(#)retrieve.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	"constants.h"
# include	"globals.h"



/*
**  RETRIEVE.C -- ret_list struct handlers
**
**	The Ret_list is a structure where references to C variables,
**	and their types, used in a target list of a "tupret" are kept
**	for outputting in the while loop generated.
*/

/*
**  ENTER_RET -- enter a variable in a ret_list
**	Concatenatest the strings in disp the calls add_ret
**	to add the new string to Ret_list.
**
**	Parameters:
**		disp -- display containing the reference to the variable
**		type -- type of the variable
*/


enter_ret(disp, type)
struct display		*disp;
int			type;
{
	char			buf [MAXSTRING + 2]; /* &buf [1] is the start
						      * of the concatenated
						      * strings
						      */
	register char		*srce, *dest;
	struct disp_node	*d;
	register		i;

	i = 0;
	dest = buf;
	for (d = disp->disp_first; d; d = d->d_next)
	{
		for (srce = d->d_elm; *srce; )
		{
			if (i < MAXSTRING)
			{
				i += 1;
				*++dest = *srce++;
			}
			else
				break;
		}

		if (i >= MAXSTRING)
		{
			yysemerr("reference to a variable too long, ']' probably missing from array subscription",
			0);
			break;
		}
	}
	*++dest = '\0';
	add_ret(salloc(&buf [1]), type);
}
/*
**  ADD_RET -- add a string (reference to a variable) to the Ret_list
**
**	Parameters:
**		s -- string to add
**		type -- type of variable being added
*/


add_ret(s, type)
char		*s;
int		type;
{
	register struct ret_list	*list;
	register struct ret_var		*node;

	if (!s)
	{
		s = "ERROR_TOKEN";
		yysemerr("alloc error", s);
	}
	list = &Ret_list;
	node = (struct ret_var *)nalloc(sizeof *node);
	if (!node)
	{
		yysemerr("alloc error", s);
		xfree(s);
		return;
	}
	node->r_elm = s;
	node->r_next = 0;
	node->r_type = type;
	if (list->ret_first == 0)
		list->ret_first = list->ret_last = node;
	else
	{
		list->ret_last->r_next = node;
		list->ret_last = node;
	}
}
/*
**  W_RET -- Generates the IIn_get() calls for the Ret_list
**
**	Any variable whose type is not string gets an '&'
**	(adress of) operand prepended.
*/


w_ret()
{
	register struct ret_var	*node;
	char			type [3];

	for (node = Ret_list.ret_first; node; node = node->r_next)
	{
		w_op("IIn_ret(");
		if (node->r_type != opSTRING)
			w_op("&");
		w_op(node->r_elm);
		w_op(",");
		itoa(node->r_type, type);
		w_op(type);
		w_op(");");
	}
}
/*
**  FRE_RET -- Free up the storage used by the Ret_list
*/

free_ret()
{
	register struct ret_list	*list;
	register struct ret_var		*n, *f;

	list = &Ret_list;
	for (f = list->ret_first; f; f = n)
	{
		n = f->r_next;
		xfree(f->r_elm);
		xfree(f);
	}
	list->ret_first = list->ret_last = 0;
}
