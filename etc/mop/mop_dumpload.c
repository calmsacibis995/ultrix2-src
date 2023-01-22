#ifndef lint
static char *sccsid = "@(#)mop_dumpload.c	1.6	ULTRIX	12/4/86";
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
int exit_status = EXIT_GOOD;
char errbuf[4096];
char *target_address = NULL;

char default_load_path[] = "/usr/lib/mop";
char dnet_default_load_path[] = "/usr/lib/dnet";
char invoker[1024], load_path[4096], download_db[1024];
u_char chk_authorization = 0;
u_char label_type = LABEL_TYPE_RSX;
int	file_format, aout_text, aout_gap;
extern int mop_syslog_debug;

/*
 *	m o p _ d u m p l o a d . c
 *
 * Version 1.0 - 1/10/85
 *		4/9/85	- log errors via syslog() instead of stderr.
 *			- use nodes data base.
 *
 *
 * Description:
 *	This program processes dump/load program requests passed by
 *	dl_mom.
 *
 * Inputs:
 *		*argv[1] is the MOP message containing the request.
 *		*argv[2] contains the physical address of the requester.
 *		*argv[3] contains the physical address used to address local node.
 *		*argv[4] contains a parameter list used to override parameters
 *		 in the program request message.
 *
 * Outputs:
 *		exit_status	= status of load upon completion.
 *
 * Notes:
 *
 */
main(argc, argv, envp)
int argc;
char **argv, **envp;
{
	
	FILE *fopen(), *fetch_load_file(), *fetch_dump_file(), *fp;
	struct mop_parse_dmpld mop_parameters;
	struct task_image task_label;
	char *fgets();
	char *path, *mop_debug, *mop_secure,  *getenv();
	u_char message[MOM_MSG_SIZE];
	struct node_id tnode, hnode;
	int i;
	register int j, k;

	bzero(tnode, sizeof(struct node_id));
	bzero(hnode, sizeof(struct node_id));

	if ( getuid() != NULL )
	{
		sprintf(errbuf, "not super-user, load request ignored");
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_NOTSU;
		exit(exit_status);
	}

	if ( argc < 4 )
	{
		sprintf(errbuf, "incorrect argument count, load request ignored");
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_BADARG;
		exit(exit_status);
	}

	/*
	 * Check for debugging environment
	 */
	if (( mop_debug = getenv( "LOADUMP_DEBUG" )) != NULL && strcmp(mop_debug, "on") == 0 )
	{
		mop_syslog_debug = 1;
	}

	/*
	 * Check for secure environment. If set, then force data base search
	 * for hardware address even if given program name.
	 */
	if (( mop_secure = getenv( "LOADUMP_SECURE" )) != NULL && strcmp(mop_secure, "on") == 0 )
	{
		chk_authorization = 1;
	}

	/*
	 * format load pathname sequence.
	 * if environment exists, add additional load pathnames.
	 */
	sprintf(load_path, "%s:%s", default_load_path, dnet_default_load_path);
	j = strlen(load_path);
	if (( path = getenv( "LOADUMP_PATH" )) != NULL && (strlen(path) + j) < sizeof(load_path) )
	{
		sprintf(load_path+j, ":%s", path);
	}

	/*
	 * verify valid source address.
	 */
	target_address = argv[2];
	sscanf(argv[2], "%2x", &i);
	if ( i & 1 )
	{
		sprintf(errbuf, "source address (%s) is multicast", argv[2]);
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_MCAST;
		exit(exit_status);
	}
 
	/*
	 * log event stating that loader has started.
	 */
	mop_syslog("mop_dumpload", "dumper/loader started", 0, LOG_DEBUG);

	/*
	 * translate message from hex to binary.
	 */
	if ( (k = strlen(argv[1])) > (sizeof(message)*2) )
	{
		sprintf(errbuf, "MOP Request Message is too large (size = %d)", k);
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_INVMSGSIZ;
		exit(exit_status);
	}
	for (i = 0, j = 0; j < k; i++, j += 2)
	{
		sscanf(argv[1]+j, "%2x", &message[i]);
	}

	/*
	 * parse the message, and attemp to service it.
	 */
	bzero(&mop_parameters, sizeof(struct mop_parse_dmpld));
	mop_syslog("mop_dumpload", "parsing request message", 0, LOG_DEBUG);
	if ( rqst_parse( &mop_parameters, message ) < 0 )
	{
		exit(exit_status);
	}
	/*
	 * allow host and target node ids as well as other parameters
	 *  to be overridden
	 */
	if ( argc == 5 )
	{
		if ( fetch_moreparms(&mop_parameters, argv[4], &tnode, &hnode) < 0 )
		{
			sprintf(errbuf, "node id argument is bad, load request ignored");
			mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
			exit_status = EXIT_BADARG;
			exit(exit_status);
		}
	}
	dumparams(&mop_parameters);
	switch ( mop_parameters.mop_dmpld_code )
	{
		case MOP_RQST_PGM:
			mop_syslog("mop_dumpload", "searching for load file and authorization", 0, LOG_DEBUG);
			if ((fp = fetch_load_file( &tnode, &hnode, &mop_parameters, &task_label, argv[2] )) != NULL)
			{
				mop_syslog("mop_dumpload", "downline load routine called", 0, LOG_DEBUG);
				mop_downline_load( &tnode, &hnode, &mop_parameters, &task_label, fp, argv[3] );
			}
			break;

		case MOP_RQST_DMPSRV:
			mop_syslog("mop_dumpload", "searching for dump file and authorization", 0, LOG_INFO);
			if ((fp = fetch_dump_file( &tnode, &hnode, &mop_parameters, &task_label, argv[2] )) != NULL)
			{
				mop_syslog("mop_dumpload", "upline dump routine called", 0, LOG_DEBUG);
				mop_upline_dump( &tnode, &mop_parameters, fp, argv[3]);
			}
			break;

		default:
			mop_syslog("mop_dumpload", "invalid dump/load message", 0, LOG_INFO);
			break;
	}

	mop_syslog("mop_dumpload", "exiting dumper/loader", 0, LOG_DEBUG);
	exit(exit_status);
}

