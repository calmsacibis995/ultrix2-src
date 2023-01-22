# @(#)Makefile	1.8	ULTRIX	8/2/85
#
#	Makefile	4.8	83/06/30
#
# 	Modifications
#
# 002-	David L Ballenger,	29-Mar-1985
#	Add routines from System V.
#
# 001-	David L Ballenger,	24-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
# doprnt is not here because it is machine dependent
#
CFLAGS=	-O
DESTROOT=

OBJS=	cleanup.o clrerr.o ctermid.o cuserid.o data.o doscan.o doopen.o\
	exit.o fclose.o fdopen.o fgetc.o fgets.o filbuf.o findiop.o \
	flsbuf.o fopen.o fprintf.o fputc.o fputs.o fread.o \
	freopen.o fseek.o ftell.o fwrite.o getstdiobuf.o\
	getchar.o gets.o getw.o printf.o putchar.o puts.o \
	putw.o rew.o scanf.o setbuf.o setbuffer.o setlinebuf.o setvbuf.o \
	sibuf.o sobuf.o \
	sprintf.o strout.o tempnam.o tmpfile.o tmpnam.o \
	ungetc.o vfprintf.o vprintf.o vsprintf.o

.c.o:
	${CC} -p ${CFLAGS} -c -Md $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -c -Mg $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c -Md $*.c
	-ld -x -r $*.o
	mv a.out $*.o

stdiolib stdiolib_p: ${OBJS}
	@echo "building profiled stdiolib"
	@cd profiled; ar cru stdiolib ${OBJS}
	@echo "building gfloat stdiolib"
	@cd gfloat; ar cru stdiolib ${OBJS}
	@echo "building normal stdiolib"
	@ar cru stdiolib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -Md -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal stdiolib"
	ar cru stdiolib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f stdiolib profiled/stdiolib gfloat/stdiolib \
	      ${OBJS} profiled/*.o gfloat/*.o

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
