#ifndef lint
static	char	*sccsid = "@(#)remnode.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * Program remnode.c,  Module NODEDB 
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
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/errno.h>

#include "nmgtdb.h"
#include "mopdb.h"
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>

extern int errno;

/*
 *		r e m n o d e
 *
 * Remove an entry from the nodes data base.
 *
 *
 * Inputs:
 *	arguments can be either -P, node name or DECnet node address.  If
 *	-P is used, the permanent data base is operated on.
 *	
 */
main(argc, argv)
int argc;
char **argv;
{
	struct nodeent *get_node;
	struct dn_naddr *address;
	int i;
	u_char node[MDB_NN_MAXL+1];
	u_char errmsg[256];
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
	 * Make sure there is at least one node to remove
	 */
	if ( argc < 2 )
	{
		fprintf(stderr, "%s usage: %s [node id1] [node id2] [node id3] ...\n", argv[0], argv[0]);
		exit(1);
	}

	/*
	 *	for each argument remove the requested entries.
	 */
	for ( i = 1; i < argc; i++ )
	{
		if ( strcmp(argv[i], "-P") == 0 )
		{
			setnodedb( ND_PERMANENT );
			break;
		}
	}

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
				fprintf(stderr, "%s: node name %s is to long\n", argv[0], argv[i]);
				continue;
			}
			bzero(node, sizeof(node));
			strcpy(node, argv[i]);
			if ( remnodebyname(node) < 0 )
			{
				sprintf(errmsg, "%s can't remove node %s from data base", argv[0], node);
				perror(errmsg);
			}
		}
		else
		{
			if ( remnodebyaddr(address->a_addr, address->a_len, AF_DECnet) < 0 )
			{
				sprintf(errmsg, "%s can't remove node %s from data base", argv[0], argv[i]);
				perror(errmsg);
			}
		}
	}
}

