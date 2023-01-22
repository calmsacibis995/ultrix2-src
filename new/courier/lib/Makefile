#	1.4	Makefile	84/10/11
SRCS=	client.c bind.c server.c misc.c
OBJS=	client.o bind.o server.o misc.o
CFLAGS=	-O -DDEBUG
DESTROOT=
INCLUDE=/usr/include
TARGET=/usr/new/lib

.c.o:
	${CC} -c ${CFLAGS} -p $*.c
	ld -x -r -o profiled/$*.o $*.o
	${CC} -c ${CFLAGS} $*.c
	ld -x -r $*.o
	mv a.out $*.o

libcr libcr_p: ${OBJS}
	@echo building profiled libcr
	@cd profiled; ar cr ../libcr_p ${OBJS}
	@echo building normal libcr
	@ar cr libcr ${OBJS}

lint:	${SRCS}
	lint -hux ${SRCS}

# beware, the compiler expects courier.h to be in /usr/include
# but it isn't put there for 1.0 compatibility reasons - rsp
install: libcr libcr_p
#	install -c -m 644 courier.h ${DESTROOT}${INCLUDE}/courier.h
	@install -c -m 644 libcr ${DESTROOT}${TARGET}/libcr.a
	@ranlib ${DESTROOT}${TARGET}/libcr.a
	@install -c -m 644 libcr_p ${DESTROOT}${TARGET}/libcr_p.a
	@ranlib ${DESTROOT}${TARGET}/libcr_p.a

clean:
	@rm -f ${OBJS} profiled/*.o libcr libcr_p

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info
clobber: clean
	@rm -f Makefile *.[chy]
