#ifndef lint
static	char	*sccsid = "@(#)lock.c	1.1	(ULTRIX)	1/8/85";
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
# include	<netinet/in.h>
# include	<netdb.h>
# include	<signal.h>
# include	<setjmp.h>


/*
** start_up_lock_driver
**	Attempt to start up a connection to the lock driver.
**	We connect to a know address (a socket server sits there).
**	If we get a connection on this location, than we are talking 
**	to the lock driver. If we timeout, then we assume the driver 
**	isn't there.
**
** Returns
**	File descriptor attached to the lock driver
**	-1 on any error.
**
** Trace Flags
**	28
*/
start_up_lock_driver()
{
	struct	sockaddr_in	addr;		/* address to attach to for server */
	register	int	to_driver;	/* we can talk to the lock driver on this one */
	auto		int	hostlen;	/* length of the hostname */
	register	char	*host;		/* name of this host */
	char			hname[BUFSIZ];
	struct		servent	*ing_ser;
	struct		hostent	*myhost;


	/*
	** Find out where the lock driver lives
	*/
	if ( (ing_ser = getservbyname("ingreslock",(char *)0)) == 0 )
	{
#		ifdef xATR1
		if ( tTf(28,4) )
			perror("set_up_lock getservbyname");
#		endif
		return ( -1 );
	}

	/*
	** Make our end of the socket
	*/
	if ( (to_driver = socket(AF_INET,SOCK_STREAM,0)) == -1 )
	{
#		ifdef xATR1
		if ( tTf(28,4) )
			perror("set_up_lock socket");
#		endif
		return ( -1 );
	}

	host = hname;
	hostlen = BUFSIZ;
	gethostname(hname,&hostlen);
	if ( (myhost = gethostbyname(host)) == 0 )
	{
#	ifdef xATR1
		if ( tTf(28,4) )
			perror("set_up_lock gethostbyname");
#	endif
		close(to_driver);
		return ( -1 );
	}
	bzero((char *) &addr,sizeof (addr));
	bcopy(myhost->h_addr,(char *)&addr.sin_addr,myhost->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = ing_ser->s_port;


	/*
	** Connect to the lock_driver
	*/
	if ( connect(to_driver,&addr,sizeof (addr)) == -1 )
	{
#	ifdef xATR1
		if ( tTf(28,4) )
			perror("set_up_lock connect");
#	endif
		close(to_driver);
		return ( -1 );
	}


	return ( to_driver );
}/* start_up_lock_driver */


struct servent *
getservbyname(name, proto)
	char *name, *proto;
{
	register struct servent *p;
	register char **cp;

	setservent(0);
	while (p = getservent()) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	endservent();
	return (p);
}
