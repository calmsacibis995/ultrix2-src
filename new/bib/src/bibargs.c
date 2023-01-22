#ifndef lint
static char sccsid[] = "@(#)bibargs.c	2.2	9/23/83";
#endif not lint
/*
        Authored by: Tim Budd, University of Arizona, 1983.
                version 7/4/83

        Various modifications suggested by:
                David Cherveny - Duke University Medical Center
                Phil Garrison - UC Berkeley
                M. J. Hawley - Yale University




        read argument strings for bib and listrefs
        do name formatting, printing lines, other actions common to both
                                                        */
# include <stdio.h>
# include <ctype.h>
# include "bib.h"
# define LINELENGTH 1024
# define MAXDEFS     500             /* maximum number of defined words */

/* global variables */
   char bibfname[120];          /* file name currently being read            */
   int  biblineno;              /* line number currently being referenced    */
   int  abbrev       = false;   /* automatically abbreviate names            */
   int  capsmcap     = false;   /* print names in caps small caps (CACM form)*/
   int  numrev       = 0;       /* number of authors names to reverse        */
   int  edabbrev     = false;   /* abbreviate editors names ?                */
   int  edcapsmcap   = false;   /* print editors in cap small caps           */
   int  ednumrev     = 0;       /* number of editors to reverse              */
   int  sort         = false;   /* sort references ? (default no)            */
   int  foot         = false;   /* footnoted references ? (default endnotes) */
   int  hyphen       = false;   /* hypenate contiguous references            */
   int  ordcite      = true;    /* order multiple citations                  */
   char sortstr[80]  = "1";     /* sorting template                          */
   char trailstr[80] = "";      /* trailing characters to output             */
   char pfile[120];             /* private file name                         */
   int  personal = false;       /* personal file given ? (default no)        */
   char citetemplate[80] = "1"; /* citation template                         */
   char *words[MAXDEFS];        /* defined words                             */
   char *defs[MAXDEFS];         /* defined word definitions                  */
   int  wordtop = -1;           /* top of defined words array                */

/* where output goes */
   extern FILE *tfd;
/* reference file information */
   extern long int refspos[];
   extern char reffile[];
   extern FILE *rfd;
   extern char *citestr[];
   extern int numrefs;

