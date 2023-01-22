#	1.3	Makefile	84/10/11
OBJS=	courier.o main.o scanner.o sem.o code1.o code2.o
SRCS=	courier.y main.c scanner.l sem.c code1.c code2.c
YFLAGS=	-d
CFLAGS=	-O
DESTROOT=
TARGET=/usr/new

courier:	${OBJS}
		${CC} ${LDFLAGS} -o courier ${OBJS} -ll

courier.o:	courier.y
scanner.o:	courier.y

${OBJS}:	Courier.h

lint:		y.tab.c
		lint -hnux ${SRCS}

install:	courier
		@install -c -s courier ${DESTROOT}${TARGET}/courier

clean:
		@rm -f ${OBJS} y.tab.[ch] lex.yy.c

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info
clobber: 	clean
		@rm -f Makefile *.[chy]
