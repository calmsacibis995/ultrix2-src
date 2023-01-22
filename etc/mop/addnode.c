#ifndef lint
static	char	*sccsid = "@(#)addnode.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * Program addnode.c,  Module NODEDB 
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
#include <ctype.h>
#include <sys/socket.h> 
#include <sys/errno.h>

#include "nmgtdb.h"
#include "mopdb.h"
#include <netdnet/node_params.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>

extern int errno;
u_char parm_block[NODE_MAX_PSIZ], udb_params[NODE_MAX_PSIZ];

/*
 *		a d d n o d e
 *
 * Add a new entry to or change an existing entry in the nodes node data base.
 *
 *
 * Inputs:
 *		see print_usage(): -h, -s, -t, -l, -d, -D, -A, -N, -c, -p
 *	
 */
main(argc, argv)
int argc;
char **argv;
{
	int i;
	u_short dn_addr;
	struct dn_naddr *address, *parse_command_line();
	struct load_parameters new_node;
	struct nodeent add_node, *get_node, *chk_node, *build_addnodeent();
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
		perror( "addnode can't ignore ^Z" );
		exit(1);
	}

	/*
	 * Parse the command line for data base parameters
	 */
	bzero(new_node, sizeof(struct load_parameters));
	address = parse_command_line(&new_node, argc, argv);

	/*
	 * Build the data base entry from arguments given in the
	 * command line and from the existing data base entry (if present).
	 */
	get_node = build_addnodeent(&new_node, &add_node, address, argv);

	/*
	 * Add entry to the nodes data base file
	 */
	modify_ndb(&new_node, get_node, &add_node, argv);

	exit(0);

}
 
/*
 *		p a r s e _ c o m m a n d _ l i n e
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This routine parses the command line for user supplied
 *	parameters to be placed in the nodes data base.  The 
 *	information is parsed from argv and placed in a load_parameters
 *	structure.
 *
 * Inputs:
 *	nn			= pointer to load parameters structure
 *	argc			= number of arguments in argv list
 *	argv			= argv list passed from command line
 *
 * Outputs:
 *	returns			= pointer to structure dn_naddr
 *	nn			= where parameters are placed
 *
 * Notes:
 *
 */
struct dn_naddr *parse_command_line(nn, argc, argv)
register struct load_parameters *nn;
register u_short argc;
register char **argv;
{
	int i, got_alpha;
	int argn = 1;
	struct dn_naddr *address;
	static struct dn_naddr target_addr;

	if ( argc < 3 )
	{
		print_usage(argv[0], "hstldDANcp", "[address]", "[node name]\n", NULL);
		exit(1);
	}

	/*
	 * fetch DECnet Address and/or node name.
	 */
	bzero(&target_addr, sizeof(target_addr));
	if ( (address = dnet_addr(argv[argn])) != 0 )
	{
		nn->parm_present |= LPAR_ADDR;
		bcopy(address->a_addr, &nn->db_parm.node_addr, address->a_len);
		bcopy(address, &target_addr, sizeof(target_addr));
		argn++;
	}

	/*
	 * fetch node name.
	 */
	if ( argv[argn][0] != '-' )
	{
		i = 0;
		got_alpha = 0;
		while( argv[argn][i] != NULL )
		{
			if( isalpha(argv[argn][i]) )
				got_alpha = 1;
			else
			{
				if ( ! isdigit(argv[argn][i]) )
				{
					fprintf(stderr, "%s: %s is an invalid node name\n", argv[0], argv[argn]);
					exit(1);
				}
			}
			i++;
		}
		if ( (! got_alpha) && argv[argn][0] != NULL )
		{
			fprintf(stderr, "%s: %s is an invalid node name\n", argv[0], argv[argn]);
			exit(1);
		}
		else if ( i > MDB_NN_MAXL )
		{
			fprintf(stderr, "%s: node name %s too long\n", argv[0], argv[argn]);
			exit(1);
		}
		else
		{
			nn->parm_present |= LPAR_NAME;
			bcopy(&argv[argn++][0], nn->db_parm.node_name, i);
		}
	}

	/*
	 * Make sure either or both DECnet address or node name exist
	 * for handle into data base.
	 */
	if ( address == 0 && strlen(nn->db_parm.node_name) == 0 )
	{
		print_usage(argv[0], "hstldDANcp", "[address]", "[node name]\n", NULL);
		exit(1);
	}

	/*
	 * parse remainder of command line for switches.
	 */
	if ( argn-- == argc )
	{
		return((nn->parm_present & LPAR_ADDR) ? &target_addr : NULL);
	}
	while ( ++argn < argc )
	{
		if ( argv[argn][0] == '-' )
		{
			if ( argv[argn][1] == 'P' )
			{
				setnodedb( ND_PERMANENT );
			}
			else
			{
				if ( (++argn >= argc) || argv[argn][0] == '-' )
				{
					print_usage(argv[0], "hstldDANcp", "[address]", "[node name]\n", NULL);
					exit(1);
				}
				if ( store_parm( argv[argn-1][1], argv[argn], nn, "AcCdDhIlNpSstTu", argv[0]) < 0 )
				{
					exit(1);
				}
			}
		}
		else
		{
			print_usage(argv[0], "hstldDANcp", "[address]", "[node name]\n", NULL);
			exit(1);
		}

	}
	return((nn->parm_present & LPAR_ADDR) ? &target_addr : NULL);
}

