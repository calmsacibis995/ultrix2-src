# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin ={stdin}, *yyout ={stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#ifndef lint
static char sccsid[] = "@(#)end.l	4.2	(Berkeley)	82/11/06";
#endif not lint

#include <stdio.h>
#include <ctype.h>
#include "names.h"
#include "ehash.c"
#include "edict.c"
#define OUT1(c) putchar(c); putchar(':'); for(i=yyleng-1;i>=0;)putchar(yytext[i--])
#define POUT1(c) putchar(c); putchar(':'); for(i=yyleng-1;i>0;)putchar(yytext[i--])
int i;
int nomin = 0;
int NOCAPS = 0;		/*if set, all caps mapped to lower, plurals stripped*/
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
{
	ECHO;
	}
break;
case 2:
{
	look(ic,2,ADJ);
	}
break;
case 3:
{
	look(ed,2,ED);
	}
break;
case 4:
{
	if(yytext[5] == 'E'){
		OUT1(NOUN_ADJ);
	}
	OUT1(ADJ_ADV);
	}
break;
case 5:
{
	look(ace,3,NOUN_VERB);
	}
break;
case 6:
{
	look(ice,3,NOUN_VERB);
	}
break;
case 7:
{
	if(look(ence,4,NOM))nomin++;
	}
break;
case 8:
{
	if(look(ance,4,NOM))nomin++;
	}
break;
case 9:
{
	look(ee,2,NOUN);
	}
break;
case 10:
{
	look(age,3,NOUN);
	}
break;
case 11:
{
	look(able,4,ADJ);
	}
break;
case 12:
{
	look(ible,4,ADJ);
	}
break;
case 13:
{
	look(cle,3,NOUN);
	}
break;
case 14:
{
	look(ure,3,NOUN);
	}
break;
case 15:
{
	look(ite,3,NOUN);
	}
break;
case 16:
{
	look(ive,3,ADJ);
	}
break;
case 17:
{
	look(ize,3,VERB);
	}
break;
case 18:
{
	look(ing,3,ING);
	}
break;
case 19:
{
	look(ish,3,ADJ);
	}
break;
case 20:
{
	look(cal,3,ADJ);
	}
break;
case 21:
{
	look(ional,5,ADJ);
	}
break;
case 22:
{
	look(ful,3,ADJ);
	}
break;
case 23:
{
	OUT1(NOUN);
	}
break;
case 24:
{
	look(man,3,NOUN);
	}
break;
case 25:
{
	OUT1(NV_PL);
	}
break;
case 26:
{
	if(look(ion,3,NOM))nomin++;
	}
break;
case 27:
{
	look(ship,4,NOUN);
	}
break;
case 28:
{
	look(lar,3,ADJ);
	}
break;
case 29:
{
	OUT1(NOUN_VERB);
	}
break;
case 30:
{
	OUT1(NOUN);
	}
break;
case 31:
{
	look(is,2,NOUN);
	}
break;
case 32:
{
	look(less,4,ADJ);
	}
break;
case 33:
{
	look(ness,4,NOUN);
	}
break;
case 34:
{
	look(ess,3,NOUN);
	}
break;
case 35:
{
	look(ss,2,NOUN);
	}
break;
case 36:
{
	look(ous,3,ADJ);
	}
break;
case 37:
{
	look(us,2,NOUN);
	}
break;
case 38:
{
	if(look(ion,4,PNOUN))nomin++;
	}
break;
case 39:
{
		if(look(ment,5,PNOUN))nomin++;
		}
break;
case 40:
{
		if(look(ence,5,PNOUN))nomin++;
		}
break;
case 41:
{
		if(look(ance,5,PNOUN))nomin++;
		}
break;
case 42:
{
	if(isupper(yytext[yyleng-1])){
		if(NOCAPS){
			yytext[yyleng-1] = tolower(yytext[yyleng-1]);
			POUT1(PNOUN);
		}
		else { OUT1(PNOUN); }
	}
	else {
		if(NOCAPS){POUT1(NV_PL);}
		else{ OUT1(NV_PL); }
	}
	}
