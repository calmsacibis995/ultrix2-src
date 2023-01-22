#include <stdio.h>
#include "../h/config.h"

/*
 * miscellaneous external declarations
 */

extern FILE *infile;		/* current input file */
extern FILE *outfile;		/* interpreter output file */
extern FILE *dbgfile;		/* debug file */
extern char inname[];		/* input file name */
extern char outname[];		/* output file name */
extern char *pname;		/* this program name (from command line) */
extern int  line;		/* source program line number (from ucode) */
extern char *file;		/* source program file name (from ucode) */
extern int  statics;		/* total number of statics */
extern int  dynoff;		/* stack offset counter for locals */
extern int  argoff;		/* stack offset counter for arguments */
extern int  static1;		/* first static in procedure */
extern int  nlocal; 		/* number of locals in local table */
extern int  nconst;		/* number of constants in constant table */
extern int  nrecords;		/* number of records in program */
extern int  trace;		/* initial setting of &trace */
extern int  Dflag;		/* debug flag */
extern char ixhdr[];		/* Header line for direct execution */
extern int  hdrloc;		/* Location to place hdr block at */
/*
 * interpreter code file header
 */
struct header {
   int size;			/* size of interpreter code */
   int trace;			/* intial value of &trace */
   int records;			/* location of record blocks */
   int ftab;			/* location of record/field table */
   int globals;			/* location of global variables */
   int gnames;			/* location of names of globals */
   int statics;			/* location of static variables */
   int ident;			/* location of identifier table */
   } hdr;

/*
 * structures for symbol table entries
 */

struct lentry {			/* local table entry */
   char *l_name;		/*   name of variable */
   int l_flag;			/*   variable flags */
   union {			/*   value field */
      int staticid;		/*     unique id for static variables */
      int offset;		/*     stack offset for args and locals */
      struct gentry *global;	/*     global table entry */
      } l_val;
   };

struct gentry {			/* global table entry */
   struct gentry *g_blink;	/*   link for bucket chain */
   char *g_name;		/*   name of variable */
   int g_flag;			/*   variable flags */
   int g_nargs; 		/*   number of args or fields */
   int g_procid;		/*   procedure or record id */
   int g_pc;
   };

struct centry {			/* constant table entry */
   int c_flag;			/*   type of literal flag */
   union {			/*   value field */
      long  ival;		/*     integer */
      double rval;		/*     real */
      char *sval;		/*     string */
      } c_val;
   int c_length;		/*   length of literal string */
   int c_pc;
   };

struct ientry {			/* identifier table entry */
   struct ientry *i_blink;	/*   link for bucket chain */
   char *i_name;		/*   pointer to string */
   int i_length;		/*   length of string */
   };

struct fentry {			/* field table header entry */
   struct fentry *f_blink;	/*   link for bucket chain */
   char *f_name;		/*   name of field */
   int f_fid;			/*   field id */
   struct rentry *f_rlist;	/*   head of list of records */
   };

struct rentry {			/* field table record list entry */
   struct rentry *r_link;	/*   link for list of records */
   int r_recid;			/*   record id */
   int r_fnum;			/*   offset of field within record */
   };

/*
 * flag values in symbol tables
 */

#define F_GLOBAL      01	/* variable declared global externally */
#define F_PROC        05	/* procedure (includes GLOBAL) */
#define F_RECORD     011	/* record (includes GLOBAL) */
#define F_DYNAMIC    020	/* variable declared local dynamic */
#define F_STATIC     040	/* variable declared local static */
#define F_BUILTIN   0101	/* identifier refers to built-in procedure */
#define F_IMPERROR  0400	/* procedure has default error */
#define F_ARGUMENT 01000	/* variable is a formal parameter */
#define F_INTLIT   02000	/* literal is an integer */
#define F_REALLIT  04000	/* literal is a real */
#define F_STRLIT  010000	/* literal is a string */
#define F_CSETLIT 020000	/* literal is a cset */
#define F_LONGLIT 040000	/* literal is a long integer */

/*
 * symbol table region pointers
 */

