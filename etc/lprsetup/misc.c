#ifndef lint
static char *sccsid = "@(#)misc.c	1.2	ULTRIX	10/16/86";
#endif

#include "lprsetup.h"
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 **********************************************************************/

/*
 *	Adapted for ULTRIX-32
 *	from ULTRIX-11 version 
 */
FILE *Popen();

/*********************************
* YesNo - get a yes or no answer
* Returns TRUE=(yes), FALSE=(no)
**********************************/
YesNo (def_ans)
int    def_ans;
{
    int     ans, done;
	
    done = FALSE;
    ans = 0;
    while (NOT done)
    {
	ans = getcmd ();
	switch (ans)
	{
	    case NOREPLY: 
		done = TRUE;
		if (def_ans == 'y')
		    ans = TRUE;
		else
		    if (def_ans == 'n')
			ans = FALSE;
		break;
	    case YES: 
		done = TRUE;
		ans = TRUE;
		break;
	    case NO: 
	    case QUIT: 
		done = TRUE;
		ans = FALSE;
		break;
	    case HELP: 
	    default: 
		printf ("\nPlease answer yes or no  [%c] ", def_ans);
	}
    }
    return (ans);
}

/******************************************************************
* UseDefault
*	returns TRUE if NULL entered (meaning use default) or
*	FALSE after reading text to use in place of default
******************************************************************/
UseDefault (str, def, i)
char   *str, *def;
int	i;		/* index into table of current symbol name */
			/* which is used for more specific help messages */
{
    extern char symbolname[];
    extern struct table tab[];
    int    done;

    /******************
    * loop until done
    ******************/
    done = FALSE;
    while (!done)
    {
        printf (" [%s] ?  ", def);
	switch (getsymbol ())
	{
	    default: 	/* this should never match! */
	    case GOT_SYMBOL: 
		strcpy (str, symbolname);
		return (FALSE);
		break;
	    case QUIT: 
		return (QUIT);
		break;
	    case HELP: 
		realhelp(i);		/* more specific help */
		printf (h_default); 	/* a general help message */
		printf ("\nEnter a value for '%s'? ", tab[i].name, def);
		break;
	    case NOREPLY: 
		if (str != def)
	    	    strcpy (str, def);
		return (TRUE);
		break;
	}
    }
}

/*********************************************
* realhelp
*	provides specific help for a given
*	symbol index.
*********************************************/

realhelp(i)
int	i;
{
    extern struct table tab[];

    switch(i) {
	case H_af:
		printf(h_af);
		break;

	case H_br:
		printf(h_br);
		break;

	case H_cf:
		printf(h_cf);
		break;

	case H_df:
		printf(h_df);
		break;

	case H_dn:
		printf(h_dn);
		break;

	case H_du:
		printf(h_du);
		break;

	case H_fc:
		printf(h_fc);
		break;

	case H_ff:
		printf(h_ff);
		break;

	case H_fo:
		printf(h_fo);
		break;

	case H_fs:
		printf(h_fs);
		break;

	case H_gf:
		printf(h_gf);
		break;

	case H_ic:
		printf(h_ic);
		break;

	case H_if:
		printf(h_if);
		break;

	case H_lf:
		printf(h_lf);
		break;

	case H_lo:
		printf(h_lo);
		break;

	case H_lp:
		printf(h_lp);
		break;

	case H_mc:
		printf(h_mc);
		break;

	case H_mx:
		printf(h_mx);
		break;

	case H_nf:
		printf(h_nf);
		break;

	case H_of:
		printf(h_of);
		break;

	case H_op:
		printf(h_op);
		break;

	case H_os:
		printf(h_os);
		break;

	case H_pl:
		printf(h_pl);
		break;

	case H_pp:
		printf(h_pp);
		break;

	case H_pw:
		printf(h_pw);
		break;

	case H_px:
		printf(h_px);
		break;

	case H_py:
		printf(h_py);
		break;

	case H_rf:
		printf(h_rf);
		break;

	case H_rm:
		printf(h_rm);
		break;

	case H_rp:
		printf(h_rp);
		break;

	case H_rs:
		printf(h_rs);
		break;

	case H_rw:
		printf(h_rw);
		break;

	case H_sb:
		printf(h_sb);
		break;

	case H_sc:
		printf(h_sc);
		break;

	case H_sd:
		printf(h_sd);
		break;

	case H_sf:
		printf(h_sf);
		break;

	case H_sh:
		printf(h_sh);
		break;

	case H_st:
		printf(h_st);
		break;

	case H_tf:
		printf(h_tf);
		break;

	case H_tr:
		printf(h_tr);
		break;

	case H_ts:
		printf(h_ts);
		break;

	case H_vf:
		printf(h_vf);
		break;

	case H_xc:
		printf(h_xc);
		break;

	case H_xf:
		printf(h_xf);
		break;

	case H_xs:
		printf(h_xs);
		break;

	default:
		printf("Sorry, no specific help is available for symbol '%s'\n",
			tab[i].name);
		break;
    }
}

