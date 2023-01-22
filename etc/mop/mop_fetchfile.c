#ifndef lint
static char *sccsid = "@(#)mop_fetchfile.c	1.6	ULTRIX	12/4/86";
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
#include <a.out.h>
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

extern int exit_status, file_format;

extern char errbuf[4096];

extern char load_path[1024];
extern char dnet_default_load_path[1024];

extern u_char chk_authorization, label_type;
extern int aout_text, aout_gap;


/*
*		f e t c h _ d u m p _ f i l e
*
* Version: 1.0 - 7/18/85  
*
*
* Description:
*	This subroutine attempts to open a file to receive the
*	memory dump .  If successful, it passes back the file
*	descriptor.
*
* Inputs:
*	node			= Pointer to structure containing DECnet
*				  address and name of target.
*	host			= Pointer to structure containing DECnet
*				  address and name of host.
*	parm			= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	task			= Pointer to structure containing task
*				  label parameters.
*	source_addr		= physical addr of source.
*
*
* Outputs:
*	fp			= Pointer to file descriptor of load module
*				  (NULL if error)
*	exit_status		= written into only if error occurs.
*
* Notes:
*
*/
FILE *fetch_dump_file( node, host, parm, task, source_addr )
register struct node_id *node, *host;
register struct mop_parse_dmpld *parm;

register struct task_image *task;
register char *source_addr;
{
	int i, size;
	long byte_offset;
	char filename[MDB_FN_MAXL+1];
	char label_buf[TSK_DBLKSIZE];
	FILE *ld_fopen();
	register FILE *fp;

	filename[0] = NULL;
	if (access_db(MOP_DUMP, source_addr, filename, node, host) == 0)
	{
		if ( strlen(filename) == 0 )
		{
			mop_syslog("mop_dumpload", "dump file not defined in data base", 0, LOG_INFO);
			exit_status = EXIT_NODUMPFILE;
			return(NULL);
		}
	}
	else
	{
		mop_syslog("mop_dumpload", "not authorized to act on dump request", 0, LOG_DEBUG);
		exit_status = EXIT_NOAUTHORIZATION;
		return(NULL);
	}

	if ((fp = ld_fopen( filename, "w" )) == NULL)
	{
		sprintf(errbuf, "unable to open dump file %s", filename);
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_FOPENFAIL;
		return(NULL);
	}

	return(fp);
}

/*
*		f e t c h _ l o a d _ f i l e
*
* Version: 1.0 - 1/10/85
* Version: 1.1 - 3/22/85 - use MOP data base.
*
*
* Description:
*	This subroutine attempts to open a file containing the
*	load module.  If successful, it passes back the file
*	descriptor.
*
* Inputs:
*	node			= Pointer to structure containing DECnet
*				  address and name of target.
*	host			= Pointer to structure containing DECnet
*				  address and name of host.
*	parm			= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	task			= Pointer to structure containing task
*				  label parameters.
*	source_addr		= physical addr of source.
*
*
* Outputs:
*	fp			= Pointer to file descriptor of load module
*				  (NULL if error)
*
* Notes:
*
*/
FILE *fetch_load_file( node, host, parm, task, source_addr )
register struct node_id *node, *host;
register struct mop_parse_dmpld *parm;

