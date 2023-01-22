#
# Sccsid = @(#)Makefile	1.1	1/2/85
#
# Makefile for Notes
# 30-Mar-84
#

#
# Set up definitions
#
DESTDIR=/usr/new
MSTDIR=/usr/spool/notes
NET=/usr/new/lib/notes
ARCHDIR=/usr/spool/oldnotes
UTILITY=$(MSTDIR)/.utilities
SEQUENCER=$(MSTDIR)/.sequencer
LOCKS=$(MSTDIR)/.locks
ALIASES=$(UTILITY)/net.aliases
NOTES=notes
NOTESGRP=staff



clean:
	rm -f $(DESTDIR)/notes $(DESTDIR)/nfprint $(DESTDIR)/nfstats
	rm -f $(DESTDIR)/checknotes $(DESTDIR)/autoseq $(DESTDIR)/nfpipe
	rm -rf $(MSTDIR) $(ARCHDIR) $(NET)
	rm -f /usr/man/mann/notes.n


base:
#
# Set up directories with proper access, owner, and group
#
	mkdir $(MSTDIR) $(ARCHDIR) $(NET)
	-/etc/chown $(NOTES) $(MSTDIR) $(ARCHDIR) $(NET)
	-chgrp $(NOTESGRP) $(MSTDIR) $(ARCHDIR) $(NET)
	echo > $(DESTDIR)/notes
	ln $(DESTDIR)/notes $(DESTDIR)/autoseq
	echo > $(DESTDIR)/nfprint
	echo > $(DESTDIR)/nfstats
	echo > $(DESTDIR)/nfpipe
	echo > $(DESTDIR)/checknotes
	-/etc/chown $(NOTES) $(DESTDIR)/notes $(DESTDIR)/nfstats
	-/etc/chown $(NOTES) $(DESTDIR)/nfprint $(DESTDIR)/nfpipe
	-/etc/chown $(NOTES) $(DESTDIR)/checknotes
	-chgrp $(NOTESGRP) $(DESTDIR)/notes $(DESTDIR)/nfstats
	-chgrp $(NOTESGRP) $(DESTDIR)/nfprint $(DESTDIR)/nfpipe
	-chgrp $(NOTESGRP) $(DESTDIR)/checknotes
	echo > $(NET)/nfxmit
	echo > $(NET)/nfrcv
	-/etc/chown $(NOTES) $(NET)/nfxmit $(NET)/nfrcv
	-chgrp $(NOTESGRP) $(NET)/nfxmit $(NET)/nfrcv
#
# Install man page
#
	-mkdir /usr/man/mann
	cp notes.1 /usr/man/mann/notes.n
	chgrp staff /usr/man/mann/notes.n

boot:
#
# Set up spool directories
#

	mkdir $(UTILITY) $(SEQUENCER) $(LOCKS) $(ALIASES)
	chmod 755 $(UTILITY) $(SEQUENCER) $(LOCKS) $(ALIASES)
	chmod 777 $(MSTDIR)
#
# Set up for automatic notesfile generation by newsinput
#
	echo 1 > $(MSTDIR)/.SEQ
	chmod 600 $(MSTDIR)/.SEQ
	echo "available notesfiles" > $(UTILITY)/avail.notes
	chmod 644 $(UTILITY)/avail.notes
#
# Place help files
#
	cp access.help $(UTILITY)/access.help
	cp dir.help $(UTILITY)/dir.help
	cp index.help $(UTILITY)/index.help
	cp lim.help $(UTILITY)/lim.help
	cp read.help $(UTILITY)/read.help
	chmod 644 $(UTILITY)/access.help $(UTILITY)/dir.help
	chmod 644 $(UTILITY)/index.help $(UTILITY)/lim.help $(UTILITY)/read.help
#
# Move user binaries into place with proper access, owner, and group
#
	cp main $(DESTDIR)/notes
	cp nfstats $(DESTDIR)/nfstats
	cp nfpipe $(DESTDIR)/nfpipe
	cp nfprint $(DESTDIR)/nfprint
	cp checknotes $(DESTDIR)/checknotes
	chmod 4711 $(DESTDIR)/notes $(DESTDIR)/nfstats $(DESTDIR)/nfpipe
	chmod 4711 $(DESTDIR)/nfprint $(DESTDIR)/checknotes
#
# Move network binaries into place with proper access, owner, and group
#
	cp nfxmit $(NET)/nfxmit
	cp nfrcv $(NET)/nfrcv
	chmod 4711 $(NET)/nfxmit $(NET)/nfrcv
#
# Move utilities binaries into place with proper access
#
	cp mknf $(UTILITY)/mknf
	cp rmnf $(UTILITY)/rmnf
	cp newsinput $(UTILITY)/newsinput
	cp bnewsinput $(UTILITY)/bnewsinput
	cp newsoutput $(UTILITY)/newsoutput
	cp nfarchive $(UTILITY)/nfarchive
	chmod 4710 $(UTILITY)/mknf $(UTILITY)/rmnf
	chmod 4711 $(UTILITY)/newsinput $(UTILITY)/bnewsinput
	chmod 4711 $(UTILITY)/newsoutput $(UTILITY)/nfarchive
#
# gripe files
#
	$(UTILITY)/mknf -ao nfgripes
	$(UTILITY)/mknf -o nfmaint
