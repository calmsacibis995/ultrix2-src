#ifndef lint
static char *sccsid = "@(#)mop_forceload.c	1.4	ULTRIX	10/3/86";
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
 *		t r i g g e r _ n o d e
 *
 * This subroutine triggers a node by sending a boot message and then 
 * exits.  The target node will send a program request with the 
 * multicast address.  Consequently, the loader will get kicked off by
 * MOM.  The parameters in the MOP request program message will be used.  
 *
 *
 * Inputs:
 *	load_parm	= ptr to structure containing load parameters.
 *	
 *	
 * Outputs:
 *	returns		= status code is returned.
 */
trigger_node( load_parm )
register struct load_parameters *load_parm;
{
	int i, msg_siz, so;
	u_short devunit;
	u_char devname[17], msg[1600];
	char decnet_addr[6]; 

	decnet_addr[0] = 0xaa; 
	decnet_addr[1] = 0x00;
	decnet_addr[2] = 0x04;
	decnet_addr[3] = 0x00;

	/*
	 * get all parameters necessary for boot message
	 */
	if ( ! (load_parm->parm_present & LPAR_PHYSADDR) && (load_parm->parm_present & LPAR_HWADDR) )
	{
		bcopy(load_parm->db_parm.node_parm.rnm_hwaddr, load_parm->db_parm.node_physaddr, sizeof(load_parm->db_parm.node_physaddr));
		load_parm->parm_present |= LPAR_PHYSADDR;
	}
	if ( access_db( load_parm, TRIGGER_NODE) < 0 )
	{
		return(exit_status);
	}

	/*
	 * build boot message
	 */
	if ( (msg_siz = build_boot_msg( msg, load_parm, TRIGGER_NODE, MOP_SYS_PROC)) < 0 )
	{
		return(exit_status);
	}

	/*
	 * fetch device to be used in terms that ULTRIX can understand.
	 */
	if ( (i = dnet_to_dev(devname, load_parm->db_parm.node_parm.rnm_sercirc)) < 0 )
	{
		exit_status = EXIT_BADEVNAME; 
		return(exit_status);
	}
	if ( sscanf(devname+i, "%hd", &devunit) == EOF )
	{
		exit_status = EXIT_BADEVNAME; 
		return(exit_status);
	}
	devname[i] = NULL;


	/*
	 * send boot message(s).
	 */
	if ( (so = dli_econn(devname, devunit, PROTO_DUMP_LOAD, load_parm->db_parm.node_physaddr, DLI_NORMAL)) < 0 )
	{
		exit_status = EXIT_DLICONFAIL;
		return(exit_status);
	}
	if ( send_boot(so, load_parm->db_parm.node_physaddr, devname, devunit, msg, msg_siz) < 0 )
	{
		return(exit_status);
	}
	if ( load_parm->db_parm.node_addr !=NULL )
	{
		bcopy(&load_parm->db_parm.node_addr, decnet_addr+4, sizeof(load_parm->db_parm.node_addr));


		if ( send_boot(so, decnet_addr, devname, devunit, msg, msg_siz) < 0 )
		{
			return(exit_status);
		}
	}

	return(exit_status);
}

/*
 *		l o a d _ n o d e
 *
 * This subroutine loads a node by sending a boot message and then 
 * exits.  The target node will send a program request with the 
 * multicast address.  Consequently, the loader will get kicked off by
 * MOM.  The parameters in the MOP request program message will be used.  
 *
 *
 * Inputs:
 *	load_parm	= ptr to structure containing load parameters.
 *	caller		= pointer to string containing name of calling
 *			  program.
 *	envp		= pointer to environment list.
 *	
 *	
 * Outputs:
 *	returns		= status code is returned.
 */
