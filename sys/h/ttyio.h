
/*
 *	@(#)ttyio.h	1.3	(ULTRIX)	7/2/86
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * ttyio.h
 *
 * Modification history
 *
 * Common tty structures and definitions
 *
 *  6-Mar-86 - Ricky Palmer
 *
 *	Created original file with contents coming from ioctl.h. V2.0
 *
 * 13-May-86 - Miriam Amos
 *
 *	Add termio structure for svid termio.
 *
 *  2-Jul-86 - Tim Burke
 *
 *	Place ifdef around termio structure to aviod redefinition from
 *	the termio.h file.
 *
 */

/* Structure for terminal special characters */
struct tchars {
	char	t_intrc;		/* Interrupt			*/
	char	t_quitc;		/* Quit 			*/
	char	t_startc;		/* Start output 		*/
	char	t_stopc;		/* Stop output			*/
	char	t_eofc; 		/* End-of-file (EOF)		*/
	char	t_brkc; 		/* Input delimiter (like nl)	*/
};

/* Structure for local terminal special characters */
struct ltchars {
	char	t_suspc;		/* Stop process signal		*/
	char	t_dsuspc;		/* Delayed stop process signal	*/
	char	t_rprntc;		/* Reprint line 		*/
	char	t_flushc;		/* Flush output (toggles)	*/
	char	t_werasc;		/* Word erase			*/
	char	t_lnextc;		/* Literal next character	*/
};

/* Window size structure */
struct winsize {
	unsigned short	ws_row, ws_col; 	/* Window charact. size */
	unsigned short	ws_xpixel, ws_ypixel;	/* Window pixel size	*/
};

#define	NCC	8

#ifndef _TERM_ST_
#define _TERM_ST_
/* Termio structure */
struct termio {
	unsigned short	c_iflag;	/* input modes			*/
	unsigned short	c_oflag;	/* output modes			*/
	unsigned short	c_cflag;	/* control modes		*/
	unsigned short	c_lflag;	/* line discipline modes	*/
	char	c_line;			/* line disciple		*/
	unsigned char	c_cc[NCC];	/* control chars		*/
};
#endif _TERM_ST_
