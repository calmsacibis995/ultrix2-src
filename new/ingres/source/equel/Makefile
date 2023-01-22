#		@(#)Makefile	1.4	(ULTRIX)	2/13/85
#
#  Makefile for INGRES EQUEL precompiler
#
#	David L Ballenger, 16-Jan-1984
# 001	Integrate into ULTRIX build procedures.
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin
SYSBIN=	/usr/new

OBJS= 	main.o yylex.o grammar.o cvar.o include.o name.o getkey.o \
	number.o operator.o prtout.o string.o comment.o retrieve.o display.o \
	getch.o cmap.o yyerror.o nalloc.o 
SRCS= 	main.c yylex.c grammar.y cvar.c include.c name.c getkey.c \
	number.c operator.c prtout.c string.c comment.c retrieve.c display.c \
	getch.c cmap.c yyerror.c nalloc.c constants.h globals.h tokens.y

GUTIL=	../gutil/gutil.a
# include -lm on the VAX
LIBS=	$(GUTIL) -lm

CFLAGS=	-I$H -O 
LDFLAGS=-z
YFLAGS=	-d
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Equel Precompiler
BINMODE=755
INSTALL=install

all: equel

equel: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o equel

install: all
	${INSTALL} -c -m ${BINMODE} -o ingres equel ${BIN}

new:
	rm -f equel
clean:	new
	rm -f *.o
	rm -f a.out y.tab.c y.tab.h core grammar.c 

clobber: clean
	rm -f *.[chsy] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: $(SRCS)

#$(SRCS):
#	sccs get $@

save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ${PATH}
	@for i in ${FILES}; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done

show_current:
	@echo ${PATH}
	@sccs what ${FILES} | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 7)" -n Makefile TraceFlags
	$(VGRIND) -h "$(HEADER) (Release 7)" *.[hm] *.[csy]
	$(VGRIND) -h "$(HEADER) (Release 7)" -x index

y.tab.h: grammar.c

depend:
	grep '^#[ 	]*include' $(SRCS) | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

main.o:constants.h
main.o:globals.h
yylex.o:constants.h
yylex.o:globals.h
yylex.o:y.tab.h
grammar.y:constants.h
grammar.y:globals.h
grammar.y:tokens.y
cvar.o:constants.h
cvar.o:globals.h
include.o:constants.h
include.o:globals.h
name.o:constants.h
name.o:globals.h
name.o:y.tab.h
getkey.o:constants.h
getkey.o:globals.h
number.o:constants.h
number.o:globals.h
number.o:y.tab.h
operator.o:constants.h
operator.o:globals.h
operator.o:y.tab.h
prtout.o:constants.h
prtout.o:globals.h
string.o:constants.h
string.o:globals.h
string.o:y.tab.h
comment.o:constants.h
comment.o:globals.h
retrieve.o:constants.h
retrieve.o:globals.h
display.o:constants.h
display.o:globals.h
getch.o:constants.h
getch.o:globals.h
cmap.o:constants.h
yyerror.o:constants.h
yyerror.o:globals.h
yyerror.o:y.tab.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