extern struct gentry **ghash;	/* hash area for global table */
extern struct ientry **ihash;	/* hash area for identifier table */
extern struct fentry **fhash;	/* hash area for field table */

extern struct lentry *ltable;	/* local table */
extern struct gentry *gtable;	/* global table */
extern struct centry *ctable;	/* constant table */
extern struct ientry *itable;	/* identifier table */
extern struct fentry *ftable;	/* field table headers */
extern struct rentry *rtable;	/* field table record lists */
extern char	     *strings;	/* string space */
extern int	     *labels;	/* label table */
extern char	     *code;	/* generated code space */

extern struct gentry *gfree;	/* free pointer for global table */
extern struct ientry *ifree;	/* free pointer for identifier table */
extern struct fentry *ffree; 	/* free pointer for field table headers */
extern struct rentry *rfree; 	/* free pointer for field table	record lists */
extern char	     *sfree;	/* free pointer for string space */
extern char	     *codep;	/* free pointer for code space */

extern int lsize;		/* size of local table */
extern int gsize;		/* size of global table */
extern int csize;		/* size of constant table */
extern int isize;		/* size of identifier table */
extern int fsize;		/* size of field table headers */
extern int rsize;		/* size of field table record lists */
extern int ssize;		/* size of string space */
extern int ihsize;		/* size of identifier table hash area */
extern int ghsize;		/* size of global table hash area */
extern int fhsize;		/* size of field table hash area */
extern int maxlabels;		/* maximum number of labels per procedure */
extern int maxcode;		/* maximum amount of code per procedure */

extern int gmask;		/* mask for global table hash */
extern int imask;		/* mask for identifier table hash */
extern int fmask;		/* mask for field table hash */

/*
 * symbol table parameters
 */

#define LSIZE	   100		/* default size of local table */
#define GSIZE	   200		/* default size of global table */
#define CSIZE	   100		/* default size of constant table */
#define ISIZE	   500		/* default size of identifier table */
#define FSIZE	   100		/* default size of field table headers */
#define RSIZE	   100		/* default size of field table record lists */
#define SSIZE	  5000		/* default size of string space */
#define GHSIZE	    64		/* default size of global table hash area */
#define IHSIZE	   128		/* default size of identifier table hash area */
#define FHSIZE      32		/* default size of field table hash area */
#define MAXLABELS  500		/* default maximum number of labels/proc */

/*
 * hash computation macros
 */

#define ghasher(x) (((int)x)&gmask)	/* for global table */
#define fhasher(x) (((int)x)&fmask)	/* for field table */

/*
 * machine-dependent constants
 */
#ifdef VAX
#define BIT32			/* integers have 32 bits */
#define MINSHORT  0100000	/* smallest short integer */
#define MAXSHORT   077777	/* largest short integer */
#define LOGINTSIZE      5       /* log of INTSIZE */
#define BITOFFMASK    037       /* mask for bit offset into word */
#define CSETSIZE        8       /* # of words to contain cset bits */
#define ADDRSIZE	4	/* # of bytes in a pointer */
#define MAXCODE	    10000	/* default maximum amount of code/proc */
#endif VAX

#ifdef PDP11
#define BIT16			/* integers have 16 bits */
#define MINSHORT  0100000	/* smallest short integer */
#define MAXSHORT   077777	/* largest short integer */
#define LOGINTSIZE      4       /* log of INTSIZE */
#define BITOFFMASK    017       /* mask for bit offset into word */
#define CSETSIZE       16       /* # of words to contain cset bits */
#define ADDRSIZE	2	/* # of bytes in a pointer */
#define MAXCODE	     2000	/* default maximum amount of code/proc */
#endif PDP11

/*
 * cset accessing macros
 */

#define CSOFF(b)     ((b) & BITOFFMASK)         /* offset in word of cs bit */
#define CSPTR(b,c)   ((c) + (((b)&0377) >> LOGINTSIZE))
                                                /* address of word of cs bit */
#define setb(b,c)    (*CSPTR(b,c) |= (01 << CSOFF(b)))
                                                /* set bit b in cset c */
#define MAXHDR	1024		/* size of autoloading header */
#define HDRFILE "/usr/new/lib/icon/icont/iconx.hdr"
