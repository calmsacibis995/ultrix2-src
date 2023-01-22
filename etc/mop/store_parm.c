#ifndef lint
static char *sccsid = "@(#)store_parm.c	1.4	ULTRIX	10/3/86";
#endif lint

/*
 * Program storparm.c,  Module MOP 
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
#include <ctype.h>
#include <sys/socket.h> 
#include <sys/errno.h>
#include <netdnet/dnetdb.h>
#include <netdnet/dn.h>

#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_proto.h"
#include <netdnet/node_params.h>

extern int errno;
/*
 *		s t o r e _ p a r m
 *
 * Store a parameter in its correct location in the mop_nodeent structure.
 *
 *
 * Inputs:
 *	cmd		= flag indicating which parameter was passed.
 *	parm		= string containing the parameter.
 *	load_parm	= pointer to structure where parameter is to be placed.
 *	_flags		= character string containing switches that are 
 *			  appropriate to the calling program.
 *	caller		= name of calling program.
 *	
 * Outputs:
 *	new_node	= structure where parameter was placed.
 */
store_parm( cmd, parm, load_parm, flags, caller )
u_char cmd;
u_char *parm;
register struct load_parameters *load_parm;
register u_char *flags;
register char *caller;
{
	struct nodeent *get_node;
	struct dn_naddr *address;
	int i, j, k, l;
	u_char password[17], *pswd;
	u_char service_switch;

	i = 0;
	while ( flags[i] != NULL && flags[i] != cmd ) i++;

	if ( flags[i] == NULL )
	{
		fprintf(stderr, " %s: -%c is an invalid flag\n", caller, cmd);
		return(-1);
	}
		

	switch ( cmd )
	{
		case 'a':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_ADDR;
				break;
			}
			if ( (address = dnet_addr(parm)) == 0 )
			{
				fprintf(stderr, "%s: %s is an invalid node address\n", caller, parm);
				return(-1);
			}
			bcopy(address->a_addr, &load_parm->db_parm.node_addr, sizeof(u_short));
			load_parm->parm_present |= LPAR_ADDR;
			break;

		case 'A':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_HOST;
				break;
			}
			if ( (address = dnet_addr(parm)) == 0 )
			{
				fprintf(stderr, "%s: %s is an invalid host node address\n", caller, parm);
				return(-1);
			}
			bcopy(address->a_addr, &load_parm->db_parm.node_parm.rnm_host.n_addr, sizeof(u_short));
			if ( get_node = getnodebyaddr( address->a_addr, address->a_len, AF_DECnet ) )
			{
				bcopy(get_node->n_name, load_parm->db_parm.node_parm.rnm_host.n_name, strlen(get_node->n_name));

			}
			load_parm->parm_present |= LPAR_HOST;
			break;

		case 'c':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SCKT;
				break;
			}
			if ( (i = check_ckt(caller, parm)) < 0 )
			{
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_sercirc, i);
			load_parm->db_parm.node_parm.rnm_sercirc[i] = NULL;
			load_parm->parm_present |= LPAR_SCKT;
			break;

		case 'C':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_CPU;
				break;
			}
			if ( (i = fetch_cpu(caller, parm)) < 0 )
			{
				return(-1);
			}
			load_parm->db_parm.node_parm.rnm_cpu = (u_char) i;
			load_parm->parm_present |= LPAR_CPU;
			break;

		case 'd':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_DGFILE;
				break;
			}
			if ( (i = strlen(parm)) > nm_MAXFIELD )
			{
				fprintf(stderr, "%s: diagnostic image file name too long\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_diagfile, i);
			load_parm->db_parm.node_parm.rnm_diagfile[i] = NULL;
			load_parm->parm_present |= LPAR_DGFILE;
			break;

		case 'D':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_DUFILE;
				break;
			}
			if ( (i = strlen(parm)) > nm_MAXFIELD )
			{
				fprintf(stderr, "%s: dump file name too long\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_dumpfile, i);
			load_parm->db_parm.node_parm.rnm_dumpfile[i] = NULL;
			load_parm->parm_present |= LPAR_DUFILE;
			break;

		case 'h':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_HWADDR;
				break;
			}
			if ( (i = strlen(parm)) != (MDB_NA_MAXL*3 - 1) )
			{
				fprintf(stderr, "%s: incorrect hardware address size\n", caller);
				return(-1);
			}
			for( j = 0, k = 0; j < i; j += 3 )
			{
				if ( check_hex( &parm[j] ) < 0 )
				{
					fprintf(stderr, "%s: hardware address format error\n", caller);
					return(-1);
				}
				if ( parm[j+2] != '-' && (j+2) < i )
				{
					fprintf(stderr, "%s: hardware address format error\n", caller);
					return(-1);
				}

				sscanf(&parm[j], "%2x", &l);
				load_parm->db_parm.node_physaddr[k] = (u_char) l;
				load_parm->db_parm.node_parm.rnm_hwaddr[k++] = (u_char) l;
			}
			if ( (load_parm->db_parm.node_parm.rnm_hwaddr[0] & 1) )
			{
				fprintf(stderr, "%s: hardware address cannot be multicast\n", caller);
				return(-1);
			}
			load_parm->parm_present |= (LPAR_HWADDR | LPAR_PHYSADDR);
			break;

		case 'I':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SWID;
				break;
			}
			if ( (i = strlen(parm)) > 16 )
			{
				fprintf(stderr, "%s: invalid software id type size\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_swident, i);
			load_parm->db_parm.node_parm.rnm_swident[i] = NULL;
			load_parm->parm_present |= LPAR_SWID;
			break;

		case 'l': 
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_LFILE;
				break;
			}
			if ( (i = strlen(parm)) > nm_MAXFIELD ) 
			{
				fprintf(stderr, "%s: system image file name too long\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_loadfile, i);
			load_parm->db_parm.node_parm.rnm_loadfile[i] = NULL;
			load_parm->parm_present |= LPAR_LFILE;
			break;

		case 'N':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_HOST;
				break;
			}
			i = toupcase( parm );
			if ( ! valid_nname(parm) )
			{
				fprintf(stderr, "%s: %s is an invalid host node name\n", caller, parm);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_host.n_name, i);
			load_parm->db_parm.node_parm.rnm_host.n_name[i] = NULL;
			if ( !(get_node = getnodebyname( parm )) )
			{
				fprintf(stderr, "%s: can't find address for host node %s in data base\n", caller, parm);

				return(-1);
			}
			bcopy(get_node->n_addr, &load_parm->db_parm.node_parm.rnm_host.n_addr, sizeof(u_short));
			load_parm->parm_present |= LPAR_HOST;
			break;

		case 'n':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_NAME;
				break;
			}
			i = toupcase( parm );
			if ( ! valid_nname(parm) )
			{
				fprintf(stderr, "%s: invalid target node name\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_name, i);
			load_parm->db_parm.node_name[i] = NULL;
			load_parm->parm_present |= LPAR_NAME;
			break;

		case 'p':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SPSWD;
				break;
			}
			if ( (i = strlen(parm)) > 16 )
			{
				fprintf(stderr, "%s: service password too long\n", caller);
				return(-1);
			}
			pswd = password;
			if ( i%2 )
			{
				*pswd++ = '0';
			}
			j = 0;
			while( (*pswd++ = parm[j++]) != NULL ) ;
			for( j = 0, k = (i-1)/2; j < i; j += 2 )
			{
				if ( check_hex( &password[j] ) < 0 )
				{
					fprintf(stderr, "%s: service password is not a hex number\n", caller);
					return(-1);
				}
				sscanf(&password[j], "%2x", &l);
				load_parm->db_parm.node_parm.rnm_serpasswd[k--] = (u_char) l;
			}
			bzero(parm, i);
			load_parm->parm_present |= LPAR_SPSWD;
			break;

		case 's':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SFILE;
				break;
			}
			if ( (i = strlen(parm)) > (nm_MAXFIELD) )
			{
				fprintf(stderr, "%s: secondary load file name too long\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_secload, i);
			load_parm->db_parm.node_parm.rnm_secload[i] = NULL;
			load_parm->parm_present |= LPAR_SFILE;
			break;

		case 'S':
			if ( (i = toupcase(parm)) == 0 )
			{
				load_parm->parm_delete |= LPAR_SERVICE;
				break;
			}
			else if ( i < 2 || i > 3 )
			{
				fprintf(stderr, "%s: invalid service switch length\n", caller);
				return(-1);
			}
			else if ( bcmp(parm, "ON", i) == 0 )
			{
				service_switch = 0;
			}
			else if ( bcmp(parm, "OFF", i) == 0 )
			{
				service_switch = 1;
			}
			else
			{
				fprintf(stderr, "%s: invalid service switch\n", caller);
				return(-1);
			}
			bcopy(&service_switch, &load_parm->db_parm.node_parm.rnm_service, sizeof(service_switch));
			load_parm->parm_present |= LPAR_SERVICE;
			break;
		case 't':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_TFILE;
				break;
			}
			if ( (i = strlen(parm)) > (nm_MAXFIELD) )
			{
				fprintf(stderr, "%s: tertiary load file name too long\n", caller);
				return(-1);
			}
			bcopy(parm, load_parm->db_parm.node_parm.rnm_terload, i);
			load_parm->db_parm.node_parm.rnm_terload[i] = NULL;
			load_parm->parm_present |= LPAR_TFILE;
			break;

		case 'T':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SWTYPE;
				break;
			}
			if ( (i = check_swtype(caller, parm)) < 0 )
			{
				return(-1);
			}
			load_parm->db_parm.node_parm.rnm_swtype = (u_char) i;
			load_parm->parm_present |= LPAR_SWTYPE;
			break;

		case 'u':
			if ( strlen(parm) == 0 )
			{
				load_parm->parm_delete |= LPAR_SDEV;
				break;
			}
			if ( (i = check_dev(caller, parm)) < 0 )
			{
				return(-1);
			}
			load_parm->db_parm.node_parm.rnm_serdev = (u_char) i;
			load_parm->parm_present |= LPAR_SDEV;
			break;

		default:
			fprintf(stderr, "%s: illegal option\n", caller);
			return(-1);
			break;
	}

	return;
}