/*
 *		b u i l d _ a d d n o d e e n t
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This routine builds an addnodeent structure to be passed into
 *	the nodes data base library routine addnodeent().  It is built
 *	from the load parameters structure containing parameters passed
 *	into the program and from the existing data base entry (if present).
 *
 * Inputs:
 *	nn			= pointer to load parameters structure
 *	an			= pointer to nodeent structure built by
 *				  this routine
 *	address			= pointer to structure containing DECnet
 *				  address supplied by user.
 *	argv			= argument list supplied by user.
 *
 * Outputs:
 *	returns			= pointer to existing data base entry
 *	an			= where nodeent structure is built
 *
 * Notes:
 *
 */
struct nodeent *build_addnodeent(nn, an, address, argv)
register struct load_parameters *nn;
register struct nodeent *an;
register struct dn_naddr *address;
char **argv;
{
	static struct nodeent keep_node;
	register struct nodeent *chk_node;
	register struct nodeent *get_node = NULL;
	u_short save_dnaddr;
	u_char *pb_user, *build_pblock(), *merge_pblocks();

	/*
	 * Build data base entry
	 */
	pb_user = build_pblock(nn);
	if ( nn->db_parm.node_addr )
	{
		if( address && (get_node = getnodebyaddr(address->a_addr, address->a_len, AF_DECnet)) )
		{
			bcopy(get_node, &keep_node, sizeof(keep_node));
			save_dnaddr = *(u_short *) get_node->n_addr;
		}

		if ( strlen(nn->db_parm.node_name) )
		{
			if ( (chk_node = getnodebyname(nn->db_parm.node_name)) != NULL )
			{
				if ( ! get_node )
				{
					fprintf(stderr, "%s: node %s already in use\n", argv[0], nn->db_parm.node_name);
					exit(1);
				}
				if ( save_dnaddr != (*(u_short *) chk_node->n_addr) )
				{
					fprintf(stderr, "%s: node %s already in use\n", argv[0], nn->db_parm.node_name);
					exit(1);
				}
			}
			else
				/* reallign data base entry */
				get_node = getnodebyaddr(address->a_addr, address->a_len, AF_DECnet);
		}
	}
	else
	{
		if ( get_node = getnodebyname(nn->db_parm.node_name) )
			bcopy(get_node, &keep_node, sizeof(keep_node));
	}
	if ( (! (nn->parm_present & LPAR_NAME)) && get_node )
	{
		strcpy(nn->db_parm.node_name, keep_node.n_name);
	}
	if ( nn->db_parm.node_addr == 0 && get_node )
	{
		bcopy(keep_node.n_addr, &nn->db_parm.node_addr, sizeof(nn->db_parm.node_addr));
	}
	an->n_name = (char *) nn->db_parm.node_name;
	an->n_addrtype = AF_DECnet;
	an->n_length = sizeof(nn->db_parm.node_addr);
	an->n_addr = (char *) &nn->db_parm.node_addr;

	if( get_node )
	{
		an->n_params = merge_pblocks(keep_node.n_params, pb_user, nn->parm_delete);
		return(&keep_node);
	}
	else
	{
		an->n_params = pb_user;
		return(NULL);
	}
}

