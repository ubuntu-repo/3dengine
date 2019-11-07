build:
	gcc -Wall -Wfatal-errors -std=c99 ./src/*.c -lm -lSDL2 -o game

run:
	./game

clean:
	rm game
