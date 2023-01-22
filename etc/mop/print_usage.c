#ifndef lint
static	char	*sccsid = "@(#)print_usage.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * Program print_usage.c,  Module MOP 
 *
 * Copyright (C) 1985 by
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
 * 1.00 30-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 */

#include <stdio.h>

/*
 *		p r i n t _ u s a g e
 *
 *
 * This subroutine prints the command line syntax.  
 *
 *
 * Inputs:
 *	caller		= name of calling program
 *	flags		= which argument switches are supported
 *	kw1		= first key word supported
 *	kw2		= second key word supported
 *	kw3		= third key word supported
 *	
 * Outputs:
 */
print_usage(caller, flags, kw1, kw2, kw3)
char *caller;
char *flags;
{
	int i;
	int flag_len = strlen(flags);


	fprintf(stderr, "%s usage:\n", caller);

	fprintf(stderr, "	%s", caller);
	if ( kw1 )
		fprintf(stderr, " %s", kw1);
	if ( kw2 )
		fprintf(stderr, " %s", kw2);
	if ( kw3 )
		fprintf(stderr, " %s", kw3);

	for ( i = 0; i < flag_len; i++ )
	{
		if ( flags[i] == 'A' )
			fprintf(stderr, "		[-A host node address]\n");
		if ( flags[i] == 'a' )
			fprintf(stderr, "		[-a target node address]\n");
		if ( flags[i] == 'h' )
			fprintf(stderr, "		[-h target hardware address]\n");
		if ( flags[i] == 'N' )
			fprintf(stderr, "		[-N host name]\n");
		if ( flags[i] == 'n' )
			fprintf(stderr, "		[-n target name]\n");
		if ( flags[i] == 'S' )
			fprintf(stderr, "		[-S service switch]\n");
		if ( flags[i] == 's' )
			fprintf(stderr, "		[-s secondary_load_file]\n");
		if ( flags[i] == 't' )
			fprintf(stderr, "		[-t tertiary_load_file]\n");
		if ( flags[i] == 'l' )
			fprintf(stderr, "		[-l system_load_file]\n");
		if ( flags[i] == 'd' )
			fprintf(stderr, "		[-d diagnostic_load_file]\n");
		if ( flags[i] == 'D' )
			fprintf(stderr, "		[-D dump_file]\n");
		if ( flags[i] == 'C' )
			fprintf(stderr, "		[-C cpu]\n");
		if ( flags[i] == 'c' )
			fprintf(stderr, "		[-c service circuit]\n");
		if ( flags[i] == 'u' )
			fprintf(stderr, "		[-u service device unit]\n");
		if ( flags[i] == 'p' )
			fprintf(stderr, "		[-p service password]\n");
		if ( flags[i] == 'P' )
			fprintf(stderr, "		[-P (permanent data base)]\n");
		if ( flags[i] == 'T' )
			fprintf(stderr, "		[-T software type]\n");
		if ( flags[i] == 'I' )
			fprintf(stderr, "		[-I software id]\n");
	}

	return;
}
