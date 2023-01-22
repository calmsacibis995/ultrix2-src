#ifndef lint
static	char	*sccsid = "@(#)rhash.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>
# include	<access.h>
# include	<lock.h>


/*
**  RHASH -- perform a randomizing hash on the full key.
**
**	Trace Flags:
**		26.12-13
*/

long
rhash(d, key)
register DESC	*d;
char		key[MAXTUP];
{
	register int	i;
	register char	*cp;
	long		bucket;
	char		tmp;
	int		j, *k, knt, numeric;

	bucket = 0;
	knt = 0;
	for (i = 1; i <= d->reldum.relatts; i++)
		if (d->relxtra[i])
		{
			/* form pointer to field */
			cp = &key[d->reloff[i]];
			numeric = d->relfrmt[i] != CHAR;
			for (j = 0; j < (d->relfrml[i] & I1MASK); j++)
				if (((tmp = *cp++) != ' ') || numeric)
					addabyte(tmp, &bucket, knt++);
		}
	/* remove sign bit from bucket the hard way */
	k = &bucket;
	*k &= 077777;
#	ifdef xATR3
	if (tTf(19, 12))
		printf("rhash:hval=%ld", bucket);
#	endif
	bucket %= d->reldum.relprim;
#	ifdef xATR3
	if (tTf(19, 12))
		printf(",returning %ld\n", bucket);
#	endif
	return (bucket);
}
/*
** ADDABYTE is used to map a long key into a four byte integer.
** As bytes are added, they are first rotated, then exclusive ored
** into the existing key.
*/

addabyte(ch, word, knt1)
char	ch;
long	*word;
int	knt1;
{
	register int	knt;
	long		i;

	knt = knt1;
	i = ch & 0377;	/*get rid of any sign extension*/
	knt += 8 * (knt & 3);	/*alternately add 0, 8, 16 or 24 to knt */
	knt &= 037;
	*word ^= (i << (knt) | i >> (32 - knt));
}