load_node( load_parm, caller, envp )
register struct load_parameters *load_parm;
char *caller;
char **envp;
{
	int i, msg_siz, rsiz;
	int stdin_sav, stdout_sav;
	u_short target;
	u_short devunit;
	u_char processor, number_of_loads;
	u_char load_stage, devname[17], msg[1600], in_msg[1600];
	char *dnaddr, decnet_addr[6];
	void catch_sigalrm();

	decnet_addr[0] = 0xaa; 
	decnet_addr[1] = 0x00;
	decnet_addr[2] = 0x04;
	decnet_addr[3] = 0x00;

	/*
	 * get all parameters necessary for boot message
	 */
	if ( ! (load_parm->parm_present & LPAR_PHYSADDR) && (load_parm->parm_present & LPAR_HWADDR) )
	{
		bcopy(load_parm->db_parm.node_parm.rnm_hwaddr, load_parm->db_parm.node_physaddr, sizeof(load_parm->db_parm.node_physaddr));
		load_parm->parm_present |= LPAR_PHYSADDR;
	}
	if ( access_db( load_parm, LOAD_NODE) < 0 )
	{
		return(exit_status);
	}

	/*
	 * build boot message
	 */
	if ( bcmp(caller, "ccr", 3) == 0 && strlen(caller) == 3 )
	{
		processor = MOP_COM_PROC;
	}
	else
	{
		processor = MOP_SYS_PROC;
	}
	if ( (msg_siz = build_boot_msg( msg, load_parm, LOAD_NODE, processor)) < 0 )
	{
		return(exit_status);
	}

	/*
	 * translate circuit id into device id that ULTRIX understands
	 */
	if ( (i = dnet_to_dev(devname, load_parm->db_parm.node_parm.rnm_sercirc)) < 0 )
	{
		exit_status = EXIT_BADEVNAME; 
		return(exit_status);
	}
	if ( sscanf(devname+i, "%hd", &devunit) == EOF )
	{
		exit_status = EXIT_BADEVNAME; 
		return(exit_status);
	}
	devname[i] = NULL;

	/*
	 * save STDIN and STDOUT
	 */
	if ( (stdin_sav = dup(STDIN)) < 0 )
	{
		exit_status = EXIT_CANTDUP;
		return(exit_status);
	}
	if ( (stdout_sav = dup(STDOUT)) < 0 )
	{
		exit_status = EXIT_CANTDUP;
		return(exit_status);
	}

	/*
	 * set up signal catcher to catch SIGALRM if timeout occurs.
	 */
	if ( signal(SIGALRM, catch_sigalrm) < 0 )
	{
		exit_status = EXIT_SIGFAIL;
		return(exit_status);
	}

	/*
	 * Send boot message to start load.  
	 */
	if ( load_parm->db_parm.node_addr !=NULL )
	{
		bcopy(&load_parm->db_parm.node_addr, decnet_addr+4, sizeof(load_parm->db_parm.node_addr));
		dnaddr = &decnet_addr[0];
	}
	else
	{
		dnaddr = NULL;
	}
	if ( (rsiz = send_load(load_parm->db_parm.node_physaddr, dnaddr,
		 devname, devunit, msg, msg_siz, in_msg, sizeof(in_msg),
		 &target)) <= 0 )
	{
		dup2(stdin_sav, STDIN);
		dup2(stdout_sav, STDOUT);
		return(exit_status);
	}

	/*
	 * Target responded with program request.  Load each stage until
	 * the operating system is loaded, retries attempts are exhausted
	 *  or a fatal error occurs.
	 */
	number_of_loads = MOP_RETRY*2;
	while ( number_of_loads-- )
	{
		load_stage = in_msg[PRQ_PGMTYPE_IDX];
		if ( spawn_dumpload(caller, load_parm, in_msg, rsiz, (target ? decnet_addr : load_parm->db_parm.node_physaddr), envp) < 0 )
		{
			dup2(stdin_sav, STDIN);
			dup2(stdout_sav, STDOUT);
			return(exit_status);
		}
		else if ( load_stage == MOP_SYSTEM )
		{
			dup2(stdin_sav, STDIN);
			dup2(stdout_sav, STDOUT);
			return(exit_status);
		}
		else if ( (rsiz = read_mop(MOP_RTIMOUT, in_msg, sizeof(in_msg))) <= 0 )
		{
			if ( errno == ETIMEDOUT )
			{
				if ( (rsiz = send_load(load_parm->db_parm.node_physaddr, dnaddr,
					 devname, devunit, msg, msg_siz, in_msg, sizeof(in_msg),
					 &target)) <= 0 )
				{
					dup2(stdin_sav, STDIN);
					dup2(stdout_sav, STDOUT);
					return(exit_status);
				}
			}
			else
			{
				dup2(stdin_sav, STDIN);
				dup2(stdout_sav, STDOUT);
				exit_status = EXIT_RCVFAIL;
				return(exit_status);
			}
		}
		else
		{
			continue;
		}
	}

	/*
	 * should only get here if load retry attempts are exhausted.
	 */
	dup2(stdin_sav, STDIN);
	dup2(stdout_sav, STDOUT);
	exit_status = EXIT_RCVFAIL;
	return(exit_status);
}

