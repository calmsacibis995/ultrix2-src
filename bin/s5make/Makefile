# ULTRIX Makefile
# The rules.c file can be modified locally for people who still like
#	things like fortran.

TESTDIR	= .
YACCRM	= rm -f
LDFLAGS	= -n
INSDIR	= /bin

HEADERS	=	defs

SOURCES	=	main.c doname.c misc.c files.c rules.c dosys.c gram.y \
		dyndep.c prtmem.c

OBJECTS	=	main.o doname.o misc.o files.o rules.o dosys.o gram.o \
		dyndep.o prtmem.o

PROF	=
CFLAGS	= -O -DASCARCH $(PROF)

all:  s5make
	@echo s5make is up to date.

s5make:  $(OBJECTS)
	$(CC) $(PROF) -o $(TESTDIR)/s5make $(LDFLAGS) $(OBJECTS)

gram.c:	gram.y

gram.o: gram.c

$(OBJECTS):  $(HEADERS)

install: all
	install -c -m 755 -s $(TESTDIR)/s5make ${DESTROOT}$(INSDIR)

clean:
	-rm -f *.o
	-$(YACCRM) gram.c

clobber: clean
	-rm -f $(SOURCES) $(HEADERS)
	-rm -f $(TESTDIR)/s5make

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<
