#ifndef lint
static char *sccsid = "@(#)mop_boot.c	1.4	ULTRIX	12/4/86";
#endif lint

/*
 * Program mop_boot.c,  Module MOP 
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
#include <sys/time.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdnet/dli_var.h>

#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_proto.h"
#include "mop_var.h"
#include <netdnet/node_params.h>

#define STDIN	0		/* used by mop for network access */
#define STDOUT	1		/* used by mop for network access */

extern int errno;
extern int exit_status;
static u_char dl_mcast[] = { MCAST_DUMP_LOAD };

/*
 *		s e n d _ b o o t
 *
 * This subroutine sends the actual boot message and then 
 * exits.  
 *
 *
 * Inputs:
 *	sock		= dli socket handle
 *	addr		= target address
 *	devname		= deivce name
 *	devunit		= device unit number
 *	msg		= boot msg
 *	msg_siz		= siz of boot message
 *	
 *	
 * Outputs:
 *	returns		= 0 if success, -1 if failure.
 *	exit_statu	= written into only if error.
 */
send_boot( sock, addr, devname, devunit, msg, msg_siz )
int sock;
u_short devunit;
u_char *addr, *devname, *msg;
int msg_siz;
{
	int i;
	struct sockaddr_dl rcdst;

	if ( ! addr )
	{
		exit_status = EXIT_SENDFAIL;
		return(-1);
	}

	if ( (i = strlen(devname)) > sizeof(rcdst.dli_device.dli_devname) )
	{
		exit_status = EXIT_BADEVNAME;
		return(-1);
	}
	/*
	 * initialize destination structure to boot target
	 */
	bzero(&rcdst, sizeof(rcdst));
	rcdst.dli_family = AF_DLI;
	rcdst.dli_substructype = DLI_ETHERNET;
	bcopy(devname, rcdst.dli_device.dli_devname, i);
	rcdst.dli_device.dli_devnumber = devunit;
	rcdst.choose_addr.dli_eaddr.dli_protype = PROTO_REM_CONS;
	bcopy(addr, rcdst.choose_addr.dli_eaddr.dli_target, DLI_EADDRSIZE);


	/*
	 * send boot message.
	 */
	if ( sendto(sock, msg, msg_siz, NULL, &rcdst, sizeof(rcdst)) < 0 )
	{
		exit_status = EXIT_SENDFAIL;
		return(-1);
	}

	return(0);
}

/*
 *		s e n d _ l o a d
 *
 * This subroutine attempts to start a force load.  
 * 3 attempts are made using the hardware address 
 * and, if present, the DECnet address as the target address.
 *
 *
 * Inputs:
 *	phaddr		= target hardware address
 *	dnaddr		= target DECnet physical address
 *	devname		= device name
 *	decunit		= device unit number
 *	out_msg		= boot msg to be transmitted
 *	out_msiz	= size of output buffer to be transmitted
 *	in_msg		= input buffer to contain program request
 *	in_msiz		= size of input buffer
 *	target		= pointer to flag indicating which address
 *			  succeeded in addressing target.
 *			  (1 = DECnet address, 0 = hardware address)
 *	
 *	
 * Outputs:
 *	returns		= number of bytes received if success, -1 if failure.
 *	exit_status	= written into only if error occurred.
 *	target		= pointer to flag indicating which address
 *			  succeeded in addressing target.
 *			  (1 = DECnet address, 0 = hardware address)
 */
