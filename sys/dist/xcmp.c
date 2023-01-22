#ifndef lint
static	char	*sccsid = "@(#)xcmp.c	1.1		11/3/84";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1983 by				*
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


#define	BLOCKSIZE	(512)
#define	FASTYPE	char

#include	<stdio.h>
main(argc,argv)
char	*argv[];
{
	register	FASTYPE	*ptr1 , *ptr2;
	register	short	size,where;
	short	fd1,fd2,count,buffer_size;
	char	*p1,*p2,*buf1,*buf2;

	
	if(0 > (fd1 = open(argv[1],0,0))){
		perror(argv[1]);
		exit(-1);
	}
	if(0 > (fd2 = open(argv[2],0,0))){
		perror(argv[2]);
		exit(-1);
	}
	if(argc <= 3){
		buffer_size = 22 * BLOCKSIZE;
		count= -1;
	} else {
		buffer_size = atoi( argv[3] );
		if(buffer_size < 512 ) buffer_size *= BLOCKSIZE;
		if(argc == 5) {
			count=atoi(argv[4]);
		} else {
			fprintf(stderr,"xcmp file1 file2 xfer_size count\n");
			exit(-1);
		}
	}

	if(!(buf1 = (char *)malloc(buffer_size))){
		fprintf(stderr,"Not enough memory\n");
		exit(-1);
	}
	if(!(buf2 = (char *)malloc(buffer_size))){
		fprintf(stderr,"Not enough memory\n");
		exit(-1);
	}

	for(where = 0;where < count;where ++){
		if(-1 == (size = read(fd1,buf1,buffer_size))){
			perror(argv[1]);
			exit(-1);
		}
		if(size == 0){
			fprintf(stderr,"End of file on %s\n",argv[1]);
			exit(0);
		}
		if(size > read(fd2,buf2,buffer_size)){
			perror(argv[2]);
			fprintf(stderr,"where = %d, size = %d\n",where ,size);
			exit(-1);
		}
		ptr1 = (FASTYPE *)buf1;
		ptr2 = (FASTYPE *)buf2;
		size=buffer_size;
		while(size--){
			if(*(char *)ptr1++ != *(char *)ptr2++){
				fprintf(stderr,"Verify failed\n");
				fprintf(stderr,"where = %d, size = %d\n"
					,where,size);
				exit(-1);
			}
		}
	}
	printf("Verify succeeded\n");
}
		
