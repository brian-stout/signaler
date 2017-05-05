CPPFLAGS+=-Wall -Wextra -Wpedantic
CPPFLAGS+=-Wwrite-strings -Wstack-usage=1024 -Wfloat-equal
CPPFLAGS+=-Waggregate-return -Winline

CFLAGS+=-std=c11 -D_GNU_SOURCE

LDLIBS+=-lm

BIN=signaler
OBJS=signaler

.PHONY: debug profile clean run val

all: $(BIN)

ticker: ticker.o stocks.o tree.o

debug: CFLAGS+=-g
debug: $(BIN)

profile: CFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: $(BIN)

clean:
	$(RM) $(OBJS) $(BIN) 

run:
	./$(BIN)

val:
	valgrind --track-origins=yes ./$(BIN)
