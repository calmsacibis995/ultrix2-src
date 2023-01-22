#ifndef lint
static	char	*sccsid = "@(#)load_interface.c	1.2	(ULTRIX)	10/21/85";
#endif lint

/*
 * Program user_load.c,  Module MOP 
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
extern int exit_status;
u_short child_is_dead;
char errbuf[4096];
extern char *target_address;

/*
 *		u s e r _ l o a d
 *
 * Force load a target node via a trigger or load boot message.  The former
 * is used if the "trigger" command is used, and the latter is used if the
 * "load" command is used. 
 *
 *
 * Inputs:
 *	see print_loaduse.c
 *
 * Outputs:
 *	exit status
 *	
 */
main(argc, argv, envp)
int argc;
char **argv, *envp;
{
	struct load_parameters load_parm;
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
		perror( "addnode: can't ignore ^Z" );
		exit(1);
	}

	/*
	 * Make sure user is super user
	 */
	if ( getuid() != NULL )
	{
		fprintf(stderr, "%s: not super user\n", argv[0]);
		exit(1);
	}

	switch ( parse_command_line( &load_parm, argc, argv ) )
	{
		case TRIGGER_NODE:
			exit_status = trigger_node( &load_parm );
			break;

		case LOAD_NODE:
			exit_status = load_node( &load_parm, argv[0], envp );
			break;

		default:
			exit_status = EXIT_PARSE_FAIL;
			break;

	}
	print_dll_errmsg(argv[0]);
	exit(exit_status);
}

char *load_errmsgs[] = {
	"",					/* EXIT_GOOD */
	"",					/* EXIT_GOODSEC */
	"",					/* EXIT_GOODTER */
	"",					/* EXIT_GOODSYS */
	"",					/* EXIT_GOODDIA */
	"",					/* EXIT_GOODDMP */
	"bad software id in program request", 	/* EXIT_BADSWID */
	"file open failure", 			/* EXIT_FOPENFAIL */
	"file read failure", 			/* EXIT_FREADFAIL */
	"file write failure", 			/* EXIT_FWRITEFAIL */
	"invalid mop message siz", 		/* EXIT_INVMSGSIZ */
	"invalid program type in request message", /* EXIT_INVPROGTYP */
	"invalid program request message", 	/* EXIT_INVRQST */
	"multicast address was used on program request", /* EXIT_MCAST */
	"operation unauthorized", 		/* EXIT_NOAUTHORIZATION */
	"can't find diagnostic file", 		/* EXIT_NODIAGFILE */
	"can't find dump file", 		/* EXIT_NODUMPFILE */
	"can't find system load file", 		/* EXIT_NOSYSFILE */
	"not chosen for downline load", 	/* EXIT_NOTCHOSEN */
	"must be super user", 			/* EXIT_NOTSU */
	"network receive failure", 		/* EXIT_RCVFAIL */
	"network transmit failure", 		/* EXIT_SENDFAIL */
	"",					/* EXIT_PARSFAIL */
	"can't access entry in nodes data base", /* EXIT_DBACCESSFAIL */
	"operation not permitted", 		/* EXIT_SERDISABL */
	"no data base key", 			/* EXIT_NODBKEY */
	"bad device name", 			/* EXIT_BADEVNAME */
	"unable to make connection", 		/* EXIT_DLICONFAIL */
	"can't create signal catcher", 		/* EXIT_SIGFAIL */
	"can't exec mop_dumpload", 		/* EXIT_CANTEXEC */
	"can't wait", 				/* EXIT_WAITFAIL */
	"can't duplicate network socket", 	/* EXIT_CANTDUP */
	"bad argument passed", 			/* EXIT_BADARG */
};
int num_load_errmsgs = sizeof(load_errmsgs)/sizeof(load_errmsgs[0]);

/*
 *		p r i n t _ e r r m s g
 *
 * This subroutine prints a load error message as a function of exit_status.
 *
 *
 * Inputs:
 *	caller		= string identifying caller.
 *	
 *	
 * Outputs:
 *
 */
