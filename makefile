all: swim_mill.c
	gcc swim_mill.c -o swim_mill
	gcc fish.c -lm -o fish
	gcc pellet.c -o pellet -lm


