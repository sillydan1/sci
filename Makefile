# sci - simple continuous integration
# See LICENSE file for copyright and license details.
# Note: If you're confused by the makefile, I do emplore you to read the info-page: $ info make
.POSIX:

NAME=sci
DESCRIPTION=$(NAME) is a simple contiuous integration system.
VERSION = 1.0.0
PREFIX ?= /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = gcc
OUTDIR := out/
OBJDIR := out/obj
BINDIR := out/bin
CFLAGS += -DSCI_VERSION="\"$(VERSION)\""
CFLAGS += -DSCI_NAME="\"$(NAME)\""
CFLAGS += -DSCI_DESCRIPTION="\"$(DESCRIPTION)\""
CFLAGS += -D_POSIX_C_SOURCE=2
CFLAGS += -D_GNU_SOURCE
CFLAGS += -Wall -Werror -std=c11 -g
CFLAGS += -Iinclude
CFLAGS += -lpthread -luuid

.PHONY: all clean dist install

all: out/bin/sci

out/obj/%.o: src/%.c | $(OBJDIR)
	$(CC) -c $? $(CFLAGS) -o $@

OBJ += out/obj/main.o
OBJ += out/obj/cli.o
OBJ += out/obj/log.o
OBJ += out/obj/notify.o
OBJ += out/obj/util.o
OBJ += out/obj/pipeline.o
OBJ += out/obj/threadlist.o
OBJ += out/obj/threadpool.o
out/bin/sci: $(OBJ) | $(BINDIR)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(OUTDIR)
	rm -rf $(NAME)-$(VERSION)

$(OUTDIR):
	mkdir -p $@

$(OBJDIR): $(OUTDIR)
	mkdir -p $@

$(BINDIR): $(OUTDIR)
	mkdir -p $@

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp -R \
		TODO.md README.md\
		Makefile src include\
		$(NAME)-$(VERSION)
	tar -cf - $(NAME)-$(VERSION) | gzip > $(NAME)-$(VERSION).tar.gz
	rm -rf $(NAME)-$(VERSION)

# NOTE: DESTDIR is meant for making packaging easier.
#		If you want to install in a different directory than the default, please
#		use: # make install PREFIX=/custom/path
install: out/bin/sci
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	# install binaries
	cp -f out/bin/sci $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/sci
	# install libraries
	# install services (only if system is using systemd though)
	# install manpages
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < src/sci.1 > $(DESTDIR)$(MANPREFIX)/man1/sci.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sci.1

uninstall:
	# uninstall binaries
	rm -f $(DESTDIR)$(PREFIX)/bin/sci
	# uninstall libraries
	# uninstall services (only if system is using systemd though)
	# uninstall manpages
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sci.1
