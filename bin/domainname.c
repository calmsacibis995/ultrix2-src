#ifndef lint
static char *sccsid = "@(#)domainname.c	1.2	ULTRIX	10/3/86";
#endif lint

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

/*
 * domainname -- get (or set domainname)
 */
#include <stdio.h>

char domainname[32];
extern int errno;

main(argc,argv)
	char *argv[];
{
	int	myerrno;

	argc--;
	argv++;
	if (argc) {
		if (setdomainname(*argv,strlen(*argv)))
			perror("setdomainname");
		myerrno = errno;
	} else {
		getdomainname(domainname,sizeof(domainname));
		myerrno = errno;
		printf("%s\n",domainname);
	}
	exit(myerrno);
}
