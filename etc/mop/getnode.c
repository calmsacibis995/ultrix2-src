#ifndef lint
static	char	*sccsid = "@(#)getnode.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * Program getnode.c,  Module NODEDB 
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
 *    getnode		get a node entry from the nodes data base.
 *    v1.00		April 10, 1985
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/errno.h>

#include <netdnet/node_params.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>
#include "nmgtdb.h"
#include "mopdb.h"

extern int errno;




static char GETNODE[] = "@(#)getnode.c	1.3		9/9/85";

/*
 *		g e t n o d e
 *
 * Get an entry from the nodes data base.
 *
 *
 * Inputs:
 *	Arguments can be node names, DECnet addresses or -P.  If no arguments,
 *	the entire data base is displayed.  If -P is among the arguments,
 *	the permanent data base is displayed.
 *	
 */
main(argc, argv)
int argc;
char **argv;
{
	struct dn_naddr *address;
	int i;
	struct nodeent *get_node;
	char node[10], errmsg[256];
	struct sigvec ctlz;

	/*
	 * Don't allow this program to be stopped and unecesarily tie
	 * up the nodes data base.
	 */
	ctlz.sv_handler = SIG_IGN;
	ctlz.sv_mask = 0;
	ctlz.sv_onstack = 0;
	if ( sigvec(SIGTSTP, &ctlz, NULL) == -1 )
	{
		sprintf(errmsg, "%s can't ignore ^Z", argv[0]);
		perror( errmsg );
		exit(1);
	}


	/*
	 * If -P issued, use permanent data base
	 */
	for ( i = 0; i < argc; i++ )
	{
		if ( strcmp(argv[i], "-P") == 0 )
		{
			setnodedb( ND_PERMANENT );
			break;
		}
	}

	/*
	 * If no nodes specified, dump the entire data base
	 */
	if ( (argc == 1) || (argc == 2 && (strcmp(argv[1], "-P") == 0)) )
	{
		if ( setnodeent(0) == -1 )
		{
			sprintf(errmsg, "%s can't access nodes data base", argv[0]);
			perror(errmsg);
			exit(1);
		}
		while ( (get_node = getnodeent()) != NULL )
		{
			print_nodeent(get_node);
		}
		endnodeent();
		exit(0);
	}

	/*
	 * Display each node entry requested by the user
	 */
	for ( i = 1; i < argc; i++ )
	{
		if ( strcmp(argv[i], "-P") == 0 )
		{
			continue;
		}
		else if ( (address = dnet_addr(argv[i])) == 0 )
		{
			if ( strlen(argv[i]) > MDB_NN_MAXL )
			{
				fprintf(stderr, "getnode: node name %s is too long\n", argv[i]);
				continue;
			}
			bzero(node, sizeof(node));
			strcpy(node, argv[i]);
			if ( ! (get_node = getnodebyname(node)) )
			{
				sprintf(errmsg, "%s can't fetch node %s from data base", argv[0], argv[i]);
				perror(errmsg);
				continue;
			}
			print_nodeent(get_node);
		}
		else
		{
			if ( ! (get_node = getnodebyaddr(address->a_addr, address->a_len, AF_DECnet)) )
			{
				sprintf(errmsg, "%s can't fetch node %s from data base", argv[0], argv[i]);
				perror(errmsg);
				continue;
			}
			print_nodeent(get_node);
		}
	}
}
