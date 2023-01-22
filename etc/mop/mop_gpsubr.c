#ifndef lint
static	char	*sccsid = "@(#)mop_gpsubr.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * Program mop_dumpload.c,  Module MOP 
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
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 */
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "/sys/net/if.h"
#include "/sys/netinet/in.h"
#include "/sys/netinet/if_ether.h"
#include <netdnet/dli_var.h>
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include <netdnet/node_params.h>
#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_var.h"
#include "mop_proto.h"

#define STDIN	0
#define STDOUT	1

extern int errno;
extern int exit_status;
extern char errbuf[4096];

/*
*		t o u p c a s e
*
*
* Version: 1.0 - 1/10/85
*
* Description:
*	This routine makes sure that a string is in upper case
*	and returns the number of characters in the string.
*
* Inputs:
*	buffer			= Pointer to buffer containing string.
*
*
* Outputs:
*	returns			string size 
*
* Notes:
*
*/
toupcase( buffer )
register char *buffer;
{
	register int i = -1;
	while ( buffer[++i] != NULL )
	{
		if ( buffer[i] >= 'a' && buffer[i] <= 'z' )
		{
			buffer[i] -= ('a' - 'A');
		}
	}
	return(i);
}

/*
*		t o l o c a s e
*
*
* Version: 3.0 - 9/4/85
*
* Description:
*	This routine makes sure that a string is in lower case
*	and returns the number of characters in the string.
*
* Inputs:
*	buffer			= Pointer to buffer containing string.
*
*
* Outputs:
*	returns			string size 
*
* Notes:
*
*/
tolocase( buffer )
register char *buffer;
{
	register int i = -1;
	while ( buffer[++i] != NULL )
	{
		if ( isupper( buffer[i] ) )
		{
			buffer[i] = tolower( buffer[i] );
		}
	}
	return(i);
}

/*
*		g e t h o s t n o d e
*
*
* Version: 1.0 - 5/9/85
*
* Description:
*	This routine fetches the host name and DECnet address.  If 
*	unspecified in host argument, the the local name and address
*	is used.
*	The size of the name is returned.
*
* Inputs:
*	host			= Pointer to struct containing host info
*	hname			= Pointer where host name is to be placed.
*	haddr			= Pointer where host addr is to be placed.
*
*
* Outputs:
*	returns			number of characters in hname
*
* Notes:
*
*/
gethostnode( host, hname, haddr )
register struct node_id *host;
register u_char *hname;
register u_short *haddr;
{
	int i;
	struct dn_naddr *addr, *dnet_getlocadd();
	struct nodeent *node;

	hname[0] = NULL;
	*(u_short *) haddr = 0;

	if ( i = toupcase(host->node_name) )
	{
		strcpy(hname, host->node_name);
		if ( host->node_dnaddr == 0 && (node = getnodebyname(host->node_name)) )
		{
			*(u_short *) haddr = * (u_short *) node->n_addr;
		}
		else
		{
			*(u_short *) haddr = host->node_dnaddr;
		}
		return(i);
	}

	else if ( host->node_dnaddr )
	{
		*(u_short *) haddr = host->node_dnaddr;
		if ( (i = toupcase(host->node_name)) == 0 &&
			(node = getnodebyaddr(&host->node_dnaddr, sizeof(host->node_dnaddr), AF_DECnet)) )
		{
			strcpy(hname, node->n_name);
			i = toupcase(node->n_name);
		}
		else
		{
			strcpy(hname, host->node_name);
		}
		return(i);
	}

	else
	{
		if ( ! (addr = dnet_getlocadd(0)) )
			return(0);

		bcopy(addr->a_addr, haddr, sizeof(*haddr));

		if ( ! (node = getnodebyaddr(haddr, sizeof(*haddr), AF_DECnet)) )
			return(0);

		strcpy(hname, node->n_name);

		return( toupcase(hname) );
	}

}

/*
*		g e t h s t i m e
*
*
* Version: 1.0 - 1/10/85
*
* Description:
*	This routine fetches the local time and places it in
*	the supplied buffer in the format specified by the
*	load parameters message in the MOP spec.
*
* Inputs:
*	buffer			= Pointer to buffer where data is to be placed.
*	bufsiz			= size of buffer to contain data.
*
*
* Outputs:
*	returns			0 if success, -1 if failure
*
* Notes:
*
*/
gethstime( buffer, bufsiz )
register u_char *buffer;
int bufsiz;
{
	register int i = 0;
	struct tm *at_the_tone, *localtime();
	struct timeval tv;
	struct timezone tz;

	if ( bufsiz < MOP_PLOAD_HSTIMSIZ )
	{
		return(-1);
	}

	if ( gettimeofday(&tv, &tz) < 0 )
	{
		return(-1);
	}

	at_the_tone = localtime(&tv.tv_sec);

	buffer[i++] = MOP_PLOAD_CENTURY;
	buffer[i++] = at_the_tone->tm_year;
	buffer[i++] = at_the_tone->tm_mon + 1;
	buffer[i++] = at_the_tone->tm_mday;
	buffer[i++] = at_the_tone->tm_hour;
	buffer[i++] = at_the_tone->tm_min;
	buffer[i++] = at_the_tone->tm_sec;
	buffer[i++] = 0;
	buffer[i++] = 0;
	buffer[i++] = 0;

	return(NULL);
}

/*
*		c a t c h _ s i g a l r m
*
* Version: 1.0 - 1/10/85
*
*
*
*
* IDENT HISTORY:
*
* 1.00 8-NOV-84
*      DECnet-ULTRIX   V1.0
*
* looper routine to catch SIGALRM signal ***
*
* Description:
*	This routine catches the alarm signal.
*	It closes the socket whenever SIGALRM occurs.
*
*
* inputs:
*	- sig: signal
*	- code: signal parameter
*	- scp: pointer to signal context
*
* outputs:
*
* Notes:
*
*/

extern int sock;

void catch_sigalrm(sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
	switch (sig)
	{
		case SIGALRM:
			close(STDIN);
			break;
		default:
			sprintf(errbuf, "unrecognized signal = %d", sig);
			mop_syslog("mop_???", errbuf, 0, LOG_INFO);
			break;
	}
	return;
}

/*
*		r e a d _ m o p
*
* Version: 1.0 - 1/10/85
*
*
*
*
* IDENT HISTORY:
*
* 1.00 8-NOV-84
*      DECnet-ULTRIX   V1.0
*
*
* Description:
*	This routine implements a read with timeout capability.
*
*
* inputs:
*	time		= amount of time in seconds to wait for data.
*	buffer		= pointer to buffer where data is to be placed.
*	bufsiz		= size of buffer area.
*
* outputs:
*	buffer		= received data placed here.
*	returns		= same as read().
*
* Notes:
*
*/
read_mop(time, buffer, bufsiz)
int time, bufsiz;
u_char *buffer;
{
	struct itimerval tvalue, ovalue;
	void catch_sigalrm();
	int data_siz;

	/*
	 * start timer and read.
	 */
	bzero(&tvalue,sizeof(struct itimerval));
	tvalue.it_value.tv_sec = time;
	if ( setitimer(ITIMER_REAL, &tvalue, &ovalue) < 0 )
	{
		return(-1);
	}
	if ( (data_siz = read(STDIN, buffer, bufsiz)) < 0 )
	{
		if ( errno == EBADF )
		{
			dup2(STDOUT, STDIN);
			errno = ETIMEDOUT;
		}
	}



	/*
	 * disable timer.
	 */
	tvalue.it_value.tv_sec = 0;
	tvalue.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tvalue, &ovalue);

	return(data_siz);

}