/* doargs - read command argument line for both bib and listrefs
            set switch values
            call rdtext on file arguments, after dumping
            default style file if no alternative style is given
*/
   int doargs(argc, argv, defstyle)
   int argc;
   char **argv, defstyle[];
{  int numfiles, i, style;
   char *p, *q, *walloc();
   FILE *fd;

   numfiles = 0;
   style = true;
   words[0] = walloc("BMACLIB");
   defs[0]  = walloc(BMACLIB);
   wordtop++;
   fputs(".ds l] ",tfd);
   fputs(BMACLIB, tfd);
   fputs("\n", tfd);

   for (i = 1; i < argc; i++)
      if (argv[i][0] == '-')
         switch(argv[i][1]) {

            case 'a':  for (p = &argv[i][2]; *p; p++)
                          if (*p == 'a' || *p == 0)
                             abbrev = true;
                           else if (*p == 'x')
                             capsmcap = true;
                           else if (*p == 'r') {
                             if (*(p+1))
                                numrev = atoi(p+1);
                              else
                                numrev = 1000;
                              break;
                              }
                       break;

            case 'c':  if (argv[i][2] == 0)
                          error("citation string expected");
                       else
                          for (p = citetemplate,q = &argv[i][2]; *p++ = *q++; );
                       break;

            case 'e':  for (p = &argv[i][2]; *p; p++)
                          if (*p == 'a')
                             edabbrev = true;
                           else if (*p == 'x')
                             edcapsmcap = true;
                           else if (*p == 'r') {
                             if (*(p+1))
                                ednumrev = atoi(p+1);
                              else
                                ednumrev = 1000;
                              break;
                              }
                       break;

            case 'f':  foot = true;
                       hyphen = false;
                       break;

            case 'h':  hyphen = ordcite = true;
                       break;

            case 'n':  for (p = &argv[i][2]; *p; p++)
                          if (*p == 'a')
                             abbrev = false;
                          else if (*p == 'f')
                             foot = false;
                          else if (*p == 'h')
                             hyphen = false;
                          else if (*p == 'o')
                             ordcite = false;
                          else if (*p == 'r')
                             numrev = 0;
                          else if (*p == 's')
                             sort = false;
                          else if (*p == 'x')
                             capsmcap = false;
                       break;

            case 'o':  ordcite = true;
                       break;

            case 'p':  if (argv[i][2])
                          p = &argv[i][2];
                       else {  /* take next arg */
                          i++;
                          p = argv[i];
                          }
                       strcpy(pfile, p);
                       personal = true;
                       break;

            case 'r':  if (argv[i][2] == 0)  /* this is now replaced by -ar */
                          numrev = 1000;
                       else
                          numrev = atoi(&argv[i][2]);
                       break;

            case 's':  sort = true;
                       if (argv[i][2])
                          for (p = sortstr,q = &argv[i][2]; *p++ = *q++; );
                       break;

            case 't':  style = false;           /* fall through */
            case 'i':  if (argv[i][2])
                          p = &argv[i][2];
                       else { /* take next arg */
                          i++;
                          p = argv[i];
                          }
                       incfile(p);
                       break;

            case 'x':  capsmcap = true; /* this is now replaced by -ax */
                       break;

            case 0:    if (style) {  /* no style command given, take default */
                          style = false;
                          incfile( defstyle );
                          }
                       strcpy(bibfname,"<stdin>");
                       rdtext(stdin);
                       numfiles++;
                       break;

            default:   fputs(argv[i], stderr);
                       error(": invalid switch");
            }
      else { /* file name */
         numfiles++;
         if (style) {
            style = false;
            incfile( defstyle );
            }
         fd = fopen(argv[i], "r");
         if (fd == NULL) {
            fputs(argv[i], stderr);
            error(": can't open");
            }
         else {
            strcpy(bibfname, argv[i]);
            rdtext(fd);
            fclose(fd);
            }
         }

   if (style) incfile( defstyle );
   return(numfiles);

}

/* incfile - read in an included file  */
incfile(np)
   char *np;
{  char name[120];
   FILE *fd;
   char *p, line[LINELENGTH], dline[LINELENGTH], word[80], *tfgets();
   int  i, j, getwrd();

   strcpy(bibfname, np);
   fd = fopen(np, "r");
   if (fd == NULL && *np != '/') {
      strcpy(name, "bib.");
      strcat(name, np);
      strcpy(bibfname, name);
      fd = fopen(name, "r");
      }
   if (fd == NULL && *np != '/') {
      strcpy(name,BMACLIB);
      strcat(name, "/bib.");
      strcat(name, np);
      strcpy(bibfname, name);
      fd = fopen(name, "r");
      }
   if (fd == NULL) {
      bibwarning("%s: can't open", np);
      exit(1);
      }

   /* now go off and process file */
   biblineno = 1;
   while (tfgets(line, LINELENGTH, fd) != NULL) {
      biblineno++;
      switch(line[0]) {

         case '#': break;

         case 'A': for (p = &line[1]; *p; p++)
                      if (*p == 'A' || *p == '\0')
                         abbrev = true;
                      else if (*p == 'X')
                         capsmcap = true;
                      else if (*p == 'R') {
                         if (*(p+1))
                            numrev = atoi(p+1);
                         else
                            numrev = 1000;
                         break;
                         }
                   break;

         case 'C': for (p = &line[1]; *p == ' '; p++) ;
                   strcpy(citetemplate, p);
                   break;

         case 'D': if ((i = getwrd(line, 1, word)) == 0)
                      error("word expected in definition");
                   for (j = 0; j <= wordtop; j++)
                      if (strcmp(word, words[j]) == 0)
                         break;
                   if (j > wordtop) {
                      if ((j = ++wordtop) > MAXDEFS)
                         error("too many defintions");
                      words[wordtop] = walloc(word);
                      }
                   for (p = &line[i]; *p == ' '; p++) ;
                   for (strcpy(dline, p); dline[strlen(dline)-1] == '\\'; ){
                       dline[strlen(dline)-1] = '\n';
                       if (tfgets(line, LINELENGTH, fd) == NULL) break;
                       strcat(dline, line);
                       }
                   defs[j] = walloc(dline);
                   break;

         case 'E': for (p = &line[1]; *p; p++)
                      if (*p == 'A')
                         edabbrev = true;
                      else if (*p == 'X')
                         edcapsmcap = true;
                      else if (*p == 'R') {
                         if (*(p+1))
                            ednumrev = atoi(p+1);
                         else
                            ednumrev = 1000;
                         break;
                         }
                   break;

         case 'F': foot = true;
                   hyphen = false;
                   break;

         case 'I': for (p = &line[1]; *p == ' '; p++);
                   expand(p);
                   incfile(p);
                   break;

         case 'H': hyphen = ordcite = true;
                   break;

         case 'O': ordcite = true;
                   break;

         case 'R': if (line[1] == 0)  /* this is now replaced by AR */
                      numrev = 1000;
                   else
                      numrev = atoi(&line[1]);
                   break;

         case 'S': sort = true;
                   for (p = &line[1]; *p == ' '; p++) ;
                   strcpy(sortstr, p);
                   break;

         case 'T': for (p = &line[1]; *p == ' '; p++) ;
                   strcpy(trailstr, p);
                   break;

         case 'X': capsmcap = true;     /* this is now replace by AX */
                   break;

         default:  fprintf(tfd,"%s\n",line);
                   while (fgets(line, LINELENGTH, fd) != NULL)
                      fputs(line, tfd);
                   return;
         }

   }
   /* close up */
   fclose(fd);
}

