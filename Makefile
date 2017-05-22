CC=gcc

FLAGS=-O3 -Wno-unused-result
LDFLAGS=-lpthread -pthread
#DEBUG=-DDEBUG
RESULT=-DRESULT

all: gol

gol: gol.c
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) $(LDFLAGS) gol.c -o gol

clean:
	rm -rf gol
