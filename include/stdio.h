/*
 *		@(#)stdio.h	1.7	(ULTRIX)	6/2/86
 *		stdio.h	1.5	83/08/11
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *			Modification History				*
 *									*
 *	Lu Anne Van de Pas, 02-Jun-1986
 * 006  Added ending "/" to P_tmpdir string.  
 * 
 *	David L Ballenger, 22-Nov-1985
 * 005	Correct definition of sprintf() for System V environment.
 *
 *	David L Ballenger, 01-Aug-1985
 * 004	Add _IOAPPEND flag for files opened with "A" or "A+".
 *
 *	David L Ballenger, 26-Jun-1985
 * 003	Add changes so that FILE structures are allocated dynamically.
 *
 *	David L Ballenger, 13-Mar-1985					*
 * 0001	Add System V definitions.					*
 *									*
 *	Larry Cohen, 23-April-1985					*
 *      - change NFILE from 20 to 64
 ************************************************************************/

# ifndef FILE
#define	BUFSIZ	1024
#define _N_STATIC_IOBS 3
#define	_NFILE	64   /* should equal NOFILE in /sys/h/param.h */
extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;
	char	*_base;
	int	_bufsiz;
	short	_flag;
	char	_file;
} _iob[_N_STATIC_IOBS];

#define	FILE	struct _iobuf
#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])

#define _IOFBF		00000
#define	_IOREAD		00001
#define	_IOWRT		00002
#define	_IONBF		00004
#define	_IOMYBUF	00010
#define	_IOEOF		00020
#define	_IOERR		00040
#define	_IOSTRG		00100
#define	_IOLBF		00200
#define	_IORW		00400
#define _IOAPPEND	01000
#define	NULL	0
#define	EOF	(-1)

#define	getc(p)		(--(p)->_cnt>=0? *(p)->_ptr++&0377:_filbuf(p))
#define	getchar()	getc(stdin)
#define putc(x,p) (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p))
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	((p)->_file)

extern FILE	*fopen(), *fdopen(), *freopen(), *popen(), *tmpfile();
extern long	ftell();
extern void	rewind(), setbuf(), setbuffer(), setlinebuf();
extern char	*fgets(), *gets(), *ctermid(), *cuserid(), 
		*tempnam(), *tmpnam();
#ifndef SYSTEM_FIVE
extern char	*sprintf();
#else
extern int	sprintf();
#endif

#define L_ctermid	9
#define L_cuserid	9
#define P_tmpdir	"/usr/tmp/"
#define L_tmpnam	(sizeof(P_tmpdir)+15)

# endif
