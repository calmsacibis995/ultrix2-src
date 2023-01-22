

# 	@(#)Makefile	1.4	(ULTRIX)	1/8/85

#
# Make file to recreate all of sccs
#

DESTROOT=

TESTDIR=cmd

FS = /usr

LIB = $(FS)/lib

INSDIR=$(FS)/bin

FEINSD=$(FS)/ucb

HELPDIR=help

PUB_HELPLIB=$(LIB)/sccs.help

TARGET = vax

OWNER = root

GROUP = staff

FEOWN = sccs

FEGRP = staff

DEBUG =

all:
	cd lib; make TARGET=$(TARGET) DEBUG=$(DEBUG)
	cd libPW; make TARGET=$(TARGET) DEBUG=$(DEBUG)
	cd cmd; make DEBUG=$(DEBUG)


install: all

	-if [ ! -d $(DESTROOT)$(FS) ]; then \
		mkdir $(DESTROOT)$(FS); \
		chmod 755 $(DESTROOT)$(FS); \
		/etc/chown root $(DESTROOT)$(FS); \
		chgrp system $(DESTROOT)$(FS); \
	else true; \
	fi
	-if [ ! -d $(DESTROOT)$(INSDIR) ]; then \
		mkdir $(DESTROOT)$(INSDIR); \
		chmod 755 $(DESTROOT)$(INSDIR); \
		/etc/chown root $(DESTROOT)$(INSDIR); \
		chgrp system $(DESTROOT)$(INSDIR); \
	else true; \
	fi
	-if [ ! -d $(DESTROOT)$(FEINSD) ]; then \
		mkdir $(DESTROOT)$(FEINSD); \
		chmod 755 $(DESTROOT)$(FEINSD); \
		/etc/chown root $(DESTROOT)$(FEINSD); \
		chgrp system $(DESTROOT)$(FEINSD); \
	else true; \
	fi
	-if [ ! -d $(DESTROOT)$(LIB) ]; then \
		mkdir $(DESTROOT)$(LIB); \
		chmod 755 $(DESTROOT)$(LIB); \
		/etc/chown root $(DESTROOT)$(LIB); \
		chgrp system $(DESTROOT)$(LIB); \
	else true; \
	fi
	-if [ ! -d $(DESTROOT)$(PUB_HELPLIB) ]; then \
		mkdir $(DESTROOT)$(PUB_HELPLIB); \
		chmod 755 $(DESTROOT)$(PUB_HELPLIB); \
		/etc/chown root $(DESTROOT)$(PUB_HELPLIB); \
		chgrp system $(DESTROOT)$(PUB_HELPLIB); \
	else true; \
	fi

	-cp $(TESTDIR)/admin $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/admin
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/admin
	-chmod 755 $(DESTROOT)$(INSDIR)/admin

	-cp $(TESTDIR)/bdiff $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/bdiff
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/bdiff
	-chmod 755 $(DESTROOT)$(INSDIR)/bdiff

	-cp $(TESTDIR)/comb $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/comb
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/comb
	-chmod 755 $(DESTROOT)$(INSDIR)/comb

	-cp $(TESTDIR)/delta $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/delta
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/delta
	-chmod 755 $(DESTROOT)$(INSDIR)/delta

	-cp $(TESTDIR)/get $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/get
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/get
	-chmod 755 $(DESTROOT)$(INSDIR)/get

	-cp $(TESTDIR)/help $(DESTROOT)$(INSDIR)/sccshelp
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/sccshelp
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/sccshelp
	-chmod 755 $(DESTROOT)$(INSDIR)/sccshelp

	-cp $(TESTDIR)/prs $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/prs
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/prs
	-chmod 755 $(DESTROOT)$(INSDIR)/prs

	-cp $(TESTDIR)/rmchg $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/rmchg
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/rmchg
	-chmod 755 $(DESTROOT)$(INSDIR)/rmchg

	-rm -f $(DESTROOT)$(INSDIR)/cdc
	ln $(DESTROOT)$(INSDIR)/rmchg $(DESTROOT)$(INSDIR)/cdc

	-rm -f $(DESTROOT)$(INSDIR)/rmdel
	ln $(DESTROOT)$(INSDIR)/rmchg $(DESTROOT)$(INSDIR)/rmdel

	-cp $(TESTDIR)/sccsdiff.sh $(DESTROOT)$(INSDIR)/sccsdiff
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/sccsdiff
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/sccsdiff
	-chmod 755 $(DESTROOT)$(INSDIR)/sccsdiff

	-cp $(TESTDIR)/unget $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/unget
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/unget
	-chmod 755 $(DESTROOT)$(INSDIR)/unget

	-rm -f $(DESTROOT)$(INSDIR)/sact
	ln $(DESTROOT)$(INSDIR)/unget $(DESTROOT)$(INSDIR)/sact

	-cp $(TESTDIR)/val $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/val
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/val
	-chmod 755 $(DESTROOT)$(INSDIR)/val

	-cp $(TESTDIR)/vc $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/vc
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/vc
	-chmod 755 $(DESTROOT)$(INSDIR)/vc

	-cp $(TESTDIR)/what $(DESTROOT)$(INSDIR)
	-/etc/chown $(OWNER) $(DESTROOT)$(INSDIR)/what
	-chgrp $(GROUP) $(DESTROOT)$(INSDIR)/what
	-chmod 755 $(DESTROOT)$(INSDIR)/what

	-cp $(HELPDIR)/ad $(DESTROOT)$(PUB_HELPLIB)/ad
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/ad
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/ad
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/ad
	-cp $(HELPDIR)/bd $(DESTROOT)$(PUB_HELPLIB)/bd
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/bd
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/bd
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/bd
	-cp $(HELPDIR)/cb $(DESTROOT)$(PUB_HELPLIB)/cb
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/cb
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/cb
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/cb
	-cp $(HELPDIR)/cm $(DESTROOT)$(PUB_HELPLIB)/cm
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/cm
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/cm
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/cm
	-cp $(HELPDIR)/cmds $(DESTROOT)$(PUB_HELPLIB)/cmds
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/cmds
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/cmds
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/cmds
	-cp $(HELPDIR)/co $(DESTROOT)$(PUB_HELPLIB)/co
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/co
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/co
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/co
	-cp $(HELPDIR)/de $(DESTROOT)$(PUB_HELPLIB)/de
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/de
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/de
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/de
	-cp $(HELPDIR)/default $(DESTROOT)$(PUB_HELPLIB)/default
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/default
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/default
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/default
	-cp $(HELPDIR)/ge $(DESTROOT)$(PUB_HELPLIB)/ge
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/ge
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/ge
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/ge
	-cp $(HELPDIR)/he $(DESTROOT)$(PUB_HELPLIB)/he
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/he
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/he
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/he
	-cp $(HELPDIR)/prs $(DESTROOT)$(PUB_HELPLIB)/prs
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/prs
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/prs
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/prs
	-cp $(HELPDIR)/rc $(DESTROOT)$(PUB_HELPLIB)/rc
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/rc
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/rc
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/rc
	-cp $(HELPDIR)/sc $(DESTROOT)$(PUB_HELPLIB)/sc
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/sc
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/sc
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/sc
	-cp $(HELPDIR)/un $(DESTROOT)$(PUB_HELPLIB)/un
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/un
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/un
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/un
	-cp $(HELPDIR)/ut $(DESTROOT)$(PUB_HELPLIB)/ut
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/ut
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/ut
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/ut
	-cp $(HELPDIR)/vc $(DESTROOT)$(PUB_HELPLIB)/vc
	-/etc/chown $(OWNER) $(DESTROOT)$(PUB_HELPLIB)/vc
	-chgrp $(GROUP) $(DESTROOT)$(PUB_HELPLIB)/vc
	-chmod 644 $(DESTROOT)$(PUB_HELPLIB)/vc

	-cp $(TESTDIR)/sccs $(DESTROOT)$(FEINSD)
	-/etc/chown $(FEOWN) $(DESTROOT)$(FEINSD)/sccs
	-chgrp $(FEGRP) $(DESTROOT)$(FEINSD)/sccs
	-chmod 4011 $(DESTROOT)$(FEINSD)/sccs

clean:
	(cd cmd; make clean)
	(cd lib; make clean)
	(cd libPW; make TARGET=$(TARGET) clean)

clobber: 
	(cd hdr; rm -f *.h)
	(cd cmd; make clobber)
	(cd lib; make clobber)
	(cd libPW; make TARGET=$(TARGET) clobber)
	(cd help; rm -f [a-z]*)
	sccs clean
	
sccsinfo:
	sccs info
	(cd hdr; sccs info)
	(cd cmd; make sccsinfo)
	(cd lib; make sccsinfo)
	(cd libPW; make TARGET=$(TARGET) sccsinfo)
	(cd help; sccs info)

sccsget:
	(cd hdr; sccs get SCCS)
	(cd cmd; sccs get Makefile; make sccsget)
	(cd lib; sccs get Makefile; make sccsget)
	(cd libPW; sccs get Makefile; make TARGET=$(TARGET) sccsget)
	(cd help; sccs get SCCS)