/*
 *		a c c e s s _ d b
 *
 * This subroutine accesses the nodes data base for parameters that
 * weren't specified by the user but are necessary to perform the
 * trigger or load. 
 *
 *
 * Inputs:
 *	lp		= ptr to structure containing load parameters.
 *	boot_type	= TRIGGER_NODE or LOAD_NODE
 *	
 *	
 * Outputs:
 *	returns		= if success, 0 is returned. Otherwise, -1 is
 *			  returned.
 *	exit_status	= written into only if error.
 *	load_parm	= structure containing load parameters added by
 *			  access_db if appropriate.
 */
access_db( lp, boot_type )
register struct load_parameters *lp;
u_short boot_type;
{
	struct nodeent *get_node, *getnodethehardway();

	/*
	 * access entry from nodes data base and verify authorization
	 */
	if ( lp->option == LOPTION_CKT &&  (LPAR_PHYSADDR & lp->parm_present) )
	{
		if ( ! ( get_node = getnodethehardway(lp->db_parm.node_physaddr) ) )
		{
			exit_status = EXIT_DBACCESSFAIL;
			return(-1);
		}
	}
	else if ( (lp->option == LOPTION_NODE) && strlen(lp->load_node_name) )
	{
		if ( ! ( get_node = getnodebyname(lp->load_node_name) ) )
		{
			exit_status = EXIT_DBACCESSFAIL;
			return(-1);
		}
	}
	else if ( (lp->option == LOPTION_NODE) &&  lp->load_node_addr != NULL )
	{
		if ( ! (get_node = getnodebyaddr(&lp->load_node_addr, sizeof(lp->load_node_addr), AF_DECnet) ) )
		{
			exit_status = EXIT_DBACCESSFAIL;
			return(-1);
		}
	}
	else
	{
		exit_status = EXIT_NODBKEY;
		return(-1);
	}


	/*
	 * make sure necessary parameters are present.  If not,
	 * fetch them from the data base.
	 */
	switch ( boot_type )
	{

		case LOAD_NODE:
		case TRIGGER_NODE:
			if ( fetch_physaddr(lp, get_node) < 0 )
			{
				exit_status = EXIT_NOAUTHORIZATION;
				return(-1);
			}
			if ( fetch_sercirc(lp, get_node) < 0 )
			{
				exit_status = EXIT_NOAUTHORIZATION;
				return(-1);
			}
			if ( fetch_serpasswd(lp, get_node) < 0 )
			{
				exit_status = EXIT_NOAUTHORIZATION;
				return(-1);
			}
			fetch_dnaddr(lp, get_node);
			break;

	}

	return(0);
}

/*
 *		f e t c h _ d n a d d r
 *
 * This subroutine accesses the nodes data base for the DECnet address 
 * if it is unspecified in the load parameters structure.  
 *
 *
 * Inputs:
 *	lp		= ptr to structure containing load parameters.
 *	gn		= ptr to structure containing data base entry.
 *	
 *	
 * Outputs:
 *	lp		= structure containing physical addr load parameter 
 *			  added if appropriate.
 */
