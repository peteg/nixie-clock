CFLAGS+=-Wall -Wextra -pedantic
CFLAGS+=-Wno-unused-parameter

CFLAGS+=-O2

CFLAGS+=-std=c99
CFLAGS+=-I../include

.PHONY: all clean

all: clock

event_loop.o: event_loop.c event_loop.h main.h ../include/remote.h

main.o: main.c event_loop.h main.h ../include/clock.h

clock: event_loop.o main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_CLOCK) $^ -o $@

clean:
	rm -f clock *.o
