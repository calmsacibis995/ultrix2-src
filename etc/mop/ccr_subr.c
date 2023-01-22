#ifndef lint
static	char	*sccsid = "@(#)ccr_subr.c	1.2	(ULTRIX)	12/4/86";
#endif lint

/*
 * MODULE - CCR_SUBR.C, contains miscellaneous subroutines
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

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"



char *nice_devtab[MAXDEV][2] =			/*  Device table - UNIX and NICE */
{
	{ "de",	"UNA"} ,
	{ "qe",	"QNA"} ,
	{ "ni", "BNT"} ,
	{ "se", "SVA"}
};

/*
 *				n i c e _ t o _ d e v
 *
 *  Converts NICE device name to UNIX device name (e.g., UNA-0 to de0 )
 *
 *  Input:  Buffer for UNIX device name and NICE device name character 
 *			string.
 *  Output:	SUCCESS if translation successful with buffer containing
 *				UNIX device name, else
 *			FAIL
 *  Note:  	Assumption is being made that the NICE device name is in
 *			uppercase.
 *		
 */


nice_to_dev(dev_name, nice_name)
char dev_name[];
u_char	nice_name[];

{

	int i, j, length, hyphen;

	for (i = 0; i < MXBYT_SERCIR; i++)
		if (nice_name[i] == '-')
			break;

	hyphen = i;


	/* Convert to UNIX device name */
	
	for (i = 0; i < MAXDEV; i++)
	{
		if (strncmp(nice_name, nice_devtab[i][1], hyphen) == 0)
		{
			length = strlen(nice_devtab[i][0]);
			bcopy(nice_devtab[i][0], dev_name, length);
			strcpy(&dev_name[length], &nice_name[hyphen + 1]);

			return(SUCCESS);
		}
	}
	return(FAIL);
}

/*
 *				g e t _ p h y s a d d r
 *
 *  Makes an ioctl to the device to find hardware address (Ethernet)
 *  and physical address (DECnet address for Ethernet)
 *
 *  Input:  Interface device structure
 *
 *  Output: SUCESS if successful call to device with addresses
 *			returned in interface device structure, else
 *          FAIL
 */

get_physaddr(dev_addr)
struct ifdevea *dev_addr;
{

	int socka,
		status;

    if ((socka = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
		return(FAIL);

   	if (ioctl(socka, SIOCRPHYSADDR, (caddr_t)dev_addr) == -1)
		status = FAIL;
	else
		status = SUCCESS;

    close(socka);
	return(status);
}

