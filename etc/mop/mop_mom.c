#ifndef lint
static	char	*sccsid = "@(#)mop_mom.c	1.7	(ULTRIX)	3/18/87";
#endif lint

/*
 * Program mop_mom.c,  Module MOP 
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

/*
*		m o p _ m o m . c
*
* Version:	1.0 - 1/10/85
*		1.1 - 3/14/85 - took out loopback since it is now in DLI.
*		    - 4/9/85 - log errors via syslog instead of stderr.
*
*
* Description:
*		This program listens for MOP messages via DLI module.
*		It analyzes the message and forks an appropriate process
*		to handle the message.  If the message is a MOP loopback
*		message, it processes the message itself and hands it
*		back to DLI.
*
* Inputs:
*		None.
*
* Outputs:
*		None.
*
* Notes:
*
*/
 
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "/sys/net/if.h"
#include "/sys/netinet/in.h"
#include "/sys/netinet/if_ether.h"
#include <netdnet/dli_var.h>
#include "mop_var.h"
#include "mop_proto.h"


extern int errno;
extern int mop_syslog_debug;
char errbuf[4096];
char *target_address = NULL;

struct proto_mcast pm_table[] = { 
	{ PROTO_DUMP_LOAD,	MCAST_DUMP_LOAD, DUMP_LOAD_PATH,	DUMP_LOAD },
	{ NULL,			MCAST_NULL, 	NULL_PATH,	NULL_PROG }
};


static char MOP_MOM[] = " @(#)mop_mom.c	1.6		9/13/85 ";

main(argc, argv, envp)
int argc;
char **argv, **envp;

