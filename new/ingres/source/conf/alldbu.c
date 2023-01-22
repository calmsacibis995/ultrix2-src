#ifndef lint
static	char	*sccsid = "@(#)alldbu.c	1.1	(ULTRIX)	1/10/85";
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

# include	<func.h>




char	Qbuf[1500];
int	QbufSize = sizeof Qbuf;

int	Noupdt;
short	tTdbu[100];

extern struct fn_def	CopyFn;
extern struct fn_def	CreateFn;
extern struct fn_def	DstroyFn;
extern struct fn_def	HelpFn;
extern struct fn_def	DsplayFn;
/* extern struct fn_def	KsortFn; */
extern struct fn_def	ModifyFn;
extern struct fn_def	PrintFn;
extern struct fn_def	ResetrFn;
extern struct fn_def	RmqmFn;
extern struct fn_def	RupdatFn;
extern struct fn_def	SaveFn;
extern struct fn_def	IndexFn;

struct fn_def	*FuncVect[] =
{
	/* 0 -- create */	&CreateFn,
	/* 1 -- destroy */	&DstroyFn,
	/* 2 -- update */	&RupdatFn,
	/* 3 -- print */	&PrintFn,
	/* 4 -- help */		&HelpFn,
	/* 5 -- resetrel */	&ResetrFn,
	/* 6 -- copy */		&CopyFn,
	/* 7 -- save */		&SaveFn,
	/* 8 -- modify */	&ModifyFn,
	/* 9 -- index */	&IndexFn,
	/* 10 -- display */	&DsplayFn,
	/* 11 -- unused */	&PrintFn,
	/* 12 -- remqm */	&RmqmFn,
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];