/*********************************************
* MapLowerCase
*     maps the given string into lower-case.
**********************************************/
MapLowerCase (b)
char   *b;
{
    while (*b)
    {
	if (isascii (*b) && isupper (*b))
	    *b = tolower (*b);
	b++;
    }
}

HasBadChars (b)
char   *b;
{
    while (*b)
    {
	if ((NOT isalpha (*b)) && (NOT isdigit (*b)) && (*b != '_'))
	    return (TRUE);
	b++;
    }
    return (FALSE);
}

/************************************************
*  Print the symbol table
* print whole table, or just the 'used' symbols 
*************************************************/
Print (flag)
int     flag;
{
    extern struct table tab[];
    extern char pnum[];
    extern char ptype[];
    int     i, j;

    printf ("\n\tPrinter #%s ",pnum);
    printf("\n\t----------");
    if (strlen(pnum) > 1)
	printf("-");		/* add one for printers numbered 10...99 */
    printf ("\nSymbol ");
    if (flag == ALL)
	printf ("used ");

    printf (" type  value\n");
    printf ("------ ");
    if (flag == ALL)
	printf ("---- ");  /* under 'used' */

    printf (" ----  -----\n");

    /*************************************************
    * for each symbol, print name, type, used, value 
    **************************************************/
    for (i=0; tab[i].name != 0; i++)
    {
   	 /* don't print it, if not being used now */
	if ((flag == USED) && (tab[i].used == NO))
	    continue;

	printf ("  %s   ", tab[i].name);
	if (flag == ALL)
	    printf ("%s", tab[i].used == YES ? "YES  " : " NO  ");

	switch (tab[i].stype)
	{
	    case BOOL: 
		printf (" BOOL ");
		break;
	    case INT: 
		printf (" INT  ");
		break;
	    case STR: 
		printf (" STR  ");
		break;
	    default: 
		printf (" ???    ??????\n");
		continue;	/* get next symbol */
	}
 
 /***
	if ((flag == ALL) && (tab[i].used == NO)) {
		printf("\n");
		continue;
	}
  ***/

	if ((tab[i].nvalue != 0) && (tab[i].nvalue != '\0')) {
	    printf ("  %s", tab[i].nvalue);
	}
	else if ((tab[i].svalue != 0) && (tab[i].svalue != '\0')) {
	    printf ("  %s", tab[i].svalue);
	}
	printf("\n");		/* end the line */
    }
}

/********************************************************************
* Verified
*	print the current printcap data, ask if it is OK, and return
*	TRUE if it is OK, otherwise false.	
*********************************************************************/
Verified ()
{
    extern char pnum[];
	int yn;
/*
 *  Clear all waiting input and output chars.
 *  Actually we just want to clear any waiting input chars so
 *  we have a chance to see the values before confirming them.
 *  We have to sleep a second to let waiting output chars print.
 */
    sleep (1);
    ioctl (0, TIOCFLUSH, 0);

    Print (USED);		/* print values being used in current
				   configuration  */

    printf ("\nAre these the final values for printer %s ? [y] ", pnum);
    fflush (stdout);
	yn = 'y';
    if (YesNo (yn) == TRUE) {
	printf("\n");
	return (TRUE);
    }
    else {
	printf("\n");
	return (FALSE);
    }
}