/*
 *	f e t c h _ m o r e p a r m s
 *
 * Version 1.0 - 8/12/85
 *
 *
 * Description:
 *	This subroutine parses an argument string for target and host
 *	node id information.
 *
 * Inputs:
 *	parm		= pointer to structure containing parameters
 *			  parsed from the program request message.
 *	node_argv	= argument containing node information
 *	tnode		= pointer to structure where target node info 
 *			  is to be placed
 *	hnode		= pointer to structure where host node info 
 *			  is to be placed
 *
 * Outputs:
 *	returns		= 0 if success, -1 if failure
 *	tnode		= where target node info was placed
 *	hnode		= where host node info was placed
 *
 * Notes:
 *
 */
fetch_moreparms(parm, node_argv, tnode, hnode)
struct mop_parse_dmpld *parm;
register u_char *node_argv;
register struct node_id *tnode, *hnode;
{
	register u_short size;
	register int i = 0;
	int arg_len = strlen(node_argv);

	while ( i < arg_len )
	{
		sscanf(&node_argv[i+1], "%2x", &size);
		switch ( node_argv[i++] )
		{
			case '0':
				i += 2;
				sscanf(&node_argv[i], "%4x", &hnode->node_dnaddr);
				break;

			case '1':
				i += 2;
				if ( size <= sizeof(hnode->node_name) )
				{
					bcopy(&node_argv[i], hnode->node_name, size);
					hnode->node_name[size] = NULL;
				}
				else
					return(-1);
				break;

			case '2':
				i += 2;
				sscanf(&node_argv[i], "%4x", &tnode->node_dnaddr);
				break;

			case '3':
				i += 2;
				if ( size <= sizeof(tnode->node_name) )
				{
					bcopy(&node_argv[i], tnode->node_name, size);
					tnode->node_name[size] = NULL;
				}
				else
					return(-1);
				break;

			case '4':
				i += 2;
				if ( size < (sizeof(load_path) - 1) )
				{
					bcopy(&node_argv[i], load_path, size);
					load_path[size] = NULL;
				}
				else
					return(-1);
				break;

			case '5':
				i += 2;
				if ( size <= sizeof(invoker) )
				{
					bcopy(&node_argv[i], invoker, size);
					invoker[size] = NULL;
				}
				else
					return(-1);
				if ( bcmp(invoker, "ccr", 3) == 0 && strlen(invoker) == 3 )
				{
					chk_authorization = 0;
					if ( parm->mop_pgm_type == MOP_SYSTEM )
						label_type = LABEL_TYPE_RT11;
				}
				if ( bcmp(invoker, "load", 4) == 0 && strlen(invoker) == 4 )
				{
					chk_authorization = 0;
				}
				break;

			default:
				return(-1);
				break;
		}
		i += size;
	}

	return(0);
}

extern int mop_syslog_debug;
/*
*	dumparams - dump mop_parameters for debugging
*
*
*/
dumparams(parm)
struct mop_parse_dmpld *parm;
{
	char errbuf[80];

	if ( ! mop_syslog_debug )
		return(0);

	sprintf(errbuf, "mop_dmpld_code=%o mop_devtyp=%o", parm->mop_dmpld_code,parm->mop_devtyp);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	sprintf(errbuf, "mop_format_vsn=%o mop_pgm_type=%o", parm->mop_format_vsn,parm->mop_pgm_type);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	sprintf(errbuf, "mop_swid_form=%o mop_swid_id=%s", parm->mop_swid_form,parm->mop_swid_id);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	sprintf(errbuf, "mop_processor=%o mop_sys_processor=%o", parm->mop_processor,parm->mop_sys_processor);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	sprintf(errbuf, "mop_hw_addr=%s mop_dl_bufsiz=%d", parm->mop_hw_addr,parm->mop_dl_bufsiz);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	sprintf(errbuf, "mop_dump_size=%d mop_compat_bits=%o", parm->mop_dump_size,parm->mop_compat_bits);
	mop_syslog("MOP PARAM DUMP:", errbuf, 0, LOG_DEBUG);
	return(0);
}
