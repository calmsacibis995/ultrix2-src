/*	Grammar for Icon Version 5, including extensions	*/

/* primitive tokens */

%token  CSETLIT EOFX IDENT INTLIT REALLIT STRINGLIT

/* reserved words */

%token  BREAK BY CASE CREATE DEFAULT DO DYNAMIC ELSE END EVERY EXTERNAL
        FAIL GLOBAL IF INITIAL LOCAL NEXT NOT OF PROCEDURE RECORD REPEAT
        RETURN STATIC SUSPEND THEN TO UNTIL WHILE

/* operators */

%token  ASSIGN AT AUGACT AUGAND AUGEQ AUGEQV AUGGE AUGGT AUGLE AUGLT
	AUGNE AUGNEQV AUGSEQ AUGSGE AUGSGT AUGSLE AUGSLT AUGSNE
        BACKSLASH BANG BAR CARET CARETASGN COLON COMMA CONCAT
	CONCATASGN CONJUNC DIFF DIFFASGN DOT EQUIV INTER INTERASGN LBRACE
	LBRACK LCONCAT LCONCATASGN LEXEQ LEXGE LEXGT LEXLE LEXLT LEXNE
	LPAREN MCOLON MINUS MINUSASGN MOD MODASGN NOTEQUIV NUMEQ NUMGE
	NUMGT NUMLE NUMLT NUMNE PCOLON PLUS PLUSASGN QMARK RBRACE RBRACK
	REVASSIGN REVSWAP RPAREN SCANASGN SEMICOL SLASH SLASHASGN STAR
        STARASGN SWAP TILDE UNION UNIONASGN

%{
#include "../h/config.h"
#include "utran.h"
#include "sym.h"
#include "tree.h"
#include "keyword.h"
#define YYSTYPE nodeptr
#define YYMAXDEPTH 500
%}

%%

%{
int argcnt;
int idflag;
int i;
#ifdef EXT
int cstack[50];   /* context stack expression lists */
int stacktop = 0; /* stack top */
#endif
%}

program : decls EOFX {gout(globfile);} ;

decls   : ;	
        | decls decl ;

decl    : record {
		if (!nocode)
		   rout(globfile, STR0($1));
		nocode = 0;
		loc_init();
		} ;
        | proc  {
		if (!nocode)
		   codegen($1);
		nocode = 0;
		treeinit();
		loc_init();
		} ;
	| global ;

global  : GLOBAL {idflag = F_GLOBAL;} idlist ;
	| EXTERNAL {
#ifdef CMP
                idflag = F_BUILTIN|F_GLOBAL;
#endif
#ifdef INT
		idflag = F_GLOBAL;
		err("external declaration not allowed", 0);
#endif
		} idlist ;

record  : RECORD {idflag = F_ARGUMENT;} IDENT LPAREN arglist RPAREN {
		install(STR0($3),F_RECORD|F_GLOBAL,(int)$5);
		$$ = $3;
		} ;

proc    : prochead SEMICOL locals initial procbody END {
		$$ = PROCNODE($1,$4,$5,$6);
		} ;

prochead: PROCEDURE {idflag = F_ARGUMENT;} IDENT LPAREN arglist RPAREN {
		$$ = $3;
		install(STR0($3),F_PROC|F_GLOBAL,(int)$5);
		} ;

arglist : {$$ = (int)0;} ;
	| idlist {$$ = (int)$1;} ;


idlist  : IDENT {
		install(STR0($1),idflag,0);
		$$ = (int)1;
		} ;
	| idlist COMMA IDENT {
		install(STR0($3),idflag,0);
		$$ = (int)$1 + 1;
		} ;

locals  : ;
	| locals retention idlist SEMICOL ;

retention: LOCAL {idflag = F_DYNAMIC;} ;
	|  STATIC {idflag = F_STATIC;} ;
	|  DYNAMIC {idflag = F_DYNAMIC;} ;

initial : {$$ = EMPTYNODE;} ;
	| INITIAL expr SEMICOL {$$ = $2;} ;

procbody: {$$ = EMPTYNODE;} ;
	| nexpr SEMICOL procbody {$$ = SLISTNODE($2, $1, $3);} ;

nexpr   : {$$ = EMPTYNODE;} ;
	| expr ;

expr    : expr1a  ;
	| expr CONJUNC expr1a  {$$ = CONJNODE($2,$1,$3);} ;

expr1a  : expr1  ;
	| expr1a QMARK expr1  {$$ = SCANNODE($2,$1,$3);} ;

