#ifndef lint
static char *sccsid = "@(#)sizer.c	1.18	ULTRIX	3/3/87";
#endif lint
/*
	Program name:	sizer.c
	Author:		Bob Fontaine
	Date Written:	13-Feb-86
	SCCS ID:	@(#)sizer.c	1.10G%
	Last Edit:	

	This program is used by the installation procedures
	to size the system, determine CPU type and determine
	the root device upon which the system is running.

	Usage:
		sizer [-c][-b][-r][-f kfile] [-n sysnam] [-t timezone]

	-b	Create the boot commands for the chosen system disk
	-c	Write the CPU type (only, no headings) to standard output
	-k	Use the next argument as an alternate kernel file
	-n	The next arguement is the system name
	-r	Write the root device (only) to standard output
	-t	The next argument is the "timezone" string for config
	-s	Write the cpu sub type to standard output
-------
 Modification History
 ~~~~~~~~~~~~~~~~~~~~
 01	13-Feb-86, Bob Fontaine
 	Adapted from 1.2 code.

 02	20-Feb-86, Bob Fontaine
 	Old kludge required that massbus tapes appear before unibus tapes
 	in the MAKEDEV line created by sizer.  This no longer applies, so
	I removed the work around in sizer.

 03	20-Mar-86, Bob Fontaine
 	Reworked the main routine and took out that horrible while loop.
	Now the argument list is searched and the appropriate routines
	are called from main as they should be.  It was hard to tell what
	was going on before.

 04	10-Jul-86, Bob Fontaine
	Modified sizer to use new config_adpt structure now available.
	It is used to size bi and uba busses.  Massbus adapter sizing
	will be done as before.  In addition, changed printub() to
	create a config file with the new config syntax required in
	2.0G baselevel.

 05	16-jul-86, Bob Fontaine
	Added get_float routine to size unibus floating address space.
	The table of devices is from VMS and must be updated as VMS
	updates their table.  Do not use until baselevel H.

 06	14-aug--86, Bob Fontaine
 	Added vaxstar support and added new -s option.  The -s will return
	the cpu subtype.

 07	Fixed incorrect floating ubs space sizing.  Changed std in MAKEDEV
 	line to bootXX.

*/


#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <nlist.h>
#include <sys/vm.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "/sys/h/buf.h"
#include "/sys/h/config.h"
#include "/sys/vaxuba/ubavar.h"
#include "/sys/vaxmba/mbavar.h"
#include "/sys/vax/cpu.h"
#include "/sys/vax/param.h"
#include "/sys/h/time.h"
#include "/sys/vax/rpb.h"
#include "sizer.h"

#define LSIZE			255
#define STSIZE			10
#define UPRC			50
#define QB_IO_OFFSET		0x400000

char	path[LSIZE];		/* place where config file is built */

char	dname[STSIZE];
char 	mname[STSIZE];
char 	sname[STSIZE];
char    ctlr_name[STSIZE];
int 	nexus;
int 	dev_index = 0,
	float_flag = 0,
        dev_cnt = 0,
 	ctlr_index = 0,
        ctlr_cnt = 0,
	config_indx = 0,
	config_cnt = 0;
int	kmem,kumem;
int CPU,CPUSUB;
char BOOT[10];
FILE *fp, *fpdevs;
char *altfile,*sysname,*tzone,devpath[LSIZE];
char pname[20],cname[20];
int pnum,cnum,devtype,ALL;
/***************************************************************/
main (argc, argv)
char  **argv;
int argc;
{
	int rcode,optflag,outflag,kflag,getargs(),retval;
	char command[LSIZE];
	rcode = optflag = outflag = kflag = 0;

	if (argc > 1)
    		rcode=getargs(argv,argc,&optflag,&outflag,&kflag);

	if(rcode < 0 || argc < 2)
	{
		usage();
		exit (1);
	}
	nlist((kflag) ? altfile : "/vmunix",nl);

	if (nl[3].n_type == 0)
		quitonerror (-1);

	if ((kmem = open ("/dev/kmem", 0)) < 0)
		quitonerror (-2);

	ALL = 0;
	if(signal(SIGINT,SIG_IGN) != SIG_IGN)
		signal(SIGINT,SIG_IGN);
	switch (optflag)
	{
		case 0: 
			getsysname(sysname);
			sprintf (devpath, "/tmp/%s.devs", sysname);
	    		if ((fpdevs = fopen (devpath, "w")) == NULL)
				fprintf (stderr, "Can not open %s file\n", devpath);
			sprintf (path, "/tmp/%s", sysname);
			if ((fp = fopen (path, "w")) == NULL)
			{
				fprintf(stderr,"Can not open %s file\n",path);
				quitonerror(-10);
			}
			fprintf (fp, "ident\t\t\"%s\"\n", sysname);
			sprintf (command, "mkdir /sys/%s", sysname);

			if(rcode = system (command) == 127)
				quitonerror(-10);

	    		fprintf (fpdevs, "MAKEDEV  ");
			getcpu (outflag);
			fprintf(fpdevs," %s ",BOOT);
			fprintf (fp, "maxuprc\t\t%d\n",UPRC);
			gmem();
			gettimezone(tzone);
			fprintf(fp,"\n\n");
			retval = getconfig("options");
			if(retval == -1)
			{
				fprintf(fp,"options\t\tLAT\n");
				fprintf(fp,"options\t\tQUOTA\n");
				fprintf(fp,"options\t\tINET\n");
				fprintf(fp,"options\t\tEMULFLT\n");
				fprintf(fp,"options\t\tNFS\n");
				fprintf(fp,"options\t\tRPC\n");
				fprintf(fp,"options\t\tDLI\n");
				fprintf(fp,"options\t\tBSC\n");
			}
			fprintf(fp,"\n");
			getroot(outflag);
			getuba();
			if(CPU ==VAX_750 || CPU == VAX_780 || CPU == VAX_8600)
				getmba();
/* get any hardware devices described in the .config file and put them
   in the real config file. */
			retval = getconfig("hardware");
			fprintf(fp,"\n\n");
			retval = getconfig("pseudo-device");
 			if(retval == -1)
 			{
 				fprintf(fp,"pseudo-device\t\tpty\n");
 				fprintf(fp,"pseudo-device\t\tloop\n");
 				fprintf(fp,"pseudo-device\t\tinet\n");
 				fprintf(fp,"pseudo-device\t\tether\n");
 				fprintf(fp,"pseudo-device\t\tlat\n");
 				fprintf(fp,"pseudo-device\t\tlta\n");
 				fprintf(fp,"pseudo-device\t\trpc\n");
 				fprintf(fp,"pseudo-device\t\tnfs\n");
 				fprintf(fp,"pseudo-device\t\tdli\n");
 				fprintf(fp,"pseudo-device\t\tbsc\n");
 			}
			fprintf(stdout,"\nConfiguration file complete.\n");
			fprintf(fpdevs,"\n");
			rcode = fclose (fpdevs);
			if(rcode == EOF)
				quitonerror(-90);
			rcode = fclose (fp);
			if(rcode == EOF)
				quitonerror(-90);
			break;

		case 1: 
			getcpu (outflag);
			break;

		case 2: 
			getroot (outflag);
			break;

		case 3:
			getcpu(2);
			getboot();
			break;

		case 4:
			retval = getsubcpu(outflag);
			break;

		default: 
			break;
	}  /* end switch */

	exit (0);
}
/*****************************************************************/
getargs(argv,argc,optflag,outflag,kflag)
char **argv;
int argc,*optflag,*outflag,*kflag;
{

	register int k;
	int error = 0;

	for(k=1;k<argc;k++)
	{
	    	switch(argv[k][1])
		{
			case 'k': 	/* -k: use alternate file */
				k++;	/* switch to next arg */
				*kflag = 1;
				if(argv[k][0] == '\0')
				{
					usage();
					error++;
		    		}
		    		else
			    		altfile = argv[k];
		    		break;

			case 'n': 	/* system name */
		    		k++;
				*optflag = 0;
		    		if(argv[k][0] == '\0')
		    		{
					usage();
					error++;
		    		}
		    		else
			    		sysname = argv[k];
		   		 break;

			case 'r': 	/* -r: display root device */
				*optflag = 2;
				*outflag = 1;
				break;

			case 'b': 	/* -b: create boot stuff */
				*optflag = 3;
				break;

			case 'c': 	/* -c: display cpu type */
				*optflag = 1;
				*outflag = 1;
				break;
			case 's': 	/* -s: display cpu subtype */
				*optflag = 4;
				*outflag = 1;
				break;

			case 't':	/* -t: get time zone for -n */
				k++;
				if(argv[k][0] == '\0')
				{
					usage();
					error++;
				}
				else
					tzone = argv[k];
				 break;

			default: 
				usage();
				error++;
				break;

		}		/* end switch (argv) */

	}

	return(error);
}
/************************************************************************
*    getboot								*
*    									*
*    get boot stuff for system disk					*
************************************************************************/

