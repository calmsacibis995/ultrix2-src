#ifndef lint
static	char	*sccsid = "@(#)finder.c	1.14	(ULTRIX)	3/18/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#include <stdio.h>
#include <nlist.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/devio.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/dkio.h>
#include <sys/param.h>
#include <vax/cpu.h>
#include <vax/rpb.h>

#define		MAXDISKS	32	/* maximum number of any disk type */
#define		MAXTAPES	4	/* maximum number of any tape type */

struct c
{
    int cpuno;
    char *cpustrg;
    int maxusers;
    int maxubas;
    int maxmbas;
} cpu[] =
{
    VAX_780,	 "VAX780",	  32,	 4,	 4,
    VAX_750,	 "VAX750",	  32,	 2,	 2,
    VAX_730,	 "VAX730",	  16,	 1,	 0,
    VAX_8200,	 "VAX8200",	  32,	 7,	 0,
    VAX_8600,	 "VAX8600",	 128,	 7,	 4,
    VAX_8800,	 "VAX8800",	 128,	 7,	 4,
    MVAX_I,	 "MVAX",	   4,	 0,	 0,
    MVAX_II,	 "MVAX",	   8,	 1,	 0,
    VAX_MAX,	 "VAX_MAX",	  32,	 0,	 0,
    -1, 	 "**UNDEFINED*..*CPU**", 0, 0, 0
    };



struct d 
{
	char *type;			/* Ultrix name of device */
	int majnum;			/* major number of device */
};


struct d roots [] =			/* disks that can be root devices */
	 {
 	"hp",	4,	"ra",	9,	"rb",	23,	"rd",	47,
	"\0",	-1
	 };

struct d installs [] =			/* install devices */
	{
	"tu",	5,	"ts",	16,	"mu",	19,	"rx",	47,
	"tms",	36,	"ra",	9,	"st",	46,	"\0",	-1
	};

struct d disks [] =			/* all disk devices */
	{
	"hp",	4,	"ra",	9,	"rk",	11,	"rd",	47,
	"rb",	23,	"rl",	32,	"\0",	-1
	};


struct f {
	char type[20];			/* device type (TU77,RA60...) */
	char name[20];			/* Ultrix name */
	int unit;			/* Unit number assigned by kernel */
	int plug;			/* physical plug number */
	int link;			/* controller device is linked to */
	int linktype;			/* bus type */
	int trlevel;			/* TR level or BI node number */
	int bus_num;			/* BI number or SBI number*/
};

struct f found [100];			/* devices found */
int FMAX = 0;

struct devget dev_info;			/* IOCTL structure */
struct rpb rpb;
/**************************************************************************/
/* FINDER:								  */
/*		This routine finds all the devices that can be used as    */
/*	installation devices,root devices, or all disks.  It searches for */
/*	devices in the previously defined in the structures.  A system    */
/*	call, using mknod, is issued to create a device special file for  */
/*	each device.  The special file is then opened and an ioctl call   */
/*	is issued to the device.  Information returned is stored in the   */
/*	'found' structure and is printed after the device list has been   */
/*	exhausted.							  */
/* 									  */
/**************************************************************************/
/*  Modification History						  */
/*									  */	
/*  000 - July,   1986	- Bob Fontaine created				  */
/*									  */
/*  001 - August, 1986  - Bob Fontaine					  */	
/*   		Added the message for boot command.			  */
/*  									  */
/*  002 - August 14, 1986 -  Tungning Cherng				  */
/*		Added the -f flag to reduce the redundant check in order  */
/*			to have a faster output.		          */
/*									  */
/**************************************************************************/

