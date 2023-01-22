
#ifndef lint
static	char	*sccsid = "@(#)hash.c	1.1	(ULTRIX)	3/20/86";
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
 *
 *   Modification History:
 *
 *
 */


#include	"hash.h"
#include	"defs.h"

#define STRCMP(A, B)	(cf(A, B) != 0)
#define FACTOR 	 		035761254233	/* Magic multiplication factor */
#define TABLENGTH		64				/* must be multiple of 2 */
#define LOG2LEN			6				/* log2 of TABLENGTH */

/*
    NOTE: The following algorithm only works on machines where
    the results of multiplying two integers is the least
    significant part of the double word integer required to hold
    the result.  It is adapted from Knuth, Volume 3, section 6.4.
*/

#define hash(str)		(int) (((unsigned) (crunch(str) * FACTOR)) >> shift)

struct node
{
	ENTRY item;
	struct node *next;
};

static struct node	**last;
static struct node	*next;
static struct node 	**table;

static unsigned int 	bitsper;		/* Bits per byte */
static unsigned int	shift;

static unsigned int crunch();

hcreate()
{
	unsigned char c = ~0;			/* A byte full of 1's */
	int j;

	table = (struct node **)alloc(TABLENGTH * sizeof(struct node *));

	for (j=0; j < TABLENGTH; ++j)  
	{
		table[j] = 0;
	}

	bitsper = 0;

	while (c)		
	{
		c >>= 1;
		bitsper++;
	}

	shift = (bitsper * sizeof(int)) - LOG2LEN;
}


void hscan(uscan)	
	void	(*uscan)();
{
	struct node		*p, *nxt;
	int				j;

	for (j=0; j < TABLENGTH; ++j)
	{
		p = table[j];
		while (p)
		{
			nxt = p->next;
			(*uscan)(&p->item);
			p = nxt;
		}
	}
}



ENTRY *
hfind(str)
	char	*str;
{
	struct node 	*p;
	struct node 	**q;
	unsigned int 	i;
	int 			res;		

	i = hash(str);

	if(table[i] == 0)
	{			
		last = &table[i];
		next = 0;
		return(0);
	}
	else 
	{
		q = &table[i];
		p = table[i];
		while (p != 0 && (res = STRCMP(str, p->item.key))) 
		{
			q = &(p->next);
			p = p->next;
		}

		if (p != 0 && res == 0)	
			return(&(p->item));
		else
		{
			last = q;
			next = p;
			return(0);
		}
	}
}

ENTRY *
henter(item)
	ENTRY item;
{
	struct node	*p = (struct node *)alloc(sizeof(struct node));

	p->item = item;
	*last = p;
	p->next = next;
	return(&(p->item));
}


static unsigned int 
crunch(key)	
	char	*key;
{
	unsigned int 	sum = 0;	
	int s;

	for (s = 0; *key; s++)				/* Simply add up the bytes */
		sum += *key++;

	return(sum + s);
}

