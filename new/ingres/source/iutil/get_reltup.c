#ifndef lint
static	char	*sccsid = "@(#)get_reltup.c	1.1	(ULTRIX)	1/8/85";
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

# include <ingres.h>
# include <access.h>
# include <aux.h>


/*
**  GET_RELTUP -- get appropriate tuple from relation catalog
**
**	Get the tuple for the relation specified by 'name'
**	and put it in the descriptor 'dx'.
**
**	First a relation named 'name' owned
**	by the current user is searched for. If that fails,
**	then a relation owned by the dba is searched for.
*/

get_reltup(d, name)
register DESC	*d;
char		*name;
{
	struct relation	rel;
	register int	i;

	clearkeys(&Admin.adreld);

	/* make believe relation relation is read only for concurrency */
	Admin.adreld.relopn = abs(Admin.adreld.relopn);

	/* relation relation is open. Search for relation 'name' */
	setkey(&Admin.adreld, (char *) &rel, name, RELID);
	setkey(&Admin.adreld, (char *) &rel, Usercode, RELOWNER);

	if ((i = getequal(&Admin.adreld, (char *) &rel, d, &d->reltid.s_tupid)) == 1)
	{
		/* not a user relation. try relation owner by dba */
		setkey(&Admin.adreld, (char *) &rel, Admin.adhdr.adowner, RELOWNER);
		i = getequal(&Admin.adreld, (char *) &rel, d, &d->reltid.s_tupid);
	}

	flush_rel(&Admin.adreld, TRUE);

#	ifdef xATR1
	if (tTf(21, 1))
		printf("get_reltup: %d\n", i);
#	endif

	/* restore relation relation to read/write mode */
	Admin.adreld.relopn = -Admin.adreld.relopn;
	return (i);
}
