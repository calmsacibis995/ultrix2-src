#ifndef lint
static	char	*sccsid = "@(#)ccr_reserve.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * MODULE CCR_RESERVE.C, reserve the remote console
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
#include <sys/types.h>
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
#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_proto.h"
#include "mop_var.h"


u_char 	recv_buffer[MAXL_RECV_BUF];
int		recv_size;

u_char ccr_reqid_msg[] = { CCR_REQID_LEN,0,MOP_CCR_REQID,0,0,0 };
u_char ccr_resrv_cons[] = { CCR_RESCON_LEN,0,MOP_CCR_RESCON,0,0,0,0,0,0,0,0 };

extern u_char devid[];
extern struct sys_id_info sysid_values;
extern int load_node();
extern int dli_xmtrec();
extern int get_physaddr();

/*
*	r e s e r v e _ c o n s o l e	
*
*	This routine reserves the remote console.
*
* Inputs:
*		ptr_params	pointer to structure containing parameters needed
*					to perform load of console carrier server.
*		argv	pointer to the array of arguments that were part
*				of command line
*		envp	pointer to an array of environment variables.
*
* Outputs:
*		SUCCESS	if remote console reserved
*		else FAIL.
*
*/

reserve_console(sock,ptr_params, argv, envp)
int sock;
struct load_parameters *ptr_params;
char *argv,
	 *envp[];
	  
{

	int status,
		retry = 6;
	u_char first_flag = 1;		/* Used to indicate calling routine for
								   the first time */


	/*
	 * Check to see if remote console is reserved.
	 */
	status = check_reserved(sock,first_flag,ptr_params,argv,envp);
	first_flag = 0;

	switch (status) {

		case(ccr_NOTRESERV):
			/*
			 * Expected response under normal conditions 
			 */
			break;
	
		case(ccr_REMCONINUSE):
			print_message(ccr_REMCONINUSE);
			return(FAIL);
			break;

		case(FAIL):
			/*
			 * Error happened during the load of CCS 
			 */
			return(FAIL);
			break;

		case(ccr_REMCONRES):
		defualt:
			/* 
			 * The remote console should never be reserved by us 
			 * the first time through check_reserved().
			 */
			fprintf(stderr,"ccr: internal error in reserve_console()\n");
			return(FAIL);
			break;
	}


	/* 
	 * Transmit a reserve console message. Try a few times, if 
	 * still unsuccessful, then fail
	 */

	if (ptr_params->parm_present & LPAR_SPSWD)
		bcopy(ptr_params->db_parm.node_parm.rnm_serpasswd,ccr_resrv_cons+RESCON_PASSWD_IDX,sizeof(ptr_params->db_parm.node_parm.rnm_serpasswd));


	while(--retry != 0)
	{	
		if (status = dli_transmit(sock,ccr_resrv_cons,sizeof(ccr_resrv_cons)))
		{
			status = check_reserved(sock,first_flag,ptr_params,argv,envp);
			switch (status) {

				case(ccr_NOTRESERV):
					break;

				case(ccr_REMCONRES):
					print_message(ccr_REMCONRES);
					return(SUCCESS);
					break;

				case(ccr_REMCONINUSE):
					print_message(ccr_REMCONINUSE);
				case(FAIL):
				default:
					return(FAIL);
					break;
			}
		}
		else
			return(FAIL);	
	}
	print_message(ccr_NOTRESERV);
	return(FAIL);

}

/*
*		c h e c k _ r e s e r v e d	
*
* Description:
*	This routine transmits a request id message and parses the
* 	received message. Then checks the status of the remote console.
*
* Inputs:
*	sock		the socket handle
*	first_flag 	flag to indicate whether this routine is being
*				called for the first time.
*	ptr_params	pointer to structure containing parameters needed
*				to perform load of console carrier server.
*	argv		pointer to the array of arguments that were part
*				of command line
*	envp		pointer to an array of environment variables.
*
* Outputs:
*	status		indicating what state the remote console is in.
*
*/
check_reserved(sock,first_flag,ptr_params,argv,envp)
int sock;
u_char first_flag;
struct load_parameters *ptr_params;
char *argv,
	 *envp[];
	  
{

	int status;

	/*
	 * Transmit request id message
	 */

	if (status = dli_xmtrec(sock,ccr_reqid_msg,sizeof(ccr_reqid_msg),recv_buffer))
	{
		recv_size = *(u_short *)recv_buffer;
		/* 
		 * Parse returned system id message
		 */
		if(status = parse_sys_id(recv_buffer,recv_size))
		{
			/*
			 * Is console carrier already loaded ? 
			 */
			if(sysid_values.functions & FUNC_CONS_CARR)
			{
				/*
				 * Is console carrier already reserved ? 
				 */
				if(sysid_values.functions & FUNC_CC_RESERVE)
				{
					/*
					 * Is console reserved by us ? 
					 */	
					if ((status = reserved_by_us()) == SUCCESS)
					{
						/* 
						 * Yes.
						 * Is it the first time this routine has been
						 * called?
						 */
						if(first_flag)
						{
							/* 
							 * Yes, then remote console in funny
							 * state. Release the remote console and
							 * continue
							 */
							release_console();
							status = ccr_NOTRESERV;
						}
						else	
							status = ccr_REMCONRES;
					}
					/*
					 * There is no else statement here, because
					 * the routine reserved_by_us() returns the
					 * appropriate status information to be returned
					 * back to the calling routine.
					 */
				}
				else
				/*
				 * No, set return code to indicate not reserved
				 */
					status = ccr_NOTRESERV;
			}
			/*
			 * No call routine to handle load 
			 */
			else		
			{
				if((status = load_node(ptr_params,argv,envp)) != EXIT_GOODSYS)
				{
					print_load_error(status);
					status = FAIL;
				}
				/*
				 * If load succeeds, set status code to indicate the
				 * remote console is not reserved.
				 */
				else
					status = ccr_NOTRESERV;
			}
		}
	}
	return(status);

}

/*
*		r e s e r v e d	_ b y _ u s
*
*	Called to see if the remote console is reserved by us.
*
* Inputs:
*		None.
*
* Outputs:
*		SUCCESS, if remote console reserved by us
*		status code = ccr_REMCONINUSE, if not reserved by us
*		FAIL, if can't obtain the physical address of our local system.
*
* Notes:
*
*/
int reserved_by_us()
{
	struct sys_id_info *ptr_sysid;
	struct ifdevea ifdevea, *dev_addr;
	u_char empty[6];

	ptr_sysid = &sysid_values; 
	dev_addr = &ifdevea;
	bzero(empty,sizeof(empty));
	bzero(dev_addr,sizeof(*dev_addr));

	/* 
	 * If console user is all zeros, the console is not
	 * reserved yet, return
	 */
	if (!(bcmp(empty,ptr_sysid->console_user,sizeof(empty))))
		return(ccr_NOTRESERV);

	strcpy(dev_addr->ifr_name,devid);
	
	if (get_physaddr(dev_addr))
	{
		/*
		 * Is the remote console reserved by us ?
		 */
		if (!(bcmp(dev_addr->current_pa,ptr_sysid->console_user,sizeof(sysid_values.console_user))))	
			/*
			 * Yes, return success to indicate that it is
			 */
			return(SUCCESS);
		else
			/*
			 * No, indicate remote console already in use
			 */
			return(ccr_REMCONINUSE);
	}
	else
	{
		print_message(ccr_NOWAY);
		return(FAIL);
	}
}
