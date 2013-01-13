# Restartd Makefile

CC = gcc
CFLAGS += -DVERSION='"$(VERSION)"' -Wall

INS = /usr/bin/install

VERSION = $(shell cat version)

###############################################################################
.PHONY: all install

all: restartd

install: restartd
	$(INS) -o root -g root -m 0640 restartd.conf $(DESTDIR)/etc
	$(INS) -o root -g root -m 0754 -b restartd $(DESTDIR)/usr/sbin
	$(INS) -o root -g root -m 0444 restartd.8 $(DESTDIR)/usr/share/man/man8
	$(INS) -o root -g root -m 0444 restartd.fr.8 $(DESTDIR)/usr/share/man/fr/man8/restartd.8

clean:
	rm -f *.o restartd

restartd: restartd.o config.o
# strip restartd

restartd.o: restartd.c
config.o: config.c