/*
 *				v a l i d _ n n a m e 
 *
 *  Check node name to see if valid.  Must be one to six upper case
 *  alphanumeric with at least one alpha character.
 *
 *  Input:  Node name
 *
 *  Output: nm_SUCCESS if valid node name, else
 *			nm_FAIL if invalid
 */

int 
valid_nname(node_name)
u_char node_name[nm_MAXNNAME + 1];

{
	int gotalpha = 0;
	int i = 0;

	while ((i < nm_MAXNNAME ) && (node_name[i] != '\0'))
	{
		if ((isupper(node_name[i]) != 0) || (isdigit(node_name[i]) != 0))
		{
			if (isalpha(node_name[i++]) != 0)
				gotalpha = 1;
		}
		else
			return(nm_FAIL);
	}

	if ((node_name[i] != '\0') || (gotalpha == 0))
		return(nm_FAIL);
	else return(nm_SUCCESS);
}

/*
 *			c h e c k _ c k t
 *
 *  Check ckt to see if valid
 *
 *  Input:  
 *	caller		= name of calling program
 *	parm		= parameter to be checked
 *
 *  Output: 
 *	returns		-1 if failure, 0 if success.
 */
check_ckt(caller, parm)
u_char *caller, *parm;
{
	toupcase(parm);
	if ( valid_line_name(parm) )
	{
		return(strlen(parm));
	}
	else
	{
		fprintf(stderr, "%s: %s is an invalid circuit id\n", caller, parm);
		return(-1);
	}
}