expr1   : expr2 ;
	| expr2 op1 expr1 {binop: $$ = BINOPNODE($2,$1,$3);} ;
	| expr2 op1a expr1 {$$ = AUGOPNODE($2,$1,$3);} ;
	| expr2 SCANASGN expr1 {$$ = SCANNODE($2,$1,$3);} ;
        | expr2 AUGAND expr1 {$$ = CONJNODE($2,$1,$3);} ;
        | expr2 AUGACT expr1 {$$ = ACTIVNODE($2,$3,$1);} ;

op1  	: SWAP ;
	| ASSIGN ;
	| REVSWAP ;
	| REVASSIGN ;

op1a    : CONCATASGN ;
	| LCONCATASGN ;
	| DIFFASGN ;
	| UNIONASGN ;
	| PLUSASGN ;
	| MINUSASGN ;
	| STARASGN ;
	| INTERASGN ;
	| SLASHASGN ;
	| MODASGN ;
	| CARETASGN ;
        | AUGEQ ;
        | AUGEQV ;
        | AUGGE ;
        | AUGGT ;
        | AUGLE ;
        | AUGLT ;
        | AUGNE ;
        | AUGNEQV ;
        | AUGSEQ ;
        | AUGSGE ;
        | AUGSGT ;
        | AUGSLE ;
        | AUGSLT ;
        | AUGSNE ;

expr2   : expr3 ;
	| expr2 TO expr3 {$$ = TONODE($2,$1,$3);} ;
	| expr2 TO expr3 BY expr3 {$$ = TOBYNODE($2,$1,$3,$5);} ;

expr3   : expr4 ;
	| expr4 BAR expr3 {$$ = ALTNODE($2,$1,$3);} ;

expr4   : expr5 ;
	| expr4 op4 expr5 {goto binop;} ;

op4  	: LEXEQ ;
	| LEXGE ;
	| LEXGT ;
	| LEXLE ;
	| LEXLT ;
	| LEXNE ;
	| NUMEQ ;
	| NUMGE ;
	| NUMGT ;
	| NUMLE ;
	| NUMLT ;
	| NUMNE ;
	| EQUIV ;
	| NOTEQUIV ;

expr5   : expr6 ;
	| expr5 op5 expr6 {goto binop;} ;

op5	: CONCAT ;
	| LCONCAT ;

expr6   : expr7 ;
	| expr6 op6 expr7 {goto binop;} ;

op6     : PLUS ;
	| DIFF ;
	| UNION ;
	| MINUS ;

expr7   : expr8 ;
	| expr7 op7 expr8 {goto binop;} ;

op7     : STAR ;
	| INTER ;
	| SLASH ;
	| MOD ;

expr8   : expr9 ;
	| expr9 CARET expr8 {goto binop;} ;

expr9   : expr10 ;
	| expr9 BACKSLASH expr10 {$$ = LIMITNODE($1,$3);} ;
        | expr9 AT expr10 {$$ = ACTIVNODE($2, $3, $1);};

expr10  : expr11 ;
	| AT expr10 {$$ = ACTIVNODE($1, $2, EMPTYNODE);} ;
        | NOT expr10 {$$ = NOTNODE($2);} ;
        | BAR expr10 {$$ = BARNODE($2);} ;
        | CONCAT expr10 {$$ = BARNODE(BARNODE($2));} ;
        | LCONCAT expr10 {$$ = BARNODE(BARNODE(BARNODE($2)));} ;
	| op10 expr10 {$$ = UNOPNODE($1,$2);} ;

op10  	: DOT ;
	| BANG ;
        | DIFF ;
	| PLUS ;
	| STAR ;
        | SLASH ;
        | CARET ;
        | INTER ;
	| TILDE ;
	| MINUS ;
	| NUMEQ ;
	| NUMNE ;
	| LEXEQ ;
	| LEXNE ;
	| EQUIV ;
        | UNION ;
	| QMARK ;
	| NOTEQUIV ;
        | BACKSLASH ;

