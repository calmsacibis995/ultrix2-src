#ifndef lint
static	char	*sccsid = "@(#)vars.c	1.4	(ULTRIX)	12/10/84";
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

/*
 * vars.c
 *
 *	sccsid[] = "@(#)vars.c	2.5 (Berkeley) 8/11/83";
 *
 *	16-Feb-84	mah. Add comments and copyright.
 *
 */

#include "rcv.h"

/*
 * Mail -- a mail program
 *
 * Variable handling stuff.
 */

/*
 * Assign a value to a variable.
 */

assign(name, value)
	char name[], value[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	vp = lookup(name);		/*find the variable in table*/
	if (vp == NOVAR) {		/*variable not found so allocate the
					 *space for it.
					 */
		vp = (struct var *) calloc(sizeof *vp, 1);
		vp->v_name = vcopy(name);	/*set ptr to name string*/
		vp->v_link = variables[h];	/*set linking*/
		variables[h] = vp;
	}
	else
		vfree(vp->v_value);	/*free up space for old value*/
	vp->v_value = vcopy(value);	/*set ptr to "new" value string*/
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */

vfree(cp)
	register char *cp;
{
	if (!equal(cp, ""))	/*if cp = null string then ret space allocated
				 *for cp.
				 */
		cfree(cp);	/*cfree==free(compatible w/ Kernighan/Ritchie)*/
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for ""
 */

char *
vcopy(str)
	char str[];
{
	register char *top, *cp, *cp2;

	if (equal(str, ""))	/*if null string return such*/
		return("");
	if ((top = calloc(strlen(str)+1, 1)) == NULL)	/*get memory for str*/
		panic ("Out of memory");
	cp = top;		/*cp = beginning of space*/
	cp2 = str;		/*cp2 ptr to string*/
	while (*cp++ = *cp2++)	/*copy string*/
		;
	return(top);		/*return ptr to location of copied str in mem*/
}

/*
 * Get the value of a variable and return it.
 * Look in the environment if its not available locally.
 */

char *
value(name)
	char name[];
{
	register struct var *vp;

	if ((vp = lookup(name)) == NOVAR)	/*if not in table, get from
						 *environment list.
						 */
		return(getenv(name));
	return(vp->v_value);
}

/*
 * Locate a variable and return its variable
 * node.
 */

struct var *
lookup(name)
	char name[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
		if (equal(vp->v_name, name))
			return(vp);
	return(NOVAR);
}

/*
 * Locate a group name and return it.
 */

struct grouphead *
findgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register int h;

	h = hash(name);
	for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
		if (equal(gh->g_name, name))
			return(gh);
	return(NOGRP);
}

/*
 * Print a group out on stdout
 */

printgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register struct group *gp;

	if ((gh = findgroup(name)) == NOGRP) {
		printf("\"%s\": not a group\n", name);
		return;
	}
	printf("%s\t", gh->g_name);
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link)
		printf(" %s", gp->ge_name);
	printf("\n");
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */

hash(name)
	char name[];
{
	register unsigned h;
	register char *cp;

	for (cp = name, h = 0; *cp; h = (h << 2) + *cp++)
		;
	if (h < 0)
		h = -h;
	if (h < 0)
		h = 0;
	return(h % HSHSIZE);
}
