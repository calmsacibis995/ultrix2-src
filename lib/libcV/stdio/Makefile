# @(#)Makefile	1.5	ULTRIX	1/29/87
#
# This takes everyting from the LIBC/stdio and recompiles it with the
# flags for System V set.
#
CFLAGS=	-O -Y
DESTROOT=
LIBC=	../../libc
STDIO=	${LIBC}/stdio

COMMON=	clrerr.c ctermid.c cuserid.c data.c doscan.c doopen.c\
	fclose.c fdopen.c fgetc.c fgets.c filbuf.c findiop.c \
	flsbuf.c fopen.c fprintf.c fputc.c fputs.c fread.c \
	freopen.c fseek.c ftell.c fwrite.c\
	getchar.c gets.c getw.c printf.c putchar.c puts.c \
	putw.c rew.c scanf.c setbuf.c setbuffer.c setvbuf.c \
	sprintf.c strout.c tempnam.c tmpfile.c tmpnam.c \
	ungetc.c vfprintf.c vprintf.c vsprintf.c


OBJS=	clrerr.o ctermid.o cuserid.o data.o doscan.o doopen.o\
	fclose.o fdopen.o fgetc.o fgets.o filbuf.o findiop.o \
	flsbuf.o fopen.o fprintf.o fputc.o fputs.o fread.o \
	freopen.o fseek.o ftell.o fwrite.o\
	getchar.o gets.o getw.o printf.o putchar.o puts.o \
	putw.o rew.o scanf.o setbuf.o setbuffer.o setvbuf.o \
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

stdiolib gfloat/stdiolib profiled/stdiolib: gfloat profiled ${OBJS}
	@for d in . profiled gfloat; \
	do ( \
		echo "Creating libcV/stdio/$$d/stdiolib" ; \
		cd $$d; ar cru stdiolib ${OBJS} \
	) ; done

gfloat profiled:
	mkdir $@

clean:
	rm -rf stdiolib profiled gfloat *.o

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

${COMMON}:
	ln -s ${STDIO}/$@

.DEFAULT:
	sccs get $<