main(argc,argv)
int argc;
char *argv[];
{

	int answer;
	char ans[8], line[128];
	register int i;
	FILE *fp1, *fp2;
	char *path1="/tmp/finder.tab";
	char *path2="/tmp/finder.dev";
	int fflag=0;
	int rflag=0;

	switch(argv[1][1])
	{

		case 'i':
			getinstalls();
			break;

		case 'r':
			getroots();
			rflag++;
			break;

		case 'd':
			getdisks();
			break;

		case 'f':
			/* to reduce redundant check, show the previous form */
			fflag++;
			break;
		default:
			fprintf(stderr,"usage: finder -[irdf]\n");
			break;
	}
	if(FMAX == 0 && fflag==0)
	{
		exit(-1);
	}
	if (fflag==0)
	{
		if ((fp1=fopen(path1,"w"))==NULL)
		{
			fprintf(stderr,"open %s failed\n",path1);
			exit(-1);
		}
		if ((fp2=fopen(path2,"w"))==NULL)
		{
			fprintf(stderr,"open %s failed\n",path2);
			exit(-1);
		}
		fprintf(fp1,"\tSELECTION\tDIGITAL\t\tDEVICE\t\tCONTROLLER\n");
		fprintf(fp1,"\tNUMBER\t\tNAME\t\tNUMBER\t\tNUMBER\n\n");
		for(i=0;i<FMAX;i++)
		{
			fprintf(fp1,"\t   %d\t\t%-15s\t",i+1,found[i].type);
			fprintf(fp1,"   %d\t\t",found[i].plug);
			fprintf(fp1,"   %d\n",found[i].link);
			fprintf(fp2,"%s %s %d\n",found[i].type,found[i].name,
				found[i].unit);
		}
		fclose(fp1);
		fclose(fp2);
	}
	if ((fp1=fopen(path1,"r"))==NULL)
	{
		fprintf(stderr,"read %s failed\n",path1);
		exit(-1);
	}
	if ((fp2=fopen(path2,"r"))==NULL)
	{
		fprintf(stderr,"read %s failed\n",path2);
		exit(-1);
	}
	answer = 0;
	for (;;)
	{
		for (FMAX= -3; fgets(line, sizeof(line), fp1)!=NULL; FMAX++)
			fprintf (stderr,"%s",line);
		fprintf(stderr,"\nEnter your selection number [no default]: ");
		answer=atoi(gets(ans));
		if(FMAX < answer || answer <= 0)
		{
			fprintf(stderr,"\nPlease select a number between 1 - %d .\n\n",FMAX);
			(void) rewind(fp1);
		}
		else 
		{
			answer--;
			break;
		}
	}
	fclose(fp1);
	for (i=0; fgets(line,sizeof(line),fp2)!=NULL && i!=answer; i++)
		;
	printf("%s",line);
	fclose(fp2);	
	if (rflag)
		showboot(answer);
	exit(0);
}
/**************************************************************************/
getinstalls()
{

	register int i;
	int retval,times,to,dev;
	char command[50],spec[25],devfile[20];
	struct d *n;



	for(n=installs;strcmp(n->type,"\0") != 0;n++)
	{
		if(strcmp(n->type,"ra") ==0)
			times = MAXDISKS;
		else
			times = MAXTAPES;
		for(i=0;i<times;i++)
		{
			sprintf(spec,"%s%d",n->type,i);
			to = 0;
			if(strcmp(n->type,"ra") == 0 || strcmp(n->type,"rx") == 0)
			{
				sprintf(devfile,"r%sa",spec);
				dev = makedev(n->majnum,i*8);
			}
			else
			{
				strcpy(devfile,"rmt0h");
				dev = makedev(n->majnum,i);
			}

			mknod(devfile,0020666,dev);
			to = 0;
			to = open(devfile,O_RDONLY | O_NDELAY);
			if(to < 0)
			{
				unlink(devfile);
				continue;
			}
			dev_info.device[0] = '\0';
			if(ioctl(to,DEVIOCGET,(char *)&dev_info) < 0)
			{
				unlink(devfile);
				close(to);
				continue;
			}
			if(strcmp(n->type,"ra") == 0)
				if(strcmp(dev_info.device,"RA60")!= 0)
					if(strncmp(dev_info.device,"RX",2)!= 0)
					{
						unlink(devfile);
						close(to);
						continue;
					}
			if(strcmp(n->type,"rx") == 0)
				if(strcmp(dev_info.device,"RX33") != 0)
				{
					unlink(devfile);
					close(to);
					continue;
				}
			strcpy(found[FMAX].name,n->type);
			strcpy(found[FMAX].type,dev_info.device);
			found[FMAX].unit = i;
			found[FMAX].plug = dev_info.slave_num;
			found[FMAX].link = dev_info.ctlr_num;
			FMAX++;
			close(to);
			unlink(devfile);
		}
	}
	return;
}
/***************************************************************************/
getroots()
{

	register int i;
	int retval,times,to,dev;
	char command[25],spec[25],devfile[10];
	struct d *n;


	for(n=roots;strcmp(n->type,"\0") != 0;n++)
	{
		for(i=0;i<MAXDISKS;i++)
		{
			sprintf(devfile,"r%s%da",n->type,i);
			dev = makedev(n->majnum,i*8);
			mknod(devfile,0020666,dev);
			to = open(devfile,O_RDONLY | O_NDELAY);
			if(to < 0)
			{
				unlink(devfile);
				continue;
			}
			if((retval=ioctl(to,DEVIOCGET,(char *)&dev_info)) < 0)
			{
				close(to);
				unlink(devfile);
				continue;
			}
			if(strncmp(dev_info.device,"RX",2) != 0
			&& strncmp(dev_info.device,"RL",2) != 0)
			{
				strcpy(found[FMAX].name,n->type);
				strcpy(found[FMAX].type,dev_info.device);
				found[FMAX].linktype = dev_info.bus;
				found[FMAX].trlevel = dev_info.nexus_num;
				found[FMAX].unit = i;
				found[FMAX].plug = dev_info.slave_num;
				if(strncmp(dev_info.device,"RM,2") == 0)
					found[FMAX].link = dev_info.bus_num;
				else
					found[FMAX].link = dev_info.ctlr_num;
				found[FMAX].bus_num = dev_info.adpt_num;
				FMAX++;
			}
			close(to);
			unlink(devfile);
		}
	}

}
/***************************************************************************/
getdisks()
{

	register int i;
	int retval,times,to,dev;
	char command[25],spec[25],devfile[10];
	struct d *n;


	for(n=disks;strcmp(n->type,"\0") != 0;n++)
	{
		for(i=0;i<MAXDISKS;i++)
		{
			sprintf(devfile,"r%s%da",n->type,i);
			dev = makedev(n->majnum,i*8);
			mknod(devfile,0020666,dev);
			to = open(devfile,O_RDONLY | O_NDELAY);
			if(to < 0)
			{
				unlink(devfile);
				continue;
			}
			if(ioctl(to,DEVIOCGET,(char *)&dev_info) < 0)
			{
				close(to);
				unlink(devfile);
				continue;
			}
			if(strncmp(dev_info.device,"RX",2) != 0)
			{
				strcpy(found[FMAX].name,n->type);
				strcpy(found[FMAX].type,dev_info.device);
				found[FMAX].unit = i;
				found[FMAX].plug = dev_info.slave_num;
				if(strncmp(dev_info.device,"RM",2) == 0)
					found[FMAX].link = dev_info.bus_num;
				else
					found[FMAX].link = dev_info.ctlr_num;
				FMAX++;
			}
			unlink(devfile);
			close(to);

		}
	}

}
/***************************************************************/
getcpu()
{
	int indx,anyint,mem;


	mem = open ("/dev/kmem", 0);


	indx = 0;
	indx = lseek(mem,0x80000000,0);
	read(mem,&rpb,sizeof(rpb));

	return(rpb.cpu);
}
/***************************************************************************/
showboot(i)
int i;
{

	int cpu;
	FILE *fp;
	char *syshalt="after the installation software halts the processor: ";
	char *youhalt="after you halt the processor: ";
	char *howhalt="\
Please wait for the message indicating that the processor can be halted.\n\
To halt the processor,";
	char *step1="^P   	( CTRL/P, to display the console mode prompt )";
	char *step2=">>> H	( H, to halt the processor )";
	fp=fopen("/tmp/showboot","w");
	fprintf(fp,"\nType the following boot sequence at the console mode prompt\n");
	cpu = getcpu();
	switch(cpu)
	{
		case MVAX_I:
			fprintf(fp,"%s\n",youhalt);
			fprintf(fp,"\n>>> b\n\n");
			fprintf(fp,"\n%s press and release the front panel HALT button.\n", howhalt);
			break;

		case MVAX_II:
			fprintf(fp,"%s\n",syshalt);
			fprintf(fp,"\n>>> b dua%x\n\n",found[i].plug);
			break;

		case VAX_730:
/* need to fix this when we really know whats going on with 730 bus type */

			fprintf(fp,"%s\n",youhalt);
			fprintf(fp,"\n>>> d/g 3 %x\n",found[i].plug);
			if(strcmp(found[i].type,"R80") == 0)
				fprintf(fp,"\n>>> @ubaidc.cmd\n\n");
			else
				fprintf(fp,"\n>>> @ubara.cmd\n\n");
			fprintf(fp,"%s type:\n%s\n",howhalt,step1);
			break;

		case VAX_750:
			fprintf(fp,"%s\n",youhalt);
			if(found[i].linktype == DEV_UB)
				fprintf(fp,"\n>>> b dua%x\n\n",found[i].plug);
			if(found[i].linktype == DEV_MB)
				fprintf(fp,"\n>>> b db%c%x\n\n",
					found[i].link + 'a', found[i].plug);
			fprintf(fp,"%s type:\n%s\n",howhalt,step1);
			break;

		case VAX_780:
			fprintf(fp,"%s\n",youhalt);
			fprintf(fp,"\n>>> d r1 %x\n",found[i].trlevel);
			fprintf(fp,">>> d r3 %x\n",found[i].plug);
			if(found[i].linktype == DEV_UB)
				fprintf(fp,">>> @ubara.cmd\n\n");
			if(found[i].linktype == DEV_MB)
				fprintf(fp,">>> @mbahp.cmd\n\n");
			fprintf(fp,"%s type:\n%s\n%s\n",howhalt,step1,step2);
			break;
			
		case VAX_8200:
			fprintf(fp,"%s\n",youhalt);
			if(found[i].linktype == DEV_UB)
				fprintf(fp,"\n>>> b du%x%x\n\n",found[i].trlevel,found[i].plug);
			if(found[i].linktype == DEV_BI)
				fprintf(fp,"\n>>> b du%x%x\n\n",found[i].trlevel,found[i].plug);
			fprintf(fp,"%s type:\n%s\n",howhalt,step1);
			break;

		case VAX_8600:
			fprintf(fp,"%s\n",youhalt);
			fprintf(fp,"\n>>> d r1 %x%x\n",found[i].bus_num,found[i].trlevel);
			fprintf(fp,">>> d r3 %x\n",found[i].plug);
			if(found[i].linktype == DEV_UB)
				fprintf(fp,">>> @ubara.com\n\n");
			if(found[i].linktype == DEV_MB)
				fprintf(fp,">>> @mbahp.com\n\n");
			fprintf(fp,"%s type:\n%s\n%s\n",howhalt,step1,step2);
			break;

		case VAX_8800:
			fprintf(fp,"%s\n",youhalt);
			fprintf(fp,"\n>>> d r1 %x%x\n",found[i].bus_num,found[i].trlevel);
			fprintf(fp,">>> d r3 %x\n",found[i].plug);
			if(found[i].linktype == DEV_UB)
				fprintf(fp,">>> @ubara.com\n\n");
			if(found[i].linktype == DEV_BI)
				fprintf(fp,">>> @bdara.com\n\n");
			fprintf(fp,"%s type:\n%s\n%s\n",howhalt,step1,step2);
			break;
		default:
			fprintf(fp,"\n**** Unspported processor ****\n");
			break;
	}
	fclose(fp);
	return;
}

