/*
 *		@(#)y.tab.h	1.1	(ULTRIX)	1/8/85
 */

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


typedef union 
{
	struct disp_node	*u_dn;
} YYSTYPE;
extern YYSTYPE yylval;
# define APPEND 257
# define COPY 258
# define CREATE 259
# define DEFINE 260
# define DELETE 261
# define DESTROY 262
# define HELP 263
# define INDEX 264
# define MODIFY 265
# define PRINT 266
# define INTEGRITY 267
# define RANGE 268
# define REPLACE 269
# define RETRIEVE 270
# define SAVE 271
# define UNIQUE 272
# define PERMIT 273
# define VIEW 274
# define INGRES 275
# define EXIT 276
# define PARAM 277
# define TYPE 278
# define ALLOC 279
# define STRUCT 280
# define STRUCT_VAR 281
# define ALL 282
# define BY 283
# define FROM 284
# define IN 285
# define INTO 286
# define IS 287
# define OF 288
# define ON 289
# define ONTO 290
# define TO 291
# define WHERE 292
# define UNTIL 293
# define AT 294
# define NAME 295
# define SCONST 296
# define I2CONST 297
# define I4CONST 298
# define F8CONST 299
# define C_CODE 300
# define COMMA 301
# define LPAREN 302
# define RPAREN 303
# define PERIOD 304
# define QUOTE 305
# define BGNCMNT 306
# define ENDCMNT 307
# define LBRACE 308
# define RBRACE 309
# define LBRKT 310
# define RBRKT 311
# define NONREF 312
# define SEMICOL 313
# define POINTER 314
# define COLON 315
# define UOP 316
# define BOP 317
# define BDOP 318
# define EOP 319
# define LBOP 320
# define LUOP 321
# define FOP 322
# define FBOP 323
# define AOP 324
# define unaryop 325