getboot ()
{
	dev_t majminrs;
	int indx,mairint,majrs,anyint;
	char line[80],cmdfile[50],askfile[50],bootfile[50];
	FILE *fpin,*fpdef,*fpask;

	indx = reset_anythg (8);
	indx = lseek (kmem, indx, 0);
	read (kmem, &majminrs, sizeof (majminrs));
	mairint = majminrs;
	majrs = mairint / 256;


	if(majrs == 0)
		if(CPU == VAX_780)
		{
			strcpy(cmdfile,"/usr/sys/780cons/mbahp.cmd");
			strcpy(askfile,"/usr/sys/780cons/askboo.cmd");
			strcpy(bootfile,"/usr/sys/780cons/defboo.cmd");
		}
		else
		{
			strcpy(cmdfile,"/usr/sys/8600cons/mbahp.com");
			strcpy(askfile,"/usr/sys/8600cons/askboo.com");
			strcpy(bootfile,"/usr/sys/8600cons/defboo.com");
		}
	if(majrs == 9)
		if(CPU == VAX_780)
		{
			strcpy(cmdfile,"/usr/sys/780cons/ubara.cmd");
			strcpy(askfile,"/usr/sys/780cons/askboo.cmd");
			strcpy(bootfile,"/usr/sys/780cons/defboo.cmd");
		}
		else if(CPU == VAX_730)
		{
			strcpy(cmdfile,"/usr/sys/730cons/ubara.cmd");
			strcpy(askfile,"/usr/sys/730cons/askboo.cmd");
			strcpy(bootfile,"/usr/sys/730cons/defboo.cmd");
		}
		else
		{
			strcpy(cmdfile,"/usr/sys/8600cons/ubara.com");
			strcpy(askfile,"/usr/sys/8600cons/askboo.com");
			strcpy(bootfile,"/usr/sys/8600cons/defboo.com");
		}
	if(majrs == 11)
		if(CPU == VAX_730)
		{
			strcpy(cmdfile,"/usr/sys/730cons/ubaidc.cmd");
			strcpy(askfile,"/usr/sys/730cons/askboo.cmd");
			strcpy(bootfile,"/usr/sys/730cons/defboo.cmd");
		}

	indx = reset_anythg (13);
	lseek(kmem,indx,0);
	read(kmem,&rpb,sizeof(rpb));

	if((fpin=fopen(cmdfile,"r")) == NULL)
	{
		fprintf(stderr,"Can't open %s!!!\n",cmdfile);
		fprintf(stderr,"Can not continue console update\n");
		quitonerror(-90);
	}
	fpdef=fopen(bootfile,"w");
	fpask=fopen(askfile,"w");

	while(fgets(line,80,fpin) != NULL)
	{
		if(CPU == VAX_780 || CPU == VAX_8600)
		{
			if(strncmp(line,"!R1",3) == 0)
			{
				sprintf(line,"DEPOSIT R1 %x",rpb.bootr1);
				strcat(line,"   ! TR LEVEL OF UNIBUS\n");
			}
			if(strncmp(line,"!R3",3) == 0)
			{
				sprintf(line,"DEPOSIT R3 %x",rpb.bootr3);
				strcat(line,"   ! PLUG # OF SYSTEM DISK\n");
			}
			if(strncmp(line,"DEPOSIT R5 1000B",16) == 0)
			{
				fprintf(fpask,"%s",line);
				sprintf(line,"DEPOSIT R5 10008");
				strcat(line,"   ! BOOT ULTRIX TO MULTI USER\n");
				fprintf(fpdef,"%s",line);
			}
			else
			{
				fprintf(fpdef,"%s",line);
				fprintf(fpask,"%s",line);
			}
		}
		if(CPU == VAX_730)
		{
			if(strncmp(line,"D/G/L 2",7) == 0)
			{
				fprintf(fpdef,"%s",line);
				fprintf(fpask,"%s",line);
				fprintf(fpdef,"D/G/L 3 %x\n",rpb.bootr3);
				fprintf(fpask,"D/G/L 3 %x\n",rpb.bootr3);
				continue;
			}
			if(strncmp(line,"D/G/L 5 1000B",13) == 0)
			{
				fprintf(fpask,"%s",line);
				sprintf(line,"D/G/L 5 10008\n");
				fprintf(fpdef,"%s",line);
			}
			else
			{
				fprintf(fpdef,"%s",line);
				fprintf(fpask,"%s",line);
			}
		}
	}
	fclose(fpdef);
	fclose(fpask);
	fclose(fpin);

	return;
}
/************************************************************************
*    getcpu								*
*    									*
*    get cpu type that this program is running on			*
************************************************************************/