/*
 *		t o u p c a s e
 *
 *
 * Version: 1.0 - 1/10/85
 *
 * Description:
 *	This routine makes sure that a string is in upper case
 *	and returns the number of characters in the string.
 *
 * Inputs:
 *	buffer			= Pointer to buffer containing string.
 *
 *
 * Outputs:
 *	returns			string size 
 *
 * Notes:
 *
 */
toupcase( buffer )
register char *buffer;
{
	register int i = -1;
	while ( buffer[++i] != NULL )
	{
		if ( buffer[i] >= 'a' && buffer[i] <= 'z' )
		{
			buffer[i] -= ('a' - 'A');
		}
	}
	return(i);
}
 
/*
 *		b u i l d _ p b l o c k
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This routine creates a parameter block from a load_parameters
 *	structure.  The parameter block is structured according to the
 *	rules stated in node_params.h.
 *
 * Inputs:
 *	np			= pointer to load parameters structure.
 *
 * Outputs:
 *	returns			= pointer to parameter block if there are
 *				  parameters listed in it.  Otherwise, NULL
 *				  is returned. 
 *
 * Notes:
 *
 */
u_char *build_pblock(np)
register struct load_parameters *np;
{
	u_char *add_parm();
	u_char *pb = parm_block;
	u_short psiz, i;

	if ( np->parm_present & LPAR_SCKT )
	{
		pb = add_parm(pb, NODE_SERVICE_CKT, strlen(np->db_parm.node_parm.rnm_sercirc), np->db_parm.node_parm.rnm_sercirc);
	}

	if ( np->parm_present & LPAR_SPSWD )
	{
		psiz = sizeof(np->db_parm.node_parm.rnm_serpasswd) - 1;
		while( np->db_parm.node_parm.rnm_serpasswd[psiz] == NULL && psiz > 0 ) psiz--;
		pb = add_parm(pb, NODE_SERVICE_PSWD, ++psiz, np->db_parm.node_parm.rnm_serpasswd);
	}

	if ( np->parm_present & LPAR_SDEV )
	{
		pb = add_parm(pb, NODE_SERVICE_DEV, sizeof(np->db_parm.node_parm.rnm_serdev), &np->db_parm.node_parm.rnm_serdev);
	}

	if ( np->parm_present & LPAR_CPU )
	{
		pb = add_parm(pb, NODE_CPU, sizeof(np->db_parm.node_parm.rnm_cpu), &np->db_parm.node_parm.rnm_cpu);
	}

	if ( np->parm_present & LPAR_HWADDR )
	{
		pb = add_parm(pb, NODE_HARDWARE_ADDR, sizeof(np->db_parm.node_parm.rnm_hwaddr), np->db_parm.node_parm.rnm_hwaddr);
	}

	if ( np->parm_present & LPAR_LFILE )
	{
		pb = add_parm(pb, NODE_LOAD_FILE, strlen(np->db_parm.node_parm.rnm_loadfile), np->db_parm.node_parm.rnm_loadfile);
	}

	if ( np->parm_present & LPAR_SFILE )
	{
		pb = add_parm(pb, NODE_SEC_LOADER, strlen(np->db_parm.node_parm.rnm_secload), np->db_parm.node_parm.rnm_secload);
	}

	if ( np->parm_present & LPAR_TFILE )
	{
		pb = add_parm(pb, NODE_TER_LOADER, strlen(np->db_parm.node_parm.rnm_terload), np->db_parm.node_parm.rnm_terload);
	}

	if ( np->parm_present & LPAR_DGFILE )
	{
		pb = add_parm(pb, NODE_DIAG_FILE, strlen(np->db_parm.node_parm.rnm_diagfile), np->db_parm.node_parm.rnm_diagfile);
	}

	if ( np->parm_present & LPAR_SWTYPE )
	{
		pb = add_parm(pb, NODE_SW_TYPE, sizeof(np->db_parm.node_parm.rnm_swtype), &np->db_parm.node_parm.rnm_swtype);
	}

	if ( np->parm_present & LPAR_SWID )
	{
		pb = add_parm(pb, NODE_SW_ID, strlen(np->db_parm.node_parm.rnm_swident), np->db_parm.node_parm.rnm_swident);
	}

	if ( np->parm_present & LPAR_DUFILE )
	{
		pb = add_parm(pb, NODE_DUMP_FILE, strlen(np->db_parm.node_parm.rnm_dumpfile), np->db_parm.node_parm.rnm_dumpfile);
	}

	if ( np->parm_present & LPAR_HOST )
	{
		i = NODE_HOST;
		bcopy(&i, pb, NODE_CSIZ);
		pb += NODE_CSIZ;
		psiz = sizeof(np->db_parm.node_parm.rnm_host.n_addr) 
			+ strlen(np->db_parm.node_parm.rnm_host.n_name); 
		bcopy(&psiz, pb, sizeof(psiz));
		pb += sizeof(psiz);
		bcopy(&np->db_parm.node_parm.rnm_host.n_addr, pb, sizeof(np->db_parm.node_parm.rnm_host.n_addr));
		bcopy(np->db_parm.node_parm.rnm_host.n_name, (pb+sizeof(np->db_parm.node_parm.rnm_host.n_addr)), psiz);
		pb += psiz;
	}

	if ( np->parm_present & LPAR_SERVICE )
	{
		pb = add_parm(pb, NODE_SERVICE, sizeof(np->db_parm.node_parm.rnm_service), &np->db_parm.node_parm.rnm_service);
	}

	i = NULL;
	bcopy(&i, pb, NODE_CSIZ);

	if (pb == parm_block)
	{
		return(NULL);
	}
	else
	{
		return(parm_block);
	}
}
 
