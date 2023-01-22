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

/************************************************************************
 *
 * Modification History:
 *
 * 08-Jan-87  rafiey (Ali Rafieymehr)
 *	Added two new ioctls to turn the video on and off. These two
 *	are to be used for save screen.
 *
 ************************************************************************/

/*
 *	@(#)qvioctl.h	1.8 (ULTRIX) 6/24/86
 *
 * Ioctl definitions for the qvss.
 *
 */
#ifdef KERNEL
#include "../vaxuba/qevent.h"
#include "../h/ioctl.h"
#include "../vaxuba/qvreg.h"
#else
#include <vaxuba/qevent.h>
#include <sys/ioctl.h>
#include <vaxuba/qvreg.h>
#endif

struct qv_kpcmd {
	char nbytes;		/* number of bytes in parameter */
	unsigned char cmd;	/* command to be sent, peripheral bit will */
				/* be forced by driver */
	unsigned char par[2];	/* bytes of parameters to be sent */
};
/*
 * qvss information block
 */

struct qv_info {
	short	mswitches;		/* current value of mouse buttons */
	vsCursor tablet;		/* current tablet position	*/
	short	tswitches;		/* current tablet buttons NI!	*/
	vsCursor cursor;		/* current cursor position	*/
	short	row;			/* screen row			*/
	short	col;			/* screen col			*/
	short	max_row;		/* max character row		*/
	short	max_col;		/* max character col		*/
	short	max_x;			/* max x position		*/
	short	max_y;			/* max y position		*/
	short	max_cur_x;		/* max cursor y position 	*/
	short	max_cur_y;		/* max cursor y position	*/
	char	*bitmap;		/* bit map position		*/
	short	*scanmap;		/* scanline map position	*/
	short	*cursorbits;		/* cursor bit position		*/
	struct	qvdevice *qvaddr;	/* virtual address of the csr	*/
	vsEvent *ibuff;			/* pointer to event queue	*/
	int 	iqsize;			/* may assume power of two 	*/
	int 	ihead;			/* atomic write			*/
	int 	itail;			/* atomic read			*/
	vsCursor mouse;			/* atomic read/write		*/
	vsBox	mbox;			/* atomic read/write		*/
	short	mthreshold;		/* mouse motion parameter	*/
	short	mscale;			/* mouse scale factor (if 
					   negative, then do square).	*/
};
typedef struct qv_info vsIoAddr;

#define QIOCGINFO 	_IOR(q, 1, struct qv_info)	/* get the info	 */
#define QIOCSMSTATE	_IOW(q, 2, vsCursor)		/* set mouse pos */
#define QIOCINIT	_IO(q, 4)			/* init screen   */
#define QIOCKPCMD	_IOW(q, 5, struct qv_kpcmd)	/* keybd. per. cmd */
#define QIOCADDR	_IOR(q, 6, struct qv_info *)	/* get address */
#define QIOVIDEOON	_IO(q, 7)			/* turn on the video */
#define	QIOVIDEOOFF	_IO(q, 8)			/* turn off the video */