/* bibwarning - print out a warning message */
  bibwarning(msg, arg)
  char *msg, *arg;
{
  fprintf(stderr,"`%s', line %d: ", bibfname, biblineno);
  fprintf(stderr, msg, arg);
}

/* error - report unrecoverable error message */
  error(str)
  char str[];
{
  bibwarning("%s\n", str);
  exit(1);
}

#ifdef READWRITE
/*
** fixrfd( mode ) -- re-opens the rfd file to be read or write,
**      depending on the mode.  Uses a static int to save the current mode
**      and avoid unnecessary re-openings.
*/
fixrfd( mode )
register int mode;
{
	static int cur_mode = WRITE;    /* rfd open for writing initially */

	if (mode != cur_mode)
	{
		rfd = freopen(reffile, ((mode == READ)? "r" : "a"), rfd);
		cur_mode = mode;
		if (rfd == NULL)
		      error("Hell!  Couldn't re-open reference file");
	}
}
#endif


/* tfgets - fgets which trims off newline */
   char *tfgets(line, n, ptr)
   char line[];
   int  n;
   FILE *ptr;
{  char *p;

   p = fgets(line, n, ptr);
   if (p == NULL)
      return(NULL);
   else
      for (p = line; *p; p++)
         if (*p == '\n')
            *p = 0;
   return(line);
}

/* getwrd - place next word from in[i] into out */
int getwrd(in, i, out)
   char in[], out[];
   int i;
{  int j;

   j = 0;
   while (in[i] == ' ' || in[i] == '\n' || in[i] == '\t')
      i++;
   if (in[i])
      while (in[i] && in[i] != ' ' && in[i] != '\t' && in[i] != '\n')
         out[j++] = in[i++];
   else
      i = 0;    /* signals end of in[i..]   */
   out[j] = 0;
   return (i);
}

/* walloc - allocate enough space for a word */
char *walloc(word)
   char *word;
{  char *i, *malloc();
   i = malloc(1 + strlen(word));
   if (i == NULL)
      error("out of storage");
   strcpy(i, word);
   return(i);
}

/* isword - see if character is legit word char */
int iswordc(c)
char c;
{
   if (isalnum(c) || c == '&' || c == '_')
      return(true);
   return(false);
}