/********************************************
*  DoSymbol - adds/modifies symbols.
********************************************/
DoSymbol ()
{
    extern struct table tab[];		/* default printer table	*/
    extern char symbolname[];		/* getcmd result		*/
    extern char oldfilter[];		/* print filter before modify	*/
    extern char ptype[];		/* for checking on 'af' use	*/
    extern char isnotused[];		/* "...feature is not used in LP11... */
    char     newval[LEN];		/* new value entered		*/
    char *addr, *curval;		/* malloc and current value	*/
    int     i, done = FALSE;
	int yn;

    /* 
     * find the symbol, print current value, and
     * ask for the new value, or initial value,
     * if any.
     */
    if (strlen (symbolname) > 2)
    {
	printf ("\nSymbol name '%s' is too long!\n", symbolname);
	return;
    }

    /* symbolname contains the line just read from stdin */
    for (i = 0; tab[i].name != 0; i++)
    {
	if (strcmp (tab[i].name, symbolname) == 0)
	{
	    curval = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;
	    break;
	}
    }
    if (tab[i].name == 0)
    {
	printf("\nSymbol '%s' not found.  Use the 'list' command for a\n",
		symbolname);
	printf("complete list of all of the symbols and their defaults.\n");
	return(ERROR);
    }

    /*
     * got symbol, now prompt for new value
     */
    do
    {
	printf ("\nEnter a new value for symbol '%s'?  [%s] ",
		tab[i].name, curval);
	switch (getsymbol() )
	{
	    case QUIT: 
		return (QUIT);
		break;
	    case HELP: 
		realhelp(i);		/* more specific help */
		printf (h_default);
		break;
	    case NOREPLY: 
    		tab[i].used = YES;
		done = TRUE;
		break;
	    case GOT_SYMBOL: 
	            default:
		strcpy (newval, symbolname);
		printf ("\nNew '%s' is '%s', is this correct?  [y] ",
			tab[i].name, newval);
			yn = 'y';
		if (YesNo (yn) == TRUE)
		{
		    if (validate(i, newval) < 0)	/* check if valid */
			continue;

		    if ((addr = (char *) malloc (strlen (newval) + 1)) == NULL)
		    {
			printf ("\nmalloc: cannot get space for symbol '%s'.\n",
				tab[i].name);
			return(ERROR);
		    }

		    if (strcmp (newval, curval) != 0)
		    {
    		    	tab[i].used = YES;
		    	tab[i].nvalue = addr;
		    	strcpy (tab[i].nvalue, newval);
		    }
		    done = TRUE;
		}
		break;
	}
    } while (NOT done);
}

/*
 * validate: check that 
 *	EITHER 
 *		entered value = "none" to remove entry
 *	OR
 *		int's are all digits
 *		baud rates are legal 
 * 		booleans are on/off
 *		directory names must all start with '/' 
 *			(except "lp" for remote should be "null")
 *
 * returns -1 if bad, else 0.
 */