print_dll_errmsg(caller)
char *caller;
{

	if ( exit_status >= num_load_errmsgs )
		fprintf(stderr, "%s: unrecognized exit code = %d\n", caller, exit_status);

	else
	{
		if ( strlen(load_errmsgs[exit_status]) )
			fprintf(stderr, "%s: %s\n", caller, load_errmsgs[exit_status]);
	}

	return;
}

/*
 *		p a r s e _ c o m m a n d _ l i n e
 *
 * This subroutine parses the command line for load parameters passed
 * by the user.  The parameters are placed in a load_parameters
 * structure.  
 *
 *
 * Inputs:
 *	load_parm	= ptr where parsed parameters are to be stored.
 *	argc		= number of arguments
 *	argv		= argument list
 *	
 *	
 * Outputs:
 *	returns		= If failure, -1 is returned.
 *			  If success, one of two values are returned.
 *			  TRIGGER_NODE is returned if argv[0] = "trigger". 
 *			  LOAD_NODE is returned if argv[0] = "load".
 *	db_parm		= structure where parameters were placed.
 */
parse_command_line( load_parm, argc, argv )
register struct load_parameters *load_parm;
int argc;
char **argv;
{
	int i, j;
	int load_type = TRIGGER_NODE;
	int argn = 1;
	int prog_id;
	struct dn_naddr *address;

	bzero(load_parm, sizeof(struct load_parameters));

	if ( argc < 2 )
	{
		print_usage(argv[0], "p", "address/name ", NULL, NULL);
		return(-1);
	}


	/*
	 * Determine whether load or trigger is in order
	 */
	i = j = strlen(argv[0]);
	while( i != 0 && argv[0][i-1] != '/' ) i--;
	if ( bcmp(&argv[0][i], "load", (j - i)) == 0 )
	{
		prog_id = LOAD_NODE;
	}
	else if ( bcmp(&argv[0][i], "trigger", (j - i)) == 0 )
	{
		prog_id = TRIGGER_NODE;
	}
	else
	{
		fprintf(stderr, "%s is an invalid request\n", argv[0]);
		return(-1);
	}


	/*
	 * fetch option (node/circuit) and validate it.
	 */
	load_parm->option = LOPTION_NODE;
	i = -1;
	while ( argv[1][++i] != NULL )
	{
		if ( argv[1][i] == '-' )
		{
			load_parm->option = LOPTION_CKT;
		}
	}


	if ( load_parm->option == LOPTION_NODE )
	{
		if ( (address = dnet_addr(argv[1])) == 0 )
		{
			toupcase( argv[1] );
			if ( ! valid_nname(argv[1]) )
			{
				return(-1);
			}
			strcpy(load_parm->load_node_name, argv[1]);
		}
		else
		{
			bcopy(address->a_addr, &load_parm->load_node_addr, address->a_len);
		}
	}
	else 
	{
		if ( i > (sizeof(load_parm->load_node_ckt)-1) )
		{
			fprintf(stderr, "invalid circuit specification size\n");
			return(-1);
		}
		if ( check_ckt(argv[0], argv[1]) < 0 )
		{
			return(-1);
		}
		strcpy(load_parm->load_node_ckt, argv[1]);
	}


	/*
	 * parse remainder of command string for possible parameters
	 */
	while ( ++argn < argc )
	{
		if ( argv[argn][0] == '-' )
		{
			if ( (++argn >= argc) || argv[argn][0] == '-' )
			{
				print_usage(argv[0], "p", "address/name ", NULL, NULL);
				return(-1);
			}
			if ( store_parm( argv[argn-1][1], argv[argn], load_parm, ((prog_id == LOAD_NODE) ? "aAcCdhlNnpstu" : "chp"), argv[0]) < 0 )
			{
				return(-1);
			}
		}
		else
		{
			print_usage(argv[0], "p", "address/name ", NULL, NULL);
			return(-1);
		}

	}
	
	return(prog_id);

}
