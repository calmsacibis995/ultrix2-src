#ifndef lint
static	char	*sccsid = "@(#)equelpr.c	1.1	(ULTRIX)	1/8/85";
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
# include	<tree.h>
# include	<batch.h>
# include	"../ctlmod/pipes.h"
# include	"../decomp/globs.h"


/*
**	This file contains all the routines needed
**	for communicating with the equel process.
**	They are called only if the flag Equel = TRUE.
*/


pb_t	EquelPb;


startequel()
{
	pb_prime(&EquelPb, PB_REG);
	EquelPb.pb_proc = PB_FRONT;
	EquelPb.pb_st = PB_FRONT;
	EquelPb.pb_stat |= PB_INFO;
}
/*
**	equelatt writes one symbol pointed to
**	by ss up the data pipe to the equel
**	process.
**
**	if a symbol is a character then *ss->value
**	contains a pointer to the character string.
**	otherwise the value is stored in successive
**	words starting in ss->value.
*/

equelatt(ss)
SYMBOL	*ss;
{
#	ifdef xOTR1
	if (tTf(80, 0))
		prstack(ss);
#	endif
	pwritesym(ss);
}
/*
**	equeleol is called at the end of the interpretation of
**	a tuple. Its purpose is to write an end-of-tuple
**	symbol to the equel process and flush the pipe.
**
**	It is also called at the end of a query to write
**	an exit symbol to equel.
*/

equeleol(code)
int	code;
{
	struct stacksym	symb;

	symb.s_type = code;
	symb.s_len = 0;

#	ifdef  xOTR1
	if (tTf(80, 3))
		printf("equeleol:writing %d to equel\n", code);
#	endif

	pb_put((char *)&symb, TYP_LEN_SIZ, &EquelPb);


	/* flush after every tuple for Equel versions before 6.2 
	 * and at end of results always
	 */
	if (code == EXIT)
		pb_flush(&EquelPb);
}
/*
**	pwritesym write the stacksymbol
**	pointed to by "ss" to the pipe
**	indicated by filedesc.
**
**	The destination will either be equel
**	or decomp
**
**	Since a CHAR isn't stored immediately following
**	the type and len of the symbol, A small bit
**	of manipulation must be done.
*/

pwritesym(s)
register SYMBOL	*s;
{
	register char	*p;
	register int	length;

	length = s->len & 0377;
	pb_put((char *)s, TYP_LEN_SIZ, &EquelPb);

	if (s->type  == CHAR)
		p = s->value.sym_data.cptype;	/* p points to the string */
	else
		p = s->value.sym_data.c0type;

	pb_put(p, length, &EquelPb);
}
