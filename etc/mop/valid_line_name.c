#ifndef lint
static char *sccsid = "@(#)valid_line_name.c	1.3	ULTRIX	12/4/86";
#endif lint

/*
 *
 * Copyright (C) 1984 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-July-1985 
 *      DECnet-ULTRIX   V1.0
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <netdnet/dn.h>

#define nm_MAXDEV	4
#define nm_MAXLINE	16
#define nm_FAIL		0
#define nm_SUCCESS	1

char *nice_devtab[nm_MAXDEV][2] =	/*  Device table - UNIX and NICE */
{
	{ "de",	"UNA"} ,
	{ "qe",	"QNA"} ,
	{ "ni",	"BNT"} ,
	{ "se",	"SVA"}
};

/*
 *				v a l i d _ l i n e _ n a m e
 *
 *  Checks the validity of a line/circuit name
 *
 *  Input:  Name of the line or circuit
 *
 *  Output: nm_SUCCESS if valid name, else
 *			nm_FAIL
 */

valid_line_name(name)
u_char name[];

{

	int i, hyphen, number;

	for (i = 0; i < nm_MAXLINE; i++)
		if (name[i] == '-')
			break;
	
	hyphen = i;

	while (name[++i] != '\0')
	{
		if (!isdigit(name[i]))
			return(nm_FAIL);
	}

	if (((number = atoi(&name[hyphen + 1])) < 0) || (number > 65535))
		return(nm_FAIL);

	for (i = 0; i < nm_MAXDEV; i++)
		if (strncmp(name, nice_devtab[i][1], hyphen) == 0)
			return(nm_SUCCESS);

	return(nm_FAIL);
}
