#ifndef lint
static	char	*sccsid = "@(#)vaxingres.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# include	<aux.h>
# include	<access.h>
# include	<func.h>


/*
**  Configuration table for VAX INGRES
*/

char	Qbuf[10000];
int	QbufSize = sizeof Qbuf;

int	Noupdt;


DESC	Reldes;
DESC	Attdes;
DESC	Inddes;
DESC	Treedes;
DESC	Prodes;
DESC	Intdes;

struct desxx	Desxx[] =
{
	"relation",	&Reldes,	&Admin.adreld,
	"attribute",	&Attdes,	&Admin.adattd,
	"indexes",	&Inddes,	NULL,
	"tree",		&Treedes,	NULL,
	"protect",	&Prodes,	NULL,
	"integrities",	&Intdes,	NULL,
	NULL
};


short	tTdbu[100];

extern struct fn_def	ParserFn;
extern struct fn_def	QryModFn;
extern struct fn_def	DefProFn;
extern struct fn_def	DefIntFn;
extern struct fn_def	DefViewFn;
extern struct fn_def	DeOvqpFn;
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
extern struct fn_def	SysDmpFn;

struct fn_def	*FuncVect[] =
{
	&ParserFn,	/* 0 -- parser */
	&QryModFn,	/* 1 -- qrymod (normal query) */
	&DefViewFn,	/* 2 -- define view */
	&DefIntFn,	/* 3 -- define integrity */
	&DefProFn,	/* 4 -- define permit */
	&DeOvqpFn,	/* 5 -- decomp/ovqp */
	&CreateFn,	/* 6 */
	&DstroyFn,	/* 7 */
	&RupdatFn,	/* 8 */
	&PrintFn,	/* 9 */
	&HelpFn,	/* 10 */
	&ResetrFn,	/* 11 */
	&CopyFn,	/* 12 */
	&SaveFn,	/* 13 */
	&ModifyFn,	/* 14 */
	&IndexFn,	/* 15 */
	&DsplayFn,	/* 16 */
	&SysDmpFn,	/* 17 -- unused */
	&RmqmFn,	/* 18 */
};

int	NumFunc = sizeof FuncVect / sizeof FuncVect[0];