register struct task_image *task;
register char *source_addr;
{
	int i, size;
	int flags;
	long byte_offset;
	char *isd, *iha;
	struct exec *execp;
	char pathname[MDB_FN_MAXL+1], filename[MDB_FN_MAXL+1];
	char label_buf[TSK_DBLKSIZE];
	FILE *ld_fopen();
	register FILE *fp;

	filename[0] = NULL;
	switch ( parm->mop_pgm_type )
	{
		case MOP_SECONDARY:
			switch ( parm->mop_swid_form )
			{
				case MOP_SWID_NONE:
				case MOP_SWID_SOS:
				case MOP_SWID_MS:
					if (access_db(MOP_LDSEC, source_addr, filename, node, host) == 0)
					{
						if ( strlen(filename) == 0 )
						{
							form_filename(filename, "sec", parm->mop_devtyp, node);
						}
					}
					else
					{
						mop_syslog("mop_dumpload", "not authorized to act on secondary load request", 0, LOG_DEBUG);
						exit_status = EXIT_NOAUTHORIZATION;
						return(NULL);
					}
					break;

				default:
					if ( parm->mop_swid_form > 0 )
					{
						if ( chk_authorization && access_db(MOP_LDSEC, source_addr, filename, node, host) < 0 )
						{
							mop_syslog("mop_dumpload", "not authorized to act on system load request", 0, LOG_DEBUG);
							exit_status = EXIT_NOAUTHORIZATION;
							return(NULL);
						}
						sprintf(filename, "%s", parm->mop_swid_id);
					}
					else 
					{
						mop_syslog("mop_dumpload", "bad software id on secondary load request", 0, LOG_DEBUG);
						exit_status = EXIT_BADSWID;
						return(NULL);
					}
					break;
			}
			break;

		case MOP_TERTIARY:
			switch ( parm->mop_swid_form )
			{
				case MOP_SWID_NONE:
				case MOP_SWID_SOS:
				case MOP_SWID_MS:
					if (access_db(MOP_LDTER, source_addr, filename, node, host) == 0)
					{
						if ( strlen(filename) == 0 )
						{
							form_filename(filename, "ter", parm->mop_devtyp, node);
						}
					}
					else
					{
						mop_syslog("mop_dumpload", "not authorized to act on tertiary load request", 0, LOG_DEBUG);
						exit_status = EXIT_NOAUTHORIZATION;
						return(NULL);
					}
					break;

				default:
					if ( parm->mop_swid_form > 0 )
					{
						if ( chk_authorization && access_db(MOP_LDSEC, source_addr, filename, node, host) < 0 )
						{
							mop_syslog("mop_dumpload", "not authorized to act on system load request", 0, LOG_DEBUG);
							exit_status = EXIT_NOAUTHORIZATION;
							return(NULL);
						}
						sprintf(filename, "%s", parm->mop_swid_id);
					}
					else 
					{
						mop_syslog("mop_dumpload", "bad software id on tertiary load request", 0, LOG_DEBUG);
						exit_status = EXIT_BADSWID;
						return(NULL);
					}
					break;
			}
			break;

		case MOP_SYSTEM:
			switch ( parm->mop_swid_form )
			{
				case MOP_SWID_NONE:
				case MOP_SWID_SOS:
					if (access_db(MOP_LDSYS, source_addr, filename, node, host) < 0)
					{
						mop_syslog("mop_dumpload", "not authorized to act on system load request", 0, LOG_DEBUG);
						exit_status = EXIT_NOAUTHORIZATION;
						return(NULL);
					}
					if ( ! strlen(filename) )
					{
						mop_syslog("mop_dumpload", "system load file not defined in nodes data base", 0, LOG_INFO);
						exit_status = EXIT_NOSYSFILE;
						return(NULL);
					}
					break;

				case MOP_SWID_MS:
					if (access_db(MOP_LDDIA, source_addr, filename, node, host) < 0)
					{
						mop_syslog("mop_dumpload", "not authorized to act on diagnostic load request", 0, LOG_DEBUG);
						exit_status = EXIT_NOAUTHORIZATION;
						return(NULL);
					}
					if ( ! strlen(filename) )
					{
						mop_syslog("mop_dumpload", "diagnostic load file not defined in nodes data base", 0, LOG_INFO);
						exit_status = EXIT_NODIAGFILE;
						return(NULL);
					}
					break;

				default:
					if ( parm->mop_swid_form > 0 )
					{
						if ( chk_authorization && access_db(MOP_LDSEC, source_addr, filename, node, host) < 0 )
						{
							mop_syslog("mop_dumpload", "not authorized to act on system load request", 0, LOG_DEBUG);
							exit_status = EXIT_NOAUTHORIZATION;
							return(NULL);
						}
						sprintf(filename, "%s", parm->mop_swid_id);
					}
					else
					{
						mop_syslog("mop_dumpload", "bad software id on system load request", 0, LOG_DEBUG);
						exit_status = EXIT_BADSWID;
						return(NULL);
					}
					break;
			}
			break;
		default:
			mop_syslog("mop_dumpload", "invalid program type requested", 0, LOG_INFO);
			exit_status = EXIT_INVPROGTYP;
			return(NULL);
			break;

	}

	if ((fp = ld_fopen( filename, "r" )) == NULL)
	{
		sprintf(errbuf, "load file %s not found", filename);
		mop_syslog("mop_dumpload", errbuf, 0, LOG_INFO);
		exit_status = EXIT_FOPENFAIL;
		return(NULL);
	}

	if ( label_type != LABEL_TYPE_RSX )
	{
		return(fp);
	}

	if ((size = read(fileno(fp), label_buf, sizeof(label_buf))) == 0)
	{
		mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
		fclose(fp);
		exit_status = EXIT_FREADFAIL;
		return(NULL);
	}

	/*
	 * determine the file format
	 */
	file_format = FF_UNKNOWN;
	if ((size >= sizeof(long)) && (!N_BADMAG((*(struct exec *)label_buf))))
	{
		execp = (struct exec *)label_buf;
		if(execp->a_magic==OMAGIC)
			file_format = FF_AOUTOLD;
		else	file_format = FF_AOUTNEW;
	}
	else if ((size == sizeof(label_buf)) && (*(short *)(&label_buf[510]) < 0))
		file_format = FF_VMS;
	else if (size >= TSK_LABEL_SIZE)
		file_format = FF_RSX;
	/*
	 * save relevant info base on image file format
	 */
	bzero(task, sizeof(struct task_image));
	switch(file_format)
	{
		case FF_AOUTNEW:
			execp = (struct exec *)label_buf;
			task->base_addr = 0;
			aout_text= execp->a_text;
			aout_gap = (aout_text % 1024) ? (1024 - (aout_text % 1024)) : 0;
			task->task_size = aout_text + aout_gap + execp->a_data;
			task->task_xferaddr = execp->a_entry & 0x7fffffff;
			byte_offset = N_TXTOFF((*(struct exec *)label_buf));
			mop_syslog("mop_dumpload", "AOUT load image found", 0, LOG_DEBUG);
			sprintf(errbuf, "size=%d text=%d gap=%d offset=%d data=%d\n",task->task_size,aout_text,aout_gap,byte_offset,execp->a_data);
			mop_syslog("mop_dumpload", errbuf, 0, LOG_DEBUG);
			break;
		case FF_AOUTOLD:
			execp = (struct exec *)label_buf;
			task->base_addr = 0;
			aout_text= execp->a_text;
			task->task_size = aout_text + execp->a_data;
			task->task_xferaddr = execp->a_entry & 0x7fffffff;
			byte_offset = N_TXTOFF((*(struct exec *)label_buf));
			mop_syslog("mop_dumpload", "AOUT load image found", 0, LOG_DEBUG);
			sprintf(errbuf, "text=%d gap=%d offset=%d\n",aout_text,aout_gap,byte_offset);
			mop_syslog("mop_dumpload", errbuf, 0, LOG_DEBUG);
			break;
		case FF_VMS:
			mop_syslog("mop_dumpload", "VMS load image found", 0, LOG_DEBUG);
			isd = label_buf + *(u_short *)(&label_buf[IHD_W_SIZE]);
			iha = label_buf + *(u_short *)(&label_buf[IHD_W_ACTIVOFF]);
			task->base_addr = ((*(u_long *)(isd + ISD_V_VPN)) * 512) & 0x1fffff;
			task->task_size = (*(u_short *)(isd + ISD_W_PAGCNT)) * 8;
			task->task_xferaddr = *(u_long *)(iha + IHA_L_TFRADR1) & 0x7fffffff;
			byte_offset = label_buf[IHD_B_HDRBLKCNT] * TSK_DBLKSIZE;
			break;
		case FF_RSX:
			mop_syslog("mop_dumpload", "RSX load image found", 0, LOG_DEBUG);
			flags = *(u_short *)(&label_buf[TSK_FLAGS_IDX]);
			if ( ! (flags & TSK_NHD) || ! (flags & TSK_CHK))
			{
				mop_syslog("mop_dumpload", "load file is not a boot image", 0, LOG_INFO);
				fclose(fp);
				exit_status = EXIT_FREADFAIL;
				return(NULL);
			}
			task->base_addr = *(u_short *)(&label_buf[TSK_BASE_IDX]);
			task->task_size = *(u_short *)(&label_buf[TSK_SIZE_IDX]);
			task->task_xferaddr = *(u_short *)(&label_buf[TSK_XFER_IDX]);
			byte_offset = (*(u_short *)(&label_buf[TSK_HEADER_IDX])) * TSK_DBLKSIZE;
			break;
		default:
			mop_syslog("mop_dumpload", "unknown load file format, ", 0, LOG_INFO);
			fclose(fp);
			return(NULL);
			break;
	}


	if ( fseek(fp, byte_offset, 0) < 0 )
	{
		mop_syslog("mop_dumpload", "read error, incorrect file size", 0, LOG_INFO);
		fclose(fp);
		exit_status = EXIT_FREADFAIL;
		return(NULL);
	}

	return(fp);
}