/* expand - expand reference, replacing defined words */
   expand(line)
   char *line;
{  char line2[REFSIZE], word[LINELENGTH], *p, *q, *w;
   int  replaced, i;

   replaced  = true;
   while (replaced) {
      replaced = false;
      p = line;
      q = line2;
      while (*p) {
         if (isalnum(*p)) {
            for (w = word; *p && iswordc(*p); )
               *w++ = *p++;
            *w = 0;
            for (i = 0; i <= wordtop; i++)
               if (strcmp(word, words[i]) == 0) {
                  strcpy(word, defs[i]);
                  replaced = true;
                  break;
                  }
            for (w = word; *w; )
               *q++ = *w++;
            }
         else
            *q++ = *p++;
         }
      *q = 0;
      p = line;
      q = line2;
      while (*p++ = *q++);
      }
}

/* rdref - read text for an already cited reference */
   rdref(i, ref)
   long int  i;
   char ref[REFSIZE];
{
   ref[0] = 0;
#ifdef READWRITE
   fixrfd( READ );                      /* fix access mode of rfd, if nec. */
#endif
   fseek(rfd, i, 0);
   fread(ref, 1, REFSIZE, rfd);
}

/* breakname - break a name into first and last name */
   breakname(line, first, last)
   char line[], first[], last[];
{  char *p, *q, *r, *t, *f;

   for (t = line; *t != '\n'; t++);
   for (t--; isspace(*t); t--);

   /* now strip off last name */
   for (q = t; isspace(*q) == 0 || ((*q == ' ') & (*(q-1) == '\\')); q--)
      if (q == line)
         break;
   f = q;
   if (q != line) {
      q++;
      for (; isspace(*f); f--);
      f++;
      }

   /* first name is start to f, last name is q to t */

   for (r = first, p = line; p != f; )
      *r++ = *p++;
   *r = 0;
   for (r = last, p = q, t++; q != t; )
      *r++ = *q++;
   *r = 0;

}

/* match - see if string1 is a substring of string2 (case independent)*/
   int match(str1, str2)
   char str1[], str2[];
{  int  i, j;
   char a, b;

   for (i = 0; str2[i]; i++) {
      for (j = 0; str1[j]; j++) {
         if (isupper(a = str2[i+j]))
            a = (a - 'A') + 'a';
         if (isupper(b = str1[j]))
            b = (b - 'A') + 'a';
         if (a != b)
            break;
         }
      if (str1[j] == 0)
         return(true);
      }
   return(false);
}

/* scopy - append a copy of one string to another */
   char *scopy(p, q)
   char *p, *q;
{
   while (*p++ = *q++)
      ;
   return(--p);
}

/* rcomp - reference comparison routine for qsort utility */
   int rcomp(ap, bp)
   long int *ap, *bp;
{  char ref1[REFSIZE], ref2[REFSIZE], field1[MAXFIELD], field2[MAXFIELD];
   char *p, *q, *getfield();
   int  neg, res;
   int  fields_found;

   rdref(*ap, ref1);
   rdref(*bp, ref2);
   for (p = sortstr; *p; p = q) {
      if (*p == '-') {
         p++;
         neg = true;
         }
      else
         neg = false;
      q = getfield(p, field1, ref1);
      fields_found = true;
      if (q == 0) {
	 res = 1;
	 fields_found = false;
      } else if (strcmp (field1, "") == 0) {	/* field not found */
         if (*p == 'A') {
            getfield("F", field1, ref1);
	    if (strcmp (field1, "") == 0) {
               getfield("I", field1, ref1);
	       if (strcmp (field1, "") == 0) {
	          res = 1;
		  fields_found = false;
	       }
	    }
	 } else {
	    res = 1;
	    fields_found = false;
	 }
      }

      if (getfield(p, field2, ref2) == 0) {
	 res = -1;
	 fields_found = false;
      } else if (strcmp (field2, "") == 0) {	/* field not found */
         if (*p == 'A') {
            getfield("F", field2, ref2);
	    if (strcmp (field2, "") == 0) {
               getfield("I", field2, ref2);
	       if (strcmp (field2, "") == 0) {
	          res = -1;
		  fields_found = false;
	       }
	    }
	 } else {
	    res = -1;
	    fields_found = false;
	 }
      }
      if (fields_found) {
         if (*p == 'A') {
            if (isupper(field1[0]))
               field1[0] -= 'A' - 'a';
            if (isupper(field2[0]))
               field2[0] -= 'A' - 'a';
            }
         res = strcmp(field1, field2);
         }
      if (neg)
         res = - res;
      if (res != 0)
         break;
      }
   if (res == 0)
      if (ap < bp)
         res = -1;
      else
         res = 1;
   return(res);
}

