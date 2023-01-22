#ifndef lint
static char *sccsid = "@(#)dliconn.c	1.3	ULTRIX	12/4/86";
#endif lint

/*
 * Program loadnode.c,  Module MOP 
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
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/errno.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdnet/dli_var.h>

#define	nm_MAXLINE	16
#define	nm_MAXDEV	4

/*
 *		d l i _ e c o n n
 *
 *
 * Version:	1.0 - 8/1/85
 *
 * Description:
 *	This subroutine opens a dli socket, binds an associated
 *	device name and protocol type 
 *
 * Inputs:
 *	devname		= ptr to device name 
 *	devunit		= device unit number 
 *	ptype		= protocol type
 *	taddr		= target address
 *	ioctl		= io control flag
 *
 *
 * Outputs:
 *	returns		= socket handle if success, otherwise NULL
 *
 * Notes:
 *
 */
dli_econn(devname, devunit, ptype, taddr, ioctl)
char *devname;
u_short devunit;
u_short ptype;
u_char *taddr;
u_char ioctl;
{
	int i, sock;
	struct sockaddr_dl out_bind;

	if ( (i = strlen(devname)) > sizeof(out_bind.dli_device.dli_devname) )
	{
		return(-1);
	}

	if ((sock = socket(AF_DLI, SOCK_DGRAM, DLPROTO_DLI)) < 0)
	{
		return(-1);
	}
	
	/*
	 * fill out bind structure
	 */
	bzero(&out_bind, sizeof(out_bind));
	out_bind.dli_family = AF_DLI;
	out_bind.dli_substructype = DLI_ETHERNET;
	bcopy(devname, out_bind.dli_device.dli_devname, i); 
	out_bind.dli_device.dli_devnumber = devunit;
	out_bind.choose_addr.dli_eaddr.dli_ioctlflg = ioctl;
	out_bind.choose_addr.dli_eaddr.dli_protype = ptype;
	if ( taddr )
	{
		bcopy(taddr, out_bind.choose_addr.dli_eaddr.dli_target, DLI_EADDRSIZE);
	}
	if ( bind(sock, &out_bind, sizeof(out_bind)) < 0 )
	{
		return(-1);
	}

	return(sock);
}

/*
 *				d n e t _ t o _ d e v
 *
 *  Converts NICE device name to UNIX device name (e.g., UNA-0 to de0 )
 *
 *  Input:  Buffer for UNIX device name and NICE device name character 
 *			string.
 *  Output:	
 *	returns		length of device name (excluding unit number)
 *				if translation successful with buffer 
 *				containing UNIX device 
 *			-1 if failure
 */
dnet_to_dev(dev_name, nice_name)
char dev_name[], nice_name[];

{
	extern char *nice_devtab[nm_MAXDEV][2];
	int i, length, hyphen;

	for (i = 0; i < nm_MAXLINE; i++)
		if (nice_name[i] == '-')
			break;

	hyphen = i;
	
	for (i = 0; i < nm_MAXDEV; i++)
	{
		if (strncmp(nice_name, nice_devtab[i][1], hyphen) == 0)
		{
			length = strlen(nice_devtab[i][0]);
			bcopy(nice_devtab[i][0], dev_name, length);
			strcpy(&dev_name[length], &nice_name[hyphen + 1]);

			return(length);
		}
	}
	return(-1);
}