fetch_dnaddr(lp, gn)
register struct load_parameters *lp;
register struct nodeent *gn;
{
	register u_char *cp;
	u_char *get_param();
	u_short psiz;

	if ( ! (lp->parm_present & LPAR_ADDR) )
	{
		bcopy(gn->n_addr, &lp->db_parm.node_addr, sizeof(lp->db_parm.node_addr));
		lp->parm_present |= LPAR_ADDR;
	}
	return;
}

/*
 *		f e t c h _ p h y s a d d r
 *
 * This subroutine accesses the nodes data base for the hardware address 
 * if it is unspecified in the load parameters structure.  
 *
 *
 * Inputs:
 *	lp		= ptr to structure containing load parameters.
 *	gn		= ptr to structure containing data base entry.
 *	
 *	
 * Outputs:
 *	returns		= if success, 0 is returned. Otherwise, -1 is
 *			  returned.
 *	lp		= structure containing physical addr load parameter 
 *			  added if appropriate.
 */
fetch_physaddr(lp, gn)
register struct load_parameters *lp;
register struct nodeent *gn;
{
register u_char *cp;
u_char *get_param();
u_short psiz;

	if ( lp->parm_present & LPAR_PHYSADDR )
	{
		return(0);
	}

	if ( (cp = get_param(gn->n_params, NODE_HARDWARE_ADDR, &psiz)) )
	{
		bcopy(cp, lp->db_parm.node_physaddr, psiz);
		lp->parm_present |= LPAR_PHYSADDR;
		return(0);
	}
	return(-1);
}

/*
 *		f e t c h _ s e r c i r c
 *
 * This subroutine accesses the nodes data base for the service circuit
 * if it is unspecified in the load parameters structure.  
 *
 *
 * Inputs:
 *	lp		= ptr to structure containing load parameters.
 *	gn		= ptr to structure containing data base entry.
 *	
 *	
 * Outputs:
 *	returns		= if success, 0 is returned. Otherwise, -1 is
 *			  returned.
 *	lp		= structure containing service circuit parameter 
 *			  added if appropriate.
 */
fetch_sercirc(lp, gn)
register struct load_parameters *lp;
register struct nodeent *gn;
{
register u_char *cp;
u_char *get_param();
u_short psiz;

	if ( lp->parm_present & LPAR_SCKT )
	{
		return(0);
	}

	if ( (cp = get_param(gn->n_params, NODE_SERVICE_CKT, &psiz)) )
	{
		bcopy(cp, lp->db_parm.node_parm.rnm_sercirc, psiz);
		lp->parm_present |= LPAR_SCKT;
		return(0);
	}
	return(-1);
}

/*
 *		f e t c h _ s e r p a s s w d
 *
 * This subroutine accesses the nodes data base for the service password 
 * if it is unspecified in the load parameters structure.  If not found
 * in the data base, 0 is assumed.
 *
 *
 * Inputs:
 *	lp		= ptr to structure containing load parameters.
 *	gn		= ptr to structure containing data base entry.
 *	
 *	
 * Outputs:
 *	returns		= 0 is returned. 
 *	lp		= structure containing service password parameter 
 *			  added if appropriate.
 */
fetch_serpasswd(lp, gn)
register struct load_parameters *lp;
register struct nodeent *gn;
{
register u_char *cp;
u_char *get_param();
u_short psiz;

	if ( lp->parm_present & LPAR_SPSWD )
	{
		return(0);
	}

	if ( (cp = get_param(gn->n_params, NODE_SERVICE_PSWD, &psiz)) )
	{
		bcopy(cp, lp->db_parm.node_parm.rnm_serpasswd, psiz);
		lp->parm_present |= LPAR_SPSWD;
		return(0);
	}
	bzero(lp->db_parm.node_parm.rnm_serpasswd, sizeof(lp->db_parm.node_parm.rnm_serpasswd));
	lp->parm_present |= LPAR_SPSWD;
	return(0);
}