getcpu (outflag)
int outflag;
{
	int indx, anyint, maxcpu, cpusub;

	indx = reset_anythg (3);
	indx = lseek (kmem, indx, 0);
	read (kmem, &anyint, sizeof (anyint));

	for (indx = 0; cpu[indx].cpuno > -1; indx++)
		if (cpu[indx].cpuno == anyint)
			break;

	CPU = cpu[indx].cpuno;
	cpusub = getsubcpu(0);
	if(cpusub == ST_VAXSTAR)
		strcpy(BOOT,"boot2000");
	else
		strcpy(BOOT,cpu[indx].bootdev);
	if(cpusub == ST_8300 || cpusub == ST_8800)
		maxcpu = 2;
	else
		maxcpu = 1;

	switch(outflag)
	{
		case 0:
			fprintf (fp, "machine\t\tvax\n");
			fprintf (fp, "cpu\t\t\"%s\"\n", cpu[indx].cpustrg);
			fprintf (fp, "maxusers	%d\n", cpu[indx].maxusers);
			fprintf (fp,"processors\t%d\n",maxcpu);
			break;
		case 1:
			if(CPU == MVAX_I)
				fprintf (stdout, "MVAX_I\n");
			else if(CPU == MVAX_II)
				fprintf (stdout, "MVAX_II\n");
			else
				fprintf (stdout, "%s\n", cpu[indx].cpustrg);
			break;
		case 2:
		default:
			break;
	}
	return;
}
/************************************************************************
*    getsubcpu								*
*    									*
*    get cpu subtype that this program is running on			*
************************************************************************/

getsubcpu (outflag)
int outflag;
{
	int indx,retval;

	indx = reset_anythg (17);
	indx = lseek (kmem, indx, 0);
	read (kmem, &CPUSUB, sizeof (CPUSUB));

	retval = 0;
	if(outflag == 1)
		fprintf (stdout, "%d\n", CPUSUB);
	else
		retval = CPUSUB;

	return(retval);
}

/****************************************************************
*    gmem							*
*								*
*    Get the size of physical memory				*
****************************************************************/

gmem()
{
	int indx, anyint, memmeg, rcode;

	indx = reset_anythg (12);
	indx = lseek (kmem, indx, 0);
	if(rcode = read (kmem, &anyint, sizeof (anyint)) < 0)
	{
		fprintf(stderr,"Can not size memory!  Assumming 16 meg.\n");
		fprintf(fp,"physmem\t\t16\n");
	}
	else
	{
/* convert "clicks" to megs of memory.  Partial megs get rounded up.*/
		memmeg = anyint / 2048;
		if (anyint % 2048)
	    		memmeg++;
		fprintf (fp, "physmem\t\t%d\n", memmeg);
	}
	return;
}

/****************************************************************
*    gettimezone						*
*								*
*    get time zone						*
****************************************************************/

gettimezone(tzone)
char *tzone;
{

	if (strcmp(tzone,"\0") != 0)
		fprintf (fp, "timezone\t%s\n", tzone);
	else
	{
		int indx, anyint;
		indx = reset_anythg (7);
		indx = lseek (kmem, indx, 0);
		read (kmem, &tzb, sizeof (tzb));
		anyint = tzb.tz_minuteswest / 60;
		fprintf (fp, "timezone\t%d dst %d\n", anyint, tzb.tz_dsttime);
	}

	return;
}

/****************************************************************
*    getroot							*
*								*
*    get root, swap, dump, and arg devices			*
****************************************************************/

getroot (outflag)
int outflag;
{
	dev_t majminrs;
	int indx, anyint, mairint, minrs, majrs, tmp, pt, retval;
	char tempstgs[10], cname[25];
	static char part[] = {'a','b','c','d','e','f','g','h'};


	if (!outflag)
		fprintf (fp, "config\tvmunix\t");

	for (anyint = 8; anyint < 11; anyint++)
	{
		indx = reset_anythg (anyint);
		indx = lseek (kmem, indx, 0);
		read (kmem, &majminrs, sizeof (majminrs));
		mairint = majminrs;
		majrs = mairint / 256;
		tmp = mairint - (majrs * 256);
		minrs = tmp / 8;
		pt = tmp % 8;


		if(majrs == 0)
			sprintf(tempstgs,"hp%d",minrs);
		if(majrs == 9)
			sprintf(tempstgs,"ra%d",minrs);
		if(majrs == 11)
			sprintf(tempstgs,"rb%d",minrs);
		if(majrs == 19)
			sprintf(tempstgs,"rd%d",minrs);

		switch (anyint)
		{
			case 8: 
				strcpy (cname, "root");
				break;

			case 9: 
				strcpy (cname, "swap");
				break;

			case 10: 
				strcpy (cname, "dumps");
				break;

			default: 
				break;
		}/* end switch */

		if (!outflag)
		{
			retval = getconfig(cname);
			if(retval == -1)
				fprintf(fp,"%s on %s%c  ",cname,tempstgs,part[pt]);
		}
		else
		{
			fprintf (stdout, "%s\n",tempstgs);
			break;
		}
	}

	if (!outflag)
		fprintf (fp, "\n\n");

	return;
}