/*
*		a c c e s s _ d b
*
* Version: 1.0 - 1/10/85
* Version: 1.1 - 3/22/85 - use MOP data base.
*
*
* Description:
*	This subroutine attempts to find a system image
*	as a function of address in the data base file.
*	It also fetches other paramters such as host name
*	and DECnet address.  Note that failure results
*	if the authorization check is requested and fails.
*
* Inputs:
*	cmd			= indicates which load file to fetch.
*	addr			= physical addr of source.
*	file			= pointer to string where filename
*				  is to be placed.
*	node_id			= Pointer to structure containing DECnet
*				  address and name of target.
*	host_id			= Pointer to structure containing DECnet
*				  address and name of host.
*
*
* Outputs:
*	file			= file name if found, NULL if not.
*	node_id			= decnet address and/or name if not user 
*				  supplied.
*	host_id			 = primary host dnet address and name.
*
* Notes:
*
*/
access_db(cmd, addr, file, node_id, host_id)
int cmd;
register char *addr;
char *file;
struct node_id *node_id, *host_id;
{
	register struct nodeent *node;
	struct nodeent *mop_getnode();
	register int i;
	int j;
	register int baddr_len;
	int addr_len = strlen(addr);
	u_short psiz;
	u_char baddr[MDB_NA_MAXL+1];
	u_char node_name[10];
	u_char *cp;
	u_char *get_param();

