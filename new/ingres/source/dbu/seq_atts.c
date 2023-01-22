#ifndef lint
static	char	*sccsid = "@(#)seq_atts.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


/*
** Seq_attributes - get all attributes of a relation in their correct order.
**
**	Seq_attributes() can be called when it is desired to get all
**	attribute tuples for a relation. They are guaranteed to be in
**	the same order as they were created in; that is "attid" order.
**
**	The calling convention is:
**
**		seq_init(Attribute_descriptor, Descriptor_for_relation)
**
**		while (seq_attribute(Att_desc, Des_for_rel, Tuple)
**		{
**		}
**
**	The first parameter is an OPEN descriptor for the attribute
**	relation. The second parameter is a descriptor for the relation.
**	It must have the relation-relation tuple (ie openr mode -1).
**	The third parameter is a place to put the attribute. Its
**	dimension must be at least sizeof (attribute structure).
**
**	Seq_attribute attempts to optimize the retrieval of the
**	attributes. It assumes initially that the are physically
**	stored sequentially. If so it will retrieve them in the
**	minimum possible number of accesses.
**
**	If it finds one tuple out of sequence then all future
**	scans will start from that first out of sequence tuple.
*/

struct tup_id	Seq_tdl, Seq_tdh, Seq_tdf;
int		Seq_seqmode, Seq_next;



seq_init(a, r)
register DESC	*a;
register DESC	*r;
{
	register int		i;
	struct attribute	attkey;

	clearkeys(a);
	setkey(a, &attkey, r->reldum.relid, ATTRELID);
	setkey(a, &attkey, r->reldum.relowner, ATTOWNER);
	if (i = find(a, EXACTKEY, &Seq_tdl, &Seq_tdh, &attkey))
		syserr("seq_init:find:%d", i);

	Seq_seqmode = TRUE;
	Seq_next = 1;
}


seq_attributes(a, r, atup)
register DESC			*a;
register DESC			*r;
register struct attribute	*atup;
{
	int	i, nxttup;

	if (Seq_next > r->reldum.relatts)
		return (0);	/* no more attributes */

	if (!Seq_seqmode)
	{
		/* attributes not stored sequencially, start from first attribute */
		bmove(&Seq_tdf, &Seq_tdl, sizeof (Seq_tdl));
		nxttup = FALSE;
	}
	else
		nxttup = TRUE;

	while ((i = get(a, &Seq_tdl, &Seq_tdh, atup, nxttup)) == 0)
	{

		nxttup = TRUE;
		/* make sure this is a tuple for the right relation */
		if (kcompare(a, atup, r))
			continue;

		/* is this the attribute we want? */
		if (atup->attid != Seq_next)
		{
			if (Seq_seqmode)
			{
				/*
				** Turn off seq mode. Save the tid of
				** the current tuple. It will be the
				** starting point for the next call
				** to seq_attribute
				*/
				bmove(&Seq_tdl, &Seq_tdf, sizeof (Seq_tdf));
				Seq_seqmode = FALSE;
			}

			continue;
		}

		/* got current attribute. increment to next attribute */
		Seq_next++;
		return (1);
	}

	/* fell out of loop. either bad get return or missing attribute */
	syserr("seq_att:bad or missing %d,%d", i, Seq_next);
}