/****************************************************************
*    getuba							*
*								*
*    now get info off the uba device				*
*****************************************************************/

getuba()
{
	int get_device(), s, i,makedevflag;
	char link[10],type[25],*get_type(),flag[10],*get_flags();
	int ubas,supp;
	u_long tmp;

	ubas = 0;
	config_indx = reset_anythg(14);
	while (1)
	{
		pname[0] = '\0';
		get_config();
		if(pname[0] == '\0')
			break;
		if(pnum == 63 && strcmp(pname,"nexus") != 0)
			continue;
		if(cnum == -1)
			continue;
		if(adpt.c_ptr == 0)
			continue;
		all[ALL].node = -1;
		all[ALL].drive = -1;
		if(devtype == 'A')
		{
			lseek(kmem,adpt.c_name,0);
			read(kmem,cname,sizeof(cname));
			if(strcmp(cname,"uba") == 0)
				ubas++;
			strcpy(type,get_type(cname,&makedevflag,&supp));
			strcpy(flag,get_flags(cname));
			all[ALL].sup = supp;
			if(adpt.c_ptr == 0)
				continue;
			if(pnum == 63)  /* 63  = '?' */
				strcpy(link,"nexus ?");
			else
				sprintf(link,"%s%d",pname,pnum);
			if(strcmp(type,"controller") == 0)
				vectors(cname);
			sprintf(all[ALL].ultname,"%s%d",cname,cnum);
			strcpy(all[ALL].link,link);
			strcpy(all[ALL].type,type);
			strcpy(all[ALL].flags,flag);
			if(makedevflag == 1)
				all[ALL].makedevflag = makedevflag;
			ALL++;
			continue;
		}
		if(devtype == 'C')
		{

/* get the controller structure pointed to */

			lseek(kmem,adpt.c_ptr,0);
			read(kmem,&ctlr,sizeof(ctlr));
			if(ctlr.um_alive  !=  1)
				continue;
			sprintf(link,"%s%d",pname,pnum);
			if(CPU == VAX_8200 || CPU == VAX_8800)
			{
				for(i=ALL-1;i>-1;i--)
					if(strcmp(all[i].ultname,link) == 0)
					{
						if(strncmp(all[i].link,"vaxbi",5) == 0)
						{
							all[i].node = ctlr.um_nexus;
							break;
						}
						else
							all[ALL].csr = (u_short)ctlr.um_addr;
					}
			}
			else
			if(CPUSUB == ST_VAXSTAR)
			{
				tmp = (u_long)ctlr.um_addr;
				all[ALL].csr = tmp;
			}
			else
			{
				all[ALL].csr = (u_short)ctlr.um_addr;
			}

			lseek(kmem,ctlr.um_ctlrname,0);
			read(kmem,cname,sizeof(cname));
			sprintf(all[ALL].ultname,"%s%d",cname,cnum);
			strcpy(type,get_type(cname,&makedevflag,&supp));
			strcpy(flag,get_flags(cname));
			all[ALL].sup = supp;
			strcpy(all[ALL].type,type);
			strcpy(all[ALL].flags,flag);
			strcpy(all[ALL].link,link);
			all[ALL].node = -1;
			all[ALL].drive = -1;
			vectors(cname);
			all[ALL].makedevflag = makedevflag;
			ALL++;	
			reset_device();
			sprintf(link,"%s%d",cname,cnum);

/* get all disks for this controller */

			while(s = get_device(1))
			{
				if (s == 2)
				{
					if(device.ui_alive == 0)
						continue;
					strcpy(type,get_type(dname,&makedevflag,&supp));
					strcpy(flag,get_flags(dname));
					all[ALL].sup =supp;
					all[ALL].node = -1;
					all[ALL].drive = device.ui_slave;
					sprintf(all[ALL].ultname,"%s%d",dname,device.ui_unit);
					sprintf(all[ALL].link,"%s",link);
					strcpy(all[ALL].type,type);
					strcpy(all[ALL].flags,flag);
					all[ALL].makedevflag = makedevflag;
					ALL++;
				 }
			}
		}
		if(devtype == 'D')
		{
			sprintf(link,"%s%d",pname,pnum);
			lseek(kmem,adpt.c_ptr,0);
			read(kmem,&device,sizeof(device));
			if(device.ui_alive != 1)
				continue;
			lseek(kmem,device.ui_devname,0);
			read(kmem,dname,sizeof(dname));
			strcpy (type, get_type (dname, &makedevflag,&supp));
			strcpy (flag, get_flags (dname));
			all[ALL].sup = supp;
			all[ALL].node = -1;
			if(strcmp(pname,"uba") == 0)
				if(CPUSUB == ST_VAXSTAR)
				{
					tmp = (u_long)device.ui_addr;
					all[ALL].csr = tmp;
				}
				else
					{
					all[ALL].csr = (u_short)device.ui_addr;
					}
			sprintf(all[ALL].ultname,"%s%d",dname,device.ui_unit);

			sprintf(all[ALL].link,"%s%d",pname,pnum);
			strcpy(all[ALL].type,type);
			strcpy(all[ALL].flags,flag);
			all[ALL].makedevflag = makedevflag;
			vectors(dname);
			if(strcmp(pname,"uba") != 0)
				if(strcmp(pname,"vaxbi") == 0)
					all[ALL].node = device.ui_nexus;
				else

/* put the node # in the controller line and stop */

					for(i=ALL-1;i>-1;i--)
						if(strcmp(all[i].ultname,link) == 0 && strncmp(all[i].link,"nexus",5) != 0)
						{
							all[i].node = device.ui_nexus;
							break;
						}

			ALL++;
		}
	}
	get_float(ubas);
	printuba();
	return;
}

/****************************************************************
*    getmba							*
*								*
*    now get info off the mba device				*
****************************************************************/

