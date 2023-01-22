#ifndef lint
static	char	*sccsid = "@(#)makpkt.c	1.2	(ULTRIX)	10/3/86";
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
#include <sas/vmb.h>
main(argc,argv)
char	*argv[];
int	argc;
{
struct	netblk nblk;
FILE	*pkt;	/* client parameter packet 	*/
int	k,i,j,c,n=0;
char	filnam[80];
sprintf(filnam,"%s.c",argv[5]);
pkt=fopen(filnam,"w");
if(pkt == NULL)
	exit(2);
nblk.selflg=atoi(argv[8]);
nblk.srvipadr=exip(argv[2]);
nblk.cliipadr=exip(argv[4]);
fprintf(pkt,"#include <sas/vmb.h>\n");
fprintf(pkt,"struct netblk 	nblk ={\n");
fprintf(pkt,"\"%s\",\n",argv[1]);
fprintf(pkt,"%ld,\n",nblk.srvipadr);
fprintf(pkt,"\"\",\n");
fprintf(pkt,"0,\n");
fprintf(pkt,"\"%s\",\n",argv[3]);
fprintf(pkt,"%ld,\n",nblk.cliipadr);
fprintf(pkt,"%d,\n",nblk.selflg);
fprintf(pkt,"\"%s\",\n",argv[6]);
fprintf(pkt,"\"%s\",\n",argv[7]);
fprintf(pkt,"};\n");
exit(0);
}
/************************************************************************/
/*									*/
/*	exip - extract ip address into a long				*/
/************************************************************************/
exip(str)
	char *str;
{
int i=0,j=0,flg=0;
char numb[4];
char temp;
unsigned long	retval=0;
while(str[i] != '\0')
	{
	temp=str[i+1];
	str[i+1]='\0';
	if(isdigit(str[i]))
		{
		if(flg==0)
			{
			flg=1;
			j=0;
			retval <<= 8;
			numb[j++]=str[i];
			}
		else	numb[j++]=str[i];
		}
	else	if(flg==1)
			{
			flg=0;
			numb[j]='\0';
			retval+= atoi(numb);
			}
	i++;
	str[i]=temp;
	}
return(retval);
}
