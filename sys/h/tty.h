/* sccsid  =  @(#)tty.h	1.15	ULTRIX	10/3/86 */
/*
 * tty.h
 */


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

/*************************************************************************
 *			Modification History
 *
 *		April 4, 1985  -- Larry Cohen
 *	001	add TS_CLOSING state to signify that tty line is closing
 *		down and should not be opened until closed completely.
 *		Primarily to ensure DTR is kept down "long enough"
 *
 *		April 13, 1985 -- Larry Cohen
 *	002     add TS_INUSE to sigify that tty line is "in use" by
 *		another process.  Advisory in nature.  Always cleared
 *		when the process that set this flag exits or closes
 *		the file descriptor associated with the line.
 *
 *		10/1/85 -- Larry Cohen
 *	003	add t_winsiz to tty structure.
 *	
 *		1/22/85 -- Larry Cohen
 *		DEC standard 52 support.  MODEM definitions.
 *
 *		03/06/86 -- Miriam Amos
 *		Add t_iflag, t_oflag, t_cflag, and t_lflag for
 *		termio interface.
 *
 *		06/17/86 -- Tim Burke
 * 		Added two state definitions (TS_LRTO, TS_LTACT) which are used
 *		in termio min/time interaction to specify that a timeout is in
 *	    	progress and that the min/time has been examined.
 *
 *		08/14/86 -- Tim Burke
 *		Added one #define for VAXstar SLU driver dec standard 52.
 *
 *		08/26/86 -- Tim Burke
 *		Changed name of #define added on 8/14/86.
 */
#ifdef KERNEL
#include "../h/ttychars.h"
#include "../h/ttydev.h"
#else
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif
#ifndef _TTY_
#define _TTY_
#ifndef _TERMIO_
#define NCC	8
#endif _TERMIO_
#endif _TTY_

/*
 * A clist structure is the head of a linked list queue
 * of characters.  The characters are stored in blocks
 * containing a link and CBSIZE (param.h) characters. 
 * The routines in tty_subr.c manipulate these structures.
 */
struct clist {
	int	c_cc;		/* character count */
	char	*c_cf;		/* pointer to first char */
	char	*c_cl;		/* pointer to last char */
};

/*
 * Per-tty structure.
 *
 * Should be split in two, into device and tty drivers.
 * Glue could be masks of what to echo and circular buffer
 * (low, high, timeout).
 */
struct tty {
	union {
		struct {
			struct	clist T_rawq;
			struct	clist T_canq;
		} t_t;
		struct {
			struct	buf *T_bufp;
			char	*T_cp;
			int	T_inbuf;
			int	T_rec;
		} t_n;
	} t_nu;
	struct {
		struct	buf *H_bufp;
		char	*H_in;
		char	*H_out;
		char 	*H_base;
		char	*H_top;
		int	H_inbuf;
		int	H_read_cnt;
		} t_h;
	struct	clist t_outq;		/* device */
	int	(*t_oproc)();		/* device */
	struct	proc *t_rsel;		/* tty */
	struct	proc *t_wsel;
				caddr_t	T_LINEP;	/* ### */
	caddr_t	t_addr;			/* ??? */
	dev_t	t_dev;			/* device */
	int	t_flags;		/* some of both */
	unsigned short	t_iflag;	/* termio input modes */
	unsigned short	t_oflag;	/* termio output modes */
	unsigned short	t_cflag;	/* termio control modes */
	unsigned short	t_lflag;	/* termio line discipline modes */
	int	t_sysv;			/* flag for sysv ioctl calls */
	int	t_state;		/* some of both */
	short	t_pgrp;			/* tty */
	char	t_delct;		/* tty */
	char	t_line;			/* glue */
	char	t_col;			/* tty */
	char	t_ispeed, t_ospeed;	/* device */
	char	t_rocount, t_rocol;	/* tty */
	unsigned char	t_cc[NCC];	/* termio control chars */
	struct	ttychars t_chars;	/* tty */
	struct	winsize t_winsize;	/* window size */
#ifdef notdef
	u_char	t_modem;		/* software copy of modem signals */
	struct  timeval t_timestamp;	/* primarily for CD drops */
#endif
#define	t_rawq	t_nu.t_t.T_rawq		/* raw characters or partial line */
#define	t_canq	t_nu.t_t.T_canq		/* raw characters or partial line */

#define	t_bufp	t_nu.t_n.T_bufp		/* buffer allocated to protocol */
#define	t_cp	t_nu.t_n.T_cp		/* pointer into the ripped off buffer */
#define	t_inbuf	t_nu.t_n.T_inbuf	/* number chars in the buffer */
#define	t_rec	t_nu.t_n.T_rec		/* have a complete record */
#define	h_bufp	t_h.H_bufp		/* buffer allocated to protocol */
#define	h_in	t_h.H_in		/* next input position in buffer */
#define	h_out	t_h.H_out		/* next output position in buffer */
#define	h_base	t_h.H_base		/* pointer to base of buffer */
#define	h_top	t_h.H_top		/* pointer to top of buffer */
#define	h_inbuf	t_h.H_inbuf	/* number chars in the buffer */
#define	h_read	t_h.H_read_cnt	/* number chars to read in */

#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#define MODEM_BADCALL 0x10

/* be careful of tchars & co. */
#define	t_erase		t_chars.tc_erase
#define	t_kill		t_chars.tc_kill
#define	t_intrc		t_chars.tc_intrc
#define	t_quitc		t_chars.tc_quitc
#define	t_startc	t_chars.tc_startc
#define	t_stopc		t_chars.tc_stopc
#define	t_eofc		t_chars.tc_eofc
#define	t_brkc		t_chars.tc_brkc
#define	t_suspc		t_chars.tc_suspc
#define	t_dsuspc	t_chars.tc_dsuspc
#define	t_rprntc	t_chars.tc_rprntc
#define	t_flushc	t_chars.tc_flushc
#define	t_werasc	t_chars.tc_werasc
#define	t_lnextc	t_chars.tc_lnextc
};

