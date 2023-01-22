#ifndef lint
static char *sccsid = "@(#)ccr_netwio.c	1.2	ULTRIX	10/3/86";
#endif lint

/*
 * MODULE CCR_NETWIO.C, handles all network related i/o
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
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include "/sys/net/if.h"
#include "/sys/netinet/in.h"
#include "/sys/netinet/if_ether.h"
#include <netdnet/dli_var.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"
#include "mop_proto.h"
#include "nmgtdb.h"
#include "mopdb.h"


u_char devid[16];

extern int errno;
extern struct load_parameters *ptr_params;

/*
*		d l i _ o p e n  
*
*	This routine opens a DLI socket.  
*
* Inputs:
*		sock	pointer to address where socket number is to be
*				stored.
*
* Outputs:
*		SUCCESS if socket is opened successfully
*		else FAIL.
*
*/
dli_open(sock)
int		*sock;
	  
{
	u_char devname[16];
	u_short devunit, i;

	/*
	 *  Fetch device to use
	 */
	if (nice_to_dev(devid, ptr_params->db_parm.node_parm.rnm_sercirc))
	{
		i = 0;
		while( devid[i] != NULL && isalpha(devid[i]) ) i++;
			strncpy(devname, devid, i);
		if ( sscanf(devid+i, "%hd", &devunit) == EOF )
		{
			fprintf(stderr,"ccr: invalid circuit-id, %s\n",ptr_params->db_parm.node_parm.rnm_sercirc);
			return(FAIL);
		}
	}
	else
	{
		fprintf(stderr,"ccr: invalid circuit-id, %s\n",ptr_params->db_parm.node_parm.rnm_sercirc);
		return(FAIL);
	}


	/*
	 * Open socket 
	 */
	if ((*sock = dli_conn(devname, devunit, ptr_params->db_parm.node_parm.rnm_hwaddr)) == NULL)
	{
		return(FAIL);
	}
	else
		return(SUCCESS);

}

/*
*		d l i _ c o n n
*
*	This routine opens a DLI socket and establishes connection to 
*	remote system.  
*
* Inputs:
*	- devname: name of device to be used.
*	- devunit: unit number of device to be used.
*	- target: destination address of target node to be used.
*
* Outputs:
*	- socket handle or NULL (error) returned
*
*/
dli_conn(devname, devunit, target_addr )
u_char *devname;
u_short devunit;
char *target_addr;
{
	int local_sock;
	struct sockaddr_dl out_bind;
	u_char ioctl;


	/*
	 * Open DLI socket.
	 */

	if ( (local_sock = socket(AF_DLI, SOCK_DGRAM, DLPROTO_DLI)) == -1 )
		return(NULL);


	/*
	 * Set up DLI bind
	 */
	
	ioctl = DLI_NORMAL;
	init_sockaddr(&out_bind, ioctl, devname, devunit, target_addr);


	/*
	 * Now bind to DLI socket
	 */
	if ( bind(local_sock, &out_bind, sizeof(struct sockaddr_dl)) < 0 )
	{
		perror("ccr");
		return(NULL);
	}

	/*
	 * If at this point, everything worked fine - return socket handle.
	 */
	return(local_sock);

}

/*
*			i n i t _ s o c k a d d r
*
*	This routine initializes a sockaddr_dl structure.
*
* Inputs:
*	- bind_data: pointer to sockaddr_dl structure to be initialized.
*	- ioctl: i/o control flag to be used.
*	- devname: name of device to be used.
*	- devunit: unit number of device to be used.
*	- target: destination address of target node to be used.
*
* Outputs:
*	None.
*
*/
init_sockaddr( bind_data, ioctl, devname, devunit, target )
register struct sockaddr_dl *bind_data;
u_char ioctl;
u_char *devname;
u_short devunit;
char *target;
{
	/*
	 * Set up DLI bind
	 *	1. clear out and load constant data.
	 *	2. load device to be used.
	 *	3. load target address.
	 */
	bzero(bind_data, sizeof(struct sockaddr_dl));
	bind_data->dli_family = AF_DLI;
	bind_data->dli_substructype = DLI_ETHERNET;
	bind_data->choose_addr.dli_eaddr.dli_protype = PROTO_REM_CONS;
	bind_data->choose_addr.dli_eaddr.dli_ioctlflg = ioctl;
	bind_data->dli_device.dli_devnumber = devunit;
	bcopy(devname, bind_data->dli_device.dli_devname, 2);
	if ( target != NULL )
		bcopy(target, bind_data->choose_addr.dli_eaddr.dli_target, DLI_EADDRSIZE);

	return;
}

/*
*		 d l i _ x m t r e c
*
* Description:
*		This routine transmits a message and then waits for a receive.
*
* Inputs:
*	sock		the socket handle
*	send_msg	the address of the send message buffer
*	send_len	the length of the message to be sent
*	recv_msg	the address of the receive message buffer
*
* Outputs:
*	recv_msg	contains the received message
*	SUCCESS 	if message sent and received successfully 
*	else FAIL
*
*/
dli_xmtrec(sock,send_msg,send_len,recv_msg)
int sock, send_len;
u_char *send_msg, *recv_msg;
{

	int retry,
		readfd;
	struct timeval tv;

	bzero(&tv,sizeof(struct timeval));
	retry = MOP_RETRY;
	while(retry--)
	{
		/*
	 	 * Transmit the message 
	 	 */

		if ( write(sock,send_msg,send_len) < 0)
		{
			perror("ccr");
			return(FAIL);
		}

		tv.tv_sec = CCR_RECV_TIMOUT;
		tv.tv_usec = 0;
		readfd = (1 << sock);	

		/* 
  	 	 * Wait for a message to be returned
 	 	 */
		if (select(sock+1,&readfd,0,0,&tv) == 0)
			errno = ETIMEDOUT;
		else
		{
			if(read(sock,recv_msg,MAXL_RECV_BUF) < 0)
			{
				perror("ccr");
				return(FAIL);
			}
			else
				return(SUCCESS);
		}
	}
	/*
	 * Read timedout on all retries
	 */
	perror("ccr");
	return(FAIL);
}

/*
*		d l i _ t r a n s m i t
*
*		This routine transmits a message.
*
* Inputs:
*	sock		the socket handle
*	msgbuf	the address of the message buffer
*	msglen	the length of the message
*
* Outputs:
*	SUCCESS if message sent without a problem
*	else FAIL
*
*/
dli_transmit(sock,msgbuf,msglen)
int sock, msglen;
u_char *msgbuf;
{

	/*
	 * Transmit the message 
	 */

	if ( write(sock,msgbuf,msglen) < 0)
	{
		perror("ccr");
		return(FAIL);
	}
	else
		return(SUCCESS);
}