/*
 *			f e t c h _ c p u
 *
 *  Translate parameter string to integer indicating cpu type.
 *
 *  Input:  
 *	caller		= name of calling program
 *	parm		= parameter to be checked
 *
 *  Output: 
 *	returns		-1 if failure, cpu value if success.
 */
fetch_cpu(caller, parm)
u_char *caller, *parm;
{
	int i;

	if ( (i = strlen(parm)) < 3 )
	{
		fprintf(stderr, "%s: %s is an invalid cpu type\n", caller, parm);
		return(-1);
	}

	toupcase( parm );

	if ( bcmp(parm, "PDP", 3) == 0 )
	{
		if ( i < 4 )
		{
			fprintf(stderr, "%s: %s is an invalid cpu type\n", caller, parm);
			return(-1);
		}
		else if ( bcmp(parm+3, "11", (i-3)) == 0 )
		{
			return(1);
		}
		else if ( bcmp(parm+3, "8", (i-3)) == 0 )
		{
			return(0);
		}
		else
		{
			fprintf(stderr, "%s: %s is an invalid cpu type\n", caller, parm);
			return(-1);
		}
	}

	else if ( bcmp(parm, "VAX", i) == 0 )
	{
		return(3);
	}
	else if ( bcmp(parm, "DECSYSTEM1020", i) == 0 )
	{
		return(2);
	}
	else
	{
		fprintf(stderr, "%s: %s is an invalid cpu type\n", caller, parm);
		return(-1);
	}
}