#define no_eintr(x) while ((x) == -1 && errno == EINTR)
send_load( phaddr, dnaddr, devname, devunit, out_msg, out_msiz, in_msg, in_msiz, target )
u_short devunit;
u_short in_msiz, out_msiz;
u_char *phaddr, *dnaddr, *devname, *in_msg, *out_msg;
u_short *target;
{
	u_char physaddr_open = 0;
	int i, j, rsiz, dnaddr_so = 0, phaddr_so = 0, rdfds_master = 0, rdfds,  nfound, so, num_so;
	u_short nmbr_of_addr;
	struct timeval tv;

	bzero(&tv,sizeof(struct timeval));

	/*
	 * Send boot message(s).  Make 3 attempts using the hardware address 
	 * and, if present, the DECnet address as the target address(es).
	 */
	if ( (phaddr_so = dll_open(phaddr, devname, devunit)) < 0 )
	{
		return(-1);
	}
	num_so = phaddr_so + 1;
	rdfds_master |= 1 << phaddr_so;
	if ( dnaddr )
	{
		if ( (dnaddr_so = dll_open(dnaddr, devname, devunit)) < 0 )
		{
			return(-1);
		}
		if ( dnaddr_so > phaddr_so )
			num_so = dnaddr_so + 1;
		rdfds_master |= 1 << dnaddr_so;
	}

	for ( i = 0; i < 3; i++ )
	{
		/* send boot messages */
		if ( send_boot(phaddr_so, phaddr, devname, devunit, out_msg, out_msiz) < 0 )
		{
			return(-1);
		}
		if ( dnaddr && send_boot(dnaddr_so, dnaddr, devname, devunit, out_msg, out_msiz) < 0 )
		{
			return(-1);
		}

		/* wait for reply */
		tv.tv_sec = MOP_BTIMOUT;
		tv.tv_usec = 0;
		rdfds = rdfds_master;
		no_eintr(nfound = select(num_so, &rdfds, NULL, NULL, &tv ));
		if ( nfound == 0 )
		{
			continue;
		}
		if ( nfound < 0 )
		{
			exit_status = EXIT_RCVFAIL;
			return(-1);
		}

		/* which address answered ? */
		if ( rdfds & (1 << phaddr_so) )
		{
			so = phaddr_so;
			close(dnaddr_so);
		}
		else if ( rdfds & (1 << dnaddr_so) )
		{
			so = dnaddr_so;
			close(phaddr_so);
		}
		else
		{
			exit_status = EXIT_RCVFAIL;
			return(-1);
		}

		/* get the message and tie socket handle to standard in and standard out */
		if ( (rsiz = read(so, in_msg, in_msiz)) <= 0 )
		{
			exit_status = EXIT_RCVFAIL;
			return(-1);
		}
		if ( dup2(so, STDIN) < 0 )
		{
			close(so);
			exit_status = EXIT_CANTDUP;
			return(-1);
		}
		close(so);
		if ( dup2(STDIN, STDOUT) < 0 )
		{
			exit_status = EXIT_CANTDUP;
			return(-1);
		}
		return(rsiz);
	}
	exit_status = EXIT_RCVFAIL;
	return(-1);
}

/*
 *		d l l _ o p e n
 *
 * This subroutine opens a socket to listen for downline load requests from
 * a particular node.  The DUMP/LOAD Assistance Multicast Address is also
 * enabled.
 *
 *
 * Inputs:
 *	addr		= target address
 *	devname		= deivce name
 *	devunit		= device unit number
 *	
 *	
 * Outputs:
 *	returns		= 0 if success, -1 if failure.
 *	exit_status	= written into only if error occurred.
 */
dll_open(addr, devname, devunit) 
char *addr, *devname;
u_short devunit;
{
	int so;

	if ( (so = dli_econn(devname, devunit, PROTO_DUMP_LOAD, addr, DLI_NORMAL)) < 0 )
	{
		exit_status = EXIT_DLICONFAIL; 
		return(-1);
	}
	/*
	 * set up dump load multicast
	 */
	if ( setsockopt(so, DLPROTO_DLI, DLI_MULTICAST, dl_mcast, sizeof(dl_mcast)) < 0 )
	{
		exit_status = EXIT_DLICONFAIL;
		return(-1);
	}
	return(so);
}

/*
 *		b u i l d _ b o o t _ m s g
 *
 * This subroutine builds a boot message for either a trigger or a
 * load.
 *
 *
 * Inputs:
 *	msg		= ptr to where message is to be placed.
 *	lp		= ptr to structure containing load parameters.
 *	boot_type	= TRIGGER_NODE or LOAD_NODE.
 *	processor	= target processor type.
 *	
 *	
 * Outputs:
 *	returns		= size of boot message is returned. 
 *	msg		= where message was placed.
 */
build_boot_msg( msg, lp, boot_type, processor )
u_char *msg;
register struct load_parameters *lp;
u_char boot_type, processor;
{
	int j;
	int i = MOP_BTCTL_IDX + 1;

	bzero(msg, sizeof(msg));
	msg[MOP_CODE_IDX] = MOP_BOOT_MSG;
	bcopy(lp->db_parm.node_parm.rnm_serpasswd, &msg[MOP_BTVER_IDX], sizeof(lp->db_parm.node_parm.rnm_serpasswd));
	msg[MOP_BTPRC_IDX] = processor;
	if ( boot_type == TRIGGER_NODE )
	{
		msg[MOP_BTCTL_IDX] = MOP_TRIGCTL;
	}
	else
	{
		msg[MOP_BTCTL_IDX] = MOP_LOADCTL;
	}
	if ( lp->parm_present & LPAR_DGFILE ) 
	{
		msg[i++] = MOP_SWID_MS;
	}
	else
	{
		msg[i++] = MOP_SWID_SOS;
	}
	*(u_short *) msg = i - 2;
	return(i);
}
