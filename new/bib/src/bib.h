/*
 *	@(#)bib.h	2.2	9/23/83
 */
/*   various arguments for bib and listrefs processors */

/* constants */

# define true  1
# define false 0
# define err  -1
# define REFSIZE 1024                /* maximum size of reference string    */
# define MAXFIELD 512                /* maximum size of any field in referece*/

/* reference citation marker genrated in pass 1 */

# define CITEMARK (char) 02
# define CITEEND  (char) 03

/* file names */

        /* output of invert, input file for references */
# define INDXFILE "INDEX"
        /* pass1 reference collection file */
# define TMPREFFILE  "/usr/tmp/bibrXXXXXX"
        /* pass2 text collection file */
# define TMPTEXTFILE "/usr/tmp/bibpXXXXXX"
        /* temp file used in invert */
# define INVTEMPFILE "/usr/tmp/invertXXXXXX"
        /* common words */
# define COMFILE "/usr/lib/bmac/common"
        /* default system dictionary */
# define SYSINDEX "/usr/dict/papers/INDEX"
        /* where macro libraries live */
# define BMACLIB "/usr/lib/bmac"
        /* default style of references */
# define DEFSTYLE "/usr/lib/bmac/bib.stdsn"

/* size limits */

	/* maximum number of characters in common file */
# define MAXCOMM 1000

char *malloc();

/* fix needed for systems where open [w]+ doesn't work */
# ifdef READWRITE

# define READ 1
# define WRITE 0

#endif
