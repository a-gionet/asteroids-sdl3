CC=gcc
RM=rm -rf


asteroids: main.o
	$(CC) -o asteroids main.o -lSDL3 -lm

main.o: main.c
	$(CC) -c main.c 


run: asteroids
	./asteroids

clean:
	$(RM) asteroids *.o
