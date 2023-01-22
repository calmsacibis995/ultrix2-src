#	@(#)Makefile	1.3		7/2/84
#	Makefile	4.2	83/07/02
#
#
#	Modification History
#
# 003-	David L Ballenger,	29-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
#	Stephen Reilly, 14-May-84
# 002 - Changed the symbol MVAXI to GFLOAT to reflect the meaning of 
#	the code.
#
#	Stephen Reilly, 04-Apr-84
# 001 - Added code that depends on the environment variable "target".
#	The reason is that some of the modules must be compiled 
#	differently for MicroVax than for the big vax.
#
# printf & co are here because they are machine dependent
#
CFLAGS=	-O
DESTROOT=

OBJS=	doprnt.o

.c.o:
	${CC} -p ${CFLAGS} -c -Md $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -c -Mg $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

stdiolib profiled/stdiolib gfloat/stdiolib: ${OBJS}
	@echo "building profiled stdiolib"
	@cd profiled; ar cru stdiolib ${OBJS}
	@echo "building gfloat stdiolib"
	@cd gfloat; ar cru stdiolib ${OBJS}
	@echo "building normal stdiolib"
	@ar cru stdiolib ${OBJS}

tools:	${OBJS}
	@echo "building normal stdiolib"
	ar cru stdiolib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f stdiolib profiled/stdiolib gfloat/stdiolib \
	      ${OBJS} profiled/*.o gfloat/*.o

doprnt.o: doprnt.s
	cp doprnt.s doprnt.c
	cc -E doprnt.c | sed -f mcount.sed | as -o doprnt.o
	-ld -x -r -o profiled/doprnt.o doprnt.o
	cc -E -DGFLOAT doprnt.c | as -o doprnt.o
	-ld -x -r -o gfloat/doprnt.o doprnt.o
	cc -E doprnt.c | as -o doprnt.o
	-ld -x -r doprnt.o
	mv a.out doprnt.o
	rm -f doprnt.c

clobber: clean
	rm -f *.[cs] mcount.sed Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

