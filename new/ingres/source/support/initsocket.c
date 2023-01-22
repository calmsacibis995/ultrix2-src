#ifndef lint
static	char	*sccsid = "@(#)initsocket.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	<sys/types.h>
# include	<sys/socket.h>
# include	<sys/ioctl.h>
# include	<netinet/in.h>
# include	<netdb.h>
# include	<signal.h>

static	char	Sccsid[] = "@(#)initsocket.c	1.1 (INGRES) 1/8/85";

/*
** init_socket
** initilize the socket to the socket server
*/
init_socket()
{
	register	int	from_socket;	/* file descriptor to attach to socket server */
	int	to_ioctl = 1;			/* used in ioctl call */
	struct	sockaddr_in	addr;		/* address where socket server is */
	char	hostname[BUFSIZ];		/* hostname */
	struct	servent		*server;
	struct	hostent		*myhost;
	auto	int		len;
	extern	int		errno;

	if ( (len = fork()) != 0 )
	{
# ifdef	DEBUG
		printf("lock driver becomes %d\n",len);
# endif	DEBUG
		if ( len == -1 )
		{
			perror("ingres lock driver, fork");
			exit(errno);
		}
		exit(0);
	}
	if ( (from_socket = socket(AF_INET,SOCK_STREAM,0)) == -1 )
	{
# ifdef	DEBUG
		perror("INIT_S socket");
# endif	DEBUG
		exit(errno);
	}
	len = BUFSIZ;
	gethostname(hostname,&len);

	if ( (server = getservbyname("ingreslock",(char *)0)) == 0 )
		exit(errno);

	if ( (myhost = gethostbyname(hostname)) == 0 )
		exit(errno);
	bzero((char *) &addr,sizeof (addr));
	bcopy(myhost->h_addr,(char *)&addr.sin_addr,myhost->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = server->s_port;
	len = sizeof (addr);
	if ( bind(from_socket,&addr,len) == -1 )
	{
# ifdef	DEBUG
		perror("INIT_S bind, assuming driver already running");
# endif	DEBUG
		exit(0);
	}

	if ( listen(from_socket,10) == -1 )
	{
		perror("Ingres lock, can't listen on port");
		exit(errno);
	}
	ioctl(from_socket,FIONBIO,&to_ioctl);
	return ( from_socket );
}/* init_socket */