/* makecites - make citation strings */
   makecites(citestr)
   char *citestr[];
{  char ref[REFSIZE], tempcite[100], *malloc();
   int  i;

   for (i = 0; i <= numrefs; i++) {
      rdref(refspos[i], ref);
      bldcite(tempcite, i, ref);
      citestr[i] = malloc(2 + strlen(tempcite)); /* leave room for disambig */
      if (citestr[i] == NULL)
         error("out of storage");
      strcpy(citestr[i], tempcite);
      }
}

/* bldcite - build a single citation string */
   bldcite(cp, i, ref)
   char *cp, ref[];
   int  i;
{  char *p, *q, c, *fp, field[REFSIZE], *getfield(), *aabet(), *astro();

   getfield("F", field, ref);
   if (field[0] != 0)
      for (p = field; *p; p++)
         *cp++ = *p;
   else {
      p = citetemplate;
      field[0] = 0;
      while (c = *p++) {
         if (isalpha(c)) {                      /* field name   */
            q = getfield(p-1, field, ref);
            if (q != 0) {
               p = q;
               for (fp = field; *fp; )
                  *cp++ = *fp++;
               }
            }
         else if (c == '1') {                   /* numeric  order */
            sprintf(field,"%d",1 + i);
            for (fp = field; *fp; )
               *cp++ = *fp++;
            }
         else if (c == '2')                     /* alternate alphabetic */
            cp = aabet(cp, ref);
         else if (c == '3')                     /* Astrophysical Journal style*/
            cp = astro(cp, ref);
/*       else if (c == '4')          here is how to add new styles */
         else if (c == '{') {                   /* other information   */
            while (*p != '}')
               if (*p == 0)
                  error("unexpected end of citation template");
               else
                  *cp++ = *p++;
            p++;
            }
         else if (c == '<') {
            while (*p != '>') {
               if (*p == 0)
                  error("unexpected end of citation template");
               else
                  *cp++ = *p++;
               }
            p++;
            }
         else if (c != '@')
            *cp++ = c;
         }
      }
   *cp++ = 0;
}

/* alternate alphabetic citation style -
        if 1 author - first three letters of last name
        if 2 authors - first two letters of first, followed by first letter of
                                seond
        if 3 or more authors - first letter of first three authors */
   char *aabet(cp, ref)
   char *cp, ref[];
{  char field[REFSIZE], temp[100], *np, *fp;
   int j, getname();

   if (getname(1, field, temp, ref)) {
      np = cp;
      fp = field;
      for (j = 1; j <= 3; j++)
         if (*fp != 0)
            *cp++ = *fp++;
      if (getname(2, field, temp, ref))
         np[2] = field[0];
      if (getname(3, field, temp, ref)) {
         np[1] = np[2];
         np[2] = field[0];
         }
      }
return(cp);
}

/* Astrophysical Journal style
        if 1 author - last name date
        if 2 authors - last name and last name date
        if 3 authors - last name, last name and last name date
        if 4 or more authors - last name et al. date */
   char *astro(cp, ref)
   char *cp, ref[];
{  char name1[100], name2[100], name3[100], temp[100], *fp;
   int getname();

   if (getname(1, name1, temp, ref)) {
      for (fp = name1; *fp; )
         *cp++ = *fp++;
      if (getname(4, name3, temp, ref)) {
         for (fp = " et al."; *fp; )
            *cp++ = *fp++;
         }
      else if (getname(2, name2, temp, ref)) {
         if (getname(3, name3, temp, ref)) {
            for (fp = "\\*(c]"; *fp; )
               *cp++ = *fp++;
            for (fp = name2; *fp; )
               *cp++ = *fp++;
            for (fp = "\\*(m]"; *fp; )
               *cp++ = *fp++;
            for (fp = name3; *fp; )
               *cp++ = *fp++;
            }
         else {
            for (fp = "\\*(n]"; *fp; )
               *cp++ = *fp++;
            for (fp = name2; *fp; )
               *cp++ = *fp++;
            }
         }
    }
return(cp);
}

