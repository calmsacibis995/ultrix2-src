#ifndef lint
static char *sccsid = "@(#)gethost.c	1.4	ULTRIX	6/5/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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
#include <ctype.h>
#include <sys/file.h>
#include <sys/../vax/rpb.h>
#include <sys/../sas/vmb.h>
main()
{
	struct rpb rpb;
	struct vmb_info vmb;
	char	srvip[40];
	char	cliip[40];
	char	netmsk[40];
	char	brdcst[40];
	char	syscall[60];
	char	netdev[5];
	int	offset;
	int	kernel;
	int	dot=4;
	int 	i;
	FILE	*hosts;
	/******* get rpb struct from kernel space **********************/
	kernel=open("/dev/kmem",O_RDONLY);
	if(kernel < 0)
		exit(2);
	lseek(kernel,0x80000000,0);
	read(kernel,&rpb,sizeof(struct rpb));
	/******* assure that a netboot has occured and set netdev ******/
	if(rpb.devtyp == BTD$K_QNA) 
		sprintf(netdev,"qe0");
	else	if(rpb.devtyp == BTD$K_KA640_NI)
			sprintf(netdev,"se0");
		else	exit(2);
	offset = ((int) rpb.vmbinfo);
	if(offset==0)
		exit(2);
	/******** get vmb_info struct from kernel space ****************/
	lseek(kernel,offset,0);
	read(kernel,&vmb,sizeof(struct vmb_info));
	close(kernel);
	/******** extract server and client ip addresses ***************/
	exip(vmb.netblk.srvipadr,srvip);
	exip(vmb.netblk.cliipadr,cliip);
	/******** create /etc/hosts file *******************************/
	/******** 	entry 1 = client *******************************/
	/********       entry 2 = server *******************************/
	hosts=fopen("/etc/hosts","w");
	if(hosts==NULL)
		exit(2);
	fprintf(hosts,"127.0.0.1 localhost\n");
	fprintf(hosts,"%s %s\n",cliip,vmb.netblk.cliname);
	fprintf(hosts,"%s %s\n",srvip,vmb.netblk.srvname);
	fclose(hosts);
	/******** create netstart file *********************************/
	/******** 	sets hostname and runs ifconfig (like rc.local)*/
	hosts=fopen("/netstart","w");
	if(hosts==NULL)
		exit(2);
	/******* form broadcast ****************************************/ 
	for(i=0;cliip[i] == srvip[i] && cliip[i] != '\0';i++)
		{
		if(cliip[i] == '.')
			dot--;
		brdcst[i]=cliip[i];
		}
	while(brdcst[i--] != '.');
	i++;
	while(dot--)
		{
		brdcst[i++]='.';
		brdcst[i++]='0';
		}
	brdcst[i]='\0';
	strcpy(netmsk,"255.0.0.0");
	fprintf(hosts,"/bin/hostname %s\n",vmb.netblk.cliname);
	fprintf(hosts,"/etc/ifconfig %s `/bin/hostname` broadcast %s netmask %s\n",netdev,brdcst,netmsk);
	fprintf(hosts,"/etc/ifconfig lo0 localhost\n");
	fclose(hosts);
	/******** output SERVER and CLIENT assignments for potential eval ***/
	fprintf(stdout,"SERVER=%s\n",vmb.netblk.srvname);
	fprintf(stdout,"CLIENT=%s\n",vmb.netblk.cliname);
	fclose(hosts);
	/******** execute netstart ******************************************/
	chmod("/netstart",0500);
	system("/netstart");
	exit(0);
}
/************************************************************************/
/*									*/
/*	exip - extract ip string from a long				*/
/************************************************************************/
exip(ipaddr,str)
	unsigned long ipaddr;
	char *str;
{
int i=0,j=0,flg=0;
int numb[4];
numb[0]= ((ipaddr >> 24) & 0377);
numb[1]= ((ipaddr >> 16) & 0377);
numb[2]= ((ipaddr >> 8) & 0377);
numb[3]= (ipaddr & 0377);
sprintf(str,"%d.%d.%d.%d",numb[0],numb[1],numb[2],numb[3]);
return(0);
}