/*
*		g e t n o d e t h e h a r d w a y
*
* Version: 1.0 - 8/1/85
*
*
* Description:
*	Fetch a nodes data base entry as a function of Ethernet
*	hardware address.
*
* Inputs:
*	enet_addr	= pointer to hardware address to be used as
*			  data base key.
*
*
* Outputs:
*	returns		if failure, NULL is returned.
*			if success, a pointer to the data base entry 
*			is returned.
*
* Notes:
*
*/
struct nodeent *getnodethehardway( enet_addr )
register char *enet_addr;
{
	register u_char *cp;
	u_short psiz;
	struct nodeent *node;

	if ( setnodeent(0) == -1 )
	{
		return(NULL);
	}

	while ( (node = getnodeent()) != NULL )
	{
		if ( node->n_params != NULL )
		{
			if ( (cp = get_param(node->n_params, NODE_HARDWARE_ADDR, &psiz)) )
			{
				if ( psiz != MDB_NA_MAXL )
				{
					endnodeent();
					return(NULL);
				}
				if ( bcmp(cp, enet_addr, psiz) == NULL)
				{
					endnodeent();
					return(node);
				}
			}
		}
	}
	endnodeent();
	return(NULL);
}

/*
*		g e t _ p a r a m
*
* Version: 1.0 - 4/9/85
*
*
* Description:
*	Fetch a parameter from the parameter list in the node data
*	base entry.
*
* Inputs:
*	ptr	= pointer to beginning of parameter list.
*	parm	= the desired parameter.
*	size	= pointer to where size of parameter is to be placed.
*
*
* Outputs:
*	returns	pointer to parameter field upon success, NULL otherwise.
*	size	= size of the parameter field.
*
* Notes:
*
*/
u_char *get_param(ptr, parm, size)
u_char *ptr;
register u_char parm;
register u_short *size;
{
	register u_short psiz;
	register u_char *cp;

	if ( ptr == NULL )
	{
		return(NULL);
	}

	cp = ptr;
	while ( *(u_short *) cp != NODE_END_PARAM )
	{
		if ( *(u_short *) cp == parm )
		{
			cp += NODE_CSIZ;
			*(u_short *) size = *(u_short *) cp;
			cp += NODE_SSIZ;
			return(cp);
		}
		else
		{
			cp += NODE_CSIZ;
			psiz = *(u_short *) cp;
			cp += (psiz + NODE_SSIZ);
		}
	}

	return(NULL);
}

/*
 *		s p a w n _ d u m p l o a d
 *
 * Version: 1.0 - 8/5/85
 *
 *
 * Description:
 *	This routine modifies the program request using user supplied
 *	parameters and spawns a copy of mop_dumpload to do the actual
 *	loading.  This routine waits for the loader to complete before
 *	returning to the caller.
 *
 * Inputs:
 *	caller		= pointer to string containing name of calling program
 *	lp		= ptr to structure containing user supplied parameters
 *	in_msg		= program request from target node
 *	rsiz		= size of program request
 *	taddr		= target address
 *	envp		= environment passed into load
 *
 *
 * Outputs:
 *	returns		0 if success, -1 if failure
 *	exit_status	= status from mop_dumpload
 *
 * Notes:
 *
 */
