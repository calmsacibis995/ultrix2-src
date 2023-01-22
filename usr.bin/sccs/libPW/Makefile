

# 	@(#)Makefile	1.7	(ULTRIX)	3/19/86

DEBUG = 
LNAME = libPW.a
CFLAGS = $(DEBUG) -c -O -I../hdr
GET = sccs get


OBJ1 = anystr.o bal.o curdir.o fdfopen.o giveup.o strspn.o strpbrk.o strtok.o
OBJ2 = imatch.o index.o lockit.o logname.o move.o patoi.o
OBJ3 = patol.o regcmp.o regex.o rename.o repeat.o repl.o satoi.o
OBJ4 = setsig.o sname.o strend.o substr.o trnslat.o userdir.o
OBJ5 = username.o verify.o any.o xalloc.o xcreat.o xlink.o
OBJ6 = xopen.o xpipe.o xunlink.o xwrite.o xmsg.o alloca.o
OBJ7 = cat.o dname.o fatal.o clean.o userexit.o zero.o zeropad.o

$(LNAME): $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)
	-rm -f $(LNAME)
	ar r $(LNAME) $(OBJ1)
	ar r $(LNAME) $(OBJ2)
	ar r $(LNAME) $(OBJ3)
	ar r $(LNAME) $(OBJ4)
	ar r $(LNAME) $(OBJ5)
	ar r $(LNAME) $(OBJ6)
	ar r $(LNAME) $(OBJ7)
	ranlib $(LNAME)

anystr.o dname.o fatal.o fdfopen.o lockit.o satoi.o setsig.o xcreat.o \
	xmsg.o: ../hdr/macros.h


fatal.o rename.o : ../hdr/fatal.h 

install:	$(LNAME)
	/etc/install -n /usr/lib $(LNAME)

clean:
	-rm -f $(OBJ1)
	-rm -f $(OBJ2)
	-rm -f $(OBJ3)
	-rm -f $(OBJ4)
	-rm -f $(OBJ5)
	-rm -f $(OBJ6)
	-rm -f $(OBJ7)

clobber: 
	(cd $(TARGET); rm -f *.s)
	-rm -f *.[cs] Makefile

sccsinfo:
	-sccs info
	(cd $(TARGET); sccs info)

sccsget:
	-$(GET) SCCS
	-(cd $(TARGET); $(GET) SCCS)
	-cp $(TARGET)/*.s .
