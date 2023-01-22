#ifndef lint
static	char	*sccsid = "@(#)invert.c	1.2	(ULTRIX)	4/23/85";
#endif lint

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
 *	David L Ballenger, 16-Apr-1985					*
 * 0001	Change tmpfile to tempfile so that it doesn't clash with 	*
 *	tmpfile() defined in <stdio.h>					*
 *									*
 ************************************************************************/
/*  input:  records of lines, separated by blank lines
    output: key:file1 start/length ... start/length:file2 start/length ...
*/

# include "stdio.h"
# include "streams.h"
# include "bib.h"
# define isnull(x)  (*(x) == NULL)
# define makelow(c) ('A'<=(c) && (c)<='Z' ? (c)-'A'+'a' : c)

int     max_kcnt = 100;     /*  max number of keys                      */
int     max_klen =   6;     /*  max length of keys                      */
char    *ignore =           /*  string of line starts to ignore         */
            "CNOPVX";
char    *common =           /*  name of file of common words            */
            COMFILE;
char    *INDEX=             /*  name of output file                     */
            INDXFILE;

char    *tempfile =          /*  name of temporary file                  */
            INVTEMPFILE;

int	silent = 0;	    /*  0 => statistics printed			*/
			    /*  1 => no statisitics printed		*/

char *sort_it =
        "sort -u +0 -1 +1 -2 +2n -3 +3n %s -o %s";
char sortcmd[maxstr];

int     argc;
char    **argv;

main(argcount,arglist)
int argcount;
char **arglist;
{   char            *filename;
    FILE            *input, *output;
    long int        start,length;
    char            word[maxstr];
    int             kcnt;
    char            tag_line[maxstr];

    long int	    records = 0;  /*  number of records read           */
    long int	    keys    = 0;  /*  number of keys read (occurences) */
    long int	    distinct;     /*  number of distinct keys          */
    long int	    shorten();

    argc= argcount-1;
    argv= arglist+1;
    mktemp(tempfile);
    output= fopen(tempfile,"w");

    for ( flags() ; argc>0 ; argc--, argv++ ,flags() )
    {   /* open input file              */
            filename=   *argv;
            input=      fopen(filename,"r");
            if (input==NULL)
            {   fprintf(stderr, "invert: error in open of %s\n", filename);
                continue;
            }
            start=      0L;
            length=     0L;

        for(;;) /* each record  */
        {   /* find start of next record (exit if none)     */
                start= nextrecord(input,start+length);
                if (start==EOF)   break;
            records++;
	    kcnt= 0;
            length= recsize(input,start);
            sprintf(tag_line, " %s %D %D\n", filename, start, length);

            while (ftell(input) < start+length && kcnt < max_kcnt)
            {   getword(input,word,ignore);
                makekey(word,max_klen,common);
                if (!isnull(word))
                {   fputs(word,output); fputs(tag_line,output);
                    kcnt++; keys++;
                }
            }
        }
        fclose(input);
    }
    fclose(output);

    sprintf(sortcmd, sort_it, tempfile, tempfile);
    system(sortcmd);

    distinct = shorten(tempfile,INDEX);
    if( silent == 0 )
	fprintf(stderr,
	    "%D documents   %D distinct keys  %D key occurrences\n",
	    records, distinct, keys);
}



/*  Flag    Meaning                             Default
    -ki     Keys per record                     100
    -li     max Length of keys                  6
    -%str   ignore lines that begin with %x     CNOPVX
            where x is in str
            str is a seq of chars
    -cfile  file contains Common words          /usr/src/local/bib/common
            do not use common words as keys
    -pfile  name of output file                 INDEX
    -s	    do not print statistics		statistics printed
*/

# define    operand     (strlen(*argv+2)==0 ? (argv++,argc--,*argv) : *argv+2)

flags()
{   for (; argc>0 && *argv[0]=='-';  argc--,argv++)
    {   switch ((*argv)[1])
        {   case 'k':   max_kcnt= atoi(operand);
                        break;
            case 'l':   max_klen= atoi(operand);
                        break;
            case 'c':   common=  operand;
                        break;
            case '%':   ignore=  *argv+2;
                        break;
            case 'p':   INDEX=  operand;
                        break;
	    case 's':	silent= 1;
			break;
            default:    fprintf(stderr, "unknown flag '%s'\n", *argv);
        }
    }
}


/*  shorten(inf,outf): file "inf" consists of lines of the form:
        key file start length
    sorted by key and file.  replace lines with the same key
    with one line of the form:
        key:file1 start/length ... start/length:file2 start/length ...
    rename as file "outf"
    returns number of lines in output
*/
long shorten(inf,outf)
char *inf, *outf;
{   FILE *in, *out;
    char line[maxstr];
    char key[maxstr],  newkey[maxstr],
         file[maxstr], newfile[maxstr];
    long int start, length;
    long int lines = 0;

    in=  fopen(inf, "r");
    out= fopen(outf, "w");
    if (in==NULL || out==NULL)
    {   fprintf(stderr, "invert: error in opening file for compression\n");
        return(0);
    }

    getline(in,line);
    sscanf(line,"%s%s83/09/23D", key, file, &start, &length);
    fprintf(out, "%s :%s %D/%D", key, file, start, length);
    for ( getline(in, line) ; !feof(in);  getline(in, line))
    {   sscanf(line,"%s%s83/09/23D", newkey, newfile, &start, &length);
        if (strcmp(key,newkey)!=0)
        {   strcpy(key, newkey);
            strcpy(file, newfile);
            fprintf(out, "\n%s :%s %D/%D",  key, file, start, length);
	    lines++;
        }
        else if (strcmp(file,newfile)!=0)
        {   strcpy(file,newfile);
            fprintf(out, ":%s %D/%D", file, start, length);
        }
        else
            fprintf(out, " %D/%D", start, length);
    }
    fprintf(out, "\n");
    lines++;

    fclose(in); fclose(out);
    unlink(inf);
    return (lines);
}
