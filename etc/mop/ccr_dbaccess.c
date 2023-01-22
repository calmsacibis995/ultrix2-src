#ifndef lint
static	char	*sccsid = "@(#)ccr_dbaccess.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * MODULE - CCR_DBACCESS.C, accesses database for necessary parameters
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include <netdnet/node_params.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"
#include "nmgtdb.h"
#include "mopdb.h"

extern struct dn_naddr	*ccr_node_addr;

/*
 *   	g e t _ d b p a r a m s
 *
 *	Access the volatile database to obtain parameters necessary
 *	for performing down line load of remote console server.
 *
 * Inputs:
 *		ptr_params pointer to the structure containing all the
 *			parameters needed for MOP messages. (parameters used for
 *			down-line load and DLI).
 *
 * Outputs:
 *		FAIL if can't find entry in database or if no hardware address
 *			can be found. 
 *		hardware address if not specified in command line and if found in
 *			the volatile database.
 *		service password and service circuit if not specified in command 
 *			line and if found in the volatile database.
 *		SUCCESS if completes examination of the database successfully.
 *
 * Notes:
 *		This routine is searching through the volatile nodes' database.
 */

int get_dbparams(ptr_params)
struct load_parameters *ptr_params;
{

	struct nodeent *ccr_node_entry;
	u_short psize;
	u_char *get_param();
	register u_char *cp;

	/*
 	 * Get the node entry from the volatile database based on the node id
	 * specified in the command line. 
	 */	

	if ( ptr_params->load_node_name[0] != NULL)
		ccr_node_entry = getnodebyname(ptr_params->load_node_name);
	else
		ccr_node_entry = getnodebyaddr(ccr_node_addr->a_addr,ccr_node_addr->a_len, AF_DECnet);

	/*
	 * If a entry found in the database then, get default values for
	 * Service Circuit, Service Password, and Hardware Address, if they
	 * are not specified on the command line. The Hardware Address must
	 * be specified in either the volatile database or the commmand line.
	 * Otherwise, fatal error.
	 */

	if (!(ccr_node_entry))
	{
		print_message(ccr_NONODE);	
		return(FAIL);
	}
	else
	{
		/* 
		 * Check for HARDWARE ADDRESS 
		 */

		if (!(ptr_params->parm_present & LPAR_HWADDR))
		{
			if ((cp = get_param(ccr_node_entry->n_params,NODE_HARDWARE_ADDR,&psize)) == NULL)
			{
				print_message(ccr_HARDADDR);	
				return(FAIL);
			}
			else
			{
				bcopy(cp,ptr_params->db_parm.node_parm.rnm_hwaddr,psize);
				ptr_params->parm_present |= LPAR_HWADDR;
			}
		}



		/*
		 * Check for SERVICE PASSWORD
		 */

		if (!(ptr_params->parm_present & LPAR_SPSWD))
		{
			if (!((cp = get_param(ccr_node_entry->n_params,NODE_SERVICE_PSWD,&psize)) == NULL))
			{
				bcopy(cp,ptr_params->db_parm.node_parm.rnm_serpasswd,psize);
				ptr_params->parm_present |= LPAR_SPSWD;
			}
		}



		/*
		 * Check for SERVICE CIRCUIT
		 */

		if (!(ptr_params->parm_present & LPAR_SCKT))
		{
			if (!((cp = get_param(ccr_node_entry->n_params,NODE_SERVICE_CKT,&psize)) == NULL))
			{
				bcopy(cp,ptr_params->db_parm.node_parm.rnm_sercirc,psize);
				ptr_params->parm_present |= LPAR_SCKT;
			}
		}
		return(SUCCESS);
	}

}

