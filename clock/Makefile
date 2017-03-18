CFLAGS+=-Wall -Wextra -pedantic
CFLAGS+=-Wno-unused-parameter

CFLAGS+=-std=c99
CFLAGS+=-I../include

.PHONY: all clean

all: clock

main.o: main.c main.h ../include/clock.h

clock: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_CLOCK) $^ -o $@

clean:
	rm -f clock *.o