spawn_dumpload(caller, lp, in_msg, rsiz, taddr, envp)
u_char *caller;
register struct load_parameters *lp;
u_char *envp, *in_msg, *taddr;
int rsiz;
{
	int i, j;
	u_char pgm_rqst[MOM_MSG_SIZE];
	u_char hex_message[MOM_MSG_SIZE*2];
	u_char taddr_ascii[20], laddr_ascii[20], laddr[6];
	u_char host_target[2048], *htinfo = NULL;
	union wait child_status;
	static u_char load_stage;
	static u_char init_load_stage_var = 1;

	/*
	 * keep track of what's already been loaded
	 */
	if ( init_load_stage_var )
	{
		load_stage = in_msg[PRQ_PGMTYPE_IDX];
		init_load_stage_var = 0;
	}
	if ( load_stage > in_msg[PRQ_PGMTYPE_IDX] )
	{
		return(0);
	}


	/*
	 * fork child process
	 */
	if ( fork() )
	{
		if ( wait(&child_status) < 0 )
		{
			exit_status = EXIT_WAITFAIL;
			return(-1);
		}

		if ( child_status.w_termsig == 0 )
		{
			exit_status = child_status.w_retcode;
		}
		else
		{
			exit_status = EXIT_WAITFAIL;
			return(-1);
		}

		if ( (exit_status == EXIT_GOODSEC) || (exit_status == EXIT_GOODTER) || (exit_status == EXIT_GOODSYS) || (exit_status == EXIT_GOOD))
		{
			load_stage = in_msg[PRQ_PGMTYPE_IDX] + 1;
			return(0);
		}
		else
		{
			return(-1);
		}
	}

	/*
	 * Only child makes it to here. 
	 * Make sure priority of child is lower than parent
	 */
	if ( setpriority(PRIO_PROCESS, getpid(), 1) < 0 )
	{
		mop_syslog("mop_forceload", "child can't lower priority", 1, LOG_INFO);
	}

	/*
	 * override parameters in load request with user supplied parameters
	 */
	i = 0;
	if ( j = strlen(caller) )
	{
		sprintf(host_target+i, "5%2x%s", j, caller);
		i += (j + 3);
	}
	if ( lp->parm_present & LPAR_HOST )
	{
		sprintf(host_target+i, "004%4x", lp->db_parm.node_parm.rnm_host.n_addr);
		i += 7;
		if ( ( j = strlen(lp->db_parm.node_parm.rnm_host.n_name)) > 0 && j <= MDB_NN_MAXL )
		{
			sprintf(host_target+i, "1%2x%s", j, lp->db_parm.node_parm.rnm_host.n_name);
			i += (j+3);
		}
	}
	if ( lp->parm_present & LPAR_ADDR )
	{
		sprintf(host_target+i, "204%4x", lp->db_parm.node_addr);
		i += 7;
	}
	if ( lp->parm_present & LPAR_NAME )
	{
		if ( ( j = strlen(lp->db_parm.node_name)) > 0 && j <= MDB_NN_MAXL )
		{
			sprintf(host_target+i, "3%2x%s", j, lp->db_parm.node_name);
			i += (j+3);
		}
	}
	rsiz = user_parameters(lp, in_msg, rsiz, pgm_rqst, host_target);
	if ( (i = strlen(host_target)) > 0 )
	{
		j = -1;
		while ( host_target[++j] != NULL )
			if ( host_target[j] == ' ' )
				host_target[j] = '0';
		htinfo = host_target;
	}

	/*
	 * translate program request message from binary to hex image
	 */
	for ( i = 0, j = 0; i < rsiz; i++, j += 2 )
	{
		sprintf(hex_message+j, "%2x", pgm_rqst[i]);
		if ( hex_message[j] ==  ' ' )
		{
			hex_message[j] = '0';
		}
	}


	/*
	 * translate local address from binary to hex image
	 */
	bzero(laddr, sizeof(laddr));
	translate_addr( laddr_ascii, laddr );


	/*
	 * translate destination address from binary to hex image
	 */
	translate_addr( taddr_ascii, taddr );

	/*
	 * exec dump/load program with sock tied to stdin and stdout.
	 */
	execle("/usr/lib/dnet/mop_dumpload", "mop_dumpload", hex_message, taddr_ascii, laddr_ascii, htinfo, NULL, envp);
	/*
	 * If here, exec failed.
	 */
	sprintf(errbuf, "can't exec /usr/lib/dnet/mop_dumpload" );
	mop_syslog("mop_forceload", errbuf, 1, LOG_INFO);
	exit(EXIT_CANTEXEC);

}

