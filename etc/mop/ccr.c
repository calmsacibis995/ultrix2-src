#ifndef lint
static	char	*sccsid = "@(#)ccr.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * MODULE - CCR.C, mainline code for Console Carrier Requestor
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
#include <signal.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdnet/dn.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"
#include "nmgtdb.h"
#include "mopdb.h"

static int 	ccr_init(),
			priv_user(),
			setup_termio(),
			reset_termio();

struct load_parameters ccr_load_params,
						*ptr_params;

struct sgttyb save_sgttyb;
struct tchars save_tchars;

char 	secondary_loader[] = "plutowl.sys";
char	system_image[] = "plutocc.sys";

struct dn_naddr	*ccr_node_addr;
int sock;

extern int dli_open();
extern int get_dbparams();
extern int reserve_console();
extern int release_console();
extern int process_command();

/*
 *			m a i n
 *
 *
 * Inputs:
 *		argc	the number of command line arguments the program was
 *				invoked with.
 *		argv	pointer to the array of arguments
 *		envp	pointer to an array of environment variables.
 *
 * Outputs:
 *		0		if the program completes successfully.
 *		1		if the program exits becuase of an error.
 * Notes:
 *		None.
 */

int main(argc,argv,envp)
int 	argc;
char 	*argv[],
		*envp[];
     
{

	int status;

	/*
	 * 	Perform any necessary initialization
	 */
	ccr_init();

	/*
	 *	Make sure user is privileged before going any further
	 */
	if (status = priv_user())
	{

		/*
 	 	 * 	Parse the command line as entered by user
	 	 */
		if (status = parse_command_line(argc,argv))

			/*
	 		 * 	Get needed parameters from volatile database, if they weren't 
	 		 * 	suppplied in the command line.
	 		 */	
			if(status = get_dbparams(ptr_params))

				/*
	 			 * 	Open a dli socket for message transmission
 	 			 */
				if(status = dli_open(&sock)) 
				{
					/*
	 				 * 	Reserve the remote console 
	 				 */
					if (status = reserve_console(sock,ptr_params,argv[0],envp)) 
					{ 
						setup_termio(); 
						status = process_command();
						reset_termio();
						release_console(sock);
					
					}
					close(sock);
				}
	}

	exit(!(status));
}

/*
 *
 *				c c r _ i n i t
 *
 *	Performs any necessary initialization.
 *
 * Inputs:
 *		None.
 * Outputs:
 *		None.
 *
 */

static int ccr_init()
{

	/*
	 * Initialize load file names 
	 */

	ptr_params = &ccr_load_params;
	bcopy(secondary_loader,ptr_params->db_parm.node_parm.rnm_secload,sizeof(secondary_loader));
	ptr_params->parm_present |= LPAR_SFILE;
	bcopy(system_image,ptr_params->db_parm.node_parm.rnm_loadfile,sizeof(system_image));
	ptr_params->parm_present |= LPAR_LFILE;


	/* 
	 * Initialize the option word for the loader 
	 */

	ptr_params->option = LOPTION_NODE;


	signal(SIGTSTP,SIG_IGN); 
	signal(SIGINT,SIG_IGN); 
	signal(SIGHUP,SIG_IGN); 
	

}

/*
 *
 *			s e t u p _ t e r m i o	
 *
 *	Set up the terminal to read characters as they are typed and not
 *	to echo them. Also change stdout to process non-buffered i/o.
 *
 * Inputs:
 *		None.
 *
 * Outputs:
 *		None.
 *
 *
 */
