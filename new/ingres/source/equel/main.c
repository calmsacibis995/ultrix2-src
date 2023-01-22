#ifndef lint
static	char	*sccsid = "@(#)main.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	"constants.h"
# include	"globals.h"



/*
**  MAIN.C -- Start up routines
**
**	Usage:
**		equel {-d | -v | -c | -y | -f | -f<integer> | <name>.q}
**
**	Files:
**		standard output -- for diagnostics
**		<name>.q -- read
**		<name>.c -- created and written
**		any file appearing in a "#include" with a name
**		<name>.q.h -- read
**		<name>.c.h -- created and written
**
**	Flags:
**		possible arguments are:
**		-d -- enables run-time errors to have the file name
**		      and line number where they occurred reported
**		      Defaults to off.
**		-f -- specify the number of characters to fill
**		      an output line on quel commands 
**		      as being very high (to get C code on the 
**		      right line invariably).
**		-f<integer> -- fill output lines to integer chars
**		      (0 is like -f alone)
**		      Defaults to FILLCNT.
**		-y -- have the parser print debugging info
**		      Defaults to off.
**		-v -- (verbose) have the lexical analizer
**		      print the kind of token it sees.
**		      (only available if xDEBUG is defined)
**		      Defaults to off.
**		-c -- have every character read or backed up
**		      echoed (only if xDEBUG is defined)
**		      Defaults to off.
**		-r -- reset all previous flags to default
**		<name>.q -- name of a file to be equel'ed
**
**	Compilation Flags:
**		xDEBUG -- enables debugging flags -v and -c
**
**	Compilation Instructions:
**		to setup equel do :
**			setup equel; setup libq
*/
/*
**  MAIN --  invokes the pre-compiler on all the argument files
**
**	Parameters:
**		argc
**		argv
**
**	Returns:
**		none
*/
int		Exit_val = 0;			/* Value to exit with */

main(argc, argv)
int	argc;
char	**argv;
{
	extern char	**argproc();


	argv [argc] = 0;

	for (argv++; *argv; )
	{
		argv = argproc(argv);
		if (!Arg_error)
			equel(Input_file_name);
	}
	exit(Exit_val);
}

/*
**  ARGPROC -- process arguments on the command line
**	Arguments have effect on all the files following them
**	until a "-r" or an argument cancelling the first
**
**	Also performs global initializations.
**
**	Parameters:
**		argv -- a 0 terminated string vector with the
**		        command lines components.
**
**	Returns:
**		a new argv with all the leading arguments taken out
**
**	Side Effects:
**		sets certain variables for certain flags
**		  -d -- Rtdb
**		  -c -- Chardebug
**		  -v -- Lex_debug
**		  -y -- yydebug
**		  -f -- Fillcnt
**		  -r -- resets all variables to default values
**		Sets Arg_error on an argument error that should abort
**		the pre-processing of the file read.
*/

char **argproc(argv)
char	**argv;
{

	/* initializations for a new file */

	C_code_flg = Pre_proc_flg = 0;
	yyline = Newline = Lineout = 1;
	Block_level = Indir_level = In_string = Fillmode = 0;
	Charcnt = Lastc = In_quote = 0;
	Arg_error = 0;

	/* free C variable trees, and symbol space */
	freecvar(&C_locals);
	freecvar(&F_locals);
	freecvar(&C_globals);
	freecvar(&F_globals);

	symspfree();

	for ( ; *argv && **argv == '-'; argv++)
	{
		switch (*++*argv)
		{
			
#		  ifdef xDEBUG
		  case 'v' :
			Lex_debug = 'v';
			break;

		  case 'c' :
			Chardebug = 'c';
			break;
#		  endif

		  case 'y' :
			yydebug = 1;
			break;

		  case 'd' :
			Rtdb = 1;
			break;

		  case 'f' :		/* line fill */
			Fillcnt = atoi(++*argv);
			if (!Fillcnt)
				/* make SURE that C_CODE is put
				 * on line that it was typed in on
				 */
				Fillcnt = 30000;
			break;

		  case 'r' :		/* reset all flags to 
					 * their default values.
					 */
			yydebug = Rtdb = 0;
			Fillcnt = FILLCNT;
#			ifdef xDEBUG
			Lex_debug = Chardebug = 0;
#			endif
			break;

		  default :
			printf("equel: invalid option: '-%c'\n", **argv);
		}
	}
	if (*argv)
		Input_file_name = *argv++;
	else
	{
		printf("equel:  missing file name after arguments\n");
		Arg_error++;
	}
	return (argv);
}


/*
**  EQUEL -- invokes the precompiler for a non-included file
**
**	Parameters:
**		filename -- the name of the file to pre-compile
**
**	Returns:
**		none
**
**	Side Effects:
**		performs the preprocessing on <filename>
*/

equel(filename)
char		*filename;
{
	char		o_file [100];
	register	l;

	l = length(filename);
	if (l > sizeof o_file - 1)
	{
		printf("equel: filename \"%s\" too long\n",
		filename);
		return;
	}
	if (!sequal(".q", &filename [l - 2]))
	{
		printf("equel: EQUEL source files must end with .q\n");
		return;
	}
	bmove(filename, o_file, l - 2);
	bmove(".c", &o_file[l - 2], 3);
	Input_file_name = filename;
	In_file = Out_file = NULL;


	if ((In_file = fopen(filename, "r")) == NULL)
		cant("read", filename);
	else if ((Out_file = fopen(o_file, "w")) == NULL)
		cant("write", o_file);
	else if (!setexit())
		yyparse();

	/* if a reset(III) is done while processing
	 * an included file, then this closes all
	 * files skipped.
	 */
	while (restoref())
		;

	if (Out_file != NULL)
		fclose(Out_file);
	if (In_file != NULL)
		fclose(In_file);
	In_file = Out_file = NULL;
	if (Block_level != 0)
		yysemerr("unclosed block", 0);
	Input_file_name = 0;
}

/*
**  CANT -- print error message on failure to open a file
**
**	Parameters:
**		operation -- "read" or "write", according to what was
**			     attempted
**		filename -- the name of the file on which attempted
**
**	Returns:
**		none
**
**	Side Effects:
**		Prints error message on standard output
*/

cant(operation, filename)
char		*operation;
char		*filename;
{
	extern	char *sys_errlist[];
	extern	int sys_nerr;
	extern	int errno;
	char	ebuf[BUFSIZ];

	if (errno > 0 && errno <= sys_nerr)
		sprintf(ebuf, "Can't %s \"%s\": %s", operation, filename,
		    sys_errlist[errno]);
	else
		sprintf(ebuf, "Can't %s \"%s\": unknown error", operation,
		    filename);
	yysemerr(ebuf, 0);
}
