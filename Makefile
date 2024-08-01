# sci - simple continuous integration
# See LICENSE file for copyright and license details.
# Note: If you're confused by the makefile, I do emplore you to read the info-page: $ info make
.POSIX:

NAME=sci
VERSION = 0.1.0

SRC = src/sci.c
OBJ = $(SRC:.c=.o)
OUTDIR := out/
OBJDIR := out/obj
BINDIR := out/bin

.PHONY: all clean

all: out/bin/sci

out/obj/%.o: src/%.c | $(OBJDIR)
	$(CC) -c $? $(CFLAGS) -o $@

out/bin/sci: out/obj/main.o | $(BINDIR)
	$(CC) -o $@ $(CFLAGS) $+

clean:
	rm -rf build

$(OUTDIR):
	mkdir -p $@

$(OBJDIR): $(OUTDIR)
	mkdir -p $@

$(BINDIR): $(OUTDIR)
	mkdir -p $@

# dist: clean
# 	mkdir -p st-$(VERSION)
# 	cp -R FAQ LEGACY TODO LICENSE Makefile README config.mk\
# 		config.def.h st.info st.1 arg.h st.h win.h $(SRC)\
# 		st-$(VERSION)
# 	tar -cf - st-$(VERSION) | gzip > st-$(VERSION).tar.gz
# 	rm -rf st-$(VERSION)
#
# install: st
# 	mkdir -p $(DESTDIR)$(PREFIX)/bin
# 	cp -f st $(DESTDIR)$(PREFIX)/bin
# 	chmod 755 $(DESTDIR)$(PREFIX)/bin/st
# 	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
# 	sed "s/VERSION/$(VERSION)/g" < st.1 > $(DESTDIR)$(MANPREFIX)/man1/st.1
# 	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/st.1
# 	tic -sx st.info
# 	@echo Please see the README file regarding the terminfo entry of st.
#
# uninstall:
# 	rm -f $(DESTDIR)$(PREFIX)/bin/st
# 	rm -f $(DESTDIR)$(MANPREFIX)/man1/st.1