/*
 *			c h e c k _ s w t y p e
 *
 *  Check software type to see if valid
 *
 *  Input:  
 *	caller		= name of calling program
 *	parm		= parameter to be checked
 *
 *  Output: 
 *	returns		-1 if failure, 0 if success.
 */
check_swtype(caller, parm)
u_char *caller, *parm;
{
	int i;

	if ( (i = strlen(parm)) < 3 )
	{
		fprintf(stderr, "%s: %s is an invalid software type\n", caller, parm);
		return(-1);
	}

	toupcase( parm );

	if ( bcmp(parm, "SECONDARY", i) == 0 )
	{
		return(MOP_SECONDARY);
	}
	else if ( bcmp(parm, "TERTIARY", i) == 0 )
	{
		return(MOP_TERTIARY);
	}
	else if ( bcmp(parm, "SYSTEM", i) == 0 )
	{
		return(MOP_SYSTEM);
	}
	else
	{
		fprintf(stderr, "%s: %s is an invalid software type\n", caller, parm);
		return(-1);
	}
}

/*
 *			c h e c k _ d e v
 *
 *  Translate device string to device number.
 *
 *  Input:  
 *	caller		= name of calling program
 *	parm		= parameter to be checked
 *
 *  Output: 
 *	returns		-1 if failure, device number if success.
 */
check_dev(caller, parm)
u_char *caller, *parm;
{
	int i;

	if ( (i = strlen(parm)) < 3 )
	{
		fprintf(stderr, "%s: %s is an invalid device type\n", caller, parm);
		return(-1);
	}

	toupcase( parm );

	if ( bcmp(parm, "UNA", i) == 0 )
	{
		return(MOP_DEVTYP_UNA);
	}
	else if ( bcmp(parm, "QNA", i) == 0 )
	{
		return(MOP_DEVTYP_QNA);
	}
	else if ( bcmp(parm, "BNT", i) == 0 )
	{
		return(MOP_DEVTYP_BNT);
	}
	else
	{
		fprintf(stderr, "%s: %s is an invalid device type\n", caller, parm);
		return(-1);
	}
}

/*
 *			c h e c k _ h e x
 *
 * Check to see if next two bytes are ascii digits.
 *
 *  Input:  
 *	parm		= parameter to be checked
 *
 *  Output: 
 *	returns		-1 if failure, 0 if success.
 */
check_hex(parm)
u_char *parm;
{
	if ( parm[0] < '0' || parm[0] > 'f' || (parm[1] < '0' && parm[1] != NULL) || parm[1] > 'f' )
		return(-1);
	else
		return(0);
}