static int setup_termio()
{
	struct sgttyb p_sgttyb;
	struct tchars p_tchars;


	/*
	 * Get the current terminal characteristics
	 */
	ioctl(0,TIOCGETP,&save_sgttyb);
	ioctl(0,TIOCGETC,&save_tchars);

	p_sgttyb = save_sgttyb;
	p_tchars = save_tchars;

	/*
	 * Put terminal into CBREAK mode
	 */
	p_sgttyb.sg_flags |= CBREAK; 
	p_sgttyb.sg_flags &= ~(ECHO|CRMOD);
	ioctl(0,TIOCSETP,&p_sgttyb); 


	/*
	 * Disable special characters on input
	 * p_tchars.t_startc = p_tchars.t_stopc =  	 
	 */
	p_tchars.t_intrc = p_tchars.t_quitc = p_tchars.t_eofc = -1;
	ioctl(0,TIOCSETC,&p_tchars);

	setbuf(stdout,NULL);				

	signal(SIGQUIT,reset_termio); 
}

/*
 *
 *			r e s e t _ t e r m i o	
 *
 *	Reset the terminal back to its original mode.
 *
 * Inputs:
 *		- sig: signal
 *		- code: signal parameter
 *		- scp: pointer to signal context
 *
 * Outputs:
 *		None.
 *
 */
static int reset_termio(sig,code,scp)
int sig, code;
struct sigcontext *scp;
{

	/* 
	 * Reset terminal back to original mode
	 */
	ioctl(0,TIOCSETP,&save_sgttyb);
	ioctl(0,TIOCSETC,&save_tchars);


}

/*
 *					 p r i v _ u s e r
 *
 *  Check to see if user is priviledged
 *
 *  Input:  Nothing
 *
 *  Output: SUCCESS if priviledged,
 *			FAIL otherwise
 */

static int priv_user()

{
	/*
	 *  ccr can only be used by privileged user.
	 */

	if (getuid())
	{
		print_message(ccr_PRIV);	
		return(FAIL);
	}
	else 
		return(SUCCESS);

}

/*
 *		p a r s e _ c o m m a n d _ l i n e
 *
 * 	Parse the command line entered by the user.
 *
 * Inputs:
 *		argc	the number of command line arguments the program was
 *				invoked with.
 *		argv	pointer to the array of arguments
 *
 * Outputs:
 *		SUCCESS	if command line parsed successfully
 *		else FAIL.
 *
 */

int parse_command_line(argc,argv)
int		argc;
char 	*argv[];

{

	int argn = 1;

	if (argc < 2)
	{
		fprintf(stderr,"ccr: usage\n");
		fprintf(stderr,"   ccr -n node-id\n");
		fprintf(stderr,"       [-p service password]\n");
		fprintf(stderr,"       [-c service circuit]\n");
		fprintf(stderr,"       [-h hardware address]\n");
		return(FAIL);
	}

	/*	
	 * If only 2 arguments on command line assume the second is the
	 * node id
	 */
	if (argc == 2)
	{
		if (!(parse_node_id(*++argv)))
			return(FAIL);
	}	
	else
	{
		*++argv;					/* Bump past "ccr" */
		while (argn++ < argc)
		{
			switch (argv[0][1]) {
				case('n'):
					if (!(parse_node_id(*++argv)))
						return(FAIL);
					break;
				case('c'):
					if(!(parse_serv_circuit(*++argv)))
						return(FAIL);
					break;
				case('p'):
					if(!(parse_serv_passwd(*++argv)))
						return(FAIL);
					break;
				case('h'):
					if(!(parse_hard_addr(*++argv)))
						return(FAIL);
					break;

				default:
					fprintf(stderr,"ccr: usage\n");
					fprintf(stderr,"   ccr -n node-id\n");
					fprintf(stderr,"       [-p service password]\n");
					fprintf(stderr,"       [-c service circuit]\n");
					fprintf(stderr,"       [-h hardware address]\n");
					return(FAIL);
			}

			*++argv;
			argn++;
		
		}
	
	}
	return(SUCCESS);
}

/*
 *		p a r s e _ n o d e _ i d
 *
 *	Parse the node id entered by the user.
 *
 * Inputs:
 *		argument	points to the node id.
 *
 * Outputs:
 *		SUCCESS	if node id parsed successfully
 *		else FAIL.
 *
 */