getmba()
{
	register int i;
	int u, supp,makedevflag = 0;
	char link[100], tmpstg[20], type[25], *get_type();



	for(i=0;cpu[i].cpuno != -1;i++)
		if(cpu[i].cpuno == CPU)
			for(u=0;u<cpu[i].maxmbas;u++)
				fprintf(fp,"adapter        mba%d      at  nexus ?\n",u);
	ctlr_index = reset_anythg (4);
	if (nl[4].n_type != N_UNDF)
	{
		ctlr_cnt = 0;
		while (get_mbac())
		{
			u = mdevice.mi_mbanum;
			if (u != '?')
				u += 060;
			sprintf (link, "mba%c", u);
			strcpy (type, get_type (mname, &makedevflag,&supp));

			print_mb (mname,mdevice.mi_unit,link,mdevice.mi_drive,
			mdevice.mi_alive,type,0,sname,makedevflag);
		}
	}

	ctlr_index = reset_anythg (5);

	if (nl[5].n_type != N_UNDF)
	{
		ctlr_index = reset_anythg (5);
		ctlr_cnt = 0;
		while (get_msdevice())
		{
			u = '?';
			if (u != '?')
				u += 060;
			sprintf (link, "mba%c", u);
			strcpy (type, get_type (sname, &makedevflag,&supp));
			sprintf (tmpstg, "%s%d", mname, sdevice.ms_ctlr);

			print_mb (sname,sdevice.ms_unit,link,sdevice.ms_ctlr,
			sdevice.ms_alive,type,sdevice.ms_slave,tmpstg,
			makedevflag);
		}
	}
	return;
}

/****************************************************************
*    print_mb							*
*								*
*    print massbus info						*
****************************************************************/

print_mb (unm, unit, link, csr, state, devnam, flags, snm, makedevflag)
char   *unm, *link, *devnam, *snm;
int unit, csr, state, flags, makedevflag;
{

	char    junk[STSIZE],ultname[20];


	sprintf(ultname,"%s%d",unm,unit);
	if (state > 0 && (csr < 0160000 || csr > 0167777))
	{
		if (makedevflag == 1)
			fprintf (fpdevs, "%s%d  ", unm, unit);

		if (strcmp (devnam, "disk") == 0
		|| strcmp (devnam, "master") == 0)
		{
			fprintf (fp, "%-15s%-10sat  %-10sdrive %d\n",
	    		devnam, ultname,link, csr);
		}
		else
		if (strcmp (devnam, "tape") == 0)
		{
			fprintf(fp,"%-15s%-10sat  %-10s",devnam,ultname,snm);
			if (strcmp (unm, "te") == 0
			|| strcmp (unm, "tj") == 0
			|| strcmp (unm, "ts") == 0
			|| strcmp (unm, "tms") == 0)
				fprintf (fp, "drive %d\n", flags);
			else
				fprintf (fp, "slave %d\n", flags);
		}
		else
		{
			fprintf (fp, "%-15s%-10sat  %-10scsr 0%6o    ",
			devnam, ultname, link, csr);

			if (flags > 0)
				fprintf (fp, "flags Ox%-6x    ", flags);
			junk[0] = '\0';
			vectors (unm, junk);
			fprintf (fp, "\n");
		}
	}
	return;
}

/****************************************************************
*    get_mbac							*
*								*
*    read a massbus device structure				*
****************************************************************/

get_mbac()
{
	ctlr_index = lseek (kmem, ctlr_index + ctlr_cnt, 0);
	ctlr_cnt = read (kmem, &mdevice, sizeof (mdevice));

    /* get the driver structure that is pointed to */

	lseek (kmem, mdevice.mi_driver, 0);
	read (kmem, &mdriver, sizeof (mdriver));

	lseek (kmem, mdriver.md_dname, 0);
	read (kmem, mname, sizeof (mname));

	lseek (kmem, mdriver.md_sname, 0);
	read (kmem, sname, sizeof (sname));

	return (mdevice.mi_driver ? 1 : 0);
}

/****************************************************************
*    get_msdevice						*
*								*
*    read an massbus slave device structure			*
****************************************************************/

get_msdevice()
{

	ctlr_index = lseek (kmem, ctlr_index + ctlr_cnt, 0);
	ctlr_cnt = read (kmem, &sdevice, sizeof (sdevice));

    /* get the driver structure that is pointed to */

	lseek (kmem, sdevice.ms_driver, 0);
	read (kmem, &mdriver, sizeof (mdriver));

	lseek (kmem, mdriver.md_dname, 0);
	read (kmem, mname, sizeof (mname));

	lseek (kmem, mdriver.md_sname, 0);
	read (kmem, sname, sizeof (sname));

	return (sdevice.ms_driver ? 1 : 0);
}

/****************************************************************
*    printuba							*
*								*
*    print unibus info						*
****************************************************************/

printuba()
{
	int i,j,num;

	for(i=0;i<ALL;i++)
	{
		if(strncmp(all[i].ultname,"idc",3) == 0)
			all[i].csr = 0175606;
		if(all[i].sup == 1)
			fprintf(fp,"#%-15s",all[i].type);
		else
			fprintf(fp,"%-15s",all[i].type);
		fprintf(fp,"%-10sat  %-10s",all[i].ultname,all[i].link);
		if(all[i].node != -1)
			fprintf(fp,"node%d  ",all[i].node);

		if(all[i].csr > 0)
		{
			if(CPU == MVAX_I || CPU == MVAX_II)
				if(CPUSUB == ST_VAXSTAR)
					fprintf(fp,"csr 0x%6x   ",all[i].csr);
				else
				{
					all[i].csr += 0160000;
					fprintf(fp,"csr 0%6o   ",all[i].csr);
				}
			else
				fprintf(fp,"csr 0%6o   ",all[i].csr);
			if(all[i].csr > 0160000 && all[i].csr < 0170000)
			{
				if(float_flag == 0)
				{
	fprintf(stdout,"\nThe installation software found these devices in the floating \naddress space:\n\n");
				}
				if(strcmp(all[i].type,"controller") == 0)
					j = i - 1;
				else
					j = i;
				fprintf(stdout,"\t%-10s\t",all[j].ultname);
				fprintf(stdout,"on %-10s\t",all[j].link);
				fprintf(stdout,"at 0%o\n",all[i].csr);
				float_flag = 1;
			}
		}
		if(all[i].flags[0] != '\0')
			fprintf(fp,"flags %s   ",all[i].flags);
		if(all[i].vectors[0] != '\0')
			fprintf(fp,"%s",all[i].vectors);
		if(all[i].drive != -1)
		{
			if(strncmp(all[i].ultname,"te",2) != 0
			&& strncmp(all[i].ultname,"tj",2) != 0
			&& strncmp(all[i].ultname,"ts",2) != 0
			&& strncmp(all[i].ultname,"tms",3) != 0
			&& strncmp(all[i].ultname,"st",2) != 0
			&& strcmp(all[i].type,"tape") == 0)
				fprintf(fp,"slave %d",all[i].drive);
			else
				fprintf(fp,"drive %d",all[i].drive);
		}
		fprintf(fp,"\n");
		if(all[i].makedevflag == 1)
		{
			if((CPU == MVAX_I || CPU == MVAX_II) && strncmp(all[i].ultname,"dhu",3) == 0)
			{
				sscanf(all[i].ultname,"dhu%d",&num);
				sprintf(all[i].ultname,"dhv%d",num);
			}
			if((CPU == MVAX_I || CPU == MVAX_II) && strncmp(all[i].ultname,"dz",2) == 0)
			{
				sscanf(all[i].ultname,"dz%d",&num);
				sprintf(all[i].ultname,"dzv%d",num);
			}
			fprintf(fpdevs,"%s  ",all[i].ultname);
		}
	}
	return;
}

