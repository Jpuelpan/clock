default: build run

build:
	gcc -Wall -Wextra -g -o clock main.c `pkg-config --cflags --libs sdl3 sdl3-ttf`

run:
	./clock
