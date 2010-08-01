run : jeconnais
	./jeconnais

jeconnais : main.c
	gcc -Wall main.c dmx.c -o jeconnais -lm `sdl-config --cflags` `sdl-config --libs`
