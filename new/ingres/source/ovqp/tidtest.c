#ifndef lint
static	char	*sccsid = "@(#)tidtest.c	1.2	(ULTRIX)	1/22/85";
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
# include	<symbol.h>
# include	<tree.h>
# include	"../decomp/globs.h"
# include	"strategy.h"


/*
** tid_only_test
** Check the qualification list to see if it
** contains exactly one simple clause, that 
** clause refers to a tid as the VAR, and that
** the binary operation is opEQ.
**
** Side Effects:
**	If the condition holds true, De.ov_hitid and De.ov_lotid
**	are set to refer to the constant value.
** 
** Returns:
**	1 if qualification holds,
**	0 on failure.
**
** Trace Flags:
**	89
**
** Called From:
**	strategy
*/
tid_only_test()
{
	register struct symbol	*c;
	register int		t;
	register struct symbol	**q;
	int			found;
	int			i;

#	ifdef xOTR1
	if (tTf(89, 0))
		printf("TID_ONLY_TEST\n");
#	endif
	found = FALSE;

	q = De.ov_qlist;	/* q holds pointer to qualification */

	if (!q)
		return (0);


	/*
	** iterate through the tree
	*/
	for (t = (*q)->type; t != QLEND; t = (*++q)->type)
	{
		/*
		** The only thing we allow is a single simple
		** expression with tids.
		*/
		if ( found == TRUE )
			return ( 0 );

		switch (t)
		{
		  case VAR:
			/*
			** Only allow tids to be vars.
			*/
			if ( (*q)->value.sym_var.attno != 0 )
				return (0);
			t = (*++q)->type;
			if ( t != INT )
				return ( 0 );
			else
			{
				c = *q;	/* save pointer to value symbol */
				t = (*++q)->type;
				if (relop(*q, FALSE) == opEQ 
				   && (t = (*++q)->type) == AND)
				{
					/* found a simple clause */
					found = TRUE;
				}
			}
			break;

		  case INT:
			c = *q++;
			if ((t = (*q)->type) != VAR)
				return ( 0 );
			else
			{
				if ( (*q)->value.sym_var.attno != 0 )
					return ( 0 );
				t = (*++q)->type;
				if ( relop(*q, TRUE) == opEQ && (t = (*++q)->type) == AND)
				{
					/* found a simple clause */
					found = TRUE;
				}
				else
					return ( 0 );
			}

		  default:
			return ( 0 );
		}
	}

#	ifdef xOTR1
	if (tTf(89, 2))
		printf("tid_only_test returning %d\n", found);
#	endif

	/*
	** We have found a simple clause using only the tid.
	** Set the low and high search keys.
	*/
	if ( found == TRUE )
	{
		register	union	symvalue	*p;

		p = &c->value;
		De.ov_lotid = De.ov_hitid = p->sym_data.i2type;
		dec_tid(&De.ov_lotid);
		return (1);
	}

	return ( 0 );
}/* tid_only_test */

/*
** dec_tid
** Decrement the line-id of a tid
*/
dec_tid(tid)
TID	*tid;
{
	tid->line_id--;
}/* dec_tid */