/*
 *		u s e r _ p a r a m e t e r s
 *
 * Version: 1.0 - 8/5/85
 *
 *
 * Description:
 *	Override parameters in program request with user specified
 *	parameters.  Also, store parameters not included in a 
 *	program request in a list pointed to by htinfo to be passed
 *	to mop_dumpload.
 *
 * Inputs:
 *	lp		= ptr to user supplied parameters
 *	old_rqst	= ptr to program request from target
 *	osiz		= size of old program request
 *	new_rqst	= ptr to buffer where modified program request
 *			  is to be placed.
 *	htinfo		= character string where additional host and target
 *			  information is to be stored.
 *
 *
 * Outputs:
 *	returns 	= size of new program request
 *
 * Notes:
 *
 */
user_parameters(lp, old_rqst, osiz, new_rqst, htinfo)
register struct load_parameters *lp;
u_char *old_rqst, *new_rqst;
int osiz;
u_char *htinfo;
{
	char filename[17], pathname[1024];
	u_short nsize;
	int htidx = strlen(htinfo);
	int i = MOP_CODE_IDX;
	int j = MOP_CODE_IDX;

	new_rqst[i++] = old_rqst[j++];
	if ( lp->parm_present & LPAR_SDEV )
	{
		new_rqst[i++] = lp->db_parm.node_parm.rnm_serdev;
	}
	else
	{
		new_rqst[i++] = old_rqst[j++];
	}
	new_rqst[i++] = old_rqst[j++];

	switch ( (new_rqst[i++] = old_rqst[j++]) )
	{
		case MOP_SECONDARY:
			if ( lp->parm_present & LPAR_SFILE )
			{
				parse_filename(lp->db_parm.node_parm.rnm_secload, pathname, filename, sizeof(filename));
				if ( strlen(pathname) )
				{
					htinfo[htidx++] = '4';
					sprintf(htinfo+htidx, "%2x%s", strlen(pathname), pathname);
					htidx += (strlen(pathname) + 2);
				}
				new_rqst[i++] = (u_char) strlen(filename);
				strcpy(&new_rqst[i], filename);
				i += (u_int) new_rqst[i-1];
				if ( (char) old_rqst[j++] > 0 )
				{
					j += (u_int) old_rqst[j-1];
				}
			}
			else
			{
				if ( ( (char) (new_rqst[i++] = old_rqst[j++])) > 0 )
				{
					bcopy(&old_rqst[j], &new_rqst[i], old_rqst[j-1]);
					i += (u_int) new_rqst[i-1];
					j += (u_int) old_rqst[j-1];
				}
			}
			break;

		case MOP_TERTIARY:
			if ( lp->parm_present & LPAR_TFILE )
			{
				parse_filename(lp->db_parm.node_parm.rnm_terload, pathname, filename, sizeof(filename));
				if ( strlen(pathname) )
				{
					htinfo[htidx++] = '4';
					sprintf(htinfo+htidx, "%2x%s", strlen(pathname), pathname);
					htidx += (strlen(pathname) + 2);
				}
				new_rqst[i++] = (u_char) strlen(filename);
				strcpy(&new_rqst[i], filename);
				i += (u_int) new_rqst[i-1];
				if ( (char) old_rqst[j++] > 0 )
				{
					j += (u_int) old_rqst[j-1];
				}
			}
			else
			{
				if ( ( (char) (new_rqst[i++] = old_rqst[j++]) ) > 0 )
				{
					bcopy(&old_rqst[j], &new_rqst[i], old_rqst[j-1]);
					i += (u_int) new_rqst[i-1];
					j += (u_int) old_rqst[j-1];
				}
			}
			break;

		case MOP_SYSTEM:
			if ( lp->parm_present & LPAR_LFILE )
			{
				parse_filename(lp->db_parm.node_parm.rnm_loadfile, pathname, filename, sizeof(filename));
				if ( strlen(pathname) )
				{
					htinfo[htidx++] = '4';
					sprintf(htinfo+htidx, "%2x%s", strlen(pathname), pathname);
					htidx += (strlen(pathname) + 2);
				}
				new_rqst[i++] = (u_char) strlen(filename);
				strcpy(&new_rqst[i], filename);
				i += (u_int) new_rqst[i-1];
				if ( ( (char) old_rqst[j++] ) > 0 )
				{
					j += (u_int) old_rqst[j-1];
				}
			}
			else if ( lp->parm_present & LPAR_DGFILE )
			{
				parse_filename(lp->db_parm.node_parm.rnm_diagfile, pathname, filename, sizeof(filename));
				if ( strlen(pathname) )
				{
					htinfo[htidx++] = '4';
					sprintf(htinfo+htidx, "%2x%s", strlen(pathname), pathname);
					htidx += (strlen(pathname) + 2);
				}
				new_rqst[i++] = (u_char) strlen(filename);
				strcpy(&new_rqst[i], filename);
				i += (u_int) new_rqst[i-1];
				if ( (char) old_rqst[j++] > 0 )
				{
					j += (u_int) old_rqst[j-1];
				}
			}
			else
			{
				if ( ( (char) (new_rqst[i++] = old_rqst[j++]) ) > 0 )
				{
					bcopy(&old_rqst[j], &new_rqst[i], old_rqst[j-1]);
					i += (u_int) new_rqst[i-1];
					j += (u_int) old_rqst[j-1];
				}
			}
			break;

		default:
			break;

	}
	bcopy(&old_rqst[j], &new_rqst[i], (osiz - j));
	nsize = (osiz - j) + (i - 2);
	bcopy(&nsize, new_rqst, sizeof(nsize));
	return(nsize + 2);
}