validate(i,value1)
int i;
char *value1;
{
	extern struct table tab[];
	int retval = OK;	/* return value from this routine */
	int k;			/* loop counter */
	char value[LEN];	/* value of the symbol just entered */

	/*
	 * save the symbol value locally
	 */
	if (strcmp("none",value1)==0) {	/* "none" = delete this entry */
		tab[i].nun = 1;
		strcpy(value1,"");
		retval = OK;
		return (retval);
	}

	    if (value1 != NULL)
	    	strcpy(value, value1);
	    else
	    	strcpy(value, "");		/* probably can't happen */

	switch (tab[i].stype)
	{
	    case BOOL: 
		/* Booleans can only be on or off */
		if    (!(!strcmp("on", value)
		    || (! strcmp("ON", value))
		    || (! strcmp("off", value))
		    || (! strcmp("OFF", value))))
		{
		    printf("\nSorry, boolean symbol '%s' can only be 'on' or 'off'\n", tab[i].name);
		    retval=BAD;
		}
		break;

	    case INT: 
		/* check first that we have all digits */
		for (k = 0; value[k]!='\0'; k++)
		{
		    if (! isdigit(value[k]))
		    {
			printf("\nSorry, integer symbol '%s' must contain only digits (0 - 9).\n", tab[i].name);
			retval=BAD;
			break;
		    }
		}

		/*
		 * See if BR was specified and check if it is valid
		 * only if we haven't encountered an error yet.
		 */
		if ((strcmp("br", tab[i].name) == 0) && (retval != BAD))
		{
		    switch(atoi(value))
		    {
		    case 0:
		    case 50:
		    case 75:
		    case 110:
		    case 134:
		    case 150:
		    case 200:
		    case 300:
		    case 600:
		    case 1200:
		    case 1800:
		    case 2400:
		    case 4800:
		    case 9600:
		    case 19200:
		    case 38400:	
			break;	/* baud rate OK */

		    default:
			printf("\nSorry, illegal baudrate: %s\n", value);
			printf("\nAvailable baud rates are:\n");
			printf("\t   0\t  134\t   600\t  4800\n");
			printf("\t  50\t  150\t  1200\t  9600\n");
			printf("\t  75\t  200\t  1800\t 19200\n");
			printf("\t 110\t  300\t  2400\t 38400\n");
			retval = BAD;
			break;
		    }
		}
		break;

	    case STR: 
		/*  NOT YET   DEBUG  
		if ((strcmp ("lp", tab[i].name)==0) 
			&& (strcmp("remote",ptype)==0)
			&& (strcmp("null",value)==0))
			{
				strcpy(value1,"");
				retval = OK;
				return(retval);
			}
		*/
		/* check if name is special and must start with '/' */
		if     (!strcmp ("af", tab[i].name) 	/* accounting file */
		    || (!strcmp ("cf", tab[i].name))	/* cifplot filter */
		    || (!strcmp ("df", tab[i].name))	/* TeX DVI filter */
		    || (!strcmp ("dn", tab[i].name))	/* daemon name */
		    || (!strcmp ("gf", tab[i].name))	/* plot filter */
		    || (!strcmp ("if", tab[i].name))	/* text/acct filter */
		    || (!strcmp ("lf", tab[i].name))	/* logfile */
		    || (!strcmp ("lp", tab[i].name))	/* device name */
		    || (!strcmp ("nf", tab[i].name))	/* ditroff filter */
		    || (!strcmp ("of", tab[i].name))	/* output filter */
		    || (!strcmp ("rf", tab[i].name))	/* FORTRAN filter */
		    || (!strcmp ("sd", tab[i].name))	/* spool directory */
		    || (!strcmp ("tf", tab[i].name))	/* troff filter */
		    || (!strcmp ("vf", tab[i].name))	/* raster filter */
		    || (!strcmp ("xf", tab[i].name)))	/* passthru filter */
		{
		    if (value[0] != '/') 
		    {
			printf("\nSorry, the value of symbol '%s' must begin with '/'.\n",tab[i].name);
			retval=BAD;
		    }
		}
		break;

	    default: 
		printf ("lprsetup: bad type %d for symbol %s.\n",tab[i].stype,tab[i].name);
		retval=BAD;
	}
	return(retval);
}