#define	TTIPRI	28
#define	TTOPRI	29

/* limits */
#define	NSPEEDS	16
#define	TTMASK	15
#define	OBUFSIZ	100
#define	TTYHOG	255
#ifdef KERNEL
short	tthiwat[NSPEEDS], ttlowat[NSPEEDS];
#define	TTHIWAT(tp)	tthiwat[(tp)->t_ospeed&TTMASK]
#define	TTLOWAT(tp)	ttlowat[(tp)->t_ospeed&TTMASK]
extern	struct ttychars ttydefaults;
#endif

/* internal state bits */
#define	TS_TIMEOUT	0x000001	/* delay timeout in progress */
#define	TS_WOPEN	0x000002	/* waiting for open to complete */
#define	TS_ISOPEN	0x000004	/* device is open */
#define	TS_FLUSH	0x000008	/* outq has been flushed during DMA */
#define	TS_CARR_ON	0x000010	/* software copy of carrier-present */
#define	TS_BUSY		0x000020	/* output in progress */
#define	TS_ASLEEP	0x000040	/* wakeup when output done */
#define	TS_XCLUDE	0x000080	/* exclusive-use flag against open */
#define	TS_TTSTOP	0x000100	/* output stopped by ctl-s */
#define	TS_HUPCLS	0x000200	/* hang up upon last close */
#define	TS_TBLOCK	0x000400	/* tandem queue blocked */
#define	TS_RCOLL	0x000800	/* collision in read select */
#define	TS_WCOLL	0x001000	/* collision in write select */
#define	TS_NBIO		0x002000	/* tty in non-blocking mode */
#define	TS_ASYNC	0x004000	/* tty in async i/o mode */
#define	TS_ONDELAY	0x008000	/* device is open; software copy of 
					 * carrier is not present */
/* state for intra-line fancy editing work */
#define	TS_BKSL		0x010000	/* state for lowercase \ work */
#define	TS_QUOT		0x020000	/* last character input was \ */
#define	TS_ERASE	0x040000	/* within a \.../ for PRTRUB */
#define	TS_LNCH		0x080000	/* next character is literal */
#define	TS_TYPEN	0x100000	/* retyping suspended input (PENDIN) */
#define	TS_CNTTB	0x200000	/* counting tab width; leave FLUSHO alone */


#define	TS_IGNCAR	0x400000	/* ignore software copy of carrier */
#define	TS_CLOSING	0x800000	/* closing down line */
#define TS_INUSE	0x1000000	/* line is in use */
#define TS_LRTO		0x2000000	/* Raw timeout - used with min/time */
#define TS_LTACT	0x4000000	/* Timeout active, used with min/time */


#define	TS_LOCAL	(TS_BKSL|TS_QUOT|TS_ERASE|TS_LNCH|TS_TYPEN|TS_CNTTB)

/* define partab character types */
#define	ORDINARY	0
#define	CONTROL		1
#define	BACKSPACE	2
#define	NEWLINE		3
#define	TAB		4
#define	VTAB		5
#define	RETURN		6