/*
 *		p a r s e _ f i l e n a m e
 *
 * Version: 1.0 - 8/14/85
 *
 *
 * Description:
 *	This routine breaks a file specification into pathname and
 *	filename.
 *
 * Inputs:
 *	filespec		= string containing file specification
 *	pathname		= ptr to string where pathname is to be placed
 *	filename		= ptr to string where filename is to be placed
 *	fnsize			= maximum number of bytes to be stored in filename
 *
 *
 * Outputs:
 *	pathname		= ptr to string where pathname is to be placed
 *	filename		= ptr to string where filename is to be placed
 *
 * Notes:
 *
 */
parse_filename(filespec, pathname, filename, fnsiz)
char *filespec, *pathname, *filename;
u_short fnsiz;
{
	int i, j;

	pathname[0] = filename[0] = NULL;

	if ( filespec[0] != '/' )
	{
		bcopy(filespec, filename, ((i = strlen(filespec)) < fnsiz ? i : ((i = fnsiz) - 1)) ); 
		filename[i] = NULL;
	}
	else
	{
		i = j = strlen(filespec);
		while ( filespec[--i] != '/' ) ;
		bcopy(filespec, pathname, i);
		pathname[i] = NULL;
	
		bcopy(filespec+i+1, filename, ((j -= i+1) < fnsiz ? j : ((j = fnsiz) - 1)) ); 
		filename[j] = NULL;
	}
	return;
}

/*
*		t r a n s l a t e _ a d d r
*
*
* Version:	1.0 - 1/10/85
*
* Description:
* 	Translate source address from binary to hex image
*
* Inputs:
*	baddr		= binary address to be translated.
*
* Outputs:
*	xaddr		= tranlated hex address.
*
* Notes:
*
*/
translate_addr( xaddr, baddr )
register u_char *xaddr;
register u_char *baddr;
{
	int i, j;
	for ( i = 0, j = 0; i < DLI_EADDRSIZE; i++, j += 3 )
	{
		sprintf(xaddr+j, "%2x-", baddr[i]);
		if ( xaddr[j] ==  ' ' )
		{
			xaddr[j] = '0';
		}
	}
	xaddr[ASCII_EADDRSIZE-2] = NULL;
	return;
}
