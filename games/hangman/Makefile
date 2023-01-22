# @(#)Makefile	1.2	ULTRIX	6/27/84
#
#
OBJS=		endgame.o extern.o getguess.o getword.o main.o playgame.o \
		prdata.o prman.o prword.o setup.o
CFILES=		endgame.c extern.c getguess.c getword.c main.c playgame.c \
		prdata.c prman.c prword.c setup.c
HDRS=		hangman.h
CFLAGS=		-O
LDFLAGS=	-g
DESTROOT=

all: hangman

tags: $(HDRS) $(CFILES)
	ctags $(HDRS) $(CFILES)

install: hangman
	install -s hangman $(DESTROOT)/usr/games/hangman
	
hangman: $(OBJS)
	$(CC) $(LDFLAGS) -o hangman $(OBJS) -lcurses -ltermlib

clean:
	rm -f $(OBJS) hangman ? core a.out errs

clobber: clean
	rm *.[ch] Makefile 

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

