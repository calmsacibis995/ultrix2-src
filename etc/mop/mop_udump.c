#ifndef lint
static	char	*sccsid = "@(#)mop_udump.c	1.2	(ULTRIX)	10/21/85";
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

/*
*		m o p _ u p l i n e _ d u m p
*
*
* Description:
*	This subroutine attempts to upline dump form a requesting system.
*	When finished, it reboots the target via trigger.
*
* Inputs:
*	tnode			= Pointer to structure containing target node 
*				  and DECnet address.
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are stored.
*	fp			= Pointer to file descriptor of load module
*	dest			= Pointer to destination address used to
*				  address the local node.
*
*
* Outputs:
*
* Notes:
*
*/
mop_upline_dump( tnode, parm, fp, dest )
register struct node_id *tnode;
register struct mop_parse_dmpld *parm;
register FILE *fp;
register u_char *dest;
{
	int error = NULL;
	u_short multicast;
	struct nodeent *gn;
	u_char serpswd[9], *cp, *get_param();
	u_short psiz;
	void catch_sigalrm();

	signal(SIGALRM, catch_sigalrm);

	sscanf(dest, "%x", &multicast);
	if (multicast & 1)
	{
		mop_syslog("mop_dumpload", "sending volunteer assistance for memory dump service", 0, LOG_DEBUG);
		error = tx_volunteer_assist(parm);
	}
	if ( ! error )
	{
		mop_syslog("mop_dumpload", "performing upline dump", 0, LOG_DEBUG);
		dump_target_memory( parm, fp );

		/*
		 * fetch service password
		 */
		bzero(serpswd, sizeof(serpswd));
		if ( tnode->node_dnaddr )
		{
			gn = getnodebyaddr(&tnode->node_dnaddr, sizeof(tnode->node_dnaddr), AF_DECnet);
		}
		else
		{
			gn = getnodebyname(tnode->node_name);
		}
		if ( ! gn )
		{
			mop_syslog("mop_dumpload", "couldn't access database for service password", 0, LOG_INFO);
		}
		else
		{
			if ( (cp = get_param(gn->n_params, NODE_SERVICE_PSWD, &psiz)) )
			{
				bcopy(cp, serpswd, psiz);
			}
		}
		trigger_target(serpswd, (sizeof(serpswd)-1));
	}
	fclose(fp);
	return;
}

/*
*		d u m p _ t a r g e t _ m e m o r y 
*
* Version: 1.0 - 7/19/85
*
*
* Description:
*	This subroutine dumps the memory of a remote system.
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop dump request are stored.
*	fp			= Pointer to file descriptor of dump module
*
*
* Outputs:
*	returns			= 0 if success, -1 if failure.
*
* Notes:
*
*/
dump_target_memory( parm, fp )
struct mop_parse_dmpld *parm;
register FILE *fp;
{
	int siz;
	u_char *dump_image, *request_dump_image();
	register u_char done = 0;
	register u_int dump_addr = 0;
	register u_int max_dump_bytes = parm->mop_dump_size; 
	register u_short dump_size_per_msg = parm->mop_dl_bufsiz - MOP_MDIMAGE_IDX;


	/*
	 * Perform upline dump
	 */
	while ( ! done )
	{
		if ( (dump_addr + dump_size_per_msg) >= max_dump_bytes )
		{
			dump_size_per_msg = max_dump_bytes - dump_addr;
			done = 1;
		}

		if ( (dump_image = request_dump_image( dump_addr, dump_size_per_msg )) == 0 )
		{
			return(-1);
		}

		if ( (siz = write(fileno(fp), dump_image, dump_size_per_msg)) < (int) dump_size_per_msg )
		{
			if ( siz < 0 )
			{
				mop_syslog("mop_dumpload", "dump file write error", 1, LOG_INFO);
			}
			else
			{
				mop_syslog("mop_dumpload", "dump file write error", 0, LOG_INFO);
			}
			exit_status = EXIT_FWRITEFAIL;
			return(-1);
		}

		dump_addr += dump_size_per_msg;
	}
	issue_dump_complete();
	exit_status = EXIT_GOODDMP;
	return(0);
}

