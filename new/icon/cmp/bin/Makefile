#	1.2	Makefile	84/10/11
DESTROOT=
all: 	iconc
	cd ../tran;       make ${MFLAGS}  
	cd ../link;       make ${MFLAGS} 
	cd ../operators;  make ${MFLAGS} 
	cd ../functions;  make ${MFLAGS} 
	cd ../lib;        make ${MFLAGS} 
	cd ../rt;         make ${MFLAGS} 
	cp Lib libi.a
	ranlib libi.a
	cp ../tran/utran ../link/ulink .

iconc: iconc.o
	cc -o iconc iconc.o

tran:
	cp ../tran/utran utran

link:
	cp ../link/ulink ulink

start:
	cp ../rt/start.o start.o

lib:
	cp Lib libi.a
	ranlib libi.a
clean:
	@rm -f iconc utran ulink libi.a *.o
clobber: clean
	@rm -f Makefile *.c
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS

clean-all: 
	rm -f iconc.o Lib
	cd ../tran;       make ${MFLAGS} clean
	cd ../link;       make ${MFLAGS} clean
	cd ../operators;  make ${MFLAGS} clean
	cd ../functions;  make ${MFLAGS} clean
	cd ../lib;        make ${MFLAGS} clean
	cd ../rt;         make ${MFLAGS} clean