/*
 *		a d d _ p a r m
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This subroutine adds a parameter entry to a parameter block
 *
 * Inputs:
 *	pb			= pointer to parameter block where entry
 *				  is to be placed.
 *	code			= parameter code
 *	size			= size of parameter string
 *	parm			= ptr to parameter
 *
 * Outputs:
 *	returns			= pointer to where next entry is to be placed
 *				  in parameter block. 
 *
 * Notes:
 *
 */
u_char *add_parm(pb, code, size, parm)
u_char *pb, *parm;
u_short code, size;
{
	*(u_short *) pb = *(u_short *) &code;
	pb += NODE_CSIZ;
	*(u_short *) pb = *(u_short *) &size;
	pb += NODE_SSIZ;
	bcopy(parm, pb, (*(u_short *) &size)); 
	pb += *(u_short *) &size;
	return(pb);
}
 
/*
 *		m e r g e _ p b l o c k s
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This subroutine merges two parameter blocks into one.  The
 *	first argument is considered the parameter block from the
 *	current data base entry while the second argument is to be
 *	added into it.
 *
 * Inputs:
 *	cdb_params		= pointer to current data base parameters
 *	usr_params		= pointer to user supplied parameters
 *	parm_delete		= bit map of parameters to be deleted.
 *
 * Outputs:
 *	returns			= pointer to parameter block if there are
 *				  parameters listed in it.  Otherwise, NULL
 *				  is returned. 
 *
 * Notes:
 *
 */