/*****************************************************
* Read a line and return the symbol; only knows about
* quit and help, but none of the other commands.
*****************************************************/
getsymbol ()
{
    extern struct cmdtyp cmdtyp[];
    extern char symbolname[];

    int     i, length, retval;
    register char  *q;
    char    line[BUF_LINE];	/* input line */
    char    line2[BUF_LINE];	/* saved version of the input line */

    if (fgets (line, BUF_LINE, stdin) == NULL) {
	printf ("\n");	/* EOF (^D) */
	return (QUIT);
    }
    if (line[0] == '\n') {
	return (NOREPLY);
    }
    if (line[strlen (line) - 1] == '\n') {
	line[strlen (line) - 1] = NULL;
    }
    for (q = line; isspace (*q); q++) {
	; /* nop */
    }

    strcpy(line2, line);	/* save original entry, including caps */
    MapLowerCase(line);

    length = strlen(line);
    for (i = 0; cmdtyp[i].cmd_name; i++) {
	if (strncmp(cmdtyp[i].cmd_name, line, length) == 0) {
	    retval = cmdtyp[i].cmd_id;
	    break;
	}
    }

    strcpy(symbolname, line2);	/* save symbol name globaly */
    if ((strcmp(symbolname, "help") == 0)
	|| (strcmp(symbolname, "quit") == 0)
	|| (strcmp(symbolname, "?") == 0)){
	return(retval);		/* return command id only if quit or help */
    } else {
	return (GOT_SYMBOL);	/* else return a symbol */
    }
}

/*****************************************************
* Read a line, decode the command, return command id
* This routine knows about the full command set.
*****************************************************/
getcmd ()
{
    extern struct cmdtyp cmdtyp[];
    extern char symbolname[];

    int     i, length;
    register char  *q;
    char    line[BUF_LINE];	/* input line */

    if (fgets (line, BUF_LINE, stdin) == NULL)
    {				/* EOF (^D) */
	printf ("\n");
	return (QUIT);
    }

    if (line[0] == '\n')
	return (NOREPLY);

    if (line[strlen (line) - 1] == '\n')
	line[strlen (line) - 1] = NULL;

    for (q = line; isspace (*q); q++)/* strip leading blanks */
	;
    MapLowerCase (line);

    length = strlen (line);
    for (i = 0; cmdtyp[i].cmd_name; i++)
	if (strncmp (cmdtyp[i].cmd_name, line, length) == 0)
	    return (cmdtyp[i].cmd_id);/* command id */

    strcpy (symbolname, line);	/* save symbol name globaly */
    return (GOT_SYMBOL);
}

leave (status)
int     status;
{
    exit (status);
}

/**************************
*  free nvalue when done
**************************/
freemem ()
{
	extern struct table tab[];
	int i;

	for (i = 0; tab[i].name != 0; ++i)
	if (tab[i].nvalue > 0)
		free (tab[i].nvalue);
}

/*
 *	finish printer setup
 *	notify user of success
 */
setupdone()
{
	printf("\nSet up activity is complete for this printer.\n");
	printf("Verify that the printer works properly by using\n");
	printf("the lpr(1) command to send files to the printer.\n");

	return;
}

/****************************************
* edit the ttys file and change the line
* to an appropriate setting: 
*	"off" for adding/modifying a printer
* 	"on"  for deleting a printer
****************************************/
fixtty(line,mode)
char *line;	/* */
int mode;	/* 0 == off, 1 == on */
{
	FILE *fp;

	fp = Popen(EDTTY, "w");
	if (fp == NULL) {
		perror("popen(%s)\n", EDTTY);
		fprintf(stderr, "\n/etc/ttys not edited (could not change line %s to %s).\n", line, mode ? "on" : "off");
		return(OK);
	} 
	if (mode == 0) {
		dprintf(stderr, 
		"/^%s[ 	]/s/[ 	]on[ 	]/	off /\n", line);
		fprintf(fp, 
		"/^%s[ 	]/s/[ 	]on[ 	]/	off /\n", line);
	} else {
		dprintf(stderr, 
		"/^%s[ 	]/s/[ 	]off[ 	]/	on /\n", line);
		fprintf(fp, 
		"/^%s[ 	]/s/[ 	]off[ 	]/	on /\n", line);
	}
	fprintf(fp, "w\n");
	fprintf(fp, "q\n");
	fflush(fp);
	pclose(fp);
	kill(1, SIGHUP);	/* send hangup to init */
	return(OK);
}

FILE*	
Popen(s1, s2)
char *s1, *s2;
{
	FILE	*fp;
	close(1);
	fp = popen(s1, s2);
	dup2(3, 1);
	return(fp);
}

/******************************************************************************
* end of misc.c
******************************************************************************/