{
	
	struct sockaddr_dl from;
	struct ifconf mom_ifconf;
	struct ifreq ifreq_table[MAX_IFREQ];
	struct mom_sock_db dli_sockets[MAX_SOCKETS];
	u_char message[MOM_MSG_SIZE];
	char src_addr[ASCII_EADDRSIZE];
	int fromlen, rsize, so_num, sock, i, j, k, l, tt;
	int nfound, rdfds, iff_mask;
	void funeral();
	char *mop_debug, *getenv();

	/*
	 * make sure operator is super user
	 */
	if ( getuid() != NULL )
	{
		fprintf(stderr, "mop_mom: You must be super-user.\n");
		exit(1);
	}

	/*
	 * Fork ourselves so that the init process becomes our parent and
	 * remove any association with the originating terminal.
	 */
	if (fork())
		exit(0);
	
	if ((tt = open("/dev/tty", O_RDWR)) != -1)
	{
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	}

	/*
	 * Check for debugging environment
	 */
	if (( mop_debug = getenv( "LOADUMP_DEBUG" )) != NULL && strcmp(mop_debug, "on") == 0 )
	{
		mop_syslog_debug = 1;
	}

	/*
	 * create database of available enet devices with each device
	 * associated with the supported protocol types.
	 */
	so_num = 0;
	if ( (sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 )
	{
		mop_syslog("mop_mom", "can't open socket to AF_UNIX", 1, LOG_INFO);
		exit(1);
	}
	mom_ifconf.ifc_len = sizeof(ifreq_table);
	mom_ifconf.ifc_req = &ifreq_table[0];
	if ( ioctl(sock, SIOCGIFCONF, &mom_ifconf) < 0 )
	{
		mop_syslog("mop_mom", "io control failed", 1, LOG_INFO);
		exit(1);
	}
	if ( ! mom_ifconf.ifc_len )
	{
		mop_syslog("mop_mom", "NULL ifreq_table", 0, LOG_INFO);
		exit(1);
	}
	j = mom_ifconf.ifc_len/sizeof(struct ifreq);
	for (i=0; i<j; i++)
	{
		if ( ioctl(sock, SIOCGIFFLAGS, &ifreq_table[i]) < 0 )
		{
			mop_syslog("mop_mom", "io control failed", 1, LOG_INFO);
			exit(1);
		}
		iff_mask = IFF_BROADCAST | IFF_UP | IFF_RUNNING | IFF_DYNPROTO;
		if ( (ifreq_table[i].ifr_flags & iff_mask) == iff_mask )
		{
			k = -1;
			while (pm_table[++k].multicast[0] & 1)
			{
				int device_found_twice = 0;
				for (l = 0; l < i; l++)
				{
					if ( strcmp(ifreq_table[i].ifr_name, ifreq_table[l].ifr_name) == 0 )
					{
						device_found_twice = 1;
						break;
					} 
				}
				if ( device_found_twice )
					break;

				l = 0;
				while ( l < IFNAMSIZ && isalpha(ifreq_table[i].ifr_name[l]) ) l++;
				if ( l <= sizeof(dli_sockets[so_num].devname) )
				{
					dli_sockets[so_num].pm = &pm_table[k];
					bcopy(ifreq_table[i].ifr_name, dli_sockets[so_num].devname, l);
					dli_sockets[so_num++].devunit = (u_short) atoi(ifreq_table[i].ifr_name+l);
				}
			}


		}
	}
	close(sock);



	/*
	 * Open sockets.
	 */
	for( i = 0; i < so_num; i++ )
	{
		open_dli_sock(&dli_sockets[i]);
	}


	/*
	 * declare signal handlers
	 */
 	if (signal(SIGCHLD, funeral) < 0)
 	{
		mop_syslog("mop_mom", "can't declare signal SIGCHLD", 1, LOG_INFO);
 		exit(1);
 	}

	while(1)
	{
		/*
		 * Listen for MOP requests.
		 */
		rdfds = 0;
		for ( i = 0; i < so_num; i++ )
		{
			if ( dli_sockets[i].so >= 0 )
				rdfds |= 1 << dli_sockets[i].so;
		}
		if ( (nfound = select(31, &rdfds, NULL, NULL, NULL )) < 0 )
		{
			if ( errno == EINTR )
			{
				continue;
			}
			else
			{
				mop_syslog("mop_mom", "select failed", 1, LOG_INFO);
				exit(1);
			}
		}
		/*
		 * Act on request
		 */
		sock = -1;
		while( nfound > 0 && ++sock < 32 )
		{
			if ( ( 1 << sock ) & rdfds )
			{
				from.dli_family = AF_DLI;
				fromlen = sizeof(struct sockaddr_dl);
				if ((rsize = recvfrom(sock, message, sizeof(message), NULL, &from, &fromlen)) <= 0)
				{
					mop_syslog("mop_mom", "receive failed", 1, LOG_INFO);
					exit(1);
				}
				translate_addr( src_addr, from.choose_addr.dli_eaddr.dli_target );
				target_address = src_addr;

				switch ( from.choose_addr.dli_eaddr.dli_protype )
				{
					case PROTO_DUMP_LOAD:
						dump_load(sock, &from, message, rsize, envp);
						break;

					default:
						mop_syslog("mop_mom", "unknown MOP message", 0, LOG_INFO);
						break;
				}
				nfound--;

			}
		}
	}
}

/*
*		o p e n _ d l i _ s o c k
*
*
* Version:	1.0 - 1/10/85
*
* Description:
*	This subroutine opens a dli socket, binds an associated
*	device name and protocol type, and enables a multicast
*	address.
*
* Inputs:
*		Pointer to structure containing info pertinent to socket.
*
* Outputs:
*		None.
*
* Notes:
*
*/
open_dli_sock(dli_socket)
	register struct mom_sock_db *dli_socket;
{
	u_char out_opt[6];
	struct sockaddr_dl out_bind;

	if ((dli_socket->so = socket(AF_DLI, SOCK_DGRAM, DLPROTO_DLI)) < 0)
	{
		mop_syslog("mop_mom", "can't open socket to AF_DLI", 1, LOG_INFO);
		return(-1);
	}

	bzero(&out_bind, sizeof(out_bind));
	out_bind.dli_family = AF_DLI;
	out_bind.dli_substructype = DLI_ETHERNET;
	out_bind.dli_device.dli_devname[0] = dli_socket->devname[0];
	out_bind.dli_device.dli_devname[1] = dli_socket->devname[1];
	out_bind.dli_device.dli_devnumber = dli_socket->devunit;
	out_bind.choose_addr.dli_eaddr.dli_ioctlflg = DLI_DEFAULT;
	out_bind.choose_addr.dli_eaddr.dli_protype = dli_socket->pm->protype;
	out_bind.choose_addr.dli_eaddr.dli_target[0] = NULL;

	if ( bind(dli_socket->so, &out_bind, sizeof(out_bind)) < 0 )
	{
		mop_syslog("mop_mom", "bind failed", 1, LOG_INFO);
		close(dli_socket->so);
		dli_socket->so = -1;
		return(-1);
	}


	bcopy(dli_socket->pm->multicast, out_opt, sizeof(out_opt));
	if ( setsockopt(dli_socket->so, DLPROTO_DLI, DLI_MULTICAST, out_opt, sizeof(out_opt)) < 0 )
	{
		mop_syslog("mop_mom", "setsockopt failed", 1, LOG_INFO);
		close(dli_socket->so);
		dli_socket->so = -1;
		return(-1);
	}

	return(0);
}

/*
*		d u m p _ l o a d
*
*
* Version:	1.0 - 1/10/85
*
* Description:
*		This subroutine processes a dump/load request by opening
*		a socket to the requester and then by executing the
*		appropriate program.
*
* Inputs:
*		socket handle.
*		sockaddr_dl structure containing source info.
*		received message.
*		received message length.
*
* Outputs:
*		None.
*
* Notes:
*
*/
dump_load(sock, from, message, msg_len, envp)
register char **envp;
register struct sockaddr_dl *from;
register u_char *message;
int msg_len;
{
	FILE *fopen(), *fp;
	register int i, j;
	int so;
	int fromlen = sizeof(struct sockaddr_dl);
	struct sockaddr_dl *out_bind;
	u_char prog[sizeof(pm_table[DMPLD_INDEX].pathname) + sizeof(pm_table[DMPLD_INDEX].program)];
	u_char dst_addr[ASCII_EADDRSIZE];
	u_char hex_message[MOM_MSG_SIZE*2];

	/*
	 * open new socket and bind to target address using 
	 * "DLI_NORMAL" i/o control.
	 */
	if ((so = socket(AF_DLI, SOCK_DGRAM, DLPROTO_DLI)) == -1)
	{
		mop_syslog("mop_mom", "can't open socket to AF_DLI", 1, LOG_INFO);
		return(-1);
	}
	out_bind = from;
	out_bind->choose_addr.dli_eaddr.dli_ioctlflg = DLI_NORMAL;

	if ( bind(so, out_bind, sizeof(struct sockaddr_dl)) < 0 )
	{
		if ( errno != EADDRINUSE )
			mop_syslog("mop_mom", "bind failed", 1, LOG_INFO);
		close(so);
		return(-1);
	}

	/*
	 * fork child process
	 */
	if (!fork())
	{
		close(sock);
	}
	else
	{
		close(so);
		return(0);
	}

	/*
	 * Only child makes it to here. 
	 */

	/*
	 * make sure priority of child is lower than parent
	 */
	if ( setpriority(PRIO_PROCESS, getpid(), 1) < 0 )
	{
		mop_syslog("mop_mom", "child can't lower priority", 1, LOG_INFO);
	}
	/*
	 * translate message from binary to hex image
	 */
	for ( i = 0, j = 0; i < msg_len; i++, j += 2 )
	{
		sprintf(hex_message+j, "%2x", message[i]);
		if ( hex_message[j] ==  ' ' )
		{
			hex_message[j] = '0';
		}
	}


	/*
	 * translate destination address from binary to hex image
	 */
	translate_addr( dst_addr, from->choose_addr.dli_eaddr.dli_dest );

	/*
	 * exec dump/load program with sock tied to stdin and stdout.
	 */
	dup2(so, 0);
	dup2(so, 1);
	close(so);
	strcpy(prog, pm_table[DMPLD_INDEX].pathname);
	strcat(prog, pm_table[DMPLD_INDEX].program);
	execle(prog, pm_table[DMPLD_INDEX].program, hex_message, target_address, dst_addr, NULL, envp);
	/*
	 * If here, exec failed.
	 */
	sprintf(errbuf, "can't exec %s", prog);
	mop_syslog("mop_mom", errbuf, 1, LOG_INFO);
	exit(2);

}

/*
*		t r a n s l a t e _ a d d r
*
*
* Version:	1.0 - 1/10/85
*
* Description:
* 	Translate source address from binary to hex image
*
* Inputs:
*	baddr		= binary address to be translated.
*
* Outputs:
*	xaddr		= tranlated hex address.
*
* Notes:
*
*/
translate_addr( xaddr, baddr )
register u_char *xaddr;
register u_char *baddr;
{
	int i, j;
	for ( i = 0, j = 0; i < DLI_EADDRSIZE; i++, j += 3 )
	{
		sprintf(xaddr+j, "%2x-", baddr[i]);
		if ( xaddr[j] ==  ' ' )
		{
			xaddr[j] = '0';
		}
	}
	xaddr[ASCII_EADDRSIZE-2] = NULL;
	return;
}

/*
 *		f u n e r a l
 *
 * This is the signal handler for SIGCHLD - the exit of a child process.
 * The status is reaped, and the exit reported.
 *
 */
void funeral( sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
	int status;
	while ( wait3(&status, WNOHANG, NULL) > 0 ) ;
}

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
