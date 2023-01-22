# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.2	(Berkeley)	10/22/82
#
DESTROOT=
CFLAGS=	-O
OBJS=	indent.o io.o lexi.o parse.o pr_comment.o

indent: indent.o io.o lexi.o parse.o pr_comment.o
	cc -o indent ${OBJS}

clean:
	rm -f ${OBJS} indent

clobber: clean
	rm -f *.[ch] README Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

install: indent
	install -c -s indent ${DESTROOT}/usr/ucb/indent
