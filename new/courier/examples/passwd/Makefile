#	1.3	Makefile	84/10/11
CFLAGS = -O
USEROBJS = lookup.o PasswordLookup_client.o
SRVROBJS = PasswordLookup.o PasswordLookup_server.o
LIBS = /usr/new/lib/libcr.a
DESTROOT =
TARGET=/usr/new/lib/courier

all:	lookup PasswordLookup

lookup:	$(USEROBJS)
	cc -o lookup $(USEROBJS) $(LIBS)

PasswordLookup:	$(SRVROBJS)
	cc -o PasswordLookup $(SRVROBJS) $(LIBS)

$(USEROBJS) $(SRVROBJS):	PasswordLookup.h

PasswordLookup.h PasswordLookup_server.c PasswordLookup_client.c: PasswordLookup.cr
	/usr/new/courier PasswordLookup.cr

install:	all
	install -c -s PasswordLookup $(DESTROOT)${TARGET}

clean:
	@rm -f *.o PasswordLookup_*.c PasswordLookup.h PasswordLookup

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info
clobber: clean
	@rm -f *.c Makefile
