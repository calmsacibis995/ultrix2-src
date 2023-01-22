#	@(#)Makefile	1.4	84/11/26
DESTROOT=
DEST	= ${DESTROOT}/usr/man/mann

all:;

install:
	-if [ ! -d ${DEST} ]; \
	then \
		mkdir ${DEST}; \
	else \
		true; \
	fi
	install -c checknews.1 $(DEST)/checknews.n
	install -c inews.1 $(DEST)/inews.n
	install -c postnews.1 $(DEST)/postnews.n
	install -c readnews.1 $(DEST)/readnews.n
	install -c newsrc.5 $(DEST)/newsrc.n
	install -c expire.8 $(DEST)/expire.n
	install -c recnews.8 $(DEST)/recnews.n
	install -c sendnews.8 $(DEST)/sendnews.n
	install -c uurec.8 $(DEST)/uurec.n

clean:;
clobber: clean
	@rm -f [a-z]* Makefile

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info