u_char *merge_pblocks(cdb_params, usr_params, parm_delete)
register u_char *cdb_params, *usr_params;
{
	u_char *cdb, *usr, *udb, *choice;
	u_short psiz;

	/*
	 * Make sure you have something to merge
	 */
	if ( cdb_params == NULL && usr_params == NULL )
	{
		return(NULL);
	}

	/*
	 * If only user parameters exist, then pass back user parameters
	 */
	udb = udb_params;
	if ( cdb_params == NULL )
	{
		cdb = usr_params;
		while ( *cdb != NULL )
		{
			bcopy(cdb+NODE_CSIZ, &psiz, sizeof(psiz));
			psiz +=  NODE_PARM_HDRSIZ;
			if ( ok_to_add(cdb, parm_delete) )
			{
				bcopy(cdb, udb, psiz);
				udb += psiz;
			}
			cdb += psiz;
		}
		*(u_short *) udb = NULL;
		if ( udb == udb_params )
			return(NULL);
		else
			return(udb_params);
	}

	/*
	 * If only data base parameters exist, then pass back data base
	 * parameters
	 */
	if ( usr_params == NULL )
	{
		cdb = cdb_params;
		while ( *cdb != NULL )
		{
			bcopy(cdb+NODE_CSIZ, &psiz, sizeof(psiz));
			psiz +=  NODE_PARM_HDRSIZ;
			if ( ok_to_add(cdb, parm_delete) )
			{
				bcopy(cdb, udb, psiz);
				udb += psiz;
			}
			cdb += psiz;
		}
		*(u_short *) udb = NULL;
		if ( udb == udb_params )
			return(NULL);
		else
			return(udb_params);
	}

	/*
	 * For each data base parameter, if there exists a user supplied
	 * parameter, replace the former with the latter in the updated
	 * data base parameter block.
	 */
	cdb = cdb_params;
	while ( *cdb != NULL )
	{
		choice = cdb;
		usr = usr_params;
		while ( *usr != NULL )
		{
			if ( *cdb == *usr )
			{
				choice = usr;
				break;
			}
			bcopy(usr+NODE_CSIZ, &psiz, sizeof(psiz));
			usr += ( psiz + NODE_PARM_HDRSIZ );
		}
		bcopy(choice+NODE_CSIZ, &psiz, sizeof(psiz));
		psiz +=  NODE_PARM_HDRSIZ;
		if ( ok_to_add(choice, parm_delete) )
		{
			bcopy(choice, udb, psiz);
			udb += psiz;
		}
		bcopy(cdb+NODE_CSIZ, &psiz, sizeof(psiz));
		cdb += psiz + NODE_PARM_HDRSIZ;
	}
	
	/*
	 * Add in leftover user supplied parameters to updated data base 
	 * parameters.
	 */
	usr = usr_params;
	while ( *usr != NULL )
	{
		choice = usr;
		cdb = cdb_params;
		while ( *cdb != NULL )
		{
			if ( *cdb == *usr )
			{
				choice = NULL;
				break;
			}
			bcopy(cdb+NODE_CSIZ, &psiz, sizeof(psiz));
			cdb += ( psiz + NODE_PARM_HDRSIZ );
		}
		if ( choice != NULL )
		{
			bcopy(choice+NODE_CSIZ, &psiz, sizeof(psiz));
			psiz +=  NODE_PARM_HDRSIZ;
			if ( ok_to_add(choice, parm_delete) )
			{
				bcopy(choice, udb, psiz);
				udb += psiz;
			}
		}
		bcopy(usr+NODE_CSIZ, &psiz, sizeof(psiz));
		usr += psiz + NODE_PARM_HDRSIZ;
	}
	*udb = NULL;
	if (udb == udb_params )
	{
		return(NULL);
	}
	else
	{
		return(udb_params);
	}
}
 
