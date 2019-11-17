build:
	gcc -Wall -Wfatal-errors -std=c99 ./src/*.c -lm -lSDL2 -lSDL2_gfx -o game

debug:
	gcc -g -Wall -Wfatal-errors -std=c99 ./src/*.c -lm -lSDL2 -lSDL2_gfx -o game

run:
	./game

clean:
	rm game
