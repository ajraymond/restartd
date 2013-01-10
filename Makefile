# Restartd Makefile

C_ARGS = -Wall -O2
CC = gcc
INS = /usr/bin/install

all:	restartd

install: restartd
	$(INS) -o root -g root -m 0640 restartd.conf $(DESTDIR)/etc
	$(INS) -o root -g root -m 0754 -b restartd $(DESTDIR)/usr/sbin
	$(INS) -o root -g root -m 0444 restartd.8 $(DESTDIR)/usr/share/man/man8
	$(INS) -o root -g root -m 0444 restartd.fr.8 $(DESTDIR)/usr/share/man/fr/man8/restartd.8

clean:
	rm -f *.o restartd

restartd:	main.o config.o
	$(CC) -o restartd main.o config.o
#	strip restartd

main.o:	main.c
	$(CC) $(C_ARGS) -c main.c

config.o:	config.c
	$(CC) $(C_ARGS) -c config.c
