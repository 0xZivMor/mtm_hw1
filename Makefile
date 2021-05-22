CC = gcc
CC_ARGS = -g -std=c99 -Wall -Werror -pedantic-errors 
DEBUG_ARGS = -Wfatal-errors -Wno-unused-variable
MODULES = chessSystem.c match.c matchnode.c tournament.c utils.c
OBJECTS = chessSystem.o match.o matchnode.o tournament.o utils.o

default: clean chess_test

%.o: %.c
	$(CC) $(CC_ARGS) $(DEBUG_ARGS) -c $< -o $@

chess_test: $(OBJECTS)
	$(CC) $(CC_ARGS) $(DEBUG_ARGS) $(OBJECTS) -L. -lmap tests/chessSystemTestsExample.c -o $@

clean:
	rm -f chess_test $(OBJECTS)

.PHONY: clean default