#	1.2	Makefile	84/10/11
DESTROOT=

icont:
	DoSamps icont >Out.it 2>&1
	diff Out.it Results.it

iconc:
	DoSamps iconc >Out.ic 2>&1
	diff Out.ic Results.ic

clean:
	@rm -f Out.ic Out.it btrees cross hello meander prefix recogn roman seqimage wordcount

clobber: clean
	@rm -f *.icn [a-z]* Makefile
sccsinfo:
	@sccs info

sccsget:
	@sccs get -s SCCS

all:

install: all
