#ifndef lint
static	char	*sccsid = "@(#)print_loadparm.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * Program mop_forceload.c,  Module MOP 
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
#include <sys/wait.h>
#include <sys/resource.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdnet/dli_var.h>
#include <syslog.h>

#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_var.h"
#include "mop_proto.h"
#include <netdnet/node_params.h>


#define STDIN	0		/* used by MOP for network access */
#define STDOUT	1		/* used by MOP for network access */

extern int errno;
int exit_status = EXIT_GOOD;
u_short child_is_dead;
char errbuf[4096];
char *target_address = NULL;

/*
 *		p r i n t _ l o a d _ p a r m
 *
 *	This routine prints out some members in a load parameter
 *	structure.
 *
 *
 * Inputs:
 *	load_parm	= ptr to structure containing load parameters.
 *	
 *	
 * Outputs:
 */
print_load_parm( parm )
register struct load_parameters *parm;
{

	int i;

	fprintf(stderr, "option = %d\n", parm->option);
	fprintf(stderr, "load_node_name = %s\n", parm->load_node_name);
	fprintf(stderr, "load_node_addr = %d\n", parm->load_node_addr);
	fprintf(stderr, "load_node_ckt = %s\n", parm->load_node_ckt);
	fprintf(stderr, "parm_present = %x\n", parm->parm_present);
	fprintf(stderr, "parm_delete = %x\n", parm->parm_delete);
	fprintf(stderr, "node_name = %s\n", parm->db_parm.node_name);
	fprintf(stderr, "node_addr = %x\n", parm->db_parm.node_addr);
	fprintf(stderr, "node_physaddr = %x ", parm->db_parm.node_physaddr[0]);
	for ( i = 1; i < 5; i++ )
	{
		fprintf(stderr, "%x ", parm->db_parm.node_physaddr[i]);
	}
	fprintf(stderr, "%x\n", parm->db_parm.node_physaddr[5]);
	fprintf(stderr, "rnm_sercirc = %s\n", parm->db_parm.node_parm.rnm_sercirc);
	fprintf(stderr, "rnm_serpasswd = %x ", (parm->db_parm.node_parm.rnm_serpasswd[0] & 0xff));
	for ( i = 1; i < 8; i++ )
	{
		fprintf(stderr, "%x ", (parm->db_parm.node_parm.rnm_serpasswd[i] & 0xff));
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "rnm_serdev = %d\n", parm->db_parm.node_parm.rnm_serdev);
	fprintf(stderr, "rnm_cpu = %d\n", parm->db_parm.node_parm.rnm_cpu);
	fprintf(stderr, "rnm_hwaddr = %x ", parm->db_parm.node_parm.rnm_hwaddr[0]);
	for ( i = 1; i < 5; i++ )
	{
		fprintf(stderr, "%x ", parm->db_parm.node_parm.rnm_hwaddr[i]);
	}
	fprintf(stderr, "%x\n", parm->db_parm.node_parm.rnm_hwaddr[5]);

	fprintf(stderr, "rnm_sernodever = %d\n", parm->db_parm.node_parm.rnm_sernodever);
	fprintf(stderr, "rnm_loadfile = %s\n", parm->db_parm.node_parm.rnm_loadfile);
	fprintf(stderr, "rnm_secload = %s\n", parm->db_parm.node_parm.rnm_secload);
	fprintf(stderr, "rnm_terload = %s\n", parm->db_parm.node_parm.rnm_terload);
	fprintf(stderr, "rnm_diagfile = %s\n", parm->db_parm.node_parm.rnm_diagfile);
	fprintf(stderr, "rnm_swtype = %d\n", parm->db_parm.node_parm.rnm_swtype);
	fprintf(stderr, "rnm_swident = %s\n", parm->db_parm.node_parm.rnm_swident);
	fprintf(stderr, "rnm_dumpfile = %s\n", parm->db_parm.node_parm.rnm_dumpfile);
	fprintf(stderr, "rnm_host.n_addr = %x\n", parm->db_parm.node_parm.rnm_host.n_addr);
	fprintf(stderr, "rnm_host.n_name = %s\n", parm->db_parm.node_parm.rnm_host.n_name);


	return;
}