/****************************************************************
*    get_type							*
*								*
*    get the type of a device and return			*
*								*
*    if passed address to makedevice is 0, then nothing		*
*    is returned in it.						*
*								*
*    also determine if device is supported = 0 or		*
*		unsupported = 1					*
****************************************************************/

char *get_type (s, makedevice,supp)
char *s;
int *makedevice;	/* 1 to make, 0 to not make */
int *supp;		/* 0 if supported; 1 if unsupported */
{
	char *name;
	struct nms *n;

	name = "#UNSUPPORTED";
	*makedevice = 0;
	*supp = 0;


	for (n = names; strcmp (n->uname, "\0") != 0; n++)
	{
		if (strcmp (n->uname, s) == 0)
		{
			name = n->devname;
			*makedevice = n->makedevflag;
			*supp = n->supp;
			break;
		}
	}
	return (name);
}
/****************************************************************
*    get_flags							*
*								*
*    get the flags for a device and return			*
*								*
****************************************************************/

char *get_flags (s)
char *s;
{
	char *flgs;
	struct f *n;

	flgs = "\0";


	for (n = fl; strcmp (n->name, "\0") != 0; n++)
	{
		if (strcmp (n->name, s) == 0)
		{
			flgs = n->flags;
			break;
		}
	}
	return (flgs);
}

/****************************************************************
*	get_config						*
*								*
*	get entry in config structure				*
****************************************************************/

get_config()
{
	config_indx = lseek(kmem,config_indx + config_cnt,0);
	config_cnt = read(kmem,&adpt,sizeof(adpt));
	pname[0] = '\0';
	lseek(kmem,adpt.p_name,0);
	read(kmem,pname,sizeof(pname));
	if(strcmp(pname,"\0") == 0)
		return(-1);
	lseek(kmem,&adpt.c_type,0);
	read(kmem,&devtype,sizeof(devtype));
	cnum = adpt.c_num;
	pnum = adpt.p_num;
	return(0);
}
/****************************************************************
*    reset_device						*
*								*
*   go back to beginning of a device structure			*
****************************************************************/

reset_device()
{
	dev_index = lseek (kmem, (long) nl[1].n_value, 0);
	return;
}
/****************************************************************
*    reset_anythg						*
*								*
*    go back to the begining of a device structure		*
****************************************************************/

int reset_anythg (thingindex)
int thingindex;
{
	int thg_index;

	thg_index = lseek(kmem,(long)nl[thingindex].n_value, 0);
		return (thg_index);
}

/****************************************************************
*    get_device							*
*								*
*    Get the next entry from the ubdinit structure.  If		*
*    match is 1 then look for a driver which is the		*
*    same as the one sitting in ctlr.um_driver.  If a		*
*    match is found then a 2 is returned.  Otherwise		*
*    when an entry is found return a 1 and when we get		*
*    to the end of the list return a 0.				*
*								*
*    match = 1 = match the driver pointed to by ctrl.um_driver	*
*    match = 0 = get the next entry.				*
*								*
*    return 0 means end of the list				*
*    return 1 means the next entry was found			*
*    return 2 means a match was found				*
****************************************************************/

get_device (match)
int match;
{


	/* read a device structure */

	dev_index = lseek (kmem, dev_index + dev_cnt, 0);
	dev_cnt = read (kmem, &device, sizeof (device));

	dname[0] = '\0';
	lseek (kmem, device.ui_devname, 0);
	read (kmem, dname, sizeof (dname));

    /* get the driver structure that is pointed to */

	lseek (kmem, device.ui_driver, 0);
	read (kmem, &driver, sizeof (driver));


	mname[0] = '\0';
	lseek (kmem, driver.ud_mname, 0);
	read (kmem, mname, sizeof (mname));

	if (match)
		if ((ctlr.um_driver == device.ui_driver) &&
		(ctlr.um_ctlr == device.ui_ctlr))
			return(2);
	else
		return(device.ui_driver ? 1 : 0);
}
/****************************************************************
*    getsysname							*
*								*
*    check the system name and if it is not a valid		*
*    name prompt the user for a correct system name.		*
*    Also create the system directory in /sys.			*
****************************************************************/

getsysname (sysname)
char   *sysname;
{
	char *ptr;
	int errorflag = 0;
	int rcode;

	while (1)
	{
		ptr = sysname;
		errorflag = 0;
		while (*ptr != '\0')
		{
			if ((*ptr > '@' && *ptr < '[') || (*ptr > '/' && *ptr < ':'))
				ptr++;
			else
			{
				fprintf (stderr,"ERROR!!  %c may not appear in the system name \n",*ptr);
				fprintf (stderr,"You may only use letters from A through Z and numbers. \n");
				errorflag = 1;
				break;
			}
		}
		if (errorflag == 0)
			break;
		else
		{
			fprintf (stdout, "Enter a valid system name --->");
			rcode = scanf ("%s", sysname);
			if(rcode < 1 || rcode  == EOF)
				quitonerror(-10);
		}
	}
	return;
}