	/*
	 * access nodes data base as a function of either DECnet physical 
	 * address or default hardware address.
	 */
	for(i = 0, baddr_len = 0; i < addr_len; i += 3)
	{
		sscanf(&addr[i], "%2x", &j);
		baddr[baddr_len++] = (u_char) j;
	}

	if ( ! (node = mop_getnode(baddr, baddr_len)) )
	{
		return(-1);
	}

	/*
	 * If an entry was found, fetch the needed parameters: load file,
	 * DECnet node address, node name, host DECnet address and host
	 * name
	 */
	if ( chk_authorization && (cp = get_param(node->n_params, NODE_SERVICE, &psiz)) )
	{
		if ( *cp == 1 )
		{
			mop_syslog("mop_dumpload", "service disabled", 0, LOG_DEBUG);
			exit_status = EXIT_NOAUTHORIZATION;
			return(0);
		}
	}

	if ( ! node_id->node_dnaddr )
	{
		bcopy(node->n_addr, &node_id->node_dnaddr, sizeof(node_id->node_dnaddr));
	}

	if ( strlen(node_id->node_name) == 0 )
	{
		strcpy(node_id->node_name, node->n_name);
	}

	if ( (cp = get_param(node->n_params, NODE_HOST, &psiz)) )
	{
		bcopy(cp, &host_id->node_dnaddr, sizeof(host_id->node_dnaddr));
		if ( psiz > sizeof(host_id->node_dnaddr) )
		{
			bcopy(cp+sizeof(host_id->node_dnaddr), host_id->node_name, (psiz - sizeof(host_id->node_dnaddr)));
			file[psiz - sizeof(host_id->node_dnaddr)] = NULL;
		}
	}