/* getfield - get a single field from reference */
   char *getfield(ptr, field, ref)
   char *ptr, field[], ref[];
{  char *p, *q, temp[100];
   int  n, len, i, getname();

   field[0] = 0;
   if (*ptr == 'A')
      getname(1, field, temp, ref);
   else
      for (p = ref; *p; p++)
         if (*p == '%' && *(p+1) == *ptr) {
            for (p = p + 2; *p == ' '; p++)
               ;
            for (q = field; (*p != '\n') && (*p != '\0'); )
               *q++ = *p++;
            *q = 0;
            break;
            }
   n = 0;
   len = strlen(field);
   if (*++ptr == '-') {
      for (ptr++; isdigit(*ptr); ptr++)
         n = 10 * n + (*ptr - '0');
      if (n > len)
         n = 0;
      else
         n = len - n;
      for (i = 0; field[i] = field[i+n]; i++)
         ;
      }
   else if (isdigit(*ptr)) {
      for (; isdigit(*ptr); ptr++)
         n = 10 * n + (*ptr - '0');
      if (n > len)
         n = len;
      field[n] = 0;
      }

   if (*ptr == 'u') {
      ptr++;
      for (p = field; *p; p++)
         if (islower(*p))
            *p = (*p - 'a') + 'A';
      }
   else if (*ptr == 'l') {
      ptr++;
      for (p = field; *p; p++)
         if (isupper(*p))
            *p = (*p - 'A') + 'a';
      }
   return(ptr);
}

/* getname - get the nth name field from reference, breaking into
             first and last names */
   int getname(n, last, first, ref)
   int  n;
   char last[], first[], ref[];
{  char *p;
   int  m;

   m = n;
   for (p = ref; *p; p++)
      if (*p == '%' & *(p+1) == 'A') {
         n--;
         if (n == 0) {
            for (p = p + 2; *p == ' '; p++) ;
            breakname(p, first, last) ;
            return(true);
            }
         }

   if (n == m)          /* no authors, try editors */
      for (p = ref; *p; p++)
         if (*p == '%' & *(p+1) == 'E') {
            n--;
            if (n == 0) {
               for (p = p + 2; *p == ' '; p++) ;
               breakname(p, first, last) ;
               return(true);
               }
            }

   if (n == m) {        /* no editors, either, try institution */
      first[0] = last[0] = '\0';
      getfield("I", last, ref);
      if (last[0] != '\0')
         return(true);
      }

   return(false);
}

/* disambiguate - compare adjacent citation strings, and if equal, add
                  single character disambiguators */
   disambiguate()
{  int i, j;
   char adstr[2];

   for (i = 0; i < numrefs; i = j) {
      j = i + 1;
      if (strcmp(citestr[i], citestr[j])==0) {
         adstr[0] = 'a'; adstr[1] = 0;
         for (j = i+1; strcmp(citestr[i],citestr[j]) == 0; j++) {
            adstr[0] = 'a' + (j-i);
            strcat(citestr[j], adstr);
            if (j == numrefs)
               break;
            }
         adstr[0] = 'a';
         strcat(citestr[i], adstr);
         }
     }
}


/* bldname - build a name field
             doing abbreviations, reversals, and caps/small caps
*/
   bldname(first, last, name, reverse)
   char *first, *last, name[];
   int reverse;
{
   char newfirst[120], newlast[120], *p, *q, *f, *l, *scopy();
   int  flag;

   if (abbrev) {
      p = first;
      q = newfirst;
      flag = false;
      while (*p) {
         while (*p == ' ')
            p++;
         if (*p == 0)
            break;
         if (isupper(*p)) {
            if (flag)           /* between initial gap */
               q = scopy(q, "\\*(a]");
            flag = true;
            *q++ = *p;
            q = scopy(q, "\\*(p]");
            }
         if (*++p == '.')
            p++;
         else while (*p != 0 && ! isspace(*p))
            p++;
         }
      *q = 0;
      f = newfirst;
      }
   else
      f = first;

   if (capsmcap) {
      p = last;
      q = newlast;
      flag = 0;  /* 1 - printing cap, 2 - printing small */
      while (*p)
         if (islower(*p)) {
            if (flag != 2)
               q = scopy(q, "\\s-2");
            flag = 2;
            *q++ = (*p++ - 'a') + 'A';
            }
         else {
            if (flag == 2)
               q = scopy(q,"\\s+2");
            flag = 1;
            *q++ = *p++;
            }
      if (flag == 2)
         q = scopy(q, "\\s+2");
      *q = 0;
      l = newlast;
      }
   else
      l = last;

   if (f[0] == 0)
      sprintf(name, "%s\n", l);
   else if (reverse)
      sprintf(name, "%s\\*(b]%s\n", l, f);
   else
      sprintf(name, "%s %s\n", f, l);
}