/****************************************************************
*    getconfig							*
*								*
*    search the /install.tmp/.config file for any options,	*
*    psuedo-devices, controllers, etc, to add to the config 	*
*    file.  It's only arguement is the type of config file      *
*    entry it is to look for in .config.			*
****************************************************************/

getconfig(type)
char type[];
{
	FILE *fpconfig;
	char line[LSIZE],conftype[20];
 	int  i,len,configflag = 0;

	configflag = -1;
	if((fpconfig = fopen("/install.tmp/.config","r")) == NULL)
	{
 		return(-1);
	}
	while(fgets(line,80,fpconfig) != NULL)
	{
		sscanf(line,"%s",conftype);
		if(strcmp(type,"hardware") == 0)
		{
			for(i=0;hardwr[i].type[0] != '\0';i++)
				if(strcmp(conftype,hardwr[i].type) == 0)
					fprintf(fp,"%s",line);
		}
		else
		if(strcmp(conftype,type) == 0)
		{
			configflag = 0;
			if(strcmp(type,"swap") == 0 || strcmp(type,"dumps") == 0)
			{
				len = strlen(line);
				line[len-1] = ' ';
				strcat(line," \0");
			}
 			fprintf(fp,"%s",line);
 		}
	}
 	fclose(fpconfig);
	return(configflag);
}

/****************************************************************
*    vectors							*
*								*
*    search interrupt vector structure to determine		*
*    if this device has any interrupt routines.  If it		*
*    does, prints them out 					*
****************************************************************/

vectors (name)
char    name[];
{
	int i;

	i = 0;
	while (strcmp (intervec[i].devnam, "") != 0)
	{
		if (strcmp (intervec[i].devnam, name) == 0)
		{
			if (all[ALL].vectors[0] == '\0')
				strcat (all[ALL].vectors,"vector ");
			strcat (all[ALL].vectors,intervec[i].ivector);
			strcat (all[ALL].vectors,"  ");
		}
		i++;
	}

	return;
}