break;
case 43:
{
	look(ant,3,NOUN_ADJ);
	}
break;
case 44:
{
	if(look(ment,4,NOM))nomin++;
	}
break;
case 45:
{
	look(est,3,ADJ);
	}
break;
case 46:
{
	look(ist,3,NOUN);
	}
break;
case 47:
{
	putchar(yytext[0]);
	}
break;
case 48:
{
	if(isupper(yytext[yyleng-1])){
		if(NOCAPS)
			yytext[yyleng-1] = tolower(yytext[yyleng-1]);
		if((yytext[0] == 'n' || yytext[0] == 'l') && yytext[1] == 'a'){
			OUT1(NOUN_ADJ);
		}
		else {
			OUT1(NOUN);
		}
	}
	else {
		OUT1(UNK);
	}
	}
break;
case 49:
{
	egetd();
	}
break;
case 50:
{
	printf("%s",yytext);
	}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
look(f,n,nc)
char (*f)();
int n;
char nc;
{
	int in,nn,ret;
	char sch;
	in=0;
	sch=yytext[yyleng-1];
	if(isupper(sch)){
		yytext[yyleng-1] = tolower(sch);
		in=1;
	}
	if((*f)(&yytext[n],1,0) != 0){
		nn = (*f)(&yytext[n],1,0);
		if(nc == PNOUN)
			if(nn == NOUN_VERB){
				if(in == 1)nn=PNOUN;
				else nn=NV_PL;
			}
		ret = 0;
	}
	else {
		nn = nc;
		ret = 1;
	}
	if(in==1){
		if(nn == NOUN_VERB)nn=NOUN;
		if(!NOCAPS)yytext[yyleng-1]=sch;	
	}
	if(nn==PNOUN && yytext[0] == 's' && NOCAPS){
		POUT1(nn);
	}
	else {
		OUT1(nn);
	}
	return(ret);
}
yywrap(){
	printf(";%d\n",nomin);
	return(1);
}
int yyvstop[] ={
0,

47,
0,

48,
0,

47,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

49,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

42,
0,

42,
48,
0,

42,
0,

42,
48,
0,

42,
48,
0,

42,
48,
0,

42,
48,
0,

42,
48,
0,

42,
48,
0,

48,
0,

48,
0,

1,
0,

50,
0,

2,
0,

2,
48,
0,

3,
0,

3,
48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

9,
0,

9,
48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

24,
48,
0,

25,
48,
0,

48,
0,

48,
0,

48,
0,

30,
0,

30,
48,
0,

30,
48,
0,

1,
42,
0,

42,
48,
0,

31,
42,
0,

31,
42,
48,
0,

42,
48,
0,

35,
42,
0,

35,
42,
48,
0,

35,
42,
48,
0,

42,
48,
0,

37,
42,
0,

37,
42,
48,
0,

37,
42,
48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

48,
0,

5,
0,

5,
48,
0,

6,
0,

6,
48,
0,

48,
0,

48,
0,

10,
0,

10,
48,
0,

48,
0,

48,
0,

13,
0,

13,
48,
0,

14,
0,

14,
48,
0,

15,
0,

15,
48,
0,

16,
0,

16,
48,
0,

17,
0,

17,
48,
0,

18,
0,

18,
48,
0,

19,
0,

19,
48,
0,

20,
0,

20,
48,
0,

48,
0,

22,
0,

22,
48,
0,

23,
0,

23,
48,
0,

24,
0,

25,
0,

26,
0,

26,
48,
0,

48,
0,

28,
0,

28,
48,
0,

30,
48,
0,

42,
48,
0,

42,
48,
0,

34,
35,
42,
0,

34,
35,
42,
48,
0,

34,
35,
42,
48,
0,

34,
35,
42,
48,
0,

42,
48,
0,

36,
37,
42,
0,

36,
37,
42,
48,
0,

43,
0,

43,
48,
0,

48,
0,

45,
0,

45,
48,
0,

46,
0,

46,
48,
0,

4,
0,

4,
48,
0,

8,
0,

8,
48,
0,

7,
0,

7,
48,
0,

11,
0,

11,
48,
0,

12,
0,

12,
48,
0,

48,
0,

27,
0,

27,
48,
0,

29,
30,
48,
0,

42,
48,
0,

42,
48,
0,

38,
42,
0,

38,
42,
48,
0,

32,
34,
35,
42,
0,

32,
34,
35,
42,
48,
0,

33,
34,
35,
42,
0,

33,
34,
35,
42,
48,
0,

42,
48,
0,

44,
0,

44,
48,
0,

21,
0,

21,
48,
0,

41,
42,
0,

41,
42,
48,
0,

40,
42,
0,

40,
42,
48,
0,

39,
42,
0,

39,
42,
48,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,3,	
22,58,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,3,	1,4,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	57,0,	5,21,	
8,21,	60,0,	62,0,	68,0,	
0,0,	0,0,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
9,21,	10,21,	12,21,	13,21,	
17,21,	11,21,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
1,4,	1,4,	1,4,	1,4,	
2,3,	26,63,	29,69,	30,70,	
30,71,	31,72,	32,73,	33,74,	
34,75,	35,76,	36,77,	38,80,	
10,25,	39,81,	11,27,	9,24,	
11,28,	40,82,	11,29,	17,43,	
41,83,	37,78,	12,35,	11,30,	
2,5,	10,26,	15,21,	42,84,	
13,36,	11,31,	43,85,	11,32,	
37,79,	11,33,	2,6,	2,4,	
44,86,	11,34,	2,5,	2,5,	
2,5,	2,5,	2,5,	2,5,	
2,5,	2,5,	2,5,	2,5,	
2,7,	2,5,	55,102,	63,106,	
79,133,	85,142,	55,103,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	56,104,	66,111,	15,39,	
90,0,	56,105,	66,112,	2,8,	
2,8,	2,9,	2,10,	2,11,	
2,8,	2,12,	2,13,	2,8,	
2,8,	2,8,	2,14,	2,15,	
2,16,	2,8,	2,17,	2,8,	
2,18,	2,19,	2,20,	2,8,	
2,8,	2,8,	2,8,	2,8,	
2,8,	4,4,	27,64,	70,115,	
93,0,	103,157,	108,0,	110,0,	
114,0,	118,0,	27,65,	70,116,	
120,0,	122,0,	124,0,	27,66,	
126,0,	128,0,	130,0,	132,0,	
133,172,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	14,21,	
16,21,	135,0,	18,21,	137,0,	
20,21,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	4,4,	
4,4,	4,4,	4,4,	7,22,	
47,0,	19,46,	49,0,	21,57,	
141,0,	24,59,	14,37,	16,40,	
7,23,	18,44,	19,0,	16,41,	
21,0,	144,0,	24,0,	154,0,	
156,0,	51,0,	25,61,	159,0,	
161,0,	16,42,	163,0,	18,45,	
20,55,	165,0,	14,38,	25,0,	
167,0,	20,56,	169,0,	28,67,	
7,22,	53,0,	19,46,	47,47,	
21,57,	49,47,	24,59,	171,0,	
28,0,	174,0,	7,22,	7,22,	
19,46,	19,47,	21,57,	21,57,	
24,59,	24,60,	45,87,	25,61,	
51,47,	179,0,	181,0,	47,47,	
183,0,	49,47,	19,48,	45,0,	
186,0,	25,61,	25,62,	7,22,	
28,67,	19,47,	188,0,	21,57,	
53,47,	24,60,	190,0,	59,0,	
51,47,	61,0,	28,67,	28,68,	
46,46,	192,0,	48,90,	194,0,	
50,92,	0,0,	25,62,	45,87,	
0,0,	46,0,	0,0,	48,0,	
53,47,	50,0,	0,0,	49,91,	
0,0,	45,87,	45,88,	28,68,	
67,0,	0,0,	0,0,	0,0,	
88,0,	19,49,	0,0,	52,95,	
0,0,	19,50,	59,59,	0,0,	
61,61,	46,46,	19,51,	48,90,	
52,0,	50,92,	45,88,	19,52,	
19,53,	19,54,	51,94,	46,46,	
46,46,	48,90,	48,90,	50,92,	
50,93,	54,99,	59,59,	0,0,	
61,61,	64,107,	0,0,	67,67,	
65,109,	53,98,	54,0,	88,88,	
52,95,	0,0,	64,0,	0,0,	
46,46,	65,0,	48,90,	0,0,	
50,93,	92,0,	52,95,	52,96,	
69,113,	0,0,	0,0,	67,67,	
0,0,	91,0,	71,117,	88,88,	
0,0,	69,0,	54,99,	45,89,	
0,0,	72,119,	64,107,	71,0,	
0,0,	65,109,	0,0,	52,96,	
54,99,	54,100,	72,0,	0,0,	
64,107,	64,108,	96,0,	65,109,	
65,110,	73,121,	0,0,	0,0,	
92,92,	69,113,	98,0,	0,0,	
0,0,	0,0,	73,0,	71,117,	
91,47,	54,100,	74,123,	69,113,	
69,114,	64,108,	72,119,	0,0,	
65,110,	71,117,	71,118,	74,0,	
92,92,	0,0,	0,0,	52,97,	
72,119,	72,120,	75,125,	0,0,	
91,47,	96,96,	73,121,	0,0,	
69,114,	76,127,	0,0,	75,0,	
0,0,	98,47,	71,118,	77,129,	
73,121,	73,122,	76,0,	74,123,	
0,0,	72,120,	0,0,	0,0,	
77,0,	96,96,	0,0,	78,131,	
0,0,	74,123,	74,124,	54,101,	
80,134,	98,47,	81,136,	75,125,	
78,0,	73,122,	0,0,	0,0,	
0,0,	80,0,	76,127,	81,0,	
0,0,	75,125,	75,126,	89,0,	
77,129,	91,146,	74,124,	0,0,	
76,127,	76,128,	0,0,	100,0,	
0,0,	107,0,	77,129,	77,130,	
78,131,	0,0,	0,0,	0,0,	
0,0,	80,134,	75,126,	81,136,	
0,0,	98,152,	78,131,	78,132,	
82,138,	76,128,	83,139,	80,134,	
80,135,	81,136,	81,137,	77,130,	
84,140,	82,0,	89,88,	83,0,	
0,0,	94,0,	86,143,	0,0,	
87,87,	84,0,	100,100,	78,132,	
107,107,	0,0,	0,0,	86,0,	
80,135,	87,0,	81,137,	0,0,	
0,0,	0,0,	89,88,	0,0,	
109,0,	82,138,	0,0,	83,139,	
113,0,	0,0,	100,100,	0,0,	
107,107,	84,140,	0,0,	82,138,	
82,82,	83,139,	83,83,	86,143,	
94,47,	87,87,	0,0,	84,140,	
84,141,	95,95,	0,0,	117,0,	
97,148,	86,143,	86,144,	87,87,	
87,87,	99,99,	95,0,	0,0,	
82,82,	97,0,	83,83,	109,109,	
94,47,	0,0,	99,0,	113,113,	
84,141,	0,0,	119,0,	0,0,	
89,145,	0,0,	86,144,	101,153,	
87,87,	0,0,	0,0,	102,155,	
0,0,	0,0,	95,95,	109,109,	
101,0,	97,148,	117,117,	113,113,	
102,0,	0,0,	99,99,	0,0,	
95,95,	95,95,	104,158,	97,148,	
97,149,	121,0,	105,160,	123,0,	
99,99,	99,99,	106,162,	104,0,	
94,147,	119,119,	117,117,	105,0,	
101,153,	0,0,	0,0,	106,0,	
102,155,	95,95,	0,0,	125,0,	
97,149,	0,0,	101,153,	101,154,	
111,164,	99,99,	102,155,	102,156,	
112,166,	119,119,	0,0,	104,158,	
0,0,	111,0,	127,0,	105,160,	
121,121,	112,0,	123,123,	106,162,	
0,0,	104,158,	104,159,	101,154,	
129,0,	105,160,	105,161,	102,156,	
131,0,	106,162,	106,163,	115,168,	
0,0,	0,0,	125,125,	134,0,	
121,121,	111,164,	123,123,	0,0,	
115,0,	112,166,	104,159,	97,150,	
116,170,	97,151,	105,161,	111,164,	
111,165,	127,127,	106,163,	112,166,	
112,167,	116,0,	125,125,	0,0,	
136,0,	138,0,	139,0,	129,129,	
0,0,	140,0,	143,0,	131,131,	
115,168,	145,0,	146,0,	149,0,	
111,165,	127,127,	134,134,	142,173,	
112,167,	0,0,	115,168,	115,169,	
0,0,	116,170,	0,0,	129,129,	
142,0,	0,0,	0,0,	131,131,	
152,0,	147,178,	0,0,	116,170,	
116,171,	153,0,	134,134,	136,136,	
138,138,	139,139,	147,0,	115,169,	
140,140,	143,143,	148,148,	0,0,	
145,88,	146,47,	149,149,	0,0,	
142,173,	155,0,	0,0,	148,0,	
116,171,	0,0,	158,0,	136,136,	
138,138,	139,139,	142,173,	142,174,	
140,140,	143,143,	147,178,	152,47,	
145,88,	146,47,	149,149,	150,180,	
153,153,	151,182,	0,0,	0,0,	
147,178,	147,179,	0,0,	148,148,	
150,0,	0,0,	151,0,	142,174,	
160,0,	157,185,	162,0,	152,47,	
155,155,	148,148,	148,148,	164,0,	
153,153,	158,158,	157,0,	166,0,	
0,0,	147,179,	168,0,	170,0,	
0,0,	146,176,	0,0,	173,0,	
150,180,	146,177,	151,182,	145,175,	
155,155,	175,0,	148,148,	0,0,	
0,0,	158,158,	150,180,	150,181,	
151,182,	151,183,	157,185,	160,160,	
0,0,	162,162,	178,0,	172,187,	
180,0,	0,0,	164,164,	176,189,	
157,185,	157,186,	166,166,	152,184,	
172,0,	168,168,	170,170,	150,181,	
176,0,	151,183,	173,173,	160,160,	
0,0,	162,162,	177,191,	182,0,	
175,88,	185,0,	164,164,	187,0,	
189,0,	157,186,	166,166,	177,0,	
0,0,	168,168,	170,170,	191,0,	
172,187,	178,178,	173,173,	180,180,	
176,189,	0,0,	193,0,	0,0,	
175,88,	0,0,	172,187,	172,188,	
0,0,	0,0,	176,189,	176,190,	
184,193,	0,0,	0,0,	177,191,	
0,0,	178,178,	182,182,	180,180,	
185,185,	184,0,	187,187,	189,189,	
0,0,	177,191,	177,192,	172,188,	
0,0,	0,0,	191,191,	176,190,	
0,0,	0,0,	0,0,	0,0,	
0,0,	193,193,	182,182,	0,0,	
185,185,	0,0,	187,187,	189,189,	
0,0,	184,193,	177,192,	0,0,	
0,0,	0,0,	191,191,	0,0,	
0,0,	0,0,	0,0,	184,193,	
184,194,	193,193,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
184,194,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+1,	0,		0,	
yycrank+114,	0,		0,	
yycrank+0,	0,		yyvstop+1,
yycrank+192,	0,		yyvstop+3,
yycrank+1,	0,		0,	
yycrank+0,	yysvec+5,	yyvstop+5,
yycrank+-314,	0,		0,	
yycrank+2,	yysvec+4,	yyvstop+7,
yycrank+34,	yysvec+4,	yyvstop+9,
yycrank+35,	yysvec+4,	yyvstop+11,
yycrank+39,	yysvec+4,	yyvstop+13,
yycrank+36,	yysvec+4,	yyvstop+15,
yycrank+37,	yysvec+4,	yyvstop+17,
yycrank+225,	yysvec+4,	yyvstop+19,
yycrank+92,	yysvec+4,	yyvstop+21,
yycrank+226,	yysvec+4,	yyvstop+23,
yycrank+38,	yysvec+4,	yyvstop+25,
yycrank+228,	yysvec+4,	yyvstop+27,
yycrank+-316,	0,		yyvstop+29,
yycrank+230,	yysvec+4,	yyvstop+31,
yycrank+-318,	0,		0,	
yycrank+-2,	yysvec+7,	0,	
yycrank+0,	0,		yyvstop+33,
yycrank+-320,	0,		yyvstop+35,
yycrank+-333,	0,		yyvstop+37,
yycrank+28,	yysvec+4,	yyvstop+39,
yycrank+141,	yysvec+4,	yyvstop+41,
yycrank+-346,	0,		yyvstop+43,
yycrank+29,	yysvec+4,	yyvstop+45,
yycrank+29,	yysvec+4,	yyvstop+47,
yycrank+12,	yysvec+4,	yyvstop+49,
yycrank+25,	yysvec+4,	yyvstop+51,
yycrank+26,	yysvec+4,	yyvstop+53,
yycrank+27,	yysvec+4,	yyvstop+55,
yycrank+28,	yysvec+4,	yyvstop+57,
yycrank+29,	yysvec+4,	yyvstop+59,
yycrank+46,	yysvec+4,	yyvstop+61,
yycrank+33,	yysvec+4,	yyvstop+63,
yycrank+32,	yysvec+4,	yyvstop+65,
yycrank+32,	yysvec+4,	yyvstop+67,
yycrank+35,	yysvec+4,	yyvstop+69,
yycrank+46,	yysvec+4,	yyvstop+71,
yycrank+50,	yysvec+4,	yyvstop+73,
yycrank+52,	yysvec+4,	yyvstop+75,
yycrank+-365,	0,		yyvstop+77,
yycrank+-391,	0,		yyvstop+79,
yycrank+-306,	yysvec+46,	yyvstop+81,
yycrank+-393,	0,		yyvstop+84,
yycrank+-308,	yysvec+46,	yyvstop+86,
yycrank+-395,	0,		yyvstop+89,
yycrank+-323,	yysvec+46,	yyvstop+92,
yycrank+-418,	0,		yyvstop+95,
yycrank+-339,	yysvec+46,	yyvstop+98,
yycrank+-440,	0,		yyvstop+101,
yycrank+77,	yysvec+4,	yyvstop+104,
yycrank+104,	yysvec+4,	yyvstop+106,
yycrank+-48,	yysvec+21,	yyvstop+108,
yycrank+0,	0,		yyvstop+110,
yycrank+-377,	yysvec+24,	yyvstop+112,
yycrank+-51,	yysvec+24,	yyvstop+114,
yycrank+-379,	yysvec+25,	yyvstop+117,
yycrank+-52,	yysvec+25,	yyvstop+119,
yycrank+56,	yysvec+4,	yyvstop+122,
yycrank+-444,	0,		yyvstop+124,
yycrank+-447,	0,		yyvstop+126,
yycrank+109,	yysvec+4,	yyvstop+128,
yycrank+-402,	yysvec+28,	yyvstop+130,
yycrank+-53,	yysvec+28,	yyvstop+132,
yycrank+-463,	0,		yyvstop+135,
yycrank+142,	yysvec+4,	yyvstop+137,
yycrank+-469,	0,		yyvstop+139,
yycrank+-476,	0,		yyvstop+141,
yycrank+-492,	0,		yyvstop+143,
yycrank+-505,	0,		yyvstop+145,
yycrank+-521,	0,		yyvstop+147,
yycrank+-528,	0,		yyvstop+149,
yycrank+-534,	0,		yyvstop+151,
yycrank+-546,	0,		yyvstop+153,
yycrank+65,	yysvec+4,	yyvstop+155,
yycrank+-551,	0,		yyvstop+157,
yycrank+-553,	0,		yyvstop+159,
yycrank+-591,	0,		yyvstop+161,
yycrank+-593,	0,		yyvstop+164,
yycrank+-599,	0,		yyvstop+167,
yycrank+62,	yysvec+4,	yyvstop+169,
yycrank+-605,	0,		yyvstop+171,
yycrank+-607,	0,		yyvstop+173,
yycrank+-406,	yysvec+87,	yyvstop+175,
yycrank+-557,	yysvec+87,	yyvstop+178,
yycrank+-198,	yysvec+48,	yyvstop+181,
yycrank+-459,	yysvec+46,	yyvstop+184,
yycrank+-451,	yysvec+50,	yyvstop+187,
yycrank+-230,	yysvec+50,	yyvstop+190,
yycrank+-595,	yysvec+46,	yyvstop+194,
yycrank+-644,	0,		yyvstop+197,
yycrank+-480,	yysvec+95,	yyvstop+200,
yycrank+-647,	0,		yyvstop+204,
yycrank+-488,	yysvec+46,	yyvstop+208,
yycrank+-652,	0,		yyvstop+211,
yycrank+-565,	yysvec+99,	yyvstop+214,
yycrank+-670,	0,		yyvstop+218,
yycrank+-674,	0,		yyvstop+222,
yycrank+132,	yysvec+4,	yyvstop+224,
yycrank+-689,	0,		yyvstop+226,
yycrank+-693,	0,		yyvstop+228,
yycrank+-697,	0,		yyvstop+230,
yycrank+-567,	yysvec+64,	yyvstop+232,
yycrank+-232,	yysvec+64,	yyvstop+234,
yycrank+-614,	yysvec+65,	yyvstop+237,
yycrank+-233,	yysvec+65,	yyvstop+239,
yycrank+-715,	0,		yyvstop+242,
yycrank+-719,	0,		yyvstop+244,
yycrank+-618,	yysvec+69,	yyvstop+246,
yycrank+-234,	yysvec+69,	yyvstop+248,
yycrank+-742,	0,		yyvstop+251,
yycrank+-755,	0,		yyvstop+253,
yycrank+-637,	yysvec+71,	yyvstop+255,
yycrank+-235,	yysvec+71,	yyvstop+257,
yycrank+-656,	yysvec+72,	yyvstop+260,
yycrank+-238,	yysvec+72,	yyvstop+262,
yycrank+-683,	yysvec+73,	yyvstop+265,
yycrank+-239,	yysvec+73,	yyvstop+267,
yycrank+-685,	yysvec+74,	yyvstop+270,
yycrank+-240,	yysvec+74,	yyvstop+272,
yycrank+-701,	yysvec+75,	yyvstop+275,
yycrank+-242,	yysvec+75,	yyvstop+277,
yycrank+-716,	yysvec+76,	yyvstop+280,
yycrank+-243,	yysvec+76,	yyvstop+282,
yycrank+-726,	yysvec+77,	yyvstop+285,
yycrank+-244,	yysvec+77,	yyvstop+287,
yycrank+-730,	yysvec+78,	yyvstop+290,
yycrank+-245,	yysvec+78,	yyvstop+292,
yycrank+151,	yysvec+4,	yyvstop+295,
yycrank+-737,	yysvec+80,	yyvstop+297,
yycrank+-275,	yysvec+80,	yyvstop+299,
yycrank+-758,	yysvec+81,	yyvstop+302,
yycrank+-277,	yysvec+81,	yyvstop+304,
yycrank+-759,	yysvec+82,	yyvstop+307,
yycrank+-760,	yysvec+83,	yyvstop+309,
yycrank+-763,	yysvec+84,	yyvstop+311,
yycrank+-310,	yysvec+84,	yyvstop+313,
yycrank+-782,	0,		yyvstop+316,
yycrank+-764,	yysvec+86,	yyvstop+318,
yycrank+-319,	yysvec+86,	yyvstop+320,
yycrank+-767,	yysvec+87,	yyvstop+323,
yycrank+-768,	yysvec+46,	yyvstop+326,
yycrank+-796,	0,		yyvstop+329,
yycrank+-809,	0,		yyvstop+332,
yycrank+-769,	yysvec+148,	yyvstop+336,
yycrank+-834,	0,		yyvstop+341,
yycrank+-836,	0,		yyvstop+346,
yycrank+-786,	yysvec+46,	yyvstop+351,
yycrank+-791,	yysvec+101,	yyvstop+354,
yycrank+-321,	yysvec+101,	yyvstop+358,
yycrank+-807,	yysvec+102,	yyvstop+363,
yycrank+-322,	yysvec+102,	yyvstop+365,
yycrank+-848,	0,		yyvstop+368,
yycrank+-812,	yysvec+104,	yyvstop+370,
yycrank+-325,	yysvec+104,	yyvstop+372,
yycrank+-838,	yysvec+105,	yyvstop+375,
yycrank+-326,	yysvec+105,	yyvstop+377,
yycrank+-840,	yysvec+106,	yyvstop+380,
yycrank+-328,	yysvec+106,	yyvstop+382,
yycrank+-845,	yysvec+111,	yyvstop+385,
yycrank+-331,	yysvec+111,	yyvstop+387,
yycrank+-849,	yysvec+112,	yyvstop+390,
yycrank+-334,	yysvec+112,	yyvstop+392,
yycrank+-852,	yysvec+115,	yyvstop+395,
yycrank+-336,	yysvec+115,	yyvstop+397,
yycrank+-853,	yysvec+116,	yyvstop+400,
yycrank+-345,	yysvec+116,	yyvstop+402,
yycrank+-886,	0,		yyvstop+405,
yycrank+-857,	yysvec+142,	yyvstop+407,
yycrank+-347,	yysvec+142,	yyvstop+409,
yycrank+-863,	yysvec+87,	yyvstop+412,
yycrank+-890,	0,		yyvstop+416,
yycrank+-905,	0,		yyvstop+419,
yycrank+-876,	yysvec+147,	yyvstop+422,
yycrank+-359,	yysvec+147,	yyvstop+425,
yycrank+-878,	yysvec+150,	yyvstop+429,
yycrank+-360,	yysvec+150,	yyvstop+434,
yycrank+-897,	yysvec+151,	yyvstop+440,
yycrank+-362,	yysvec+151,	yyvstop+445,
yycrank+-935,	0,		yyvstop+451,
yycrank+-899,	yysvec+157,	yyvstop+454,
yycrank+-366,	yysvec+157,	yyvstop+456,
yycrank+-901,	yysvec+172,	yyvstop+459,
yycrank+-372,	yysvec+172,	yyvstop+461,
yycrank+-902,	yysvec+176,	yyvstop+464,
yycrank+-376,	yysvec+176,	yyvstop+467,
yycrank+-909,	yysvec+177,	yyvstop+471,
yycrank+-383,	yysvec+177,	yyvstop+474,
yycrank+-916,	yysvec+184,	yyvstop+478,
yycrank+-385,	yysvec+184,	yyvstop+481,
0,	0,	0};
struct yywork *yytop = yycrank+1000;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,'"' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,',' ,'-' ,01  ,01  ,
'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,01  ,'"' ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