/* prtauth - print author or editor field */
   prtauth(c, line, num, max, ofd, abbrev, capsmcap, numrev)
   char c, *line;
   int  num, max, abbrev, capsmcap, numrev;
   FILE *ofd;
{  char first[LINELENGTH], last[LINELENGTH];

   if (num <= numrev || abbrev || capsmcap) {
      breakname(line, first, last);
      bldname(first, last, line, num <= numrev);
      }
   if (num == 1)
      fprintf(ofd,".ds [%c %s", c, line);
   else if (num < max)
      fprintf(ofd,".as [%c \\*(c]%s", c, line);
   else if (max == 2)
      fprintf(ofd,".as [%c \\*(n]%s", c, line);
   else
      fprintf(ofd,".as [%c \\*(m]%s", c, line);
   if (num == max && index(trailstr, c))
      fprintf(ofd,".ds ]%c %c\n", c, line[strlen(line)-2]);
}

/* doline - actually print out a line of reference information */
   doline(c, line, numauths, maxauths, numeds, maxeds, ofd)
   char c, *line;
   int numauths, maxauths, numeds, maxeds;
   FILE *ofd;
{

   switch(c) {
      case 'A':
          prtauth(c, line, numauths, maxauths, ofd, abbrev, capsmcap, numrev);
          break;

       case 'E':
          prtauth(c, line, numeds, maxeds, ofd, edabbrev, edcapsmcap, ednumrev);
          if (numeds == maxeds)
             fprintf(ofd,".nr [E %d\n", maxeds);
          break;

       case 'P':
          if (index(line, '-'))
             fprintf(ofd,".nr [P 1\n");
          else
             fprintf(ofd,".nr [P 0\n");
          fprintf(ofd,".ds [P %s",line);
          if (index(trailstr, 'P'))
             fprintf(ofd,".ds ]P %c\n",line[strlen(line)-2]);
          break;

       case 'F':
       case 'K': break;

       default:
          fprintf(ofd,".ds [%c %s", c, line);
          if (index(trailstr, c))
             fprintf(ofd,".ds ]%c %c\n", c, line[strlen(line)-2]);
          }
}

/* dumpref - dump reference number i */
   dumpref(i, ofd)
   int i;
   FILE *ofd;
{  char ref[REFSIZE], *p, line[REFSIZE];
   int numauths, maxauths, numeds, maxeds;

   rdref(refspos[i], ref);
   maxauths = maxeds = 0;
   numauths = numeds = 0;
   for (p = ref; *p; p++)
      if (*p == '%')
         if (*(p+1) == 'A') maxauths++;
         else if (*(p+1) == 'E') maxeds++;
   fprintf(ofd, ".[-\n");
   fprintf(ofd, ".ds [F %s\n",citestr[i]);
   fseek(rfd, (long) refspos[i], 0);
   while (fgets(line, REFSIZE, rfd) != NULL) {
      if (line[0] == 0)        break;
      else if (line[0] == '.') fprintf(ofd,"%s",line);
      else {
         if (line[0] == '%') {
            for (p = &line[2]; *p == ' '; p++);
            if (line[1] == 'A')       numauths++;
            else if (line[1] == 'E')  numeds++;

            doline(line[1], p, numauths, maxauths, numeds, maxeds, ofd);
            }
         }
      }
   fprintf(ofd,".][\n");
}