expr11  : literal ;
	| section ;
	| return ;
	| if ;
	| case ;
        | while ;
	| until ;
	| every ;
	| repeat ;
	| CREATE expr {$$ = CREATENODE($1,$2);} ;
	| IDENT {VAL0($1) = putloc(STR0($1),0);} ;
        | NEXT {$$ = NEXTNODE($1);} ;
        | BREAK nexpr {$$ = BREAKNODE($1,$2);} ;
	| LPAREN {
#ifdef EXT
                  pushcs(0);
#endif
                  } exprlist {
#ifdef EXT
                  popcs();
#endif
                  } RPAREN {
		  $$ = INVOKNODE($1,EMPTYNODE,$3);} ;
	| LBRACE compound RBRACE {$$ = $2;} ;
        | LBRACK {
#ifdef EXT
                  pushcs(0);
#endif
                  } exprlist {
#ifdef EXT
                  popcs();
#endif
                  } RBRACK {
		$$ = LISTNODE($1,$3);} ;
	| expr11 LBRACK expr RBRACK {$$ = BINOPNODE($2,$1,$3);} ;
	| expr11 LBRACE {
#ifdef EXT
                 pushcs(1);
#endif
                 } exprlist {
#ifdef EXT
                 popcs();
#else
                 err("missing semicolon or operator", 0);
#endif
                 } RBRACE {
		$$ = INVOKNODE($2,$1,LISTNODE($2,$4));
		} ;
	| expr11 LPAREN {
#ifdef EXT
                 pushcs(0);
#endif
                 } exprlist {
#ifdef EXT
                 popcs();
#endif
                 } RPAREN {
		$$ = INVOKNODE($2,$1,$4);
		} ;
	| expr11 DOT IDENT {$$ = FIELDNODE($2,$1,$3);} ;
        | CONJUNC FAIL {$$ = KEYNODE($1, K_FAIL);} ;
        | CONJUNC IDENT {
		if ((i = klocate(STR0($2))) == NULL)
		   err("invalid keyword",STR0($2));
		$$ = KEYNODE($1, i);
		} ;

while   : WHILE expr {$$ = LOOPNODE($1,$2,EMPTYNODE);} ;
	| WHILE expr DO expr {$$ = LOOPNODE($1,$2,$4);} ;

until   : UNTIL expr {$$ = LOOPNODE($1,$2,EMPTYNODE);} ;
	| UNTIL expr DO expr {$$ = LOOPNODE($1,$2,$4);} ;

every   : EVERY expr {$$ = LOOPNODE($1,$2,EMPTYNODE);} ;
	| EVERY expr DO expr {$$ = LOOPNODE($1,$2,$4);} ;

repeat  : REPEAT expr {$$ = LOOPNODE($1,$2,EMPTYNODE);} ;

return  : FAIL {$$ = RETNODE($1,EMPTYNODE);} ;
	| RETURN nexpr {$$ = RETNODE($1,$2);} ;
	| SUSPEND nexpr {$$ = SUSPNODE($1,$2);} ;

if      : IF expr THEN expr {$$ = IFNODE($1,$2,$4,EMPTYNODE);} ;
	| IF expr THEN expr ELSE expr {$$ = IFNODE($1,$2,$4,$6);} ;

case    : CASE expr OF LBRACE caselist RBRACE {$$ = CASENODE($1,$2,$5);} ;

caselist: cclause ;
	| caselist SEMICOL cclause {$$ = CLISTNODE($2,$1,$3);} ;

cclause : DEFAULT COLON expr {$$ = CCLSNODE($2,$1,$3);} ;
	| expr COLON expr {$$ = CCLSNODE($2,$1,$3);} ;

exprlist: nexpr {
#ifdef EXT
                 $$ = cswitch($1,$1);
#endif
                 } ;
	| exprlist COMMA nexpr {
#ifdef EXT
                 $$ = ELISTNODE($2,$1,cswitch($3,$2));
#else
                $$ = ELISTNODE($2,$1,$3);
#endif
                } ;

literal : INTLIT {VAL0($1) = putlit(STR0($1),F_INTLIT,0);} ;
	| REALLIT {VAL0($1) = putlit(STR0($1),F_REALLIT,0);} ;
	| STRINGLIT {VAL0($1) = putlit(STR0($1),F_STRLIT,VAL1($1));} ;
        | CSETLIT {VAL0($1) = putlit(STR0($1),F_CSETLIT,VAL1($1));} ;

section : expr11 LBRACK expr sectop expr RBRACK {$$ = SECTNODE($4,$1,$3,$5);} ;

sectop  : COLON ;
	| PCOLON ;
	| MCOLON ;

compound: nexpr ;
	| nexpr SEMICOL compound {$$ = SLISTNODE($2, $1, $3);} ;

program : error decls EOFX ;
proc    : prochead error procbody END ;
expr    : error ;
%%

#ifdef EXT
pushcs(val)
int val;
{
  stacktop = stacktop + 1;
  cstack[stacktop] = val;
}

popcs()
{
  stacktop = stacktop - 1;
}

int cswitch(x,y)
   int x, y;
   {
  if (cstack[stacktop]) return(CREATENODE(y,x));
   else return(x);
}
#endif
