# @(#)Makefile	1.7	ULTRIX	6/28/85
#
#	Makefile	1.7	85/06/28
#
# 	Modifications
# 
#	David L Ballenger, 31-May-1985
# 004	Add ulimit and utime to libc
#
# 003-	David L Ballenger,	23-Apr-1985
#	Correct OBJS declaration, ie. stime.o instead of stime.c.
#
# 002-	David L Ballenger,	29-Mar-1985
#	Add stime(), tmpnam() now in ../stdio
#
# 001-	David L Ballenger,	24-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
CFLAGS=	-O
OBJS=	stime.o strcatn.o strcmpn.o strcpyn.o ulimit.o utime.o
DESTROOT=

.c.o:
	${CC} -p ${CFLAGS} -c $*.c 
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -c -Mg $*.c
	-ld -X -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

compat-sys5lib profiled/compat-sys5lib gfloat/compat-sys5lib: ${OBJS}
	@echo "building profiled compat-sys5lib"
	@cd profiled; ar cru compat-sys5lib ${OBJS}
	@echo "building gfloat compat-sys5lib"
	@cd gfloat; ar cru compat-sys5lib ${OBJS}
	@echo "building normal compat-sys5lib"
	@ar cru compat-sys5lib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal compat-sys5lib"
	ar cru compat-sys5lib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f compat-sys5lib profiled/compat-sys5lib gfloat/compat-sys5lib \
	      ${OBJS} profiled/*.o gfloat/*.o

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

