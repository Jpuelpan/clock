default: build run

clean:
	rm ./clock

build:
	gcc -Wall -Wextra -o clock main.c `pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig`

run:
	./clock -font "IosevkaTermNerdFontMono:style=Bold"