/*
*		r e q u e s t _ d u m p _ i m a g e
*
* Version: 1.0 - 7/19/85
*
*
* Description:
*	This subroutine requests a single dump image from a remote system.
*
* Inputs:
*	dump_addr		= beginning address of requested dump image
*	dump_size_per_msg	= size of expected dump image
*
*
* Outputs:
*	returns			= 0 if failure, 
*				  address of buffer containing dump image
*				  if success.
*
* Notes:
*
*/
u_char *request_dump_image( dump_addr, dump_size_per_msg )
u_int dump_addr;
u_short dump_size_per_msg;
{

	register int rsiz;
	int retry = MOP_RETRY;
	u_char in_msg[MOM_MSG_SIZE];
	u_char out_msg[MOM_MSG_SIZE];

	/*
	 * build request memory dump message 
	 */
	*(u_short *) &out_msg[MOP_MLDSIZ_IDX] = MOP_RMD_SIZ;
	out_msg[MOP_CODE_IDX] = MOP_RQST_MEMDMP;
	bcopy(&dump_addr, &out_msg[MOP_RMDADDR_IDX], sizeof(dump_addr));
	bcopy(&dump_size_per_msg, &out_msg[MOP_RMDCNT_IDX], sizeof(dump_size_per_msg));
	

	/*
	 * send request memory dump message and receive memory dump data
	 */
	while ( retry > 0 )
	{
		if ( write(STDOUT, out_msg, MOP_RMD_SIZ+2) < 0 )
		{
			mop_syslog("mop_dumpload", "dump request failed; can't transmit ", 1, LOG_INFO);
			exit_status = EXIT_SENDFAIL;
			return(0);
		}
		else if ((rsiz = read_mop(MOP_RTIMOUT, in_msg, sizeof(in_msg))) < 0 )
		{
			if ( errno == ETIMEDOUT )
			{
				mop_syslog("mop_dumpload", "memory dump timeout, retry attempted", 0, LOG_INFO);
				retry--;
			}
			else
			{
				mop_syslog("mop_dumpload", "memory dump failed, read error", 1, LOG_INFO);
				exit_status = EXIT_RCVFAIL;
				return(0);
			}
		}
		else if ( rsiz == 0 )
		{
			mop_syslog("mop_dumpload", "memory dump failed, read error", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(0);
		}
		else if ( rsiz < (dump_size_per_msg + MOP_MDIMAGE_IDX) )
		{
			mop_syslog("mop_dumpload", "memory dump failed, requested dump size not returned", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(0);
		}
		else if ( in_msg[MOP_CODE_IDX] != MOP_MEMDMP_DATA )
		{
			mop_syslog("mop_dumpload", "dump failed at destination, invalid memory dump data message code", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(0);
		}
		else if ( (*(u_int *) &in_msg[MOP_RMDADDR_IDX]) != dump_addr )
		{
			mop_syslog("mop_dumpload", "dump failed at destination, invalid address field in memory dump data message", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(0);
		}
		else
		{
			return(in_msg+MOP_MDIMAGE_IDX);
		}
	}
	mop_syslog("mop_dumpload", "memory dump failed", 0, LOG_INFO);
	exit_status = EXIT_RCVFAIL;
	return(0);
}

/*
*		i s s u e _ d u m p _ c o m p l e t e
*
* Version: 1.0 - 7/19/85
*
*
* Description:
*	This subroutine notifies the target node that the dump has been
*	completed.
*
* Inputs:
*
*
* Outputs:
*
* Notes:
*
*/
issue_dump_complete()
{

	u_char out_msg[MOM_MSG_SIZE];

	/*
	 * build dump complete message 
	 */
	*(u_short *) &out_msg[MOP_MLDSIZ_IDX] = 1;
	out_msg[MOP_CODE_IDX] = MOP_DUMP_DONE;

	/*
	 * send dump complete message 
	 */
	if ( write(STDOUT, out_msg, 3) < 0 )
	{
		mop_syslog("mop_dumpload", "dump complete failed; can't transmit ", 1, LOG_INFO);
		exit_status = EXIT_SENDFAIL;
		return(-1);
	}

	return(0);
}

/*
 *		t r i g g e r _ t a r g e t
 *
 * This subroutine builds a boot message for a trigger and transmits it.
 *
 *
 * Inputs:
 *	
 *	
 * Outputs:
 */
trigger_target(serpasswd, serpasswd_len)
u_char *serpasswd;
u_short serpasswd_len;
{
	u_char msg[50];
	int i = MOP_BTCTL_IDX + 1;

	bzero(msg, sizeof(msg));
	msg[MOP_CODE_IDX] = MOP_BOOT_MSG;
	bcopy(serpasswd, &msg[MOP_BTVER_IDX], serpasswd_len);
	msg[MOP_BTPRC_IDX] = MOP_SYS_PROC;
	msg[MOP_BTCTL_IDX] = MOP_TRIGCTL;
	msg[i++] = MOP_SWID_SOS;
	*(u_short *) msg = i - 2;
	if( write(STDOUT, msg, i) < 0 )
	{
		mop_syslog("mop_dumpload", "trigger failed; can't transmit ", 1, LOG_INFO);
		exit_status = EXIT_SENDFAIL;
	}
	return;
}
