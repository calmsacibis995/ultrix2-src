#
# Sccsid = @(#)Makefile	1.1	1/2/85
#
# Binary Kit Installation Makefile	28-Jun-84

# definitions
SPOOLDIR = /usr/spool/news
LIBDIR = /usr/lib/news
BINDIR = /usr/bin
NEWSUSR = news
NEWSGRP = news
DEST	= ${DESTDIR}/usr/man/mann
OTHERS = uurec recnews sendnews expire batch unbatch caesar recmail


install:
	cp inews readnews checknews postnews $(BINDIR)
	-cd $(BINDIR); /etc/chown $(NEWSUSR) inews readnews checknews postnews \
		; chgrp $(NEWSGRP) inews
	chmod 6755 $(BINDIR)/inews
	chmod 755 $(BINDIR)/readnews $(BINDIR)/checknews $(BINDIR)/postnews
	sh install.v7 $(SPOOLDIR) $(LIBDIR) $(NEWSUSR) $(NEWSGRP)
	cp help $(OTHERS) $(LIBDIR)
	cd $(LIBDIR) ; chmod 755 $(OTHERS)
	-rm -f $(BINDIR)/rnews
	ln $(BINDIR)/inews $(BINDIR)/rnews


	newactive.sh active $(LIBDIR) $(SPOOLDIR)
	/etc/chown $(NEWSUSR) $(LIBDIR)/*
	/etc/chown $(NEWSUSR) $(SPOOLDIR)/*
	/etc/chown $(NEWSUSR) $(SPOOLDIR)/*/*
	/etc/chown $(NEWSUSR) $(SPOOLDIR)/*/*/*

	-mkdir /usr/man/mann
	cp man/checknews.1 $(DEST)/checknews.n
	cp man/inews.1 $(DEST)/inews.n
	cp man/postnews.1 $(DEST)/postnews.n
	cp man/readnews.1 $(DEST)/readnews.n
	cp man/newsrc.5 $(DEST)/newsrc.n
	cp man/expire.8 $(DEST)/expire.n
	cp man/recnews.8 $(DEST)/recnews.n
	cp man/sendnews.8 $(DEST)/sendnews.n
	cp man/uurec.8 $(DEST)/uurec.n
	-chmod 444 $(DEST)/checknews.n $(DEST)/inews.n $(DEST)/postnews.n
	-chmod 444 $(DEST)/readnews.n $(DEST)/newsrc.n $(DEST)/expire.n
	-chmod 444 $(DEST)/recnews.n $(DEST)/sendnews.n $(DEST)/uurec.n

clean:
	
	rm -f $(BINDIR)/inews $(BINDIR)/readnews $(BINDIR)/checknews
	rm -f $(BINDIR)/postnews
	rm -f $(LIBDIR)/uurec $(LIBDIR)/recnews $(LIBDIR)/sendnews
	rm -f $(LIBDIR)/batch $(LIBDIR)/unbatch $(LIBDIR)/caesar 
	rm -f $(LIBDIR)/expire $(LIBDIR)/recmail
	rm -f /usr/bin/rnews
	rm -f $(DEST)/recnews.n $(DEST)/sendnews.n $(DEST)/uurec.n
	rm -f $(DEST)/readnews.n $(DEST)/newsrc.n $(DEST)/expire.n
	rm -f $(DEST)/checknews.n $(DEST)/inews.n $(DEST)/postnews.n
	rm -rf $(SPOOLDIR) $(LIBDIR)

