#ifndef lint
static char *sccsid = "@(#)print_nodeent.c	1.4	ULTRIX	10/3/86";
#endif lint

/*
 * Program printnode.c,  Module NODEDB 
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
 * 1.00 6-Aug-1985
 *      DECnet-ULTRIX   V1.1
 *
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


static char PRINT_NODEENT[] = "@(#)print_nodeent.c	1.3		9/13/85";

char *sw_type[] = {
	"secondary loader",
	"tertiary loader",
	"system load file",
};
int sw_type_size = sizeof(sw_type)/sizeof(sw_type[0]);

/*
 *		p r i n t _ n o d e e n t
 *
 * Print an entry from the MOP node data base.
 *
 *
 * Inputs:
 *		get_node 	= pointer to structure containing data from nodes data base
 *	
 */
print_nodeent(get_node)
register struct nodeent *get_node;
{
	int i,j,k;
	u_short dn_address, dn_area, psiz;
	u_char passwd[17];
	u_char buffer[MDB_NA_MAXL*3];
	u_char name[MDB_FN_MAXL+1];
	u_short host_area, host_addr;
	u_char *pb;


	/*
	 * fetch DECnet address and node name
	 */
	bcopy(get_node->n_addr, &dn_address, sizeof(dn_address));
	dn_area = (dn_address & 0xfc00) >> 10;
	dn_address &= 0x3ff;

	if ( dn_address )
	{
		printf("\nnode = %2d.%-4d  %s\n", dn_area, dn_address, get_node->n_name);
	}
	else
	{
		printf("\nnode =          %s\n", get_node->n_name);
	}

	/*
	 * fetch any existing parameters
	 */
	if ( (pb = get_node->n_params) != NULL )
	{
		k = 0;
		while ( pb[k] != NODE_END_PARAM )
		{
			bcopy(&pb[k+NODE_CSIZ], &psiz, sizeof(psiz));

			switch ( *(u_short *) &pb[k] )
			{
				case NODE_SERVICE:
					if ( pb[k+NODE_PARM_HDRSIZ] == 0 )
						strcpy(name, "enabled");
					else if ( pb[k+NODE_PARM_HDRSIZ] == 1 )
						strcpy(name, "disabled");
					else
						strcpy(name, "unknown");
					printf("	service = %s\n", name);
					break;

				case NODE_SERVICE_CKT:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	service circuit = %s\n", name);
					break;

				case NODE_SERVICE_PSWD:
					if ( psiz > ((sizeof(passwd)-1)/2) )
					{
						break;
					}
					bzero(passwd, sizeof(passwd));
					for ( i = psiz-1, j = 0; i > -1; i--, j += 2 )
					{
						sprintf(&passwd[j], "%2x", pb[k+NODE_PARM_HDRSIZ+i]);
					}
					i = -1;
					while ( passwd[++i] != NULL )
					{
						if ( i != 0 && passwd[i] == ' ' )
							passwd[i] = '0';
					}
					printf("	service password = %s\n", passwd);

					break;

				case NODE_SERVICE_DEV:
					fetch_sdevname(pb[k+NODE_PARM_HDRSIZ], name);
					printf("	service device = %s\n", name);
					break;

				case NODE_CPU:
					fetch_cpuname(pb[k+NODE_PARM_HDRSIZ], name);
					printf("	cpu = %s\n", name);
					break;

				case NODE_LOAD_FILE:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	load file = %s\n", name);
					break;

				case NODE_DIAG_FILE:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	diagnostic file = %s\n", name);
					break;

				case NODE_DUMP_FILE:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	dump file = %s\n", name);
					break;

				case NODE_SEC_LOADER:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	secondary loader = %s\n", name);
					break;

				case NODE_TER_LOADER:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	tertiary loader = %s\n", name);
					break;

				case NODE_HOST:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], &host_addr, sizeof(host_addr));
					host_area = (host_addr & 0xfc00) >> 10;
					host_addr &= 0x3ff;
					printf("	host addr = %d.%d\n", host_area, host_addr);
					if ( (i = psiz - sizeof(host_addr)) )
					{
						bcopy(&pb[k+NODE_PARM_HDRSIZ+sizeof(host_addr)], name, i);
						name[i] = NULL;
						printf("	host name = %s\n", name);
					}
					break;

				case NODE_HARDWARE_ADDR:
					for( j = 0, i = 0; i < psiz; j += 3, i++ )
					{
						if ( i == (psiz - 1) )
							sprintf(buffer+j, "%2x", pb[k+i+NODE_PARM_HDRSIZ]);
						else
							sprintf(buffer+j, "%2x-", pb[k+i+NODE_PARM_HDRSIZ]);

						if ( buffer[j] == ' ' )
							buffer[j] = '0';
					}
					printf("	hardware address = %s\n", buffer);
					break;

				case NODE_SW_ID:
					bcopy(&pb[k+NODE_PARM_HDRSIZ], name, psiz);
					name[psiz] = NULL;
					printf("	software id = %s\n", name);
					break;

				case NODE_SW_TYPE:
					if ( pb[k+NODE_PARM_HDRSIZ] >= sw_type_size )
						printf("	unrecognized software type = %d\n", pb[k+NODE_PARM_HDRSIZ]);
					else
						printf("	software type = %s\n", sw_type[pb[k+NODE_PARM_HDRSIZ]]);
					break;

				default:
					break;
			}

			k += psiz + NODE_PARM_HDRSIZ;
		}
	}
	return;

}

/*
 *		f e t c h _ s d e v n a m e
 *
 * Fetch a device name as a function of device code.  Appendix A of
 * MOP spec used as basis of translation.
 *
 *
 * Inputs:
 *		dev_code	= device code
 *		dev_name	= ptr where device name is to be placed
 *
 * Outputs:	dev_name	= name of device mapped to code.
 *	
 */
fetch_sdevname(dev_code, dev_name)
u_char dev_code, *dev_name;
{

	switch ( dev_code )
	{
		case 1:
			sprintf(dev_name, "UNA");
			break;

		case 3:
			sprintf(dev_name, "CNA");
			break;

		case 5:
			sprintf(dev_name, "QNA");
			break;

		case 23:
			sprintf(dev_name, "BNT");
			break;

		case 12:
			sprintf(dev_name, "DMC");
			break;

		case 34:
			sprintf(dev_name, "DMV");
			break;

		case 40:
			sprintf(dev_name, "DMR");
			break;

		default:
			sprintf(dev_name, "UNKNOWN");
			break;
	}
	return;
}

/*
 *		f e t c h _ c p u n a m e
 *
 * Fetch a CPU name as a function of CPU code.  Chapter 7 of
 * Network Mgt spec used as basis of translation.
 *
 *
 * Inputs:
 *		cpu_code	= cpu code
 *		cpu_name	= ptr where cpu name is to be placed
 *
 * Outputs:	cpu_name	= name of cpu mapped to code.
 *	
 */
fetch_cpuname(cpu_code, cpu_name)
u_char cpu_code, *cpu_name;
{

	switch ( cpu_code )
	{
		case 0:
			sprintf(cpu_name, "PDP8");
			break;

		case 1:
			sprintf(cpu_name, "PDP11");
			break;

		case 2:
			sprintf(cpu_name, "DECSYSTEM1020");
			break;

		case 3:
			sprintf(cpu_name, "VAX");
			break;

		default:
			sprintf(cpu_name, "UNKNOWN");
			break;
	}
	return;
}