	switch (cmd)
	{
		case MOP_LDSYS:
			cp = get_param(node->n_params, NODE_LOAD_FILE, &psiz);
			break;

		case MOP_LDTER:
			cp = get_param(node->n_params, NODE_TER_LOADER, &psiz);
			break;

		case MOP_LDSEC:
			cp = get_param(node->n_params, NODE_SEC_LOADER, &psiz);
			break;

		case MOP_LDDIA:
			cp = get_param(node->n_params, NODE_DIAG_FILE, &psiz);
			break;

		case MOP_DUMP:
			cp = get_param(node->n_params, NODE_DUMP_FILE, &psiz);
			break;

		default:
			return(-1);
			break;
	}

	/*
	 * return filename
	 */
	if ( cp )
	{
		bcopy(cp, file, psiz);
		file[psiz] = NULL;
	}
	else
	{
		file[0] = NULL;
	}

	return(NULL);
}

/*
*		m o p _ g e t n o d e
*
* Version: 1.0 - 4/9/85
*
*
* Description:
*	This routine searches the nodes database for a node
*	whose hardware address matches that passed to the routine,
*	or whose decnet address matches that passed to the routine.
*
* Inputs:
*	addr			= physical addr of source.
*	len			= length of address.
*
*
* Outputs:
*	returns 		= pointer to data base entry if success,
*				  NULL if failure.
*	exit_status		= written into only if error occurs.
*
* Notes:
*
*/
struct nodeent *mop_getnode(addr, len)
u_char *addr;
int len;
{
	struct nodeent *node = 0;
	u_short psiz, area_addr;
	u_char dn_addr[4];
	u_char *get_param();
	register u_char *cp;
	dn_addr[0] = 0xaa;
	dn_addr[1] = 0x00;
	dn_addr[2] = 0x04;
	dn_addr[3] = 0x00;

	/*
	 * find entry in the nodes data base as a function of either DECnet
	 * address or hardware address.
	 */
	if ( bcmp(addr, dn_addr, (len - 2)) == NULL )
	{
		/* fetch node as a function of DECnet address */
		bcopy(addr+4, &area_addr, sizeof(area_addr));
		if ( ! ( node = getnodebyaddr(&area_addr, sizeof(area_addr), AF_DECnet) ) )
		{
			mop_syslog("mop_dumpload", "can't access MOP data base", 1, LOG_INFO);
			exit_status = EXIT_NOAUTHORIZATION;
			return(0);
		}
		if ( node->n_params != NULL )
		{
			if ( (cp = get_param(node->n_params, NODE_HARDWARE_ADDR, &psiz)) )
			{
				return(node);
			}
		}
	}
	else
	{
		if ( setnodeent(0) == -1 )
		{
			mop_syslog("mop_dumpload", "can't access MOP data base", 1, LOG_INFO);
			exit_status = EXIT_NOAUTHORIZATION;
			return(0);
		}
		while ( (node = getnodeent()) != NULL )
		{
			if ( node->n_params != NULL )
			{
				if ( (cp = get_param(node->n_params, NODE_HARDWARE_ADDR, &psiz)) )
				{
					if ( bcmp(cp, addr, len) == NULL)
					{
						endnodeent();
						return(node);
					}
				}
			}
		}
		endnodeent();
	}

		return(0);
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
register u_short parm;
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
*		f o r m _ p a t h n a m e
*
* Version: 1.0 - 3/22/85
*
*
* Description:
*	This subroutine forms a string containing pathname and filename
*	of requested load file. 
*
* Inputs:
*	pathname		= pointer to string where pathname/filename
*				  is to be placed.
*	filename		= pointer to string containing filename
*				  of requested load file.
*	extension		= pointer to string containing filenam
*				  extension.
*
*
* Outputs:
*	pathname		= pathname/filename.
*
* Notes:
*
*/
form_pathname(pathname, filename, extension)
char *pathname, *filename, *extension;
{
	int i = strlen(filename);
	u_char extend = 1;

	while( (--i != 0) && filename[i] != '/' )
	{
		if ( filename[i] == '.' )
		{
			extend = 0;
			break;
		}
	}

	if ( filename[0] == '/' )
	{
		if ( extend )
			sprintf(pathname, "%s.%s", filename, extension);
		else 
			sprintf(pathname, "%s", filename);
	}
	else
	{
		if ( extend )
			sprintf(pathname, "%s/%s.%s", dnet_default_load_path, filename, extension);
		else 
			sprintf(pathname, "%s/%s", dnet_default_load_path, filename);
	}

	tolocase( pathname );

	return;
}

/*
*		f o r m _ f i l e n a m e
*
* Version: 1.0 - 1/10/85
*
*
* Description:
*	This subroutine forms program name as a funciton of device type
*	and appends it to the given pathname.  If device type is invalid
*	and node id exits, it attempts to fetch the device type from
*	the data base.
*
* Inputs:
*	file			= pointer to string where filename
*				  is to be placed.
*	prefix			= string to be prefixed to program
*				  name.
*	devtyp			= device type.
*	tnode			= pointer to structure containing target
*				  node id.
*
*
* Outputs:
*	filename		= file name.
*
* Notes:
*
*/
form_filename(file, prefix, devtype, tnode)
char *file, *prefix;
u_char devtype;
register struct node_id *tnode;
{
	struct nodeent *gn = NULL;
	u_char *cp, *get_param();
	u_short psiz;
	char device[10];

	switch (devtype)
	{
		case MOP_DEVTYP_UNA:
			sprintf(device, "una");
			break;

		case MOP_DEVTYP_QNA:
			sprintf(device, "qna");
			break;

		default:
			if ( ! tnode )
			{
				device[0] = NULL;
			}
			else
			{
				if ( tnode->node_dnaddr )
					gn = getnodebyaddr(&tnode->node_dnaddr, sizeof(tnode->node_dnaddr), AF_DECnet);
				else if ( strlen(tnode->node_name) )
					gn = getnodebyname(tnode->node_name);
				else
					device[0] = NULL;

				if ( ! gn )
					mop_syslog("mop_dumpload", "couldn't access database for service device", 0, LOG_INFO);
				else
				{
					if ( (cp = get_param(gn->n_params, NODE_SERVICE_DEV, &psiz)) )
					{
						form_filename(file, prefix, *cp, NULL);
						return;
					}
				}
			}
			break;
	}

	sprintf(file, "%s%s", prefix, device);
	return;

}

/*
 *
 * Version: 1.0 - 7/9/86
 *
 *
 * Description:
 *	This subroutine chooses one of two sets of rules in
 *	its attempt to open a file.  If the path name starts
 *	with the root, then the first set of rules is as 
 *	follows:
 *		if the directory path name is /usr/lib/dnet,
 *			then make the filename lowercase and
 *			     add the '.sys' extension before
 *			     opening the file
 *		otherwise open the file as given.
 *
 *	If the pathname does not begin with the root, the
 *	second set of rules is as follows:
 *		for each pathname listed in the load path array
 *			if the directory path name is /usr/lib/dnet,
 *				then make the filename lowercase and
 *				     add the '.sys' extension before
 *				     opening the file
 *			otherwise open the file as given.
 *			if the file opens successfully, 
 *				stop search and return with file descriptor
 *
 *
 * Inputs:
 *	pathname		= indicates which load file to open.
 *	mode			= access mode for file.
 *
 *
 * Outputs:
 *	fp			= pointer to file.
 *
 * Notes:
 *
 */
FILE *ld_fopen( filename, mode )
char *filename, *mode;
{
	FILE *fopen();
	register FILE *fp = NULL;
	register int i = 0, j;
	char pathname[1024];

	if ( filename[0] == '/' )
	{
		strcpy(pathname, filename);
		j = strlen(pathname);
		while( pathname[--j] != '/' ) ;
		if ( j && bcmp(pathname, dnet_default_load_path, j) == 0 )
			form_pathname(pathname, filename, "sys");
		return(fopen( pathname, mode ));
	}
	else
	{
		while ( load_path[i] != NULL )
		{
			j = 0;
			while( load_path[i] != ':' && load_path[i] != NULL )
				pathname[j++] = load_path[i++];
			if ( bcmp(pathname, dnet_default_load_path, j) == 0 )
				form_pathname(pathname, filename, "sys");
			else
				sprintf(pathname+j, "/%s", filename);
			if (fp = fopen( pathname, mode ))
				return(fp);
			if ( load_path[i] == ':' ) i++;
		}
	}
	return(NULL);
}
