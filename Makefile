CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic-errors -DNDEBUG
EXEC = chess
OBJS = chessSystem.o match.o matchnode.o tournament.o utils.o

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -L. -lmap exampleMain.c -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $@

clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: clean