parse_node_id(argument)
char	*argument;
{


	/* Is the node id a node_address? */

	if ( (ccr_node_addr = dnet_addr(argument)) == NULL)
	{
		/* 
		* No, therefore it must be a node name. Check to see
		* if it is a valid node name.
		*/
		if (strlen(argument) > MXBYT_NODNAM)
		{
			fprintf(stderr,"ccr: node name %s is too long\n");
			return(FAIL);
		}
		else
			bcopy(argument,ptr_params->load_node_name,strlen(argument));

	}
	else
		/* 
		 * Move node address into load parameters structure for
		 * future use 
		 */
		bcopy(ccr_node_addr->a_addr,&ptr_params->load_node_addr,sizeof(ptr_params->load_node_addr));


	return(SUCCESS);

}

/*
 *		p a r s e _ s e r v _ c i r c u i t
 *  
 *
 *	Parse the service circuit.
 *
 * Inputs:
 *		argument	points to the service circuit.
 *
 * Outputs:
 *		SUCCESS	if service circuit parsed successfully
 *		else FAIL.
 *
 */

parse_serv_circuit(argument)
char	*argument;

{
	int i,j;
	u_char 	temp_name[MXBYT_SERCIR],
			*cc_tempname;

	if ((i=strlen(argument)) > MXBYT_SERCIR )
	{
		fprintf(stderr,"ccr: service circuit name too long\n");
		return(FAIL);
	}
	else
	{
		/* Convert device name to uppercase if necessary */

		cc_tempname = temp_name;
		for(j=0; j < i; j++)
			*cc_tempname++ = islower(*argument) ? toupper(*argument++) : *argument++;

		bcopy(temp_name,ptr_params->db_parm.node_parm.rnm_sercirc,i);
		ptr_params->parm_present |= LPAR_SCKT;
		return(SUCCESS);
	}
}


/*
 *		p a r s e _ s e r v _ p a s s w d
 *
 *	Parse the service password.
 *
 * Inputs:
 *		argument	points to the service password.
 *
 * Outputs:
 *		SUCCESS	if service password parsed successfully
 *		else FAIL.
 *
 */
parse_serv_passwd(argument)
char	*argument;
{

	int i,j,k,l;
	u_char password[MXBYT_SERPAS], *pswd;


	if ((i=strlen(argument)) > MXBYT_SERPAS )
	{
		fprintf(stderr,"ccr: service password too long\n");
		return(FAIL);
	}
	else
	{
		pswd = password;
		if ( i%2 )
		{
			*pswd++ = '0';
		}
		j = 0;
		while( (*pswd++ = argument[j++]) != NULL ) ;
		for( j = 0, k = (i-1)/2; j < i; j += 2 )
		{
			sscanf(&password[j], "%2x", &l);
			ptr_params->db_parm.node_parm.rnm_serpasswd[k--] = (u_char) l;
		}

		ptr_params->parm_present |= LPAR_SPSWD;
		return(SUCCESS);
	}

}

/*
 *		p a r s e _ h a r d _ a d d r
 *
 *	Parse the hardware address.
 *
 * Inputs:
 *		argument	points to the hardware address.
 *
 * Outputs:
 *		SUCCESS	if hardware address parsed successfully
 *		else FAIL.
 */
parse_hard_addr(argument)
char	*argument;
{

	int i,j,k,l;

	if ((i=strlen(argument)) != (MXBYT_HARADD_ASCII))
	{
		fprintf(stderr,"ccr: hardware address format error\n");
		return(FAIL);
	}
	else
	{
		for( j = 0, k = 0; j < i; j += 3 )
		{
			if ( argument[j+2] != '-' && (j+2) < i )
			{
				fprintf(stderr, "ccr: hardware address format error\n");
				return(FAIL);
			}

			sscanf(&argument[j], "%2x", &l);
			ptr_params->db_parm.node_parm.rnm_hwaddr[k++] = (u_char) l;
		}
		ptr_params->parm_present |= LPAR_HWADDR;
		return(SUCCESS);
	}

}