/*
 *		o k _ t o _ a d d
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This subroutine determines whether a parameter should
 *	be added or deleted.
 *
 * Inputs:
 *	parm		= pointer to parameter number
 *	dmask		= parameter deletion bit mask
 *
 * Outputs:
 *	returns		= 0 if delete, 1 if add
 *
 * Notes:
 *
 */
ok_to_add(parm, dmask)
u_char *parm;
int dmask;
{

	switch ( *(u_short *) parm )
	{
		case NODE_HOST:
			return( !(dmask &= LPAR_HOST) );
			break;

		case NODE_SERVICE_CKT:
			return( !(dmask &= LPAR_SCKT) );
			break;

		case NODE_SERVICE:
			return( !(dmask &= LPAR_SERVICE) );
			break;

		case NODE_CPU:
			return( !(dmask &= LPAR_CPU) );
			break;

		case NODE_DIAG_FILE:
			return( !(dmask &= LPAR_DGFILE) );
			break;

		case NODE_DUMP_FILE:
			return( !(dmask &= LPAR_DUFILE) );
			break;

		case NODE_HARDWARE_ADDR:
			return( !(dmask &= LPAR_HWADDR) );
			break;

		case NODE_SW_ID:
			return( !(dmask &= LPAR_SWID) );
			break;

		case NODE_LOAD_FILE: 
			return( !(dmask &= LPAR_LFILE) );
			break;

		case NODE_SERVICE_PSWD:
			return( !(dmask &= LPAR_SPSWD) );
			break;

		case NODE_SEC_LOADER:
			return( !(dmask &= LPAR_SFILE) );
			break;

		case NODE_TER_LOADER:
			return( !(dmask &= LPAR_TFILE) );
			break;

		case NODE_SW_TYPE:
			return( !(dmask &= LPAR_SWTYPE) );
			break;

		case NODE_SERVICE_DEV:
			return( !(dmask &= LPAR_SDEV) );
			break;

		default:
			return(1);
			break;
	}
}

/*
 *		m o d i f y _ n d b
 *
 *
 * Version: 1.0 - 8/6/85
 *
 * Description:
 *	This subroutine modifies the nodes data base by either adding
 *	a new node entry or changing the contents of an existing entry.
 *
 * Inputs:
 *	new_node	= pointer to structure containing parameters
 *			  parsed from command line
 *	get_node	= pointer to existing entry in data base to be
 *			  changed
 *	add_node	= pointer to new nodes data base entry
 *	argv		= argument list passed from command line
 *
 * Outputs:
 *
 * Notes:
 *
 */
modify_ndb(new_node, get_node, add_node, argv)
register struct load_parameters *new_node;
register struct nodeent *get_node, *add_node;
char **argv;
{
	u_short dn_addr;
	u_char errmsg[256];


	/*
	 * add node to nodes data base.
	 */
	if ( setnodeentw(0) == -1 )
	{
		sprintf(errmsg, "%s can't access nodes data base\n", argv[0]);
		perror(errmsg);
		exit(1);
	}
	if ( get_node )
	{
		if ( *(u_short *) get_node->n_addr)
		{
			bcopy(get_node->n_addr, &dn_addr, sizeof(dn_addr));
			if ( remnodebyaddr(&dn_addr, sizeof(dn_addr), AF_DECnet) < 0 )
			{
				endnodeent();
				sprintf(errmsg, "%s can't modify entry", argv[0]);
				perror(errmsg);
				exit(1);
			}
		}
		else
		{
			if ( remnodebyname(new_node->db_parm.node_name) < 0 )
			{
				endnodeent();
				sprintf(errmsg, "%s can't modify entry", argv[0]);
				perror(errmsg);
				exit(1);
			}
		}
	}


	if (addnodeent(add_node) < 0)
	{
		sprintf(errmsg, "%s can't add node %s to data base", argv[0], argv[1]);
		perror(errmsg);
		endnodeent();
		exit(1);
	}

	endnodeent();

	return;
}