/************************************************************************
*    quitonerror							*
*									*
*    if an error occurs control is passed to this routine to allow for a*
*    graceful exit							*
************************************************************************/
quitonerror (code)
int code;
{

	switch (code)
	{
		case -1: 
			fprintf (stderr, "No namelist\n");
			break;

		case -2: 
			fprintf (stderr, "Cannot read memory\n");
			break;

		case -10: 
			fprintf (stderr, "TOO MANY ERRORS,  I QUIT\n");
			break;

		case -20: 
			fprintf (stderr,
	    		"cannot get the cpu number from kernel memory.\n");
			break;

		case -25: 
			fprintf (stderr, "Can not get physical memory size!\n");
			break;

		case -30: 
			fprintf (stderr,
			"cannot get the time zone from kernel memory.\n");
			break;

		case -40: 
			fprintf (stderr, "Cannot get info from kernel memory.\n");
			break;

		case -41: 
			fprintf (stderr, "cannot get device stats.\n");
			break;

		case -50: 
		case -60: 
		case -70: 
	    		break;

		case -80: 
			fprintf(stderr,"Can't get boot structure\n");
			break;
		case -90:
			fprintf(stderr,"Can't open defboo file\n");
			break;
		default: 
			fprintf (stderr, "ERROR Return code %d out of range!\n", code);
			break;
	}
	if (code < -10)
		fclose (fp);

	exit (-1);
}
/************************************************************************
*									*
* Display usage message							*
*									*
************************************************************************/
usage()
{

	fprintf(stderr,"\nUSAGE: sizer \n\n");
	fprintf(stderr,"-b\t\t Create a boot command file.\n");
	fprintf(stderr,"-c\t\t Returns cpu type.\n");
	fprintf(stderr,"-r\t\t Returns root device.\n");
	fprintf(stderr,"-k image\t Use image instead of /vmunix.\n");
	fprintf(stderr,"-n name\t Create a config file using this name.\n");
	fprintf(stderr,"-t timezone\t Use timezone in the config file.\n");
	return;
}
/***********************************************************************/
get_float(ubas)
int ubas;
{

	char ttype[15],tlink[10],flag[10],*get_flags();
	int unit,addr,retval,i,umem_start,makedevflag,rval;
	int gap = 2;  /*  gap between device types */
	int x,supp,tcsr;
	short shortint;
	struct d *ptr;

	if (CPUSUB == ST_VAXSTAR)	/* No Unibus or Qbus */
		return;

	if ((kumem = open ("/dev/kUmem", 0)) < 0)
		exit(1);

	if(CPU == MVAX_I || CPU == MVAX_II)
		umem_start = nl[18].n_value;	/* get start of qmem */
	else
		umem_start = nl[16].n_value;	/* get start of umem */
	
	for(i=0;i<ubas;i++)
	{
		if(CPU == MVAX_I || CPU == MVAX_II)
			addr = umem_start + QB_IO_OFFSET;
		else
			addr = umem_start + (i * 01000000) + 0760000;
		ptr = devs;
		addr += ptr->gap * 2;
		while(strcmp(ptr->name,"\0") != 0)
		{
			lseek(kumem,addr,0);
			retval = read(kumem,&shortint,sizeof(shortint));
			unit = 0;
			while(retval > 0)
			{
				if(retval > 0)
				{
					strcpy(ttype,get_type(ptr->name,&makedevflag,&supp));
					strcpy (flag, get_flags (ptr->name));
					sprintf(tlink,"uba%d",i);
					if(CPU == MVAX_I || CPU == MVAX_II)
						tcsr = (u_short)(addr - umem_start - QB_IO_OFFSET);
					else
						tcsr = (u_short)(addr - umem_start - 0600000 - (i * 01000000));
					rval = foundit(tcsr,tlink,ttype);
					if(rval == 0)
					{
						strcpy(all[ALL].type,ttype);
						strcpy(all[ALL].flags,flag);
						strcpy(all[ALL].link,tlink);
						if(strcmp("controller",all[ALL].type) != 0)
							all[ALL].csr = tcsr;

						all[ALL].makedevflag = makedevflag;
						all[ALL].sup = supp;
						if(supp == 0)
						{
							unit = getunit(ptr->name);
							sprintf(all[ALL].ultname,"%s%d",ptr->name,unit);
						}
						else
							strcpy(all[ALL].ultname,ptr->name);
						all[ALL].drive = -1;
						all[ALL].node = -1;
						vectors(ptr->name);
						ALL++;
						if(strcmp(all[ALL-1].type,"controller") == 0)
							float_ctlr(addr,umem_start,unit,all[ALL-1].ultname);
						unit++;
					}
					addr += ptr->gap * 2;
					x = (addr - umem_start) % (ptr->gap * 2);
					if(x > 0)
						addr += ptr->gap * 2 - x;
					lseek(kumem,addr,0);
					retval = read(kumem,&shortint,sizeof(shortint));
				}
				else
				{
					addr += gap;
					ptr++;
					if(ptr->gap == 0)
						continue;
					x = (addr - umem_start) % (ptr->gap * 2);
					if(x > 0)
						addr += ptr->gap * 2 - x;
					else
						addr += ptr->gap * 2;
				}
			}
			ptr++;
			if(ptr->gap == 0)
				continue;
			x = (addr - umem_start) % (ptr->gap * 2);
			if(x > 0)
				addr += (ptr->gap * 2) - x;
			else
				addr += ptr->gap * 2;
		}
	}
	return;
}
/**********************************************************************/
getunit(name)
char name[];
{

	register int i;
	int newunit,unit,len,found;
	char tmp[15];

	newunit = unit = found = 0;
	len = strlen(name);
	for(i=0;i<ALL;i++)
	{
		if((strncmp(name,all[i].ultname,len) == 0) &&
			(all[i].ultname[len] >= '0' && 
			all[i].ultname[len] <= '9'))
		{
			found = 1;
			sscanf(all[i].ultname,"%[a-z]%d",tmp,&newunit);
			if(newunit > unit)
				unit = newunit;
		}
	}
	if(found > 0)
		unit++;
	return(unit);
}
/*************************************************************************/
/* Check structure to see if this device has already been found.	 */
/* Devices are straight forward, but controllers are tricky.  For 	 */
/* controllers we must first find a device with the same csr as ours and */
/* then check the device before it (i - 1) to see if it is on the same   */
/* bus as we are.  This depends on the fact that the order of things in  */
/* the structure is as follows:						 */
/*									 */
/*	1. uda on uba	 2. uq on uda  at csr   3. ra on uq		 */
/*************************************************************************/
foundit(csr,link,type)
int csr;
char link[],type[];
{
	register int i;
	int found;

	found = 0;
	for(i=0;i<ALL;i++)
	{
		if(all[i].csr == csr)
			if(strcmp(all[i].link,link) == 0)
			{
				found = -1;
				break;
			}
			else
			if(strcmp(type,"controller") == 0)
				if(strcmp(all[i - 1].link,link) == 0)
				{
					found = -1;
					break;
				}
	}
	return(found);
}
/**************************************************************************/
float_ctlr(addr,umem_start,unit,name)
int addr,umem_start,unit;
char name[20];
{
	int ctlrunit,j,devunit;

	if(strncmp(name,"uda",3) == 0)
	{
		ctlrunit = getunit("uq");
		sprintf(all[ALL].ultname,"uq%d",ctlrunit);
		all[ALL].csr = (u_short)(addr - 0600000 - umem_start);
		strcpy(all[ALL].type,"controller");
		all[ALL].sup = 0;
		all[ALL].makedevflag = 0;
		sprintf(all[ALL].link,"uda%d",unit);
		all[ALL].drive = -1;
		all[ALL].node = -1;
		vectors("uq");
		ALL++;
		devunit = getunit("ra");		
		for(j=0;j<4;j++)
		{
			strcpy(all[ALL].type,"disk");
			sprintf(all[ALL].ultname,"ra%d",devunit);
			all[ALL].sup = 0;
			all[ALL].drive = j;
			all[ALL].node = -1;
			all[ALL].makedevflag = 1;    /* force the MAKEDEV */
			sprintf(all[ALL].link,"%s%d","uq",ctlrunit);
			devunit++;
			ALL++;
		}
	}
	if(strncmp(name,"klesiu",6) == 0)
	{
		ctlrunit = getunit("tmscp");
		sprintf(all[ALL].ultname,"tmscp%d",ctlrunit);
		all[ALL].csr = (u_short)(addr - 0600000 - umem_start);
		strcpy(all[ALL].type,"controller");
		all[ALL].makedevflag = 0;
		all[ALL].sup = 0;
		sprintf(all[ALL].link,"klesiu%d",unit);
		all[ALL].drive = -1;
		all[ALL].node = -1;
		vectors("tmscp");
		ALL++;
		devunit = getunit("tms");
		strcpy(all[ALL].type,"tape");
		sprintf(all[ALL].ultname,"tms%d",devunit);
		all[ALL].drive = 0;
		all[ALL].node = -1;
		all[ALL].sup = 0;
		all[ALL].makedevflag = 1;    /* force the MAKEDEV */
		sprintf(all[ALL].link,"%s%d","tmscp",ctlrunit);
		ALL++;
	}		
	if(strncmp(name,"hl",2) == 0)
	{
		ALL++;
		devunit = getunit("rl");
		for(j=0;j<4;j++)
		{
			strcpy(all[ALL].type,"disk");
			sprintf(all[ALL].ultname,"rl%d",devunit);
			all[ALL].drive = j;
			all[ALL].node = -1;
			all[ALL].sup = 0;
			all[ALL].makedevflag = 1;    /* force the MAKEDEV */
			sprintf(all[ALL].link,"%s%d","hl",unit);
			ALL++;
			devunit++;
		}
	}
	if(strncmp(name,"fx",2) == 0)
	{
		ALL++;
		devunit = getunit("rx");
		for(j=0;j<2;j++)
		{
			strcpy(all[ALL].type,"disk");
			sprintf(all[ALL].ultname,"rx%d",devunit);
			all[ALL].drive = j;
			all[ALL].node = -1;
			all[ALL].sup = 1;
			all[ALL].makedevflag = 1;    /* force the MAKEDEV */
			sprintf(all[ALL].link,"%s%d","fx",unit);
			ALL++;
			devunit++;
		}
	}

return;
}

