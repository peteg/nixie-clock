# C compiler
#CROSS_COMPILE=arm-linux-gnueabi-
#CFLAGS+=-march=armv4t
LDFLAGS+=

# Where we want the artifacts installed.
PREFIX=/usr/local/
#DESTDIR=$(HOME)/armel/ts7250-nfs
DESTDIR=

# Event-driven programming makes a lot of things take extra parameters...
CFLAGS+=-O3
CFLAGS+=-std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -g
#LDFLAGS_CLOCK+=-lmpdclient -lrt -lpthread -lm
#LDFLAGS_CLOCK+=-static -Wl,--whole-archive -lmpdclient -lpthread -lrt -lm -Wl,--no-whole-archive

#CC=$(CROSS_COMPILE)gcc-4.4
CC=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip
READELF=$(CROSS_COMPILE)readelf

.PHONY: all clean install

all: remote_daemon

remote_daemon: remote_daemon.c DViCO.h main.h
	$(CC) $(CFLAGS) $(LDFLAGS) remote_daemon.c -o $@
#	$(STRIP) $@

clean:
	rm -f *.o remote_daemon

install: clock remote_daemon
	sudo cp clock remote_daemon $(DESTDIR)/$(PREFIX)/sbin